
#ifndef OVERLOAD_HPP
#define OVERLOAD_HPP

// This module provides facilities used in the declaration of and
// resolution of overloaded functions.

namespace steve {

struct Decl;
struct Overload;

bool overload(Overload&, Decl*);

} // namespace steve

#endif

