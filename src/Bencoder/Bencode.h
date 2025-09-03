#ifndef BENCODE
#define BENCODE

// Bencode will be the library that provides facilities to interact
// with/represent Bee-data From a metainfo File to actual objects we can
// manipulate in code
//
// Classes:
//    Bendata -> An abstraction fo an actual bee-data instance
//               can be of underlying type integer, string, dictionary or list,
//               Each instance corresponds to a single constant underlying type,
//               E.G: once an integer alway an inetger.
// Functions:
//    Bendecode_integer -> overload 2
//        1) One for reading data directly from some file stream and converting
//        it to bee-data 2) One for converting some string to its appropriat
//        bee-data represenation
//    Bendecode_string -> overload 2 (same motive as previous function but for
//    strings) Bendecode_dictionary -> overload 2 (same motive as previous
//    function but for dictionaries) Bendecode_list -> overload 2 (same motive
//    as previous function but for lists)

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

enum class Bendata_init_flag { integer, string, dictionary, list };
using ben_t = char;
constexpr ben_t BEN_DIC_T = 'd';
constexpr ben_t BEN_LIS_T = 'l';
constexpr ben_t BEN_NUM_T = 'n';
constexpr ben_t BEN_STR_T = 's';
constexpr ben_t BEN_DELIMETER = 'e';

class Bendata {
private:
  std::variant<int, std::string, std::vector<Bendata>,
               std::map<std::string, Bendata>>
      actual_value;
  ben_t _t;

public:
  Bendata() = default;
  Bendata(int number);
  Bendata(std::string string);
  explicit Bendata(Bendata_init_flag flag);

  template <typename T> T &get_data();
  template <typename T> const T &get_data() const;
  ben_t get_t() const;

  friend std::ostream &operator<<(std::ostream &os, const Bendata &ben_object);
};

template <typename T> T &Bendata::get_data() {
  T &value = std::get<T>(actual_value);
  return value;
}

template <typename T> const T &Bendata::get_data() const {
  const T &value = std::get<T>(actual_value);
  return value;
}

bool bendecode_integer(std::ifstream &, Bendata &);
bool bendecode_string(std::ifstream &, Bendata &);
bool bendecode_dictionary(std::ifstream &, Bendata &);
bool bendecode_list(std::ifstream &, Bendata &);
bool get_benkey_from_stream(std::ifstream &, std::string &);
bool get_bendata_from_stream(std::ifstream &, Bendata &);

namespace ben {
using str = std::string;
using num = int;
using lis = std::vector<Bendata>;
using dic = std::map<str, Bendata>;
} // namespace ben

#endif
