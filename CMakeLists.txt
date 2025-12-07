cmake_minimum_required(VERSION 3.15)
project(pixxbind LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(pixxbind_cpp src/pixxbind.cpp)
target_include_directories(pixxbind_cpp PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_features(pixxbind_cpp PRIVATE cxx_std_17)

install(TARGETS pixxbind_cpp
        LIBRARY DESTINATION .
        RUNTIME DESTINATION .
        ARCHIVE DESTINATION .)
