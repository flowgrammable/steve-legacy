
#ifndef STEVE_INTRINSIC_HPP
#define STEVE_INTRINSIC_HPP

#include <steve/Ast.hpp>

// The intrinsic module houses the implementations of all steve
// functions built into the language. This includes the definition
// of all builtin type constructors (e.g., bitfield, array, etc),
// and all builtin operators (addition of nats, ints, etc).

namespace steve {

struct Decl;

Decl* get_bitfield();


} // namespace steve

#endif
