
#ifndef STEVE_PRINT_HPP
#define STEVE_PRINT_HPP

// This module defines debug and printing support.

#include <iosfwd>

#include <steve/String.hpp>
#include <steve/Integer.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Printer class

// The printer maintains informatoin about the current printing
// state, including the indentation level, and other various flags.
struct Printer {
  Printer(std::ostream&);
  std::ostream& os;
  int indent;
};

Printer& indent(Printer& p);
Printer& undent(Printer& p);

void print(Printer&, char);
void print(Printer&, const char*);
void print(Printer&, String);
void print(Printer&, const Integer&);
void print(Printer&, bool);

void print_spaced(Printer& p, const char* str);
void print_space(Printer&);
void print_newline(Printer&);
void print_indent(Printer&);

// Separators

template<typename S, typename P, typename F>
  void print_separated(Printer& p, S*, P, F);

struct space_separator { void operator()(Printer&) const; };
struct comma_separator { void operator()(Printer&) const; };

} // namespace steve

#include <steve/Print.ipp>

#endif
