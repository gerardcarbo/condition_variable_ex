cmake_minimum_required(VERSION 3.0.0)
project(condition_variable_ex VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(condition_variable_ex main.cpp)

include_directories(include)

enable_testing()

# define tests
add_test(
    NAME test_timeout
    COMMAND condition_variable_ex 0 100 110 120
    )
set_tests_properties(test_timeout PROPERTIES TIMEOUT 30)     

add_test(
    NAME test_signaled
    COMMAND condition_variable_ex 1 120 100 110
    )
set_tests_properties(test_signaled PROPERTIES TIMEOUT 30)  

add_test(
    NAME test_predicate
    COMMAND condition_variable_ex 2 205 210 200
    )
set_tests_properties(test_predicate PROPERTIES TIMEOUT 30)     

