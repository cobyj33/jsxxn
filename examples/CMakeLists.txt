add_executable(reading ${JSON_EXAMPLES_DIRECTORY}/reading.cpp)
target_link_libraries(reading json)
target_compile_options(reading PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(pretty ${JSON_EXAMPLES_DIRECTORY}/pretty.cpp)
target_link_libraries(pretty json)
target_compile_options(pretty PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(object_creation ${JSON_EXAMPLES_DIRECTORY}/object_creation.cpp)
target_link_libraries(object_creation json)
target_compile_options(object_creation PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(array_creation ${JSON_EXAMPLES_DIRECTORY}/array_creation.cpp)
target_link_libraries(array_creation json)
target_compile_options(array_creation PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(array_creation_short ${JSON_EXAMPLES_DIRECTORY}/array_creation_short.cpp)
target_link_libraries(array_creation_short json)
target_compile_options(array_creation_short PRIVATE ${JSON_COMPILE_OPTIONS})


add_executable(from_str_literal ${JSON_EXAMPLES_DIRECTORY}/from_str_literal.cpp)
target_link_libraries(from_str_literal json)
target_compile_options(from_str_literal PRIVATE ${JSON_COMPILE_OPTIONS})