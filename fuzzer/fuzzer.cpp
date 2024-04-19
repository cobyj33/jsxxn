#include "jsxxn.h"
#include <stdexcept>
#include <iostream>

// fuzz_target.cc

/**
 * Technically, llvm states "[A fuzzer target] must tolerate any kind of input
 * (empty, huge, malformed, etc).", which I assume means the code won't 
 * give a actual runtime error rather than a thrown exception. 
 * 
 * With sanitizers enabled, the fuzzer especially still helps to detect
 * memory errors, undefined accesses, signed integer overflows,
 * null pointer dereferences, etc... Especially with edge cases in tokenizing
 * and string manipulation
 * 
 * For example, on https://llvm.org/docs/LibFuzzer.html#rejecting-unwanted-inputs,
 * we can see a code snippet that basically is a C version of what is below. 
 * The difference between an error code and an exception in this case is
 * negligible in my opinion
*/

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string_view view(reinterpret_cast<const char*>(Data), Size);
  try {
    jsxxn::JSON json = jsxxn::parse(view);
    std::string prettified = jsxxn::prettify(json);
    std::string stringified = jsxxn::stringify(json);
    jsxxn::JSON pprettified = jsxxn::parse(prettified);
    jsxxn::JSON pstringified = jsxxn::parse(stringified);
    return 0;  // Values other than 0 and -1 are reserved for future use.
  } catch (const std::runtime_error& err) {
    return -1;
  }
}