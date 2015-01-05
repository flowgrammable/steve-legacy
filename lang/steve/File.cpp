
#include <steve/File.hpp>
#include <steve/Error.hpp>

#include <fstream>
#include <iterator>
#include <map>

namespace steve {

// -------------------------------------------------------------------------- //
// File

// Initialize the file. This sets the path of the file to its
// canonical 
inline
File::File(const Path& p)
  : path_(fs::canonical(p)) 
{ 
  using Iter = std::istreambuf_iterator<char>;

  // Open the inpout file.
  std::ifstream fs(path_.c_str());
  if (not fs)
    throw std::system_error(errno, std::generic_category());

  // Copy the text to be returned.
  //
  // BUG: There's a libstdc++ bug that causes exceptions to be thrown
  // when they aren't expected (e.g., underflow errors). 
  try {
    text_ = std::string(Iter{fs}, Iter{});
  } catch (...) { 
    // This is probably not an error.
  }
}

// -------------------------------------------------------------------------- //
// File

namespace {

using File_set = std::map<Path, File*>;

// The set of loaded files.
File_set files_;

} // namespace

// Get the file corresponding to the given path name. Each unique
// path corresponds to a unique File object.
File*
get_file(const Path& p) {
  auto iter = files_.find(p);
  if (iter == files_.end()) 
    return files_.insert({p, new File(p)}).first->second;
  else
    return iter->second;
}

} // namespace steve
