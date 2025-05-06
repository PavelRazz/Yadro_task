cmake_minimum_required(VERSION 3.10)
project(task)

# Используем стандарт C++17 или C++20 (по необходимости)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Оптимизации и предупреждения (опционально)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# Добавляем исполняемый файл из main.cpp
add_executable(task main.cpp)