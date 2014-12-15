
#include <iostream>

#include <steve/Print.hpp>

// These functions are defined in the C file solely for the reason
// that to do otherwise would require including <iostream> in a
// header file.

namespace steve {

void 
print(Printer& p, char c) { p.os << c; }

void 
print(Printer& p, const char* s) { p.os << s; }

void
print(Printer& p, String s) { p.os << s; }

void
print(Printer& p, const Integer& n) { p.os << n; }

void
print(Printer& p, bool b) { p.os << (b ? "true" : "false"); }

void 
print_space(Printer& p) { p.os << ' '; }

void 
print_newline(Printer& p) { p.os << '\n'; }

void 
print_indent(Printer& p) { p.os << std::string(2 * p.indent, ' '); }

} // namespace steve