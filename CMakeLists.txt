cmake_minimum_required(VERSION 3.7) 
# * I decided that 3.7 was a good place to settle. Since it was released in
# * 2018, it shouldn't be hard to get on your system.

# A great resource for CMake is https://cliutils.gitlab.io/modern-cmake/ if
# you'd like to learn

#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# Dev: Developer comments start with "# D:". If specific implementation details
# Dev: don't really matter and you'd just like to simply build jsxxn, just 
# Dev: disregard these.

# Usr: User comments start with a "# U:". Both developers and library users
# Usr: should read these comments.

# Dev: PROJECT_IS_TOP_LEVEL workaround for pre CMake 3.21
# Dev: Mostly all building options should work completely independent on if
# Dev: jsxxn is not the top-level directory through using target properties
if(NOT DEFINED PROJECT_NAME)
  set(JSXXN_IS_TOP_LEVEL ON)
else()
  set(JSXXN_IS_TOP_LEVEL OFF)
endif()

project(jsxxn
  LANGUAGES CXX
  VERSION 0.0.0)

# Dev: I decided that setting CMAKE_BUILD_TYPE to Debug automatically wasn't a
# Dev: great idea. May be unintuitive though, something to note for the future.
OPTION(JSXXN_ENABLE_GPROF "Add -pg to compiler flags on gcc and clang (Default OFF). Note that you should also make sure CMAKE_BUILD_TYPE=Debug" OFF)
OPTION(JSXXN_BUILD_EXAMPLES "Build Examples" OFF)
OPTION(JSXXN_BUILD_TESTS "Build Tests" OFF)

if (JSXXN_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(JSXXN_COMPILE_FEATURES cxx_std_17)
if (((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))) 
  list(APPEND JSXXN_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
elseif(MSVC)
  list(APPEND JSXXN_COMPILE_OPTIONS /W4 /WX)
endif()


if(JSXXN_ENABLE_GPROF AND ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
  message("[jsxxn] Enabling gprof flags...")
  list(APPEND JSXXN_COMPILE_OPTIONS -pg)

  # Dev: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
  # Dev: "Generate extra code to write profile information suitable for the 
  # Dev: analysis program prof (for -p) or gprof (for -pg). You must use this
  # Dev: option when compiling the source files you want data about, and
  # Dev: **you must also use it when linking**."
  list(APPEND CMAKE_EXE_LINKER_FLAGS "-pg")
  list(APPEND CMAKE_SHARED_LINKER_FLAGS "-pg")
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