# Creating Your First Plugin

This tutorial walks you through creating a complete plugin from scratch, covering all the essential concepts and best practices.

## What You'll Learn

By the end of this tutorial, you'll understand:

- ✅ Plugin interface implementation
- ✅ Plugin metadata and configuration
- ✅ Command system and error handling
- ✅ Plugin lifecycle management
- ✅ Building and testing plugins
- ✅ Advanced plugin features

## Prerequisites

Before starting, ensure you have:

- QtPlugin installed and working (see [Installation Guide](installation.md))
- Basic understanding of C++ and Qt
- CMake 3.21+ and a C++20 compiler
- Completed the [Quick Start Guide](quick-start.md)

## Project Setup

Let's create a more sophisticated plugin - a **Text Processor Plugin** that can manipulate text in various ways.

### 1. Create Project Structure

```bash
mkdir text-processor-plugin
cd text-processor-plugin
mkdir src include tests
```

Create the following files:

```
text-processor-plugin/
├── CMakeLists.txt
├── src/
│   ├── text_processor_plugin.cpp
│   └── text_processor_plugin.hpp
├── include/
│   └── text_processor_interface.hpp
├── tests/
│   └── test_text_processor.cpp
├── metadata.json
└── README.md
```

### 2. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.21)
project(TextProcessorPlugin VERSION 1.0.0)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Find QtPlugin
find_package(QtPlugin REQUIRED)

# Create the plugin library
add_library(TextProcessorPlugin SHARED
    src/text_processor_plugin.cpp
    src/text_processor_plugin.hpp
    include/text_processor_interface.hpp
)

# Set target properties
set_target_properties(TextProcessorPlugin PROPERTIES
    OUTPUT_NAME "text_processor"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Link libraries
target_link_libraries(TextProcessorPlugin
    PRIVATE
    QtPlugin::Core
    Qt6::Core
)

# Include directories
target_include_directories(TextProcessorPlugin
    PRIVATE
    src
    include
)

# Copy metadata file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json"
    "${CMAKE_BINARY_DIR}/plugins/text_processor.json"
    COPYONLY
)

# Optional: Build tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    find_package(Qt6 COMPONENTS Test QUIET)
    if(Qt6Test_FOUND)
        add_executable(TestTextProcessor tests/test_text_processor.cpp)
        target_link_libraries(TestTextProcessor
            PRIVATE
            TextProcessorPlugin
            QtPlugin::Core
            Qt6::Core
            Qt6::Test
        )
    endif()
endif()
```

## Plugin Interface Design

### 3. Custom Interface (Optional)

First, let's define a custom interface for text processing plugins:

**include/text_processor_interface.hpp**:

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QString>
#include <QStringList>

namespace textprocessor {

/**
 * @brief Text processing operations
 */
enum class TextOperation {
    ToUpperCase,
    ToLowerCase,
    Reverse,
    RemoveSpaces,
    WordCount,
    CharacterCount,
    FindReplace,
    SortLines,
    RemoveDuplicates,
    Base64Encode,
    Base64Decode
};

/**
 * @brief Text processor plugin interface
 */
class ITextProcessor : public virtual qtplugin::IPlugin {
public:
    virtual ~ITextProcessor() = default;
    
    /**
     * @brief Get supported text operations
     * @return List of supported operations
     */
    virtual std::vector<TextOperation> supported_operations() const = 0;
    
    /**
     * @brief Process text with specified operation
     * @param text Input text
     * @param operation Operation to perform
     * @param parameters Optional operation parameters
     * @return Processed text or error
     */
    virtual qtplugin::expected<QString, qtplugin::PluginError>
    process_text(const QString& text, 
                TextOperation operation,
                const QJsonObject& parameters = {}) = 0;
    
    /**
     * @brief Validate operation parameters
     * @param operation Operation to validate
     * @param parameters Parameters to validate
     * @return true if parameters are valid
     */
    virtual bool validate_parameters(TextOperation operation,
                                   const QJsonObject& parameters) const = 0;
};

} // namespace textprocessor
```

## Plugin Implementation

### 4. Plugin Header

**src/text_processor_plugin.hpp**:

