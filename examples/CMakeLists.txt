add_executable(reading ${JSON_EXAMPLES_DIRECTORY}/reading.cpp)
target_link_libraries(reading json)
target_compile_options(reading PRIVATE ${JSON_COMPILE_OPTIONS})