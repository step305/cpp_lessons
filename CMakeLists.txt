cmake_minimum_required(VERSION 3.10)
project(simple)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package (Threads REQUIRED)
find_package(realsense2 REQUIRED)

add_executable(simple main.cpp camera_driver.cpp utils.cpp RealsenseD455.cpp)
target_link_libraries(
        ${PROJECT_NAME}
        ${OpenCV_LIBS}
        ${realsense2_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
)
