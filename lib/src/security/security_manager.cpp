/**
 * @file security_manager.cpp
 * @brief Implementation of security manager
 * @version 3.0.0
 */

#include <qtplugin/security/security_manager.hpp>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <fstream>
#include <mutex>

namespace qtplugin {

SecurityManager::SecurityManager() = default;

SecurityManager::~SecurityManager() = default;

SecurityValidationResult SecurityManager::validate_plugin(const std::filesystem::path& file_path,
                                                         SecurityLevel required_level) {
    m_validations_performed.fetch_add(1);
    
    SecurityValidationResult result;
    result.validated_level = SecurityLevel::None;
    
    // Basic file validation (always performed)
    auto file_result = validate_file_integrity(file_path);
    if (!file_result.is_valid) {
        result.errors.insert(result.errors.end(), file_result.errors.begin(), file_result.errors.end());
        m_validations_failed.fetch_add(1);
        return result;
    }
    result.validated_level = SecurityLevel::Basic;
    result.warnings.insert(result.warnings.end(), file_result.warnings.begin(), file_result.warnings.end());
    
    // Metadata validation
    if (required_level >= SecurityLevel::Basic) {
        auto metadata_result = validate_metadata(file_path);
        if (!metadata_result.is_valid) {
            result.errors.insert(result.errors.end(), metadata_result.errors.begin(), metadata_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.warnings.insert(result.warnings.end(), metadata_result.warnings.begin(), metadata_result.warnings.end());
    }
    
    // Signature validation
    if (required_level >= SecurityLevel::Standard && m_signature_verification_enabled) {
        auto signature_result = validate_signature(file_path);
        if (!signature_result.is_valid) {
            result.errors.insert(result.errors.end(), signature_result.errors.begin(), signature_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.validated_level = SecurityLevel::Standard;
        result.warnings.insert(result.warnings.end(), signature_result.warnings.begin(), signature_result.warnings.end());
    }
    
    // Permission validation
    if (required_level >= SecurityLevel::Strict) {
        auto permission_result = validate_permissions(file_path);
        if (!permission_result.is_valid) {
            result.errors.insert(result.errors.end(), permission_result.errors.begin(), permission_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.validated_level = SecurityLevel::Strict;
        result.warnings.insert(result.warnings.end(), permission_result.warnings.begin(), permission_result.warnings.end());
    }
    
    result.is_valid = true;
    m_validations_passed.fetch_add(1);
    
    return result;
}

bool SecurityManager::is_trusted(std::string_view plugin_id) const {
    std::shared_lock lock(m_trusted_plugins_mutex);
    return m_trusted_plugins.find(std::string(plugin_id)) != m_trusted_plugins.end();
}

void SecurityManager::add_trusted_plugin(std::string_view plugin_id, SecurityLevel trust_level) {
    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins[std::string(plugin_id)] = trust_level;
}

void SecurityManager::remove_trusted_plugin(std::string_view plugin_id) {
    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins.erase(std::string(plugin_id));
}

SecurityLevel SecurityManager::security_level() const noexcept {
    return m_security_level;
}

void SecurityManager::set_security_level(SecurityLevel level) {
    m_security_level = level;
}

QJsonObject SecurityManager::security_statistics() const {
    return QJsonObject{
        {"validations_performed", static_cast<qint64>(m_validations_performed.load())},
        {"validations_passed", static_cast<qint64>(m_validations_passed.load())},
        {"validations_failed", static_cast<qint64>(m_validations_failed.load())},
        {"trusted_plugins_count", static_cast<int>(m_trusted_plugins.size())},
        {"current_security_level", security_level_to_string(m_security_level)},
        {"signature_verification_enabled", m_signature_verification_enabled}
    };
}

qtplugin::expected<void, PluginError> SecurityManager::load_trusted_plugins(const std::filesystem::path& file_path) {
    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound, "Trusted plugins file not found");
    }
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return make_error<void>(PluginErrorCode::FileSystemError, "Cannot open trusted plugins file");
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(content), &parse_error);
    
    if (parse_error.error != QJsonParseError::NoError) {
        return make_error<void>(PluginErrorCode::ConfigurationError, 
                               "Invalid JSON in trusted plugins file: " + parse_error.errorString().toStdString());
    }
    
    if (!doc.isObject()) {
        return make_error<void>(PluginErrorCode::ConfigurationError, "Trusted plugins file must contain a JSON object");
    }
    
    QJsonObject root = doc.object();
    if (!root.contains("trusted_plugins") || !root["trusted_plugins"].isArray()) {
        return make_error<void>(PluginErrorCode::ConfigurationError, "Missing 'trusted_plugins' array");
    }
    
    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins.clear();
    
    QJsonArray plugins = root["trusted_plugins"].toArray();
    for (const auto& plugin_value : plugins) {
        if (!plugin_value.isObject()) continue;
        
        QJsonObject plugin = plugin_value.toObject();
        if (!plugin.contains("id") || !plugin["id"].isString()) continue;
        
        std::string plugin_id = plugin["id"].toString().toStdString();
        SecurityLevel trust_level = SecurityLevel::Basic;
        
        if (plugin.contains("trust_level") && plugin["trust_level"].isString()) {
            trust_level = security_level_from_string(plugin["trust_level"].toString().toStdString());
        }
        
        m_trusted_plugins[plugin_id] = trust_level;
    }
    
    return make_success();
}

qtplugin::expected<void, PluginError> SecurityManager::save_trusted_plugins(const std::filesystem::path& file_path) const {
    QJsonArray plugins_array;
    
    {
        std::shared_lock lock(m_trusted_plugins_mutex);
        for (const auto& [plugin_id, trust_level] : m_trusted_plugins) {
            QJsonObject plugin{
                {"id", QString::fromStdString(plugin_id)},
                {"trust_level", security_level_to_string(trust_level)}
            };
            plugins_array.append(plugin);
        }
    }
    
    QJsonObject root{
        {"version", "1.0"},
        {"trusted_plugins", plugins_array}
    };
    
    QJsonDocument doc(root);
    
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return make_error<void>(PluginErrorCode::FileSystemError, "Cannot create trusted plugins file");
    }
    
    std::string json_string = doc.toJson().toStdString();
    file.write(json_string.c_str(), json_string.size());
    file.close();
    
    if (file.fail()) {
        return make_error<void>(PluginErrorCode::FileSystemError, "Failed to write trusted plugins file");
    }
    
    return make_success();
}

void SecurityManager::set_signature_verification_enabled(bool enabled) {
    m_signature_verification_enabled = enabled;
}

bool SecurityManager::is_signature_verification_enabled() const noexcept {
    return m_signature_verification_enabled;
}

SecurityValidationResult SecurityManager::validate_file_integrity(const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    
    if (!is_safe_file_path(file_path)) {
        result.errors.push_back("Unsafe file path detected");
        return result;
    }
    
    if (!has_valid_extension(file_path)) {
        result.errors.push_back("Invalid file extension");
        return result;
    }
    
    QFileInfo file_info(QString::fromStdString(file_path.string()));
    if (!file_info.exists()) {
        result.errors.push_back("File does not exist");
        return result;
    }
    
    if (!file_info.isFile()) {
        result.errors.push_back("Path is not a regular file");
        return result;
    }
    
    if (!file_info.isReadable()) {
        result.errors.push_back("File is not readable");
        return result;
    }
    
    // Check file size (reasonable limits)
    qint64 file_size = file_info.size();
    if (file_size == 0) {
        result.errors.push_back("File is empty");
        return result;
    }
    
    if (file_size > 100 * 1024 * 1024) { // 100MB limit
        result.warnings.push_back("File is very large (>100MB)");
    }
    
    result.is_valid = true;
    return result;
}

SecurityValidationResult SecurityManager::validate_metadata(const std::filesystem::path& file_path) const {
    SecurityValidationResult result;

    // This is a placeholder - in a real implementation, you would:
    // 1. Load the plugin metadata
    // 2. Validate required fields
    // 3. Check for suspicious metadata
    // 4. Validate version constraints
    (void)file_path; // Suppress unused parameter warning

    result.is_valid = true;
    return result;
}

SecurityValidationResult SecurityManager::validate_signature(const std::filesystem::path& file_path) const {
    SecurityValidationResult result;

    // This is a placeholder - in a real implementation, you would:
    // 1. Check for digital signature
    // 2. Validate signature against trusted certificates
    // 3. Verify signature integrity
    (void)file_path; // Suppress unused parameter warning

    if (!m_signature_verification_enabled) {
        result.warnings.push_back("Signature verification is disabled");
        result.is_valid = true;
        return result;
    }

    // For now, just warn that signature verification is not implemented
    result.warnings.push_back("Signature verification not fully implemented");
    result.is_valid = true;
    return result;
}

SecurityValidationResult SecurityManager::validate_permissions(const std::filesystem::path& file_path) const {
    SecurityValidationResult result;

    // This is a placeholder - in a real implementation, you would:
    // 1. Check file permissions
    // 2. Validate plugin requested permissions
    // 3. Check for privilege escalation attempts
    (void)file_path; // Suppress unused parameter warning

    result.is_valid = true;
    return result;
}

bool SecurityManager::is_safe_file_path(const std::filesystem::path& file_path) const {
    std::string path_str = file_path.string();
    
    // Check for path traversal attempts
    if (path_str.find("..") != std::string::npos) {
        return false;
    }
    
    // Check for suspicious characters
    if (path_str.find_first_of("<>:\"|?*") != std::string::npos) {
        return false;
    }
    
    return true;
}

bool SecurityManager::has_valid_extension(const std::filesystem::path& file_path) const {
    auto extensions = get_allowed_extensions();
    std::string extension = file_path.extension().string();
    
    return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
}

std::vector<std::string> SecurityManager::get_allowed_extensions() const {
    return {".dll", ".so", ".dylib", ".qtplugin"};
}

// Factory implementation

std::unique_ptr<ISecurityManager> SecurityManagerFactory::create_default() {
    return std::make_unique<SecurityManager>();
}

std::unique_ptr<SecurityManager> SecurityManagerFactory::create_with_level(SecurityLevel level) {
    auto manager = std::make_unique<SecurityManager>();
    manager->set_security_level(level);
    return manager;
}

// Utility functions

const char* security_level_to_string(SecurityLevel level) noexcept {
    switch (level) {
        case SecurityLevel::None: return "None";
        case SecurityLevel::Basic: return "Basic";
        case SecurityLevel::Standard: return "Standard";
        case SecurityLevel::Strict: return "Strict";
        case SecurityLevel::Maximum: return "Maximum";
    }
    return "Unknown";
}

SecurityLevel security_level_from_string(std::string_view str) noexcept {
    if (str == "None") return SecurityLevel::None;
    if (str == "Basic") return SecurityLevel::Basic;
    if (str == "Standard") return SecurityLevel::Standard;
    if (str == "Strict") return SecurityLevel::Strict;
    if (str == "Maximum") return SecurityLevel::Maximum;
    return SecurityLevel::None;
}

} // namespace qtplugin
