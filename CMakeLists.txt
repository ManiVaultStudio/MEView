cmake_minimum_required(VERSION 3.5.0)

# -----------------------------------------------------------------------------
# MEViewer
# -----------------------------------------------------------------------------
PROJECT("MEView")

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
#find_package(OpenSSL REQUIRED)
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets OpenGLWidgets REQUIRED)

find_package(ManiVault COMPONENTS Core PointData TextData CONFIG REQUIRED)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
# Define the plugin sources
set(PLUGIN_SOURCES
    src/MEView.h
    src/MEView.cpp
    #src/MorphologyWidget.h
    #src/MorphologyWidget.cpp
    src/MEWidget.h
    src/MEWidget.cpp
    src/NeuronDescriptor.h
    src/MorphologyDescription.h
    src/MorphologyDescription.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/HoverPopup.h
    src/HoverPopup.cpp
    src/Scene.h
    src/Scene.cpp
    src/LRUCache.h
    #src/Query.h
    #src/Query.cpp
    #src/ImageQuery.h
    src/LayerDepthsReader.h
    src/LayerDepthsReader.cpp
    src/MEView.json
)

set(RENDER_SOURCES
    #src/MorphologyRenderer.h
    #src/MorphologyRenderer.cpp
    #src/MorphologyLineRenderer.h
    #src/MorphologyLineRenderer.cpp
    #src/MorphologyTubeRenderer.h
    #src/MorphologyTubeRenderer.cpp
    src/EMRenderer.h
    src/EMRenderer.cpp
    #src/TraceRenderer.h
    #src/TraceRenderer.cpp
    #src/MorphologyLineSegments.h
    #src/MorphologyLineSegments.cpp
    src/LayerDrawing.h
    src/LayerDrawing.cpp
    src/Rendering/CellRenderObject.h
    src/Rendering/CellRenderObject.cpp
    src/Rendering/RenderObjectBuilder.h
    src/Rendering/RenderObjectBuilder.cpp
    src/Rendering/RenderState.h
    src/Rendering/RenderState.cpp
    src/Rendering/RenderRegion.h
    src/Rendering/RenderRegion.cpp
)

set(EPHYS_SOURCES
    src/Electrophysiology/EphysWidget.h
    src/Electrophysiology/EphysWidget.cpp
    src/Electrophysiology/EphysWebWidget.h
    src/Electrophysiology/EphysWebWidget.cpp
    #src/Electrophysiology/TraceRenderer.h
    #src/Electrophysiology/TraceRenderer.cpp
)

set(QRESOURCES
    res/resources.qrc
)
QT6_ADD_RESOURCES(RESOURCE_FILES ${QRESOURCES})

source_group( Plugin FILES ${PLUGIN_SOURCES})
source_group( Rendering FILES ${RENDER_SOURCES})
source_group( Electrophysiology FILES ${EPHYS_SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
# Create dynamic library for the plugin
add_library(${PROJECT_NAME} SHARED ${PLUGIN_SOURCES} ${RENDER_SOURCES} ${EPHYS_SOURCES} ${RESOURCE_FILES})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault headers, including system data plugins
target_include_directories(${PROJECT_NAME} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
#target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/curl/include/")


# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)


# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
# Link to Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Network)
#target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
#target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)

# Link to ManiVault and data plugins
# The link path in this repo assumes that the ManiVault core was built locally
# in contrast to having been installed with an installer. In the latter case you'll have
# to adapt the MV_LINK_PATH and PLUGIN_LINK_PATH to your install folder
set(MV_LINK_PATH "${ManiVault_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${ManiVault_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(CELL_MORPHOLOGY_DATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}CellMorphologyData${MV_LINK_SUFFIX}")
set(EPHYS_DATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}EphysData${MV_LINK_SUFFIX}")

# Link to ManiVault and data plugins
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::Core)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::PointData)
target_link_libraries(${PROJECT_NAME} PRIVATE ManiVault::TextData)

target_link_libraries(${PROJECT_NAME} PRIVATE "${CELL_MORPHOLOGY_DATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT_NAME} PRIVATE "${EPHYS_DATA_LINK_LIBRARY}")

#target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/3rdparty/curl/lib/libcurl_imp.lib")

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
# Install the shared plugin libary to the "Plugins" folder in the ManiVault install directory
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug/ManiVault\ Studio.exe,${ManiVault_INSTALL_DIR}/release/ManiVault\ Studio.exe>)
endif()
