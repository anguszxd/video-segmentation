#####################################
cmake_minimum_required (VERSION 2.8)
set(CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}")
project (graphcut)

find_package( OpenCV REQUIRED )
include_directories(GCv2p3)

link_libraries( graphcut /usr/local/lib)

add_executable (graphcut grabcut.cpp)

target_link_libraries( graphcut ${OpenCV_LIBS})
