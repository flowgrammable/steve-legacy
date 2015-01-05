
namespace steve {

// Default initialize a location into the current file.
inline
Location::Location(File* f)
  : file(f) { }

// Initialize an empty location.
inline
Location::Location(no_location_t)
  : line(0), col(0), file(nullptr) { }

// Initialize the location to the end the given file.
inline
Location::Location(eof_location_t, File* f)
  : line(-1), col(0), file(f) { }

inline bool
Location::is_internal() const { return not file; }

inline bool
Location::is_eof() const { return line == -1; }

} // namespace steve
