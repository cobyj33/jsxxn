#include "json.h"

typedef std::string JSONPrinterFunc(const json::JSONValue& value);
int printer_main(int argc, char** argv, JSONPrinterFunc printer);