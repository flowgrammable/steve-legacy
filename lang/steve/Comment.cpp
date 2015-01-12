
#include <steve/Comment.hpp>

#include <sstream>

namespace steve {

namespace {

// The global comment manager.
Comment_manager cm_;

// Remove the terminator from the comment block, if present.
void strip(Comment_block& b) {
  if (b.last_location() == no_location)
    b.pop_back();
}

} // namespace


Comment_manager&
comments() { return cm_; }

std::string
Comment_block::text() const {
  std::stringstream ss;
  for (const Comment& c : *this)
    ss << c.text() << '\n';
  return ss.str();
}

// Initialize
void
Comment_manager::init(const Location& loc, String text) {
    blocks.emplace_back();
    append(loc, text);
}

void
Comment_manager::append(const Location& loc, String text) {
  blocks.back().emplace_back(loc, text);
}

void
Comment_manager::save(const Location& loc, String text) {
  // If there are not yet any blocks, create the first one and
  // add the comment to that.
  if (blocks.empty())
    init(loc, text);

  // Determine if the comment belongs to the current block
  // or if we should create a new block. Note that the current
  // block is never empty.
  Comment_block& block = blocks.back();
  if (adjacent(block.last_location(), loc)) {
    append(loc, text);
  }
  else {
    strip(block);
    init(loc, text);
  }
}

// Add a terminator to the last comment (if present). This will
// force the next 
void
Comment_manager::reset() {
  if (blocks.empty())
    return;
  Comment_block& b = blocks.back();
  if (b.last_location() != no_location)
    save(no_location, String());
}

// Returns true if the comment block appertains to the entity
// at the given location. A comment appertains to a declaration
// if they are in the same file, and
//
//    1. the declaration appears on the same line as the beginning
//       of the comment block, or
//    2. the end of the comment block is immediately above the 
//       declaration, or
//    3. the start of the comment block is immediately below the
//       declaration.
//
// Examples:
//
//    def r : typename = record {
//      x : int; // appertains to x because of rule 1
//    }
//
//    // appertains to y because of rule 2
//    def y : int;
//
//    def f(x : int) -> int
//    // appertains to f because of rule 3.
//    { return x; }
//  
// TODO: Re-think rule 3 for cases where the declaration spans multiple
// lines and allow the column to be indented.
//
// TOOD: Implement rule 3.
bool
appertains(const Comment_block& b, const Location& l) {
  if (b.file() != l.file)
    return false;
  if (above(b.last_location(), l))
    return true;
  else
    return false;
}

// Find the nearest comment block associated with a location.
//
// TODO: This is wildly inefficient. We should be mapping comments
// by file and we should have a data structure that optimizes
// location-based lookup.
Comment_block*
Comment_manager::find(const Location& loc) {
  for (Comment_block& b : blocks) {
    if (appertains(b, loc))
      return &b;
  }
  return nullptr;
}

} // namespace steve


