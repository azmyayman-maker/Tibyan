cmake_minimum_required(VERSION 3.22)
project(Tibyan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TIBYAN_BUILD_TESTS "Build unit tests" ON)

add_executable(Tibyan src/main.cpp)
