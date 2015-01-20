#include <steve/Cli.hpp>
#include <steve/Config.hpp>
#include <steve/Ast.hpp>
#include <steve/Error.hpp>
#include <steve/Extract.hpp>
#include <steve/Module.hpp>
#include <steve/String.hpp>

#include <cctype>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <string>

#include "JSON.hpp"



namespace steve {
namespace json {

std::string
String::stringify() {
  return value;
}

std::string
Number::stringify() {
  return std::to_string(value);
}

std::string
Bool::stringify() {
  return value ? "true" : "false";
}

std::string
Array::stringify() {
  std::string s = "[";
  
  for(size_t i = 0; i < size(); i++) {
    if(i != 0)
      s += "," + at(i);
    else
      s += at(i);
  }
  
  return s + "]";
}


} // namespace json
} // namespace steve