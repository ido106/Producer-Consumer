cmake_minimum_required(VERSION 3.22)
project(ProducerConsumer)

set(CMAKE_CXX_STANDARD 14)

add_executable(ProducerConsumer ex3.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(ProducerConsumer Threads::Threads)