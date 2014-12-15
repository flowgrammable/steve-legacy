
#ifndef STEVE_FORMAT_HPP
#define STEVE_FORMAT_HPP

#include <iosfwd>

// This module imports the cppformat library by Victor Zverovich.
// In particular, it makes a number of facilities available in the
// steve namesapce.

#include <steve/contrib/cppformat/format.h>

namespace steve {

using fmt::format; // The format facilitiy

// Text formatters.
using fmt::bin; 
using fmt::oct;
using fmt::hex;
using fmt::pad;

// Import the Writer class as an alternative to stringstream.
using fmt::Writer;


// Other formatting and streaming utilities.
int stream_base(const std::ios_base&);

} // namespace steve

#endif
