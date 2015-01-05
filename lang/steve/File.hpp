
#ifndef STEVE_FILE_HPP
#define STEVE_FILE_HPP

// The File module contains facilities for working with paths and
// files.

#include <vector>

#include <boost/filesystem.hpp>

namespace steve {

// Alias the file system namespace.
namespace fs = boost::filesystem;

template<typename T> class Expected;

// -------------------------------------------------------------------------- //
// Paths

// A path represents the location of a file in the host file system.
using Path = fs::path;

// A sequence of the paths.
using Path_list = std::vector<Path>;


// -------------------------------------------------------------------------- //
// Files

// A file is a container of input source of a program and are
// represented by a path to that file in the operating system.
//
// A file does not own the text that it contains, but rather retrieves
// it on request via the text() operation.
//
// Note that files need not contain Steve programs; they can also
// represent other input files to the system.
class File {
public:
  File(const Path&);

  Expected<std::string> text() const;

private:
  Path path_;
};

} // namespace steve

#endif
