
#ifndef STEVE_VARIANT_HPP
#define STEVE_VARIANT_HPP

// This module defines a number of elaboration and resolution
// facilities for variant types in the Steve Programming Language.

namespace steve {

struct Expr;
struct Type;

Type* instantiate_variant(Type*, Expr*);

} // namespace steve

#endif
