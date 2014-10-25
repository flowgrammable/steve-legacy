
#ifndef STEVE_LOCATION_HPP
#define STEVE_LOCATION_HPP

#include <iosfwd>

namespace steve {

// A univalent type used to construct a location that does not
// refer to a source code location.
enum no_location_t { no_location };

// A univalent type used to construct the EOF location.
enum eof_location_t { eof_location };

// A location represents a position in a source file, indicated by its
// line and character offset.
//
// TODO: Ensure that we can enregister source locations.
//
// TODO: Map source locations to files? 
struct Location {
  Location() = default;
  Location(no_location_t);
  Location(eof_location_t);
  
  bool is_internal() const;
  bool is_eof() const;

  int line = 1;
  int col = 1;
};


// Output formatting
template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>&, const Location&);

} // namespace steve

#include "Location.ipp"

#endif