```cpp
#pragma once

#include <qtplugin/qtplugin.hpp>
#include <include/text_processor_interface.hpp>
#include <QObject>
#include <QJsonObject>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <memory>
#include <atomic>

class TextProcessorPlugin : public QObject,
                           public qtplugin::IPlugin,
                           public textprocessor::ITextProcessor {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(TextProcessorPlugin, 
                           "com.example.TextProcessor/1.0", 
                           "metadata.json")

public:
    explicit TextProcessorPlugin(QObject* parent = nullptr);
    ~TextProcessorPlugin() override;

    // === IPlugin Implementation ===
    
    // Metadata
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    std::string_view license() const noexcept override;
    std::string_view category() const noexcept override;
    
    // Lifecycle
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    
    // Configuration
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> 
    configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;
    
    // Commands
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, 
                   const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;
    
    // === ITextProcessor Implementation ===
    
    std::vector<textprocessor::TextOperation> supported_operations() const override;
    
    qtplugin::expected<QString, qtplugin::PluginError>
    process_text(const QString& text,
                textprocessor::TextOperation operation,
                const QJsonObject& parameters = {}) override;
    
    bool validate_parameters(textprocessor::TextOperation operation,
                           const QJsonObject& parameters) const override;

private:
    // Helper methods
    QString to_upper_case(const QString& text);
    QString to_lower_case(const QString& text);
    QString reverse_text(const QString& text);
    QString remove_spaces(const QString& text);
    QString find_replace(const QString& text, const QJsonObject& params);
    QString sort_lines(const QString& text);
    QString remove_duplicates(const QString& text);
    QString base64_encode(const QString& text);
    QString base64_decode(const QString& text);
    
    int word_count(const QString& text);
    int character_count(const QString& text);
    
    // State management
    std::atomic<qtplugin::PluginState> m_state;
    QJsonObject m_configuration;
    mutable std::mutex m_config_mutex;
    
    // Statistics
    std::atomic<size_t> m_operations_count{0};
    std::atomic<size_t> m_characters_processed{0};
};
```

### 5. Plugin Implementation

**src/text_processor_plugin.cpp**:

