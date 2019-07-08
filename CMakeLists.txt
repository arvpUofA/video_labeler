cmake_minimum_required(VERSION 3.10.2)
project( VideoLabeler )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# 3.2.0 Exact as to not conflict with 3.3.1 built from catkin_ws/build_external
find_package( OpenCV EXACT 3.2.0 REQUIRED )
if (OpenCV_FOUND)
   message("Found OpenCV")
   message("Includes: " ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

include_directories( ${CMAKE_SOURCE_DIR}/tracker ${OpenCV_INCLUDE_DIRS} )

add_executable( VideoLabeler 
    tracker/kcftracker.cpp
    tracker/fhog.cpp
    src/main.cpp )

target_link_libraries( VideoLabeler ${OpenCV_LIBS} stdc++fs)