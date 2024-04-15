#include "jsxxn.h"

// see comment in printer.cpp

typedef std::string JSONPrinterFunc(const jsxxn::JSONValue& value);
int printer_main(int argc, char** argv, JSONPrinterFunc printer);