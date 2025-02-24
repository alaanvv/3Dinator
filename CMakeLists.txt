cmake_minimum_required(VERSION 3.16)
project(3Dinator LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(inc/glfw)
add_subdirectory(inc/glad)
add_subdirectory(inc/cglm)
add_executable("Script")

set_property(TARGET "Script" PROPERTY C_STANDARD 11)

file(GLOB MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
target_sources("Script" PRIVATE ${MY_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/script.c")

target_compile_definitions("Script" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories("Script" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
target_include_directories("Script" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/glfw")
target_include_directories("Script" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/glad")
target_include_directories("Script" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/cglm")

target_link_libraries("Script" PRIVATE cglm glfw glad)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/shd" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/img" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/obj" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
