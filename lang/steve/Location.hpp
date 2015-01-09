
#ifndef STEVE_LOCATION_HPP
#define STEVE_LOCATION_HPP

#include <iosfwd>

namespace steve {

struct File;

// A univalent type used to construct a location that does not
// refer to a source code location.
enum no_location_t { no_location };

// A univalent type used to construct the EOF location.
enum eof_location_t { eof_location };

// A location represents a position in a source file, indicated by its
// line and character offset.
//
// TODO: Can we further compact source locations so they can be
// enregistered? This is a large project and probably entails
// the restructing of locations as simply pointers into another
// data structure.
struct Location {
  Location() = default;
  Location(File*);
  Location(no_location_t);
  Location(eof_location_t, File*);
  
  bool is_internal() const;
  bool is_eof() const;

  int   line = 1;
  int   col  = 1;
  File* file = nullptr;
};

// Equality comparison
bool operator==(const Location&, const Location&);
bool operator!=(const Location&, const Location&);

bool operator==(no_location_t, const Location&);
bool operator==(const Location&, no_location_t);
bool operator!=(no_location_t, const Location&);
bool operator!=(const Location&, no_location_t);

// Operations
bool adjacent(const Location&, const Location&);

// Streaming
std::ostream& operator<<(std::ostream&, const Location&);

} // namespace steve

#include "Location.ipp"

#endif
