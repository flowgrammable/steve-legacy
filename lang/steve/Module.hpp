
#ifndef STEVE_MODULE_HPP
#define STEVE_MODULE_HPP

#include <steve/String.hpp>
#include <steve/Location.hpp>
#include <steve/File.hpp>

namespace steve {

struct Expr;
struct Module;

Module* load_file(const Path&);
Module* load_module(Module*, String);
Module* load_module(Location, Module*, String);

Expr* load_name(const std::string&);

} // namespace steve

#endif
