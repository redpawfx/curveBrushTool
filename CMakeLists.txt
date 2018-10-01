project(curveBrushTool)

cmake_minimum_required(VERSION 2.8)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()

set(CMAKE_CXX_STANDARD 11)

if (LINUX)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--no-undefined")
endif ()

find_package(Maya REQUIRED)

set(HDR
    brushContext.h
    brushContextCommand.h
    brushToolCommand.h)

set(SRC
    brushContext.cpp
    brushContextCommand.cpp
    brushToolCommand.cpp
    pluginMain.cpp)

add_library(curveBrushTool SHARED ${SRC} ${HDR})

set_target_properties(curveBrushTool PROPERTIES PREFIX "")

if (APPLE)
    set_target_properties(curveBrushTool PROPERTIES SUFFIX ".bundle")
endif ()

target_include_directories(curveBrushTool PRIVATE "${MAYA_INCLUDE_DIRS}")
target_link_libraries(curveBrushTool PRIVATE ${MAYA_LIBRARIES})

if (LINUX)
    target_link_libraries(curveBrushTool PRIVATE GL pthread)
elseif (APPLE)
    target_link_libraries(curveBrushTool PRIVATE "-framework OpenGL")
endif ()

MAYA_SET_PLUGIN_PROPERTIES(curveBrushTool)

install(
    TARGETS curveBrushTool
    DESTINATION plug-ins)

install(
    DIRECTORY scripts
    DESTINATION .)

install(
    DIRECTORY icons
    DESTINATION .)

install(CODE
    "file(WRITE \"${CMAKE_INSTALL_PREFIX}/curveBrushTool.mod\"
\"+ curveBrushTool any ${CMAKE_INSTALL_PREFIX}
MAYA_SCRIPT_PATH +:= scripts
PYTHONPATH +:= scripts
\")")
