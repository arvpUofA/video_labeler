cmake_minimum_required(VERSION 3.10.2)
project( VideoLabeler )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( OpenCV REQUIRED )

include_directories( ${CMAKE_SOURCE_DIR}/tracker ${OpenCV_INCLUDE_DIRS} )

add_executable( VideoLabeler 
    tracker/kcftracker.cpp
    tracker/fhog.cpp
    src/main.cpp )

target_link_libraries( VideoLabeler ${OpenCV_LIBS} stdc++fs)