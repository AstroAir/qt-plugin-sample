# Service Plugin Example

This example demonstrates a comprehensive service plugin that showcases:

- Background service implementation
- Lifecycle management with state transitions
- Configuration management and validation
- Timer-based operations
- Performance monitoring
- Error handling and recovery
- Inter-plugin communication
- Resource management

## Features Demonstrated

### 1. Service Lifecycle
- Proper initialization and shutdown sequences
- State management (Starting, Running, Paused, Stopping)
- Graceful error handling and recovery
- Resource cleanup

### 2. Configuration Management
- JSON-based configuration with validation
- Dynamic configuration updates
- Default configuration provision
- Configuration persistence

### 3. Background Operations
- Timer-based periodic tasks
- Asynchronous operations
- Work queue management
- Progress tracking

### 4. Performance Monitoring
- Execution time tracking
- Resource usage monitoring
- Performance metrics collection
- Health status reporting

### 5. Communication
- Command execution interface
- Event emission and handling
- Status reporting
- Logging integration

## Plugin Architecture

```
ServicePlugin
├── Core Service Logic
│   ├── Timer Management
│   ├── Work Queue Processing
│   └── State Management
├── Configuration System
│   ├── JSON Configuration
│   ├── Validation Rules
│   └── Dynamic Updates
├── Performance Monitoring
│   ├── Metrics Collection
│   ├── Health Checks
│   └── Resource Tracking
└── Communication Interface
    ├── Command Handlers
    ├── Event Emission
    └── Status Reporting
```

## Building and Running

### Prerequisites
- Qt6 Core module
- QtPlugin library
- C++20 compatible compiler

### Build Instructions

```bash
cd examples/service-plugin
mkdir build && cd build
cmake ..
cmake --build .
```

### Running the Example

```bash
# Run the test application
./ServicePluginExample

# The application will:
# 1. Load the service plugin
# 2. Initialize and start the service
# 3. Demonstrate various operations
# 4. Show performance metrics
# 5. Gracefully shutdown
```

## Expected Output

```
=== Service Plugin Example ===
Loading service plugin...
Plugin loaded: Advanced Service Plugin v1.0.0

=== Service Lifecycle ===
Starting service...
Service started successfully
Service state: Running

=== Configuration Management ===
Default configuration loaded
Updating configuration...
Configuration updated successfully

=== Background Operations ===
Service performing background tasks...
Task 1 completed (processing time: 15ms)
Task 2 completed (processing time: 12ms)
Task 3 completed (processing time: 18ms)

=== Performance Metrics ===
Total tasks processed: 3
Average processing time: 15ms
Memory usage: 2.4MB
CPU usage: 0.8%
Uptime: 5.2 seconds

=== Service Commands ===
Available commands: start, stop, pause, resume, status, configure, metrics
Executing 'status' command...
Status: {"state":"running","uptime":5200,"tasks_processed":3}

=== Graceful Shutdown ===
Stopping service...
Service stopped successfully
Plugin shutdown complete
```

## Key Implementation Details

### Service State Management

The plugin implements a comprehensive state machine:

```cpp
enum class ServiceState {
    Stopped,
    Starting,
    Running,
    Pausing,
    Paused,
    Resuming,
    Stopping,
    Error
};
```

### Configuration Schema

```json
{
    "timer_interval": 1000,
    "max_queue_size": 100,
    "enable_monitoring": true,
    "log_level": "info",
    "auto_start": true,
    "performance_tracking": {
        "enabled": true,
        "sample_rate": 1.0,
        "history_size": 1000
    }
}
```

### Performance Monitoring

The plugin tracks various metrics:
- Task execution times
- Memory usage
- CPU utilization
- Queue sizes
- Error rates
- Uptime statistics

### Error Handling

Comprehensive error handling includes:
- Graceful degradation
- Automatic recovery
- Error logging
- State restoration
- Resource cleanup

## Code Structure

```
service-plugin/
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── metadata.json               # Plugin metadata
├── src/
│   ├── service_plugin.hpp      # Plugin header
│   ├── service_plugin.cpp      # Plugin implementation
│   ├── service_worker.hpp      # Background worker
│   ├── service_worker.cpp      # Worker implementation
│   ├── performance_monitor.hpp # Performance monitoring
│   ├── performance_monitor.cpp # Monitor implementation
│   └── main.cpp                # Test application
├── config/
│   ├── default_config.json     # Default configuration
│   └── schema.json             # Configuration schema
├── tests/
│   ├── test_service_plugin.cpp # Unit tests
│   └── test_performance.cpp    # Performance tests
└── docs/
    ├── architecture.md         # Architecture documentation
    ├── configuration.md        # Configuration guide
    └── performance.md          # Performance guide
```

## Usage in Applications

### Basic Integration

```cpp
#include <qtplugin/qtplugin.hpp>

// Load and start the service
qtplugin::PluginManager manager;
auto result = manager.load_plugin("service_plugin.qtplugin");
if (result) {
    auto plugin = manager.get_plugin(result.value());
    auto service = qobject_cast<qtplugin::IServicePlugin*>(plugin.get());
    
    if (service) {
        // Initialize and start
        plugin->initialize();
        service->start_service();
        
        // Configure the service
        QJsonObject config;
        config["timer_interval"] = 500;
        plugin->configure(config);
        
        // Monitor performance
        auto metrics = plugin->execute_command("metrics");
        // Handle metrics...
    }
}
```

### Advanced Integration

```cpp
// Connect to service events
connect(plugin.get(), &qtplugin::IPlugin::command_executed,
        this, &MyApp::on_service_command);

// Set up monitoring
QTimer* monitor = new QTimer(this);
connect(monitor, &QTimer::timeout, [=]() {
    auto status = plugin->execute_command("status");
    // Update UI with status...
});
monitor->start(1000);

// Handle service lifecycle
connect(service, &qtplugin::IServicePlugin::service_started,
        this, &MyApp::on_service_started);
connect(service, &qtplugin::IServicePlugin::service_stopped,
        this, &MyApp::on_service_stopped);
```

## Testing

The example includes comprehensive tests:

```bash
# Run unit tests
cd build
ctest --verbose

# Run performance tests
./test_performance

# Run integration tests
./test_integration
```

## Customization

You can customize this example by:

1. **Modifying the service logic** in `service_worker.cpp`
2. **Adding new configuration options** in the schema
3. **Implementing additional commands** in the command handler
4. **Extending performance monitoring** with custom metrics
5. **Adding new service states** for specific workflows

## Related Examples

- [Basic Plugin](../basic/README.md) - Simple plugin foundation
- [Configuration Management](../configuration/README.md) - Advanced configuration
- [Performance Monitoring](../performance/README.md) - Detailed performance tracking
- [Inter-Plugin Communication](../communication/README.md) - Plugin messaging

## Troubleshooting

### Common Issues

1. **Service won't start**: Check configuration and dependencies
2. **High CPU usage**: Adjust timer intervals and processing logic
3. **Memory leaks**: Verify resource cleanup in shutdown
4. **Configuration errors**: Validate JSON schema compliance

### Debug Mode

Enable debug logging by setting the log level:

```json
{
    "log_level": "debug",
    "enable_monitoring": true
}
```

## Performance Considerations

- Timer intervals affect CPU usage
- Queue sizes impact memory usage
- Monitoring overhead should be considered
- Background thread management is crucial

For detailed performance analysis, see [Performance Guide](docs/performance.md).
