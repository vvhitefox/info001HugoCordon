cmake_minimum_required(VERSION 3.5)

project(video LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(video main.cpp)

install(TARGETS video
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
