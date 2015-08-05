#####################################
cmake_minimum_required (VERSION 2.8)
set(CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}")
project (seg)

find_package( OpenCV REQUIRED )

link_libraries( seg /usr/local/lib)

add_executable (seg train_HOG.cpp)

target_link_libraries( seg ${OpenCV_LIBS})
