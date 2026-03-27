cmake_minimum_required(VERSION 3.22)
project(Tibyan VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets Concurrent LinguistTools REQUIRED)
find_package(Eigen3 3.4 REQUIRED)

option(TIBYAN_BUILD_TESTS "Build unit tests" ON)

add_executable(Tibyan src/ui/main.cpp src/ui/MainWindow.cpp src/ui/MainWindow.h)
set_target_properties(Tibyan PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(Tibyan PRIVATE Qt6::Widgets Qt6::Concurrent Eigen3::Eigen)

if(TIBYAN_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_executable(ui_tests tests/unit/ui_tests.cpp src/ui/MainWindow.cpp src/ui/MainWindow.h)
    target_link_libraries(ui_tests PRIVATE Qt6::Widgets GTest::gtest GTest::gtest_main)
    add_test(NAME ui_tests COMMAND ui_tests)
endif()
