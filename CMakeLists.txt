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
    COMMAND condition_variable_ex 0 1000 2000 1500
    )
set_tests_properties(test_timeout PROPERTIES TIMEOUT 30)     

add_test(
    NAME test_signaled
    COMMAND condition_variable_ex 1 1000 500 2000
    )
set_tests_properties(test_signaled PROPERTIES TIMEOUT 30)  

add_test(
    NAME test_predicate
    COMMAND condition_variable_ex 2 1000 500 200
    )
set_tests_properties(test_predicate PROPERTIES TIMEOUT 30)     

#[[
# create the testing file and list of tests
create_test_sourcelist (Tests
  TimeoutTest.cpp
  SignaledTest.cpp
  otherArrays.cxx
  otherEmptyCell.cxx
  TestSmartPointer.cxx
  SystemInformation.cxx
  ...
  )

# add the executable
add_executable (CommonCxxTests ${Tests})

# remove the test driver source file
set (TestsToRun ${Tests})
#remove (TestsToRun CommonCxxTests.cxx)

# Add all the ADD_TEST for each test
foreach (test ${TestsToRun})
  get_filename_component (TName ${test} NAME_WE)
  add_test (NAME ${TName} COMMAND CommonCxxTests ${TName})
endforeach ()
]]

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)


