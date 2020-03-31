cmake_minimum_required(VERSION 3.16)

project(DEV4_Project)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_executable (DEV4 main.cpp)