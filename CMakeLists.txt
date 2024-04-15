cmake_minimum_required(VERSION 3.7)

# * Developer comments start with "# >". If specific implementation details
# * don't really matter and you'd just like to simply build jsxxn, just 
# * disregard these.

# * User comments start with a "# *". Both developers and library users should
# * read these comments.

project(jsxxn
  LANGUAGES CXX
  VERSION 0.0.0)

# I'm not sure if this forces any project that includes jsxxn from an
# add_subdirectory call to export compile commands. Will have to experiment
# with this more in the future. 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(JSXXN_COMPILE_FEATURES cxx_std_17)

set(JSXXN_COMPILE_OPTIONS "")
if (((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))) 
  set(JSXXN_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
elseif(MSVC)
  set(JSXXN_COMPILE_OPTIONS /W4 /WX)
endif()

# I decided that setting CMAKE_BUILD_TYPE to Debug automatically wasn't a great
# idea. Possibly other people
OPTION(JSXXN_ENABLE_GPROF "Add -pg to compiler flags on gcc and clang (Default OFF). Note that you should also make sure CMAKE_BUILD_TYPE=Debug" OFF)
OPTION(JSXXN_BUILD_EXAMPLES "Build Examples" OFF)
OPTION(JSXXN_BUILD_TESTS "Build Tests" OFF)

if(JSXXN_ENABLE_GPROF AND ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
  message("[jsxxn] Enabling gprof flags...")

  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
  set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
  set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} -pg)
endif()

set(JSXXN_EXAMPLES_DIRECTORY ${CMAKE_SOURCE_DIR}/examples)
set(JSXXN_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
set(JSXXN_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/include)
set(JSXXN_IMPL_INCLUDE_DIRECTORY ${JSXXN_SRC_DIRECTORY})
set(JSXXN_TEST_DIRECTORY ${CMAKE_SOURCE_DIR}/test)

set(JSXXN_SOURCE_FILES
${JSXXN_SRC_DIRECTORY}/jsxxn.cpp
${JSXXN_SRC_DIRECTORY}/equality.cpp
${JSXXN_SRC_DIRECTORY}/parse.cpp
${JSXXN_SRC_DIRECTORY}/serialize.cpp
${JSXXN_SRC_DIRECTORY}/tokenize.cpp
${JSXXN_SRC_DIRECTORY}/util.cpp)

add_library(jsxxn STATIC ${JSXXN_SOURCE_FILES})
target_include_directories(jsxxn PUBLIC ${JSXXN_INCLUDE_DIRECTORY} PRIVATE ${JSXXN_IMPL_INCLUDE_DIRECTORY})
target_compile_options(jsxxn PUBLIC ${JSXXN_COMPILE_OPTIONS})
target_compile_features(jsxxn PRIVATE ${JSXXN_COMPILE_FEATURES})

if (JSXXN_BUILD_EXAMPLES)
  message("[jsxxn] Entering Examples Directory")
  add_subdirectory(${JSXXN_EXAMPLES_DIRECTORY})
endif()

if (JSXXN_BUILD_TESTS)
  message("[jsxxn] Entering Tests Directory")
  add_subdirectory(${JSXXN_TEST_DIRECTORY})
endif()