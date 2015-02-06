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
  Number(double);
  std::string stringify();
  
  double value;
};

struct Object : Json, std::map<std::string, Json *> {
  Object();
  std::string stringify();
  
  ~Object();
};

struct Array : Json, std::vector<Json *> {
  std::string stringify();
  
  ~Array();
};

struct String : Json, std::string {
  String(std::string);
  std::string stringify();  
};

struct Bool : Json {
  Bool(bool);
  std::string stringify();
  
  bool value;
};

} // namespace json
} // namespace steve


#endif