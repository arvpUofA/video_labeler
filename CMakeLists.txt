cmake_minimum_required(VERSION 3.10.2)
project( VideoLabeler )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -lstdc++fs)

# 3.2.0 Exact as to not conflict with 3.3.1 built from catkin_ws/build_external
find_package( OpenCV EXACT 3.2.0 REQUIRED )
if (OpenCV_FOUND)
   message("Found OpenCV")
   message("Includes: " ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)


# darknet sources
# Define path of darknet folder here.
find_path(DARKNET_PATH
	NAMES "README.md"
	HINTS "${CMAKE_CURRENT_SOURCE_DIR}/darknet/")
message(STATUS "Darknet path dir = ${DARKNET_PATH}")

# file(GLOB ...) should not be used here as it prevents CMake from detecting changes,
# and thus rebuilding, when any of the files are modified or removed
set(DARKNET_SOURCE
  ${DARKNET_PATH}/src/activation_layer.c
  ${DARKNET_PATH}/src/activations.c
  ${DARKNET_PATH}/src/avgpool_layer.c
  ${DARKNET_PATH}/src/batchnorm_layer.c
  ${DARKNET_PATH}/src/blas.c
  ${DARKNET_PATH}/src/box.c
  ${DARKNET_PATH}/src/col2im.c
  ${DARKNET_PATH}/src/connected_layer.c
  ${DARKNET_PATH}/src/convolutional_layer.c
  ${DARKNET_PATH}/src/conv_lstm_layer.c
  ${DARKNET_PATH}/src/cost_layer.c
  ${DARKNET_PATH}/src/crnn_layer.c
  ${DARKNET_PATH}/src/crop_layer.c
  ${DARKNET_PATH}/src/dark_cuda.c
  ${DARKNET_PATH}/src/data.c
  ${DARKNET_PATH}/src/deconvolutional_layer.c
  ${DARKNET_PATH}/src/demo.c
  ${DARKNET_PATH}/src/detection_layer.c
  ${DARKNET_PATH}/src/detector.c
  ${DARKNET_PATH}/src/dropout_layer.c
  ${DARKNET_PATH}/src/gemm.c
  ${DARKNET_PATH}/src/gru_layer.c
  ${DARKNET_PATH}/src/http_stream.cpp
  ${DARKNET_PATH}/src/im2col.c
  ${DARKNET_PATH}/src/image.c
  ${DARKNET_PATH}/src/image_opencv.cpp
  ${DARKNET_PATH}/src/layer.c
  ${DARKNET_PATH}/src/list.c
  ${DARKNET_PATH}/src/local_layer.c
  ${DARKNET_PATH}/src/lstm_layer.c
  ${DARKNET_PATH}/src/matrix.c
  ${DARKNET_PATH}/src/maxpool_layer.c
  ${DARKNET_PATH}/src/network.c
  ${DARKNET_PATH}/src/normalization_layer.c
  ${DARKNET_PATH}/src/option_list.c
  ${DARKNET_PATH}/src/parser.c
  ${DARKNET_PATH}/src/region_layer.c
  ${DARKNET_PATH}/src/reorg_layer.c
  ${DARKNET_PATH}/src/reorg_old_layer.c
  ${DARKNET_PATH}/src/rnn_layer.c
  ${DARKNET_PATH}/src/route_layer.c
  ${DARKNET_PATH}/src/shortcut_layer.c
  ${DARKNET_PATH}/src/softmax_layer.c
  ${DARKNET_PATH}/src/tree.c
  ${DARKNET_PATH}/src/upsample_layer.c
  ${DARKNET_PATH}/src/utils.c
  ${DARKNET_PATH}/src/yolo_layer.c
)

set(DARKNET_KERNELS
  ${DARKNET_PATH}/src/activation_kernels.cu
  ${DARKNET_PATH}/src/avgpool_layer_kernels.cu
  ${DARKNET_PATH}/src/blas_kernels.cu
  ${DARKNET_PATH}/src/col2im_kernels.cu
  ${DARKNET_PATH}/src/convolutional_kernels.cu
  ${DARKNET_PATH}/src/crop_layer_kernels.cu
  ${DARKNET_PATH}/src/deconvolutional_kernels.cu
  ${DARKNET_PATH}/src/dropout_layer_kernels.cu
  ${DARKNET_PATH}/src/im2col_kernels.cu
  ${DARKNET_PATH}/src/maxpool_layer_kernels.cu
  ${DARKNET_PATH}/src/network_kernels.cu
)

include_directories(
  ${DARKNET_PATH}/src
  ${DARKNET_PATH}/include
)

# darknet (use GPU because CPU is far too slow)
set(USE_GPU 0)

if(${USE_GPU})
  message("Compiling for GPU")

  add_definitions(-DGPU=1)

  find_package(CUDA 10 REQUIRED)
  include_directories(
    ${CUDA_INCLUDE_DIRS}
  )

  link_directories(
    "${CUDA_TOOLKIT_ROOT_DIR}/lib64"
  )
 
  cuda_add_library(yolo
    src/darknet_util.cpp
    ${DARKNET_SOURCE}
    ${DARKNET_KERNELS}
  )
 
  target_link_libraries(yolo
    m
    pthread
    stdc++
    cuda
    cudart
    cublas
    curand
    yaml-cpp
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${catkin_LIBRARIES}
  )
else()
  message("Compiling for CPU")
 
  unset(GPU)
 
  add_library(yolo
    src/darknet_util.cpp
    ${DARKNET_SOURCE}
  )
 
  target_link_libraries(yolo
    m
    pthread
    stdc++
    yaml-cpp
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${catkin_LIBRARIES}
  )
endif()

include_directories( ${CMAKE_SOURCE_DIR}/tracker ${OpenCV_INCLUDE_DIRS} )

add_executable( VideoLabeler 
    tracker/kcftracker.cpp
    tracker/fhog.cpp
    src/main.cpp )

target_link_libraries( VideoLabeler yolo ${OpenCV_LIBS} stdc++fs)