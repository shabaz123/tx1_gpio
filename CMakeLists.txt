cmake_minimum_required(VERSION 3.23)
project(gpio_tx1)

set(CMAKE_CXX_STANDARD 14)

add_executable(gpio_tx1 main.cpp gpio.cpp gpio.h)
