
#include <steve/Location.hpp>
#include <steve/File.hpp>

namespace steve {

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