```cpp
#include "text_processor_plugin.hpp"
#include <QDebug>
#include <QStringList>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <algorithm>
#include <set>

TextProcessorPlugin::TextProcessorPlugin(QObject* parent)
    : QObject(parent)
    , m_state(qtplugin::PluginState::Unloaded)
{
    qDebug() << "TextProcessorPlugin created";
}

TextProcessorPlugin::~TextProcessorPlugin() {
    shutdown();
    qDebug() << "TextProcessorPlugin destroyed";
}

// === IPlugin Implementation ===

std::string_view TextProcessorPlugin::name() const noexcept {
    return "Text Processor Plugin";
}

std::string_view TextProcessorPlugin::description() const noexcept {
    return "A comprehensive text processing plugin with various text manipulation operations";
}

qtplugin::Version TextProcessorPlugin::version() const noexcept {
    return {1, 0, 0};
}

std::string_view TextProcessorPlugin::author() const noexcept {
    return "QtPlugin Tutorial";
}

std::string TextProcessorPlugin::id() const noexcept {
    return "com.example.textprocessor";
}

std::string_view TextProcessorPlugin::license() const noexcept {
    return "MIT";
}

std::string_view TextProcessorPlugin::category() const noexcept {
    return "Text Processing";
}

qtplugin::expected<void, qtplugin::PluginError>
TextProcessorPlugin::initialize() {
    qDebug() << "Initializing TextProcessorPlugin";

    m_state = qtplugin::PluginState::Initializing;

    try {
        // Initialize default configuration
        if (m_configuration.isEmpty()) {
            m_configuration = default_configuration().value_or(QJsonObject{});
        }

        // Validate configuration
        if (!validate_configuration(m_configuration)) {
            m_state = qtplugin::PluginState::Failed;
            return qtplugin::make_error<void>(
                qtplugin::PluginErrorCode::ConfigurationError,
                "Invalid plugin configuration"
            );
        }

        m_state = qtplugin::PluginState::Running;
        qDebug() << "TextProcessorPlugin initialized successfully";

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Failed;
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed,
            "Initialization failed: " + std::string(e.what())
        );
    }
}

void TextProcessorPlugin::shutdown() noexcept {
    qDebug() << "Shutting down TextProcessorPlugin";
    m_state = qtplugin::PluginState::Stopped;

    // Log statistics
    qDebug() << "Plugin statistics:";
    qDebug() << "  Operations performed:" << m_operations_count.load();
    qDebug() << "  Characters processed:" << m_characters_processed.load();
}

qtplugin::PluginState TextProcessorPlugin::state() const noexcept {
    return m_state.load();
}

qtplugin::PluginCapabilities TextProcessorPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapability::DataProcessing |
           qtplugin::PluginCapability::Configuration;
}

std::optional<QJsonObject> TextProcessorPlugin::default_configuration() const {
    QJsonObject config;
    config["max_text_length"] = 1000000; // 1MB limit
    config["enable_statistics"] = true;
    config["case_sensitive_operations"] = false;
    config["preserve_line_endings"] = true;
    return config;
}

qtplugin::expected<void, qtplugin::PluginError>
TextProcessorPlugin::configure(const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided"
        );
    }

    std::lock_guard<std::mutex> lock(m_config_mutex);
    m_configuration = config;

    qDebug() << "TextProcessorPlugin configured with:" << config;
    return qtplugin::make_success();
}

QJsonObject TextProcessorPlugin::current_configuration() const {
    std::lock_guard<std::mutex> lock(m_config_mutex);
    return m_configuration;
}

bool TextProcessorPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate max_text_length
    if (config.contains("max_text_length")) {
        auto max_length = config["max_text_length"].toInt(-1);
        if (max_length < 0 || max_length > 10000000) { // 10MB max
            return false;
        }
    }

    // Validate boolean options
    const QStringList bool_options = {
        "enable_statistics", "case_sensitive_operations", "preserve_line_endings"
    };

    for (const auto& option : bool_options) {
        if (config.contains(option) && !config[option].isBool()) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> TextProcessorPlugin::available_commands() const {
    return {
        "process_text",     // Main text processing command
        "list_operations",  // List supported operations
        "statistics",       // Get plugin statistics
        "validate_params",  // Validate operation parameters
        "reset_stats"       // Reset statistics
    };
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
TextProcessorPlugin::execute_command(std::string_view command,
                                   const QJsonObject& params) {
    if (m_state.load() != qtplugin::PluginState::Running) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InvalidState,
            "Plugin is not in running state"
        );
    }

    QJsonObject result;

    if (command == "process_text") {
        return handle_process_text_command(params);
    }
    else if (command == "list_operations") {
        return handle_list_operations_command();
    }
    else if (command == "statistics") {
        return handle_statistics_command();
    }
    else if (command == "validate_params") {
        return handle_validate_params_command(params);
    }
    else if (command == "reset_stats") {
        return handle_reset_stats_command();
    }
    else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command)
        );
    }
}
```

### 6. Plugin Metadata

**metadata.json**:

```json
{
    "name": "Text Processor Plugin",
    "version": "1.0.0",
    "description": "A comprehensive text processing plugin with various text manipulation operations",
    "author": "QtPlugin Tutorial",
    "license": "MIT",
    "homepage": "https://github.com/example/text-processor-plugin",
    "category": "Text Processing",
    "capabilities": ["DataProcessing", "Configuration"],
    "dependencies": [],
    "optional_dependencies": [],
    "metadata": {
        "supported_operations": [
            "ToUpperCase", "ToLowerCase", "Reverse", "RemoveSpaces",
            "WordCount", "CharacterCount", "FindReplace", "SortLines",
            "RemoveDuplicates", "Base64Encode", "Base64Decode"
        ],
        "max_text_size": 1000000,
        "thread_safe": true,
        "hot_reload_supported": false
    },
    "configuration_schema": {
        "type": "object",
        "properties": {
            "max_text_length": {
                "type": "integer",
                "minimum": 1,
                "maximum": 10000000,
                "default": 1000000
            },
            "enable_statistics": {
                "type": "boolean",
                "default": true
            },
            "case_sensitive_operations": {
                "type": "boolean",
                "default": false
            },
            "preserve_line_endings": {
                "type": "boolean",
                "default": true
            }
        }
    }
}
```
