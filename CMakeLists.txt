cmake_minimum_required(VERSION 3.10)

# Project name and version
project(hashdict VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Debug symbols and compiler flags
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g -Wall -Wextra -Werror -pedantic)

# Define the hashdict library
add_library(hashdict
    hashdict.c
)

# Set include directories for the library
target_include_directories(hashdict
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create the demo executable
add_executable(hashdict_demo
    hashdict_demo.c
)

# Link the demo with the library
target_link_libraries(hashdict_demo
    PRIVATE
        hashdict
)

# Installation rules (optional)
install(TARGETS hashdict hashdict_demo
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES hashdict.h
    DESTINATION include
)

# Enable testing (optional)
enable_testing()

# Output information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")