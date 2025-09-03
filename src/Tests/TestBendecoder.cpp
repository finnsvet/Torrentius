#include "../Bencoder/Bencode.h"
#include <cassert>
#include <ios>

int main() {
  Bendata parsed;
  std::ifstream file{"regular-show-complete.torrent"};
  file >> std::noskipws;
  assert(file);
  if (!get_bendata_from_stream(file, parsed)) {
    std::cerr << "error in parsing, somethin wrong bobbie\n";
    return -1;
  }
  std::cout << parsed << "\n";
}
