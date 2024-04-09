add_executable(tester ${JSON_TEST_DIRECTORY}/tester.cpp)
target_link_libraries(tester json)
target_compile_options(tester PRIVATE ${JSON_COMPILE_OPTIONS})

add_executable(benchmarker ${JSON_TEST_DIRECTORY}/benchmarker.cpp)
target_link_libraries(benchmarker json)
target_compile_options(benchmarker PRIVATE ${JSON_COMPILE_OPTIONS})
target_include_directories(benchmarker PRIVATE ${JSON_IMPL_INCLUDE_DIRECTORY})

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG    v3.0.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)

set(JSON_UNITTEST_DIRECTORY ${JSON_TEST_DIRECTORY}/unittest)

set(JSON_UNITTEST_SOURCE_FILES
${JSON_UNITTEST_DIRECTORY}/parsing.cpp
${JSON_UNITTEST_DIRECTORY}/serializing.cpp
${JSON_UNITTEST_DIRECTORY}/reserialization.cpp
${JSON_UNITTEST_DIRECTORY}/reparsing.cpp
${JSON_UNITTEST_DIRECTORY}/equality.cpp)

add_executable(unittest ${JSON_UNITTEST_SOURCE_FILES})
target_link_libraries(unittest PRIVATE json Catch2::Catch2WithMain)
target_compile_options(unittest PRIVATE ${JSON_COMPILE_OPTIONS})

