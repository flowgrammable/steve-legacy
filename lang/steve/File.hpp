
#ifndef STEVE_FILE_HPP
#define STEVE_FILE_HPP

// The File module contains facilities for working with paths and
// files.
//
// TODO: Whenever possible, replace the use of Boost.Filesystem with
// a std filesystem library. Otherwise, we have to translate error
// codes between two different systems. It's fine for now, but not
// ideal.

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
// Note that files need not contain Steve programs; they can also
// represent other input files to the system.
//
// TODO: What if I want an output file? Do I actually need file
// modes or different kinds of file?
class File {
public:
  File(const Path&);

  const Path& path() const;
  const std::string& text() const;

private:
  Path        path_;
  std::string text_;
};


// -------------------------------------------------------------------------- //
// File set

File* get_file(const Path&);

} // namespace steve

#include <steve/File.ipp>

#endif
