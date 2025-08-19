/**
 * @file plugin_dependency_resolver.cpp
 * @brief Implementation of plugin dependency resolver
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_dependency_resolver.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/core/plugin_manager.hpp"
#include <QLoggingCategory>
#include <algorithm>
#include <queue>

Q_LOGGING_CATEGORY(dependencyResolverLog, "qtplugin.dependency")

namespace qtplugin {

PluginDependencyResolver::PluginDependencyResolver(QObject* parent)
    : QObject(parent) {
    qCDebug(dependencyResolverLog) << "Plugin dependency resolver initialized";
}

PluginDependencyResolver::~PluginDependencyResolver() {
    clear();
    qCDebug(dependencyResolverLog) << "Plugin dependency resolver destroyed";
}

qtplugin::expected<void, PluginError> 
PluginDependencyResolver::update_dependency_graph(IPluginRegistry* plugin_registry) {
    if (!plugin_registry) {
        return make_error<void>(PluginErrorCode::InvalidParameter, "Plugin registry cannot be null");
    }
    
    // Clear existing dependency graph
    m_dependency_graph.clear();
    
    // Get all plugin information from registry
    auto all_plugin_info = plugin_registry->get_all_plugin_info();
    
    // Build dependency graph from loaded plugins
    for (const auto& plugin_info : all_plugin_info) {
        DependencyNode node;
        node.plugin_id = plugin_info.id;
        
        // Convert vector to unordered_set
        for (const auto& dep : plugin_info.metadata.dependencies) {
            node.dependencies.insert(dep);
        }
        node.dependents.clear();
        
        // Set load order based on dependency count (will be refined later)
        node.load_order = static_cast<int>(plugin_info.metadata.dependencies.size());
        
        m_dependency_graph[plugin_info.id] = std::move(node);
    }
    
    // Build reverse dependencies (dependents)
    for (auto& [plugin_id, node] : m_dependency_graph) {
        for (const auto& dependency : node.dependencies) {
            auto dep_it = m_dependency_graph.find(dependency);
            if (dep_it != m_dependency_graph.end()) {
                dep_it->second.dependents.insert(plugin_id);
            }
        }
    }
    
    // Detect circular dependencies
    detect_circular_dependencies();
    
    qCDebug(dependencyResolverLog) << "Dependency graph updated with" << m_dependency_graph.size() << "plugins";
    emit dependency_graph_updated();
    
    return make_success();
}

std::unordered_map<std::string, DependencyNode> PluginDependencyResolver::get_dependency_graph() const {
    return m_dependency_graph;
}

std::vector<std::string> PluginDependencyResolver::get_load_order() const {
    return topological_sort();
}

bool PluginDependencyResolver::can_unload_safely(const std::string& plugin_id) const {
    auto it = m_dependency_graph.find(plugin_id);
    if (it == m_dependency_graph.end()) {
        return true; // Plugin not in graph, safe to unload
    }
    
    // Plugin can be safely unloaded if no other plugins depend on it
    return it->second.dependents.empty();
}

qtplugin::expected<void, PluginError> 
PluginDependencyResolver::check_plugin_dependencies(const PluginInfo& plugin_info) const {
    // Check if all dependencies are available in the graph
    for (const auto& dep : plugin_info.metadata.dependencies) {
        if (m_dependency_graph.find(dep) == m_dependency_graph.end()) {
            return make_error<void>(PluginErrorCode::DependencyMissing, 
                                   "Missing dependency: " + dep);
        }
    }
    
    return make_success();
}

bool PluginDependencyResolver::has_circular_dependencies() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;
    
    for (const auto& [plugin_id, node] : m_dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (has_circular_dependency(plugin_id, visited, recursion_stack)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<std::string> PluginDependencyResolver::get_dependents(const std::string& plugin_id) const {
    auto it = m_dependency_graph.find(plugin_id);
    if (it == m_dependency_graph.end()) {
        return {};
    }
    
    return std::vector<std::string>(it->second.dependents.begin(), it->second.dependents.end());
}

std::vector<std::string> PluginDependencyResolver::get_dependencies(const std::string& plugin_id) const {
    auto it = m_dependency_graph.find(plugin_id);
    if (it == m_dependency_graph.end()) {
        return {};
    }
    
    return std::vector<std::string>(it->second.dependencies.begin(), it->second.dependencies.end());
}

void PluginDependencyResolver::clear() {
    size_t count = m_dependency_graph.size();
    m_dependency_graph.clear();
    
    qCDebug(dependencyResolverLog) << "Dependency graph cleared," << count << "nodes removed";
}

std::vector<std::string> PluginDependencyResolver::topological_sort() const {
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> temp_visited;
    
    // Helper function for DFS-based topological sort
    std::function<bool(const std::string&)> visit = [&](const std::string& plugin_id) -> bool {
        if (temp_visited.find(plugin_id) != temp_visited.end()) {
            return false; // Circular dependency detected
        }
        
        if (visited.find(plugin_id) != visited.end()) {
            return true; // Already processed
        }
        
        temp_visited.insert(plugin_id);
        
        auto it = m_dependency_graph.find(plugin_id);
        if (it != m_dependency_graph.end()) {
            for (const auto& dep : it->second.dependencies) {
                if (!visit(dep)) {
                    return false;
                }
            }
        }
        
        temp_visited.erase(plugin_id);
        visited.insert(plugin_id);
        result.push_back(plugin_id);
        
        return true;
    };
    
    // Visit all nodes
    for (const auto& [plugin_id, node] : m_dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (!visit(plugin_id)) {
                qCWarning(dependencyResolverLog) << "Circular dependency detected during topological sort";
                return {}; // Return empty vector on circular dependency
            }
        }
    }
    
    // Reverse the result to get correct load order
    std::reverse(result.begin(), result.end());
    
    return result;
}

int PluginDependencyResolver::calculate_dependency_level(const std::string& plugin_id,
                                                       const std::vector<std::string>& dependencies) const {
    if (dependencies.empty()) {
        return 0;
    }
    
    int max_level = 0;
    for (const auto& dep : dependencies) {
        auto it = m_dependency_graph.find(dep);
        if (it != m_dependency_graph.end()) {
            auto dep_dependencies = get_dependencies(dep);
            int dep_level = calculate_dependency_level(dep, dep_dependencies);
            max_level = std::max(max_level, dep_level + 1);
        }
    }
    
    return max_level;
}

void PluginDependencyResolver::detect_circular_dependencies() {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;
    QStringList circular_plugins;
    
    for (const auto& [plugin_id, node] : m_dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (has_circular_dependency(plugin_id, visited, recursion_stack)) {
                circular_plugins.append(QString::fromStdString(plugin_id));
                qCWarning(dependencyResolverLog) << "Circular dependency detected involving plugin:"
                                                << QString::fromStdString(plugin_id);
            }
        }
    }
    
    if (!circular_plugins.isEmpty()) {
        emit circular_dependency_detected(circular_plugins);
    }
}

bool PluginDependencyResolver::has_circular_dependency(const std::string& plugin_id,
                                                     std::unordered_set<std::string>& visited,
                                                     std::unordered_set<std::string>& recursion_stack) const {
    visited.insert(plugin_id);
    recursion_stack.insert(plugin_id);
    
    auto it = m_dependency_graph.find(plugin_id);
    if (it != m_dependency_graph.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (recursion_stack.find(dep) != recursion_stack.end()) {
                return true; // Circular dependency found
            }
            
            if (visited.find(dep) == visited.end()) {
                if (has_circular_dependency(dep, visited, recursion_stack)) {
                    return true;
                }
            }
        }
    }
    
    recursion_stack.erase(plugin_id);
    return false;
}

} // namespace qtplugin

#include "plugin_dependency_resolver.moc"
