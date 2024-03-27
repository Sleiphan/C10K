
set(TEST "sum-T")
add_executable(${TEST} sum-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main)
gtest_discover_tests(${TEST})

set(TEST "queue-T")
add_executable(${TEST} queue-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})

set(TEST "workers-T")
add_executable(${TEST} workers-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})

set(TEST "boolset-T")
add_executable(${TEST} boolset-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})

set(TEST "endpoint-T")
add_executable(${TEST} endpoint-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})

set(TEST "http-T")
add_executable(${TEST} http-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})

set(TEST "stack-T")
add_executable(${TEST} stack-T.cpp)
target_link_libraries(${TEST} PRIVATE GTest::gtest_main ${UNIX_LIBRARY})
gtest_discover_tests(${TEST})