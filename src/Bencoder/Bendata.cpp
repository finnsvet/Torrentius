#include "Bencode.h"

Bendata::Bendata(int number) : actual_value{number}, _t{BEN_NUM_T} {}

Bendata::Bendata(std::string string) : actual_value{string}, _t{BEN_STR_T} {}

Bendata::Bendata(Bendata_init_flag flag) {
  switch (flag) {
  case Bendata_init_flag::integer:
    actual_value.emplace<int>();
    _t = BEN_NUM_T;
    break;
  case Bendata_init_flag::string:
    actual_value.emplace<std::string>();
    _t = BEN_STR_T;
    break;
  case Bendata_init_flag::dictionary:
    actual_value.emplace<std::map<std::string, Bendata>>();
    _t = BEN_DIC_T;
    break;
  case Bendata_init_flag::list:
    actual_value.emplace<std::vector<Bendata>>();
    _t = BEN_LIS_T;
    break;
  }
}

template <typename T> T &Bendata::get_data() {
  T &value = std::get<T>(actual_value);
  return value;
}

template <typename T> const T &Bendata::get_data() const {
  const T &value = std::get<T>(actual_value);
  return value;
}

ben_t Bendata::get_t() const { return _t; }

std::ostream &operator<<(std::ostream &os, const Bendata &ben_object) {
  switch (ben_object._t) {
  case BEN_STR_T: {
    os << '\"' << std::get<std::string>(ben_object.actual_value) << '\"';
    break;
  }
  case BEN_NUM_T: {
    os << std::get<int>(ben_object.actual_value);
    break;
  }
  case BEN_LIS_T: {
    const std::vector<Bendata> &list =
        ben_object.get_data<std::vector<Bendata>>();
    os << '[';
    for (int i = 0; i < list.size(); ++i) {
      if (i == list.size() - 1)
        os << list[i];
      else
        os << list[i] << ", ";
    }
    os << ']';
    break;
  }
  case BEN_DIC_T: {
    const std::map<std::string, Bendata> &dicts =
        ben_object.get_data<std::map<std::string, Bendata>>();
    std::size_t d_size = dicts.size(), index = 0;
    os << '{';
    for (const auto &x : dicts) {
      ++index;
      if (index != d_size)
        os << x.first << " : " << x.second << ", ";
      else
        os << x.first << " : " << x.second;
    }
    os << '}';
  }
  }
  return os;
}
