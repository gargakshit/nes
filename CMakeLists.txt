cmake_minimum_required(VERSION 3.26)
project(nes)

set(CMAKE_CXX_STANDARD 20)

if (APPLE)
    enable_language(OBJC)
    file(GLOB_RECURSE SOURCES src/*.cpp src/*.mm)
else()
    file(GLOB_RECURSE SOURCES src/*.cpp)
endif ()

add_executable(nes ${SOURCES})

# Libraries

# Link with spdlog (our logging library).
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(nes PRIVATE spdlog::spdlog)

# Link with Dear ImGUI.
find_package(imgui CONFIG REQUIRED)
target_link_libraries(nes PRIVATE imgui::imgui)

# Link with GLFW3 (our windowing and graphics context library).
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(nes PRIVATE glfw)

# Link with OpenGL.
find_package(OpenGL REQUIRED)
target_link_libraries(nes PRIVATE OpenGL::GL)

# Silence OpenGL warnings on MacOS.
add_compile_definitions(GL_SILENCE_DEPRECATION)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable memory and address sanitizers on debug build.
    target_compile_options(nes PRIVATE -fsanitize=address)
    target_link_options(nes PRIVATE -fsanitize=address)

    # Also enable runtime CPU tracing.
    add_compile_definitions(NES_CPU_RT)
endif ()
