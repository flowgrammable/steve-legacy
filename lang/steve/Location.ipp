
namespace steve {

// Default initialize a location into the current file.
inline
Location::Location(File* f)
  : file(f) { }

// Initialize an empty location. For a location l initialized in this
// way, l.is_internal() is true.
inline
Location::Location(no_location_t)
  : line(0), col(0), file(nullptr) { }

// Initialize the location to the end the given file. For a
// location l initialized in this way, l.is_eof() is true.
inline
Location::Location(eof_location_t, File* f)
  : line(-1), col(0), file(f) { }

inline bool
Location::is_internal() const { return not file; }

inline bool
Location::is_eof() const { return line == -1; }

// Equality comparison
inline bool
operator==(const Location& a, const Location& b) {
  return a.file == b.file && a.line == b.line && a.col == b.col;
}

inline bool
operator!=(const Location& a, const Location& b) {
  return !(a == b);
}

inline bool
operator==(no_location_t, const Location& b) {
  return b.is_internal();
}

inline bool
operator==(const Location& a, no_location_t) {
  return a.is_internal();
}

inline bool
operator!=(no_location_t, const Location& b) {
  return not b.is_internal();
}

inline bool
operator!=(const Location& a, no_location_t) {
  return not a.is_internal();
}


} // namespace steve
