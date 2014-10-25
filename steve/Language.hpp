
#ifndef STEVE_LANGUAGE_HPP
#define STEVE_LANGUAGE_HPP

#include <steve/Token.hpp>
#include <steve/Ast.hpp>

namespace steve {

// The language class provides a global initialization of resources
// for steve-related programs (compiler, analyzers, etc). In particular,
// it allocates a number of internal types and facilities used by the
// various routines the steve core.
struct Language {
  Language();
  ~Language();
};

} // namespace steve

#endif
