#include "file_parsing_errors.h"
#include <iostream>

int error_with_reason(const char *reason) {
  std::cout << reason << '\n';
  return -1;
}
