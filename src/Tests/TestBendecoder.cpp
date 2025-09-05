#include "../Bencoder/Bencode.h"
#include <cassert>

int main() {
  std::ifstream file{"Tests/regular-show-complete.torrent"};
  assert(file);
  Bendata parsed = bendecode_from_file(file);
  std::cout << parsed.get_encode() << "\n";
}
