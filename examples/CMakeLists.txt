add_executable(reading ${JSON_EXAMPLES_DIRECTORY}/reading.cpp)
target_link_libraries(reading json)
target_compile_options(reading PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(pretty ${JSON_EXAMPLES_DIRECTORY}/pretty.cpp)
target_link_libraries(pretty json)
target_compile_options(pretty PRIVATE ${JSON_COMPILE_OPTIONS})