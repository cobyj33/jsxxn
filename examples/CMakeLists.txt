add_executable(reading ${JSON_EXAMPLES_DIRECTORY}/reading.cpp)
target_link_libraries(reading json)
target_compile_options(reading PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(parser ${JSON_EXAMPLES_DIRECTORY}/parser.cpp)
target_link_libraries(parser json)
target_compile_options(reading PRIVATE ${JSON_COMPILE_OPTIONS})