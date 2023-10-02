cmake_minimum_required(VERSION 3.26)
project(nes)

set(CMAKE_CXX_STANDARD 20)

if (APPLE)
    enable_language(OBJC)
    file(GLOB_RECURSE SOURCES src/*.cpp src/*.mm)
else ()
    file(GLOB_RECURSE SOURCES src/*.cpp)
endif ()

file(GLOB_RECURSE SOURCES_TEST_CPU test/cpu/*.cpp)
set(SOURCES_TEST_CPU ${SOURCES_TEST_CPU} src/cpu.cpp)

add_executable(nes ${SOURCES})

add_executable(test-cpu ${SOURCES_TEST_CPU})
target_include_directories(test-cpu PRIVATE src)

# Libraries

# Link with spdlog (our logging library).
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(nes PRIVATE spdlog::spdlog)

target_link_libraries(test-cpu PRIVATE spdlog::spdlog)

# Link with Dear ImGUI.
find_package(imgui CONFIG REQUIRED)
target_link_libraries(nes PRIVATE imgui::imgui)

# Link with GLFW3 (our windowing and graphics context library).
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(nes PRIVATE glfw)

# Link with OpenGL.
find_package(OpenGL REQUIRED)
target_link_libraries(nes PRIVATE OpenGL::GL)

# Link with JSON.
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(test-cpu PRIVATE nlohmann_json::nlohmann_json)

# Silence OpenGL warnings on MacOS.
add_compile_definitions(GL_SILENCE_DEPRECATION)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable memory and address sanitizers on debug build.
    target_compile_options(nes PRIVATE -fsanitize=address)
    target_link_options(nes PRIVATE -fsanitize=address)
    target_compile_options(test-cpu PRIVATE -fsanitize=address)
    target_link_options(test-cpu PRIVATE -fsanitize=address)

    # Also enable runtime CPU tracing.
    add_compile_definitions(NES_CPU_RT)
else ()
    # Enable LTO.
    target_compile_options(nes PRIVATE -flto)
    target_link_options(nes PRIVATE -flto)
    target_compile_options(test-cpu PRIVATE -flto)
    target_link_options(test-cpu PRIVATE -flto)
endif ()
