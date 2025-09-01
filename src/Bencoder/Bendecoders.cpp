#include "Bencode.h"

bool noexcept_stoi(std::string str, int &result) {
  try {
    result = std::stoi(str);
  } catch (...) {
    return false;
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
  Bendata temp_str{Bendata_init_flag::string};
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
  if (noexcept_stoi(number_str, value))
    return false;
  Bendata temp_int{Bendata_init_flag::integer};
  data = std::move(temp_int);
  return true;
}

bool get_bendata_from_stream(std::ifstream &of, Bendata &data) {
  switch (of.peek()) {
  case 'i':
    if (!bendecode_integer(of, data))
      return false;
    break;
  case 'd':
    if (!bendecode_dictionary(of, data))
      return false;
    break;
  case 'l':
    if (!bendecode_list(of, data))
      return false;
    break;
  default:
    if (!bendecode_string(of, data))
      return false;
    break;
  }
  return true;
}

bool bendecode_list(std::ifstream &of, Bendata &data) {
  Bendata new_list{Bendata_init_flag::list};
  ben::lis &ref_list = new_list.get_data<ben::lis>();
  char c;
  of >> c;
  if (c != 'l')
    return false;
  while ((c = of.peek()) != 'e') {
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
  switch (of.peek()) {
  case 'i':
    if (!bendecode_integer(of, data))
      return false;
    key = data.get_data<ben::num>();
    break;
  default:
    if (!bendecode_string(of, data))
      return false;
    key = data.get_data<ben::str>();
    break;
  }
  return false;
}

bool bendecode_dictionary(std::ifstream &of, Bendata &data) {
  Bendata new_dict{Bendata_init_flag::dictionary};
  ben::dic &ref_dic = new_dict.get_data<ben::dic>();
  char c;
  of >> c;
  while ((c = of.peek()) != 'e') {
    ben::str key;
    Bendata new_data{};
    if (!get_benkey_from_stream(of, key))
      return false;
    if (!get_bendata_from_stream(of, new_data))
      return false;
    ref_dic[key] = new_data;
  }
  of >> c;
  data = std::move(new_dict);
  return true;
}
