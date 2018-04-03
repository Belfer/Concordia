cmake_minimum_required(VERSION 2.8)

project(ConcordiaECS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Define external modules cmake path.
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# Find packages
# find_package(OpenGL REQUIRED)
# find_package(SFML 2.3 REQUIRED graphics window system audio network)

add_subdirectory(src)
include_directories(src)

add_library(concordia ${CONCORDIA_SRCS} ${CONCORDIA_HDRS})

enable_testing()

add_executable(ecs_test test/ecs_test.cpp)
add_executable(ecs_showcase test/ecs_basic_showcase)
add_test(ECSTest ecs_test)
add_test(ECSExample ecs_showcase)
target_link_libraries(ecs_test concordia)
target_link_libraries(ecs_showcase concordia)
