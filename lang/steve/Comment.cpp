
#include <steve/Comment.hpp>

#include <sstream>

namespace steve {

namespace {

// Remove the terminator from the comment block, if present.
void strip(Comment_block& b) {
  if (b.last_location() == no_location)
    b.pop_back();
}

} // namespace

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

  // 
  Comment_block& b = blocks.back();
  if (b.last_location() != no_location)
    save(no_location, String());
}

} // namespace steve



