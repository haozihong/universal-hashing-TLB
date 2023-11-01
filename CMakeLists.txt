cmake_minimum_required(VERSION 3.21)

project(tlbsim LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/driver.cpp
)

add_executable(tlbsim ${SOURCES})

target_compile_options(tlbsim PRIVATE -fsanitize=address)
target_link_options(tlbsim PRIVATE -fsanitize=address)
target_include_directories(tlbsim PRIVATE .)