#include "../Bencode.h"

int main() {
  Bendata ben_int1{Bendata_init_flag::integer};
  ben_int1.get_data<int>() = 27;
  Bendata ben_int2{Bendata_init_flag::integer};
  ben_int2.get_data<int>() = 70;
  Bendata ben_string1{Bendata_init_flag::string};
  ben_string1.get_data<std::string>() = "This is a string";

  Bendata ben_list1{Bendata_init_flag::list};
  ben_list1.get_data<std::vector<Bendata>>().push_back(ben_int1);
  ben_list1.get_data<std::vector<Bendata>>().push_back(ben_int2);

  Bendata ben_list2{Bendata_init_flag::list};
  ben_list2.get_data<std::vector<Bendata>>().push_back(ben_int1);
  ben_list2.get_data<std::vector<Bendata>>().push_back(ben_int2);
  ben_list2.get_data<std::vector<Bendata>>().push_back(ben_list1);
  ben_list2.get_data<std::vector<Bendata>>().push_back(ben_string1);

  Bendata ben_dict1{Bendata_init_flag::dictionary};
  ben_dict1.get_data<std::map<std::string, Bendata>>()["Flapjack"] =
      Bendata{12};
  ben_dict1.get_data<std::map<std::string, Bendata>>()["Knuckie"] =
      Bendata{"loves flapjack"};
  ben_dict1.get_data<std::map<std::string, Bendata>>()["List"] = ben_list2;

  ben_list2.get_data<std::vector<Bendata>>().push_back(ben_dict1);

  std::cout << ben_list2 << '\n';
}
