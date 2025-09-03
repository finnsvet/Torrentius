#include "../Errorhandlers/file_parsing_errors.h"
#include "Bencode.h"
#include <cctype>
#include <cwctype>

bool noexcept_stoi(std::string str, int &result) {
  try {
    result = std::stoi(str);
  } catch (...) {
    return error_with_reason("noexcept_stoi failed");
  }
  return true;
}

bool bendecode_string(std::ifstream &of, Bendata &data) {
  char c;
  std::string size;
  while (of >> c && c != ':')
    size += c;
  int size_int;
  if (!noexcept_stoi(size, size_int))
    return false;
  ben::str result;
  while (size_int-- > 0) {
    of >> c;
    result += c;
  }
  Bendata temp_str(result);
  data = std::move(temp_str);
  return true;
}

// Reads an expected bencoder delimeter
// then discards it
inline void read_delimeter(std::ifstream &of) {
  char c;
  of >> c;
}

bool bendecode_integer(std::ifstream &of, Bendata &data) {

  read_delimeter(of);
  std::string number_str;
  char c;
  while (of >> c && c != BEN_DELIMETER)
    number_str += c;
  int value;
  if (!noexcept_stoi(number_str, value))
    return false;
  Bendata temp_int(value);
  data = std::move(temp_int);
  return true;
}

// peek_skipws returns-> ifstream::peek()
//    peeks into stream and if a whitsepace is there
//    it reads "skips" it by reading it and discarding
//    its value.
auto peek_skipws(std::ifstream &of) {
  while (std::iswspace(of.peek()))
    of.get(); // read that whitespace
  return of.peek();
}

bool get_bendata_from_stream(std::ifstream &of, Bendata &data) {
  switch (peek_skipws(of)) {
  case BEN_NUM_T:
    if (!bendecode_integer(of, data))
      return error_with_reason("bendecode_integer failed");
    break;
  case BEN_DIC_T:
    if (!bendecode_dictionary(of, data))
      return error_with_reason("bendecode_dictionary failed");
    break;
  case BEN_LIS_T:
    if (!bendecode_list(of, data))
      return error_with_reason("bendecode_list failed");
    break;
  default:
    if (!bendecode_string(of, data))
      return error_with_reason("bendecode_string failed");
    break;
  }
  return true;
}

bool bendecode_list(std::ifstream &of, Bendata &data) {
  Bendata new_list{Bendata_init_flag::list};
  ben::lis &ref_list = new_list.get_data<ben::lis>();
  read_delimeter(of);
  while (peek_skipws(of) != BEN_DELIMETER) {
    Bendata new_data{};
    if (!get_bendata_from_stream(of, new_data))
      return false;
    ref_list.push_back(new_data);
  }
  read_delimeter(of);
  data = std::move(new_list);
  return true;
}

bool get_benkey_from_stream(std::ifstream &of, std::string &key) {
  Bendata data{};
  switch (peek_skipws(of)) {
  // case 'i':
  //   if (!bendecode_integer(of, data))
  //     return error_with_reason("get_benkey failed: integer");
  //   key = data.get_data<ben::num>();
  //   break;
  default:
    if (!bendecode_string(of, data))
      return error_with_reason("get_benkey failed: string");
    key = data.get_data<ben::str>();
    break;
  }
  return true;
}

bool bendecode_dictionary(std::ifstream &of, Bendata &data) {
  Bendata new_dict{Bendata_init_flag::dictionary};
  ben::dic &ref_dic = new_dict.get_data<ben::dic>();
  read_delimeter(of);
  while (peek_skipws(of) != BEN_DELIMETER) {
    ben::str key;
    Bendata new_data{};
    if (!get_benkey_from_stream(of, key))
      return false;
    if (!get_bendata_from_stream(of, new_data))
      return false;
    ref_dic[key] = new_data;
  }
  read_delimeter(of);
  data = std::move(new_dict);
  return true;
}
