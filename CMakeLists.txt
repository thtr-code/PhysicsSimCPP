cmake_minimum_required(VERSION 3.26)
project(BlackholeSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DGL_SILENCE_DEPRECATION)


find_package(glfw3 REQUIRED)

include_directories(
        ${CMAKE_SOURCE_DIR}/libs/glad/include
        /opt/homebrew/include
)


add_executable(BlackholeSim
        src/main.cpp
        src/Shader.h
        libs/glad/src/glad.c
        src/Planet.h
        src/Planet.cpp
        src/Camera.h
        src/Camera.cpp
)

target_link_libraries(BlackholeSim
        PRIVATE
        glfw
        "-framework OpenGL"
)

add_custom_command(TARGET BlackholeSim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:BlackholeSim>/shaders
)
