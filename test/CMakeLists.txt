add_executable(tester ${JSON_TEST_DIRECTORY}/tester.cpp)
target_link_libraries(tester json)
target_compile_options(tester PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(benchmarker ${JSON_TEST_DIRECTORY}/benchmarker.cpp)
target_link_libraries(benchmarker json)
target_compile_options(benchmarker PRIVATE ${JSON_COMPILE_OPTIONS})