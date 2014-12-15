
#ifndef STEVE_MODULE_HPP
#define STEVE_MODULE_HPP

#include <steve/String.hpp>
#include <steve/Location.hpp>
#include <steve/File.hpp>

namespace steve {

struct Expr;
struct Module;

Module* load_file(String);
Module* load_module(Module*, String);
Module* load_module(Location, Module*, String);

} // namespace steve

#endif
