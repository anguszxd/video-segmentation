#####################################
cmake_minimum_required (VERSION 2.8)
set(CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}")
project (main)

find_package( OpenCV REQUIRED )

link_libraries( main /usr/local/lib)

add_executable (main example.cpp)

target_link_libraries( main ${OpenCV_LIBS})
