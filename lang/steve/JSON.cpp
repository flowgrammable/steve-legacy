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
#include "JSON.ipp"



namespace steve {
namespace json {

std::string
String::stringify() {
  return std::string(*this);
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
      s += ", " + at(i)->stringify();
    else
      s += at(i)->stringify();
  }
  
  return s + "]";
}

Array::~Array() {
  for(auto i : *this)
    delete i;
}

std::string
Object::stringify() {
  std::string s = "{";
  int count = 0;
  
  for(auto i : *this) {
    if(count++ != 0)
      s += ", " + i.first + ": " + i.second->stringify();
    else
      s += i.first + ": " + i.second->stringify();
  }
  
  return s + "}";
}

Object::~Object() {
  for(auto i : *this) {
    delete i.second;
  }
}



} // namespace json
} // namespace steve