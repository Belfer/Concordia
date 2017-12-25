cmake_minimum_required(VERSION 2.8)

project(ConcordiaECS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Define external modules cmake path.
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# Find packages
# find_package(OpenGL REQUIRED)
# find_package(SFML 2.3 REQUIRED graphics window system audio network)

add_subdirectory(src)

include_directories(src
    #${SFML_INCLUDE_DIR}
)

add_library(concordia ${CONCORDIA_SRCS} ${CONCORDIA_HDRS})
set_target_properties(concordia PROPERTIES LINKER_LANGUAGE CXX)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
  concordia
  # ${SFML_LIBRARIES}
)
