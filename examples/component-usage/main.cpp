/**
 * @file main.cpp
 * @brief Example demonstrating direct component usage in QtPlugin v3.0.0
 * @version 3.0.0
 */

#include <qtplugin/qtplugin.hpp>      // Core functionality
#include <qtplugin/components.hpp>    // Component access

#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <memory>

/**
 * @brief Example 1: Using the standard high-level API (recommended for most users)
 */
void example_standard_api() {
    std::cout << "\n=== Example 1: Standard High-Level API ===\n";
    
    // This is the recommended approach for most users
    qtplugin::PluginManager manager;
    
    // Load a plugin using the high-level API
    auto result = manager.load_plugin("example_plugin.so");
    if (result) {
        std::cout << "Plugin loaded successfully using standard API\n";
    } else {
        std::cout << "Failed to load plugin: " << result.error().message << "\n";
    }
    
    // Get plugin information
    auto plugins = manager.get_loaded_plugins();
    std::cout << "Total loaded plugins: " << plugins.size() << "\n";
}

/**
 * @brief Example 2: Using components directly for advanced scenarios
 */
void example_component_usage() {
    std::cout << "\n=== Example 2: Direct Component Usage ===\n";
    
    // Create components independently
    auto registry = std::make_unique<qtplugin::PluginRegistry>();
    auto validator = std::make_unique<qtplugin::SecurityValidator>();
    auto allocator = std::make_unique<qtplugin::ResourceAllocator>();
    
    std::cout << "Created independent components:\n";
    std::cout << "- PluginRegistry\n";
    std::cout << "- SecurityValidator\n";
    std::cout << "- ResourceAllocator\n";
    
    // Use security validator directly
    std::filesystem::path plugin_path = "example_plugin.so";
    if (std::filesystem::exists(plugin_path)) {
        auto validation_result = validator->validate_file_integrity(plugin_path);
        if (validation_result.is_valid) {
            std::cout << "Plugin file validation: PASSED\n";
        } else {
            std::cout << "Plugin file validation: FAILED\n";
            for (const auto& error : validation_result.errors) {
                std::cout << "  Error: " << error << "\n";
            }
        }
    } else {
        std::cout << "Plugin file not found for validation\n";
    }
    
    // Use resource allocator directly
    auto allocation_result = allocator->allocate_resource(
        qtplugin::ResourceType::Memory,
        "example_plugin",
        qtplugin::ResourcePriority::Normal
    );
    
    if (allocation_result) {
        std::cout << "Resource allocated: " << allocation_result->allocation_id << "\n";
        
        // Clean up allocation
        allocator->deallocate_resource(allocation_result->allocation_id);
        std::cout << "Resource deallocated\n";
    } else {
        std::cout << "Resource allocation failed: " << allocation_result.error().message << "\n";
    }
}

/**
 * @brief Example 3: Custom component configuration
 */
void example_component_configuration() {
    std::cout << "\n=== Example 3: Component Configuration ===\n";
    
    // Create and configure a resource pool
    auto memory_pool = std::make_unique<qtplugin::ResourcePool<std::vector<char>>>(
        "memory_pool", qtplugin::ResourceType::Memory
    );
    
    // Configure quota
    qtplugin::ResourceQuota quota;
    quota.max_instances = 10;
    quota.max_memory_bytes = 1024 * 1024; // 1MB
    quota.max_lifetime = std::chrono::minutes(30);
    memory_pool->set_quota(quota);
    
    // Set factory for creating resources
    memory_pool->set_factory([]() {
        return std::make_unique<std::vector<char>>(1024); // 1KB buffers
    });
    
    std::cout << "Configured memory pool:\n";
    std::cout << "- Max instances: " << quota.max_instances << "\n";
    std::cout << "- Max memory: " << quota.max_memory_bytes << " bytes\n";
    std::cout << "- Max lifetime: " << quota.max_lifetime.count() << " minutes\n";
    
    // Test resource acquisition
    auto resource_result = memory_pool->acquire_resource("test_plugin");
    if (resource_result) {
        auto [handle, resource] = std::move(resource_result.value());
        std::cout << "Acquired resource with handle: " << handle.id << "\n";
        std::cout << "Resource size: " << resource->size() << " bytes\n";
        
        // Release resource back to pool
        memory_pool->release_resource(handle, std::move(resource));
        std::cout << "Released resource back to pool\n";
    }
}

/**
 * @brief Example 4: Component monitoring and metrics
 */
void example_component_monitoring() {
    std::cout << "\n=== Example 4: Component Monitoring ===\n";
    
    // Create resource monitor
    auto monitor = std::make_unique<qtplugin::ResourceMonitor>();
    
    // Configure monitoring
    qtplugin::MonitoringConfig config;
    config.monitoring_interval = std::chrono::milliseconds(1000);
    config.enable_usage_tracking = true;
    config.enable_performance_tracking = true;
    config.enable_leak_detection = true;
    monitor->set_monitoring_config(config);
    
    std::cout << "Configured resource monitor:\n";
    std::cout << "- Monitoring interval: " << config.monitoring_interval.count() << "ms\n";
    std::cout << "- Usage tracking: " << (config.enable_usage_tracking ? "enabled" : "disabled") << "\n";
    std::cout << "- Leak detection: " << (config.enable_leak_detection ? "enabled" : "disabled") << "\n";
    
    // Add a resource alert
    qtplugin::ResourceAlert alert;
    alert.name = "high_memory_usage";
    alert.resource_type = qtplugin::ResourceType::Memory;
    alert.condition = "memory_usage > 80%";
    alert.enabled = true;
    alert.cooldown = std::chrono::seconds(30);
    
    auto alert_result = monitor->add_alert(alert);
    if (alert_result) {
        std::cout << "Added resource alert: " << alert.name << "\n";
    }
    
    // Get current snapshot
    auto snapshot = monitor->get_current_snapshot();
    std::cout << "Current resource snapshot:\n";
    std::cout << "- Active allocations: " << snapshot.active_allocations << "\n";
    std::cout << "- Total memory usage: " << snapshot.total_memory_usage << " bytes\n";
    std::cout << "- CPU usage: " << snapshot.cpu_usage_percent << "%\n";
}

/**
 * @brief Example 5: Component information and discovery
 */
void example_component_discovery() {
    std::cout << "\n=== Example 5: Component Discovery ===\n";
    
    // Get available components
    auto components = qtplugin::components::get_available_components();
    std::cout << "Available components (" << components.size() << "):\n";
    
    for (const auto& component : components) {
        std::cout << "- " << component.name << " v" << component.version 
                  << ": " << component.description << "\n";
    }
    
    // Check specific component availability
    std::vector<std::string> check_components = {
        "PluginRegistry", "SecurityValidator", "ResourcePool", "NonExistentComponent"
    };
    
    std::cout << "\nComponent availability check:\n";
    for (const auto& name : check_components) {
        bool available = qtplugin::components::is_component_available(name);
        std::cout << "- " << name << ": " << (available ? "available" : "not available") << "\n";
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "QtPlugin v3.0.0 Component Usage Examples\n";
    std::cout << "========================================\n";
    
    try {
        // Run examples
        example_standard_api();
        example_component_usage();
        example_component_configuration();
        example_component_monitoring();
        example_component_discovery();
        
        std::cout << "\n=== All Examples Completed Successfully ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
