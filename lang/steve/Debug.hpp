
#ifndef STEVE_DEBUG_HPP
#define STEVE_DEBUG_HPP

// This module contains general purpose debugging facilities. Note that
// debugging support for printing tokens and terms is defined along-side
// those entities.
//
// TODO: Build an exception-based diagnostic facility for reporting
// semantic errors.

#include <steve/Format.hpp>

#include <cassert>
#include <stdexcept>

// Check the given condition. If it it not satisfied, trigger an
// assertion with the given message. Note that assertions are
// removed when compiling without debugging informaiton.
#ifndef NDEBUG
#  define steve_assert(cond, msg) \
  if (not (cond)) \
    throw steve::Assertion_error(steve::format("{}:{}: {}", __FILE__, __LINE__, msg));
#else
#  define steve_assert(cond, msg)
#endif

// Throw an exception with the given error. Note that this is
// defined in every compilation mode.
#define steve_unreachable(msg) \
  throw steve::Assertion_error(steve::format("{}:{}: {}", __FILE__, __LINE__, msg));

namespace steve {

// An assertion is intended to be an uncatchable error thrown by the
// steve compiler infrastructure due to the violation of some precondition 
// or invariant. Assertions are most frequently used to guard various
// tree construction algorithms, preventing users from creating
// totally nonsensical programs.
struct Assertion_error : std::runtime_error {
  Assertion_error(const char*);
  Assertion_error(const std::string&);
};

std::string demangle(const char*);

} // namespace steve

#include "Debug.ipp"

#endif
