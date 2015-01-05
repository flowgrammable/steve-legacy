
#include <steve/File.hpp>
#include <steve/Error.hpp>

#include <cstring>
#include <iterator>
#include <fstream>

namespace steve {

// -------------------------------------------------------------------------- //
// File

// Return a string containing the text of an input file. If the
// input file cannot be opened, throw an exception.
//
// TOOD: Improve error handling for this function. 
Expected<std::string>
File::text() const {
  using Iter = std::istreambuf_iterator<char>;

  // Open the inpout file.
  std::ifstream fs(path_.c_str());
  if (fs)
    return Error_code(errno, boost::system::system_category());

  // Copy the text to be returned.
  //
  // BUG: There's a libstdc++ bug that causes exceptions to be thrown
  // when they aren't expected (e.g., underflow errors). 
  std::string text;
  try {
    text = std::string(Iter{fs}, Iter{});
  } catch (...) { 
    // Don't do anything here...
  }
  return text;  
}

} // namespace steve
