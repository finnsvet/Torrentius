#ifndef TORRENT_FILE
#define TORRENT_FILE

#include "../Bencoder/Bencode.h"
#include <filesystem>

class TorrentFile {
private:
  const ben::dic transcibe;

public:
  TorrentFile(const std::string_view filename);
  TorrentFile(const std::filesystem::path pathname);
  TorrentFile() = delete;
  std::string get_tracker_url() const;
  std::vector<std::string> get_trackers_if_any() const;
  std::map<std::string, Bendata> get_info_key() const;
  std::string get_torrent_name() const;
  long get_piece_length() const;
  std::map<long, std::string> get_piece_hash_dict() const;
  std::string get_piece_hash(long &index) const;
  std::string get_piece_encoding_if_any() const;
  bool torrent_is_file() const;
};

#endif
