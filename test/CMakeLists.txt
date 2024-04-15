add_executable(tester ${JSXXN_TEST_DIRECTORY}/tester.cpp)
target_link_libraries(tester jsxxn)
target_compile_options(tester PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(tester PRIVATE ${JSXXN_COMPILE_FEATURES})

add_executable(benchmarker ${JSXXN_TEST_DIRECTORY}/benchmarker.cpp)
target_link_libraries(benchmarker jsxxn)
target_compile_options(benchmarker PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_include_directories(benchmarker PRIVATE ${JSXXN_IMPL_INCLUDE_DIRECTORY})
target_compile_features(benchmarker PRIVATE ${JSXXN_COMPILE_FEATURES})

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG    v3.0.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)

set(JSXXN_UNITTEST_DIRECTORY ${JSXXN_TEST_DIRECTORY}/unittest)

set(JSXXN_UNITTEST_SOURCE_FILES
${JSXXN_UNITTEST_DIRECTORY}/parsing.cpp
${JSXXN_UNITTEST_DIRECTORY}/serializing.cpp
${JSXXN_UNITTEST_DIRECTORY}/reserialization.cpp
${JSXXN_UNITTEST_DIRECTORY}/reparsing.cpp
${JSXXN_UNITTEST_DIRECTORY}/equality.cpp)

add_executable(unittest ${JSXXN_UNITTEST_SOURCE_FILES})
target_link_libraries(unittest PRIVATE jsxxn Catch2::Catch2WithMain)
target_compile_options(unittest PRIVATE ${JSXXN_COMPILE_OPTIONS})
target_compile_features(unittest PRIVATE ${JSXXN_COMPILE_FEATURES})

