cmake_minimum_required(VERSION 3.0.0)
project(condition_variable_ex VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(condition_variable_ex main.cpp)

include_directories(include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)