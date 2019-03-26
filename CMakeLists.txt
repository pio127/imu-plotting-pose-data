
cmake_minimum_required(VERSION 3.7)
project (plotting_TCP)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(RTIMULib)

find_package(PythonLibs 2.7 REQUIRED)
find_package(Threads REQUIRED)


include_directories(include ${PYTHON_INCLUDE_DIRS} header_only_libs)

add_executable(server server.cpp)
add_executable(clientPlot clientPlot.cpp)

target_link_libraries(server ${PYTHON_LIBRARIES} RTIMULib ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(clientPlot ${PYTHON_LIBRARIES} )
