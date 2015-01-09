
#include <steve/Location.hpp>
#include <steve/File.hpp>

#include <cstdlib>

namespace steve {

// Return true if two source locations are vertically adjacent. That
// is, they locations are in the same file, and same column, but differ
// by only one line.
bool
adjacent(const Location& a, const Location& b) {
  if (a.file != b.file)
    return false;
  if (a.col != b.col)
    return false;
  if (std::abs(a.line - b.line) != 1)
    return false;
  return true;
}

// Output for source locations.
std::ostream&
operator<<(std::ostream& os, const Location& loc) {
  if (loc.is_internal())
    return os << "<internal>";
  os << loc.file->path().c_str() << ':';
  if (loc.is_eof())
    return os << "<eof>";
  return os << loc.line << ':' << loc.col;
}

} // namespace steve
