cmake_minimum_required(VERSION 3.15)
project(RWorld VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library
add_library(rworld
    src/world.cpp
)

target_include_directories(rworld PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(rworld PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Compiler warnings
if(MSVC)
    target_compile_options(rworld PRIVATE /W4)
else()
    target_compile_options(rworld PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Find SDL2 and SDL2_ttf
find_package(SDL2 QUIET)
find_package(SDL2_ttf QUIET)

# Examples
add_executable(world_demo examples/world_demo.cpp)
target_link_libraries(world_demo PRIVATE rworld)

# Add SDL2 support if available
if(SDL2_FOUND)
    target_compile_definitions(world_demo PRIVATE USE_SDL2)
    target_link_libraries(world_demo PRIVATE SDL2::SDL2)
    message(STATUS "SDL2 found - building with graphical visualization")
    
    # Add SDL2_ttf if available
    if(SDL2_ttf_FOUND)
        target_compile_definitions(world_demo PRIVATE USE_SDL2_TTF)
        target_link_libraries(world_demo PRIVATE SDL2_ttf::SDL2_ttf)
        message(STATUS "SDL2_ttf found - text rendering enabled")
    else()
        message(STATUS "SDL2_ttf not found - using basic visualization without text")
        message(STATUS "Install SDL2_ttf for text rendering: sudo apt-get install libsdl2-ttf-dev")
    endif()
else()
    message(STATUS "SDL2 not found - building text-only demo")
    message(STATUS "Install SDL2 for graphical visualization: sudo apt-get install libsdl2-dev")
endif()

# Optional: Install targets
install(TARGETS rworld
    EXPORT RWorldTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)
