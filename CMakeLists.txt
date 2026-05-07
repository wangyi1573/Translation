cmake_minimum_required(VERSION 3.15)
project(VolcengineTranslationPlugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt
find_package(Qt5 REQUIRED COMPONENTS Core Gui Network Widgets)

# Plugin core sources (PluginCore module)
set(PLUGIN_CORE_SOURCES
    src/PluginCore/HttpClient.cpp
    src/PluginCore/SignatureGenerator.cpp
    src/PluginCore/TranslationEngine.cpp
    src/PluginCore/ConfigManager.cpp
)

set(PLUGIN_CORE_HEADERS
    include/PluginCore/HttpClient.h
    include/PluginCore/SignatureGenerator.h
    include/PluginCore/TranslationEngine.h
    include/PluginCore/ConfigManager.h
    include/PluginCore/TranslationResult.h
)

# Plugin entry point
set(PLUGIN_ENTRY_SOURCES
    src/NddPluginMain.cpp
)

set(PLUGIN_ENTRY_HEADERS
    include/NddPluginInterface.h
)

# All sources
set(ALL_SOURCES
    ${PLUGIN_CORE_SOURCES} ${PLUGIN_CORE_HEADERS}
    ${PLUGIN_ENTRY_SOURCES} ${PLUGIN_ENTRY_HEADERS}
)

# Create shared library (DLL for notepad--)
add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Qt linking
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core Qt5::Gui Qt5::Network Qt5::Widgets
)

# Windows-specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE crypt32)
    add_compile_definitions(_UNICODE UNICODE NDD_EXPORTDLL)
endif()

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install rule
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION bin)