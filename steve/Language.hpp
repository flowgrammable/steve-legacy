
#ifndef STEVE_LANGUAGE_HPP
#define STEVE_LANGUAGE_HPP

#include <steve/Scope.hpp>

namespace steve {

// The language class provides a global initialization of resources
// for steve-related programs (compiler, analyzers, etc). In particular,
// it allocates a number of internal types and facilities used by the
// various routines the steve core.
//
// Note that the laenguage object pushes a base scope into which
// builtin-declarations are added.
struct Language : Scope_guard {
  Language();
  ~Language();
};

} // namespace steve

#endif
