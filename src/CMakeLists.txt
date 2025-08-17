# Source files for the main application
set(SOURCES
    main.cpp
    # UI Components
    ui/MainWindow.cpp
    ui/DashboardWidget.cpp
    ui/PluginStoreWidget.cpp
    ui/PluginWizard.cpp
    ui/PluginWidgets.cpp
    # Modern Ribbon Interface
    ui/RibbonInterface.cpp
    ui/RibbonControls.cpp
    ui/RibbonMainWindow.cpp
    # Core Plugin System
    core/PluginRegistry.cpp
    # Managers
    managers/ApplicationManager.cpp
    managers/PluginSecurityManager.cpp
    managers/ThemeManager.cpp
    managers/PluginDependencyManager.cpp
    managers/PluginUpdateManager.cpp
    managers/PluginSandbox.cpp
    managers/PluginSignatureVerifier.cpp
    managers/PluginPermissionSystem.cpp
    managers/PluginPerformanceProfiler.cpp
    managers/PluginResourceMonitor.cpp
    # Utilities
    utils/PluginHelpers.cpp
    utils/PluginTemplateGenerator.cpp
    # utils/PluginTemplateWizard.cpp  # Temporarily disabled due to compilation issues
    utils/PluginValidator.cpp
    utils/PluginDebugger.cpp
    utils/PluginHotReload.cpp
    utils/PluginLiveDebugger.cpp
    utils/PluginCodeAnalyzer.cpp

    # Integration
    integration/PluginRESTAPI.cpp
    integration/PluginCommunication.cpp
    integration/ExternalToolIntegration.cpp
    # Examples
    examples/ExampleUIPlugin.cpp
)

# Header files
set(HEADERS
    # UI Components
    ui/MainWindow.h
    ui/DashboardWidget.h
    ui/PluginStoreWidget.h
    ui/PluginWizard.h
    ui/PluginWidgets.h
    # Modern Ribbon Interface
    ui/RibbonInterface.h
    ui/RibbonControls.h
    ui/RibbonThemes.h
    ui/RibbonMainWindow.h
    ui/RibbonCustomization.h
    # Core Plugin System
    core/PluginRegistry.h
    # Managers
    managers/ApplicationManager.h
    managers/PluginSecurityManager.h
    managers/ThemeManager.h
    managers/PluginDependencyManager.h
    managers/PluginUpdateManager.h
    managers/PluginMarketplace.h
    managers/PluginSandbox.h
    managers/PluginSignatureVerifier.h
    managers/PluginPermissionSystem.h
    managers/PluginPerformanceProfiler.h
    managers/PluginResourceMonitor.h
    # Utilities
    utils/PluginHelpers.h
    utils/PluginTemplateGenerator.h
    utils/PluginValidator.h
    utils/PluginDebugger.h
    utils/PluginHotReload.h
    utils/PluginLiveDebugger.h
    utils/PluginCodeAnalyzer.h

    # Integration
    integration/PluginRESTAPI.h
    integration/PluginCommunication.h
    integration/ExternalToolIntegration.h

    # Examples
    examples/ExampleUIPlugin.h
)

# Resource files
set(RESOURCES
    # ../resources/resources.qrc  # Temporarily disabled
)

# Create the main executable
add_executable(AdvancedPluginManager ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt6 libraries and QtPlugin library
target_link_libraries(AdvancedPluginManager
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Qml
    Qt6::Gui
    QtPlugin::Core
)

# Link Qt6 Charts if available
if(Qt6Charts_FOUND)
    target_link_libraries(AdvancedPluginManager Qt6::Charts)
endif()

# Link Qt6 HttpServer if available
if(Qt6HttpServer_FOUND)
    target_link_libraries(AdvancedPluginManager Qt6::HttpServer)
endif()

# Link Qt6 WebSockets if available
if(Qt6WebSockets_FOUND)
    target_link_libraries(AdvancedPluginManager Qt6::WebSockets)
endif()

# Set target properties
set_target_properties(AdvancedPluginManager PROPERTIES
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME "AdvancedPluginManager"
)

# Platform-specific linking
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(AdvancedPluginManager
        user32
        shell32
        advapi32
    )
    
    # Windows-specific properties handled above

    # MSVC-specific linker flags
    if(MSVC)
        set_target_properties(AdvancedPluginManager PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )
    endif()
    
    # Add Windows resource file if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
        target_sources(AdvancedPluginManager PRIVATE resources.rc)
    endif()
    
elseif(APPLE)
    # macOS-specific configuration
    set_target_properties(AdvancedPluginManager PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Advanced Plugin Manager"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER "com.example.advancedpluginmanager"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.advancedpluginmanager"
        MACOSX_BUNDLE_ICON_FILE "app.icns"
    )
    
    # Add app icon if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../resources/app.icns")
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/../resources/app.icns" 
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        target_sources(AdvancedPluginManager PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../resources/app.icns")
    endif()
    
elseif(UNIX)
    # Linux-specific libraries
    target_link_libraries(AdvancedPluginManager
        ${CMAKE_DL_LIBS}
        pthread
    )
endif()

# Compiler-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(AdvancedPluginManager PRIVATE /O2 /GL)
        target_link_options(AdvancedPluginManager PRIVATE /LTCG)
    else()
        target_compile_options(AdvancedPluginManager PRIVATE -O3 -flto)
        target_link_options(AdvancedPluginManager PRIVATE -flto)
    endif()
endif()

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(AdvancedPluginManager PRIVATE /Zi)
    else()
        target_compile_options(AdvancedPluginManager PRIVATE -g)
    endif()
endif()

# Include current directory for headers
target_include_directories(AdvancedPluginManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add compile definitions
target_compile_definitions(AdvancedPluginManager PRIVATE
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    APP_VERSION="${PROJECT_VERSION}"
    APP_NAME="Advanced Plugin Manager"
    ORGANIZATION_NAME="Example Corporation"
    ORGANIZATION_DOMAIN="example.com"
)

# Enable all Qt warnings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(AdvancedPluginManager PRIVATE
        QT_DEPRECATED_WARNINGS
        QT_WARN_ON
    )
endif()

# Create plugins directory in build output
add_custom_command(TARGET AdvancedPluginManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
    $<TARGET_FILE_DIR:AdvancedPluginManager>/plugins
    COMMENT "Creating plugins directory"
)

# Copy Qt6 DLLs on Windows (for development) - temporarily disabled
# if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
#     find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
#     if(WINDEPLOYQT_EXECUTABLE)
#         add_custom_command(TARGET AdvancedPluginManager POST_BUILD
#             COMMAND ${WINDEPLOYQT_EXECUTABLE} --debug --qmldir ${CMAKE_SOURCE_DIR}/qml $<TARGET_FILE:AdvancedPluginManager>
#             COMMENT "Deploying Qt libraries for development"
#         )
#     endif()
# endif()

# IDE support - organize files in folders
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Resource Files" FILES ${RESOURCES})

# Set startup project for Visual Studio
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AdvancedPluginManager)
endif()
