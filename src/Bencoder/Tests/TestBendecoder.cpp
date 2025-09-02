#include "../Bencode.h"
#include <cassert>

int main() {
  Bendata parsed;
  std::ifstream file{"test.ben"};
  assert(file);
  if (!get_bendata_from_stream(file, parsed)) {
    std::cerr << "error in parsing, somethin wrong bobbie";
    return -1;
  }
  std::cout << parsed << "\n";
}
