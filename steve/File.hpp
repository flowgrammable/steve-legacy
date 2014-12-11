
#ifndef STEVE_FILE_HPP
#define STEVE_FILE_HPP

// The File module contains facilities for working with paths and
// files.
//
// TODO: Bring more boost::filesystem operations into scope.

#include <boost/filesystem.hpp>

namespace steve {

// Alias the file system namespace.
namespace fs = boost::filesystem;

// A path represents the location of a file in the host file system.
using Path = fs::path;

} // namespace steve

#endif
