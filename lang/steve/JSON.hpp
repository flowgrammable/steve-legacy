#ifndef JSON_HPP
#define JSON_HPP

#include <map>
#include <vector>
#include <string>

namespace steve {
namespace json {

struct Json {
  virtual std::string stringify() = 0;
  virtual ~Json();
};

struct Number : Json {
  std::string stringify();
  
  double value;
};

struct Object : Json, std::map<std::string, Json *> {
  std::string stringify();
};

struct Array : Json, std::vector<std::string> {
  std::string stringify();
};

struct String : Json {
  std::string stringify();
  
  std::string value;
};

struct Bool : Json {
  std::string stringify();
  
  bool value;
};

} // namespace json
} // namespace steve


#endif