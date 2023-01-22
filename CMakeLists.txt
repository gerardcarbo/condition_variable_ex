cmake_minimum_required(VERSION 3.0.0)
project(condition_variable_ex VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(CTest)
enable_testing()

add_executable(condition_variable_ex main.cpp)

include_directories(include)

enable_testing()

# define tests
#[[
add_test(
    NAME test_timeout
    COMMAND condition_variable_ex 1 100 0 100 150 200
)
set_tests_properties(test_timeout PROPERTIES TIMEOUT 30)     
    
add_test(
    NAME test_signaled
    COMMAND condition_variable_ex 1 100 1 200 100 150
)
set_tests_properties(test_signaled PROPERTIES TIMEOUT 30)  
    
add_test(
    NAME test_predicate
    COMMAND condition_variable_ex 1 100 2 200 150 100
)
set_tests_properties(test_predicate PROPERTIES TIMEOUT 30)    
]]

add_test(
    NAME test_bgnd_thread_0
    COMMAND condition_variable_ex 2 5000 0 
)
set_tests_properties(test_bgnd_thread_0 PROPERTIES TIMEOUT 200)

add_test(
    NAME test_bgnd_thread_01
    COMMAND condition_variable_ex 2 5000 10 
)
set_tests_properties(test_bgnd_thread_01 PROPERTIES TIMEOUT 2000)  

add_test(
    NAME test_bgnd_thread_001
    COMMAND condition_variable_ex 2 5000 50 
)
set_tests_properties(test_bgnd_thread_001 PROPERTIES TIMEOUT 2000)   

add_test(
    NAME test_bgnd_thread_1
    COMMAND condition_variable_ex 2 5000 100 
)
set_tests_properties(test_bgnd_thread_1 PROPERTIES TIMEOUT 2000)   

add_test(
    NAME test_bgnd_thread_2
    COMMAND condition_variable_ex 2 5000 200 
)
set_tests_properties(test_bgnd_thread_2 PROPERTIES TIMEOUT 2000) 

add_test(
    NAME test_bgnd_thread_3
    COMMAND condition_variable_ex 2 5000 500 
)
set_tests_properties(test_bgnd_thread_3 PROPERTIES TIMEOUT 2000)  
