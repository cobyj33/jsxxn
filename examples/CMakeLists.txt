add_executable(reading ${JSXXN_EXAMPLES_DIRECTORY}/reading.cpp)
target_link_libraries(reading jsxxn)
target_compile_options(reading PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(reading PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(prettify ${JSXXN_EXAMPLES_DIRECTORY}/prettify.cpp ${JSXXN_EXAMPLES_DIRECTORY}/printer.cpp)
target_link_libraries(prettify jsxxn)
target_include_directories(prettify PRIVATE ${JSXXN_EXAMPLES_DIRECTORY})
target_compile_options(prettify PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(prettify PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(stringify ${JSXXN_EXAMPLES_DIRECTORY}/stringify.cpp ${JSXXN_EXAMPLES_DIRECTORY}/printer.cpp)
target_link_libraries(stringify jsxxn)
target_include_directories(stringify PRIVATE ${JSXXN_EXAMPLES_DIRECTORY})
target_compile_options(stringify PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(stringify PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(object_creation ${JSXXN_EXAMPLES_DIRECTORY}/object_creation.cpp)
target_link_libraries(object_creation jsxxn)
target_compile_options(object_creation PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(object_creation PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(array_creation ${JSXXN_EXAMPLES_DIRECTORY}/array_creation.cpp)
target_link_libraries(array_creation jsxxn)
target_compile_options(array_creation PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(array_creation PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(array_creation_short ${JSXXN_EXAMPLES_DIRECTORY}/array_creation_short.cpp)
target_link_libraries(array_creation_short jsxxn)
target_compile_options(array_creation_short PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(array_creation_short PRIVATE ${JSXXN_COMPILE_FEATURES})


add_executable(from_str_literal ${JSXXN_EXAMPLES_DIRECTORY}/from_str_literal.cpp)
target_link_libraries(from_str_literal jsxxn)
target_compile_options(from_str_literal PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(from_str_literal PRIVATE ${JSXXN_COMPILE_FEATURES})
