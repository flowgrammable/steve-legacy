
#ifndef STEVE_ERROR_HPP
#define STEVE_ERROR_HPP

#include <steve/Memory.hpp>
#include <steve/Location.hpp>
#include <steve/Debug.hpp>

#include <cerrno>
#include <iosfwd>
#include <vector>
#include <system_error>

// This module provides facilities for recording and printing compiler
// diagnostics.
//
// TODO: There are lots and lots of interesting things that we can
// do with diagnostics. For example:
//
//    - implement sub-diagnostics (notes pertaining to errors)
//    - map the diagnostic back to the original source code
//    - color the output
//    - print programs as trees
//    - provide structured output (XML/Json) to be consumed by other tools
//    - define the context (i.e., stack) in which an error occurs
//    - related to the above, show the root cause for type errors
//    - define a streaming interface for emitting diagnostics

namespace steve {

class String;
class Integer;
class Location;
struct debug_node;

// An error code.
using Error_code = std::error_code;

// -------------------------------------------------------------------------- //
// Expected value

// The Expected class represents a value that may be returned from 
// a system interface. In the presence of system errors, returned values 
// may not be produced, in which case the state of this object is an
// error code.
//
// FIXME: Implement assignment operators.
template<typename T>
  class Expected {
  public:
    using error_type = Error_code;
    using value_type = T;

    Expected() = delete;

    // Move semantics
    Expected(Expected&&);
    Expected& operator=(Expected&&) = delete;

    // Copy semantics
    Expected(const Expected&);
    Expected& operator=(const Expected&) = delete;

    // Value initialization
    Expected(const value_type&);
    Expected(value_type&&);
    Expected(error_type);

    ~Expected();

    explicit operator bool() const;

    const value_type& get() const;
    error_type error() const;

  private:
    bool valid_;
    union Value {
      Value() { }
      Value(const value_type& x) : value(x) { }
      Value(value_type&& x) : value(std::move(x)) { }
      Value(error_type x) : error(x) { }
      ~Value() { }
      error_type error;
      value_type value;
    } state_;
  };


// -------------------------------------------------------------------------- //
// Diagnostics

// The Diagnostic kind describes various flavors of diagnostics
// emitted by the compiler.
enum Diagnostic_kind {
  diag_error,
  diag_warning,
  diag_note,
  diag_sorry,
};

// The diagnostic class is the base class of all diagnostics.
// These come in various flavors defined by the Diagnostic_kind
// enumeration.
//
// This structure is not dissimilar from the AST structure defined
// for the language. In particular, certain kinds of diagnostics
// may contain sub-messages that provide additional information.
struct Diagnostic : Gc {
  Diagnostic(Diagnostic_kind k, Location l)
    : kind(k), loc(l), msg() { }

  Diagnostic(Diagnostic_kind k, Location l, const std::string& m)
    : kind(k), loc(l), msg(m) { }

  Diagnostic_kind kind;
  Location loc;
  std::string msg;
};

// An error is emitted to diagnost a lexical, syntactic, or semantic
// problem in the text of a program. The occurrence of any kind of
// error during processing precludes the execution of subsequent
// parsing phases. That is, a program with syntax errors is not usually
// semantically analyzed.
struct Error : Diagnostic {
  Error(Location l) : Diagnostic(diag_error, l) { }
  Error(Location l, const std::string& s) : Diagnostic(diag_error, l, s) { }
};

// A warning is emitted to diagnose programs that are valid, but
// may not be correct.
struct Warning : Diagnostic {
  Warning(Location l) : Diagnostic(diag_warning, l) { }
  Warning(Location l, const std::string& s) : Diagnostic(diag_error, l, s) { }
};

// A note is emitted to provide additional information for an error
// or a warning. 
struct Note : Diagnostic {
  Note(Location l) : Diagnostic(diag_note, l) { }
  Note(Location l, const std::string& s) : Diagnostic(diag_error, l, s) { }
};

// A sorry message is emitted when a feature is not implemented. 
struct Sorry : Diagnostic {
  Sorry(Location l) : Diagnostic(diag_sorry, l) { }
  Sorry(Location l, const std::string& s) : Diagnostic(diag_error, l, s) { }
};

// -------------------------------------------------------------------------- //
// Diagnostic management

// The diagnostics class defines a sequence of diagnostic failures.
using Diagnostics = std::vector<Diagnostic*, Gc_allocator<Diagnostic*>>;

// A diagnostic stream is a small object that supports
// stream-based construction of diagnostic messages.
//
// TODO: This model is not especially effective since message
// building requires string concatenation. We probably want an
// underlying stringstream, but that's a little harder to manage:
// we'd need to track it with a shared or unique pointer or
// something like that. 
struct Diagnostic_stream {
  Diagnostic_stream(const Diagnostic_stream&) = default;
  Diagnostic* diag;
};

Diagnostic_stream operator<<(Diagnostic_stream, char);
Diagnostic_stream operator<<(Diagnostic_stream, const char*);
Diagnostic_stream operator<<(Diagnostic_stream, const std::string&);
Diagnostic_stream operator<<(Diagnostic_stream, String);
Diagnostic_stream operator<<(Diagnostic_stream, int);
Diagnostic_stream operator<<(Diagnostic_stream, Integer);
Diagnostic_stream operator<<(Diagnostic_stream, debug_node);

Diagnostic_stream error();
Diagnostic_stream error(const Location&);
Diagnostic_stream error(Diagnostics&, const Location&);

Diagnostic_stream warn();
Diagnostic_stream warn(const Location&);
Diagnostic_stream warn(Diagnostics&, const Location&);

Diagnostic_stream note();
Diagnostic_stream note(const Location&);
Diagnostic_stream note(Diagnostics&, const Location&);

Diagnostic_stream sorry();
Diagnostic_stream sorry(const Location&);
Diagnostic_stream sorry(Diagnostics&, const Location&);

// Diagnostic access
Diagnostics* current_diagnostics();
void use_diagnostics(Diagnostics&);
void reset_diagnostics();

// An RAII helper that replaces the current diagnostics with
// an empty set.
struct Diagnostics_guard {
  Diagnostics_guard();
  Diagnostics_guard(Diagnostics&);
  ~Diagnostics_guard();

  Diagnostics* saved;   // The previous diagnostics
  Diagnostics* current; // The current diagnostics
};


void emit(std::ostream&, const Diagnostics&);

// Streaming
template<typename C, typename T>
  std::basic_ostream<C, T>& operator<<(std::basic_ostream<C, T>&, const Diagnostics&);

} // namespace steve

#include <steve/Error.ipp>

#endif
