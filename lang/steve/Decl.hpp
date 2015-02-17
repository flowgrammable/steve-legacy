
#ifndef STEVE_DECL_HPP
#define STEVE_DECL_HPP

#include <steve/Ast.hpp>

namespace steve {

struct Name;
struct Decl;
struct Fn;

Decl* context(Decl*);
Name* name(Decl*);

Fn* get_declared_fn(Decl*);

} /// namespace steve

#include "Decl.ipp"

#endif
