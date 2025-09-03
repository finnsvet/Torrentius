#include "../Bencoder/Bencode.h"
#include <cassert>
#include <ios>

int main() {
  Bendata parsed;
  std::ifstream file{"project3.torrent"};
  file >> std::noskipws;
  assert(file);
  if (!get_bendata_from_stream(file, parsed)) {
    std::cerr << "error in parsing, somethin wrong bobbie";
    return -1;
  }
  std::cout << parsed << "\n";
}
