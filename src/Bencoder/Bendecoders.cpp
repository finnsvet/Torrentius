#include "../Errorhandlers/file_parsing_errors.h"
#include "Bencode.h"
#include <cctype>
#include <cwctype>

bool noexcept_stoi(std::string str, int &result) {
  try {
    result = std::stoi(str);
  } catch (...) {
    return error_with_reason("Bro Stoi no except failed");
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
    return error_with_reason("Bro Bendecode_string failed");

  ben::str result;
  while (size_int-- > 0) {
    of >> c;
    result += c;
  }
  Bendata temp_str(result);
  data = std::move(temp_str);
  return true;
}

bool bendecode_integer(std::ifstream &of, Bendata &data) {
  char c;
  of >> c;
  std::string number_str;
  while (of >> c && c != 'e')
    number_str += c;
  int value;
  if (!noexcept_stoi(number_str, value))
    return error_with_reason("bro Bendecode_integer failed");
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
  auto error = [] {
    return error_with_reason("Bro Bendata_from_stream failed");
  };
  switch (peek_skipws(of)) {
  case 'i':
    if (!bendecode_integer(of, data))
      return error();
    break;
  case 'd':
    if (!bendecode_dictionary(of, data))
      return error();
    break;
  case 'l':
    if (!bendecode_list(of, data))
      return error();
    break;
  default:
    if (!bendecode_string(of, data))
      return error();
    break;
  }
  return true;
}

bool bendecode_list(std::ifstream &of, Bendata &data) {
  Bendata new_list{Bendata_init_flag::list};
  ben::lis &ref_list = new_list.get_data<ben::lis>();
  if (peek_skipws(of) != 'l')
    return error_with_reason("bro bendecode_list failed");
  char c;
  of >> c;
  while ((c = peek_skipws(of)) != 'e') {
    Bendata new_data{};
    if (!get_bendata_from_stream(of, new_data))
      return false;
    ref_list.push_back(new_data);
  }
  of >> c;
  data = std::move(new_list);
  return true;
}

bool get_benkey_from_stream(std::ifstream &of, std::string &key) {
  Bendata data{};
  switch (peek_skipws(of)) {
  case 'i':
    if (!bendecode_integer(of, data))
      return error_with_reason("Bro getbenkey from stream failed");
    key = data.get_data<ben::num>();
    break;
  default:
    if (!bendecode_string(of, data))
      return error_with_reason("Bro getbenkey from stream failed");
    key = data.get_data<ben::str>();
    break;
  }
  return true;
}

bool bendecode_dictionary(std::ifstream &of, Bendata &data) {
  Bendata new_dict{Bendata_init_flag::dictionary};
  ben::dic &ref_dic = new_dict.get_data<ben::dic>();
  peek_skipws(of);
  char c;
  of >> c;
  while ((c = peek_skipws(of)) != 'e') {
    ben::str key;
    Bendata new_data{};
    if (!get_benkey_from_stream(of, key))
      return error_with_reason("Bro bendecode key dictionary failed");
    if (!get_bendata_from_stream(of, new_data))
      return error_with_reason("Bro bendecode data dictionary failed");
    ref_dic[key] = new_data;
  }
  of >> c;
  data = std::move(new_dict);
  return true;
}
