
#ifndef STEVE_COMMENT_HPP
#define STEVE_COMMENT_HPP

#include <steve/Token.hpp>

#include <vector>

namespace steve {

// A comment is token that documents some aspect of a program. 
// Comments are reprensented as tokens, having a source location
// and associated text.
struct Comment {
  Comment(const Location&, String);

  String text() const;
  const Location& location() const;

  Token tok;
};

using Comment_list = std::vector<Comment>;

// A comment block is a list of comments written on contiguous
// lines of a program that start in the same column. For example:
//
//    // first line
//    // second line
//
// This is a comment block with two lines, and this:
//
//    // first line
//      // first line
//
// is two contiguous comment blocks.
struct Comment_block : Comment_list {
  std::string text() const;
  
  File* file() const;
  const Location& first_location() const;
  const Location& last_location() const;
};


// A list of comment blocks.
using Comment_block_list = std::vector<Comment_block>;


// The comment manager is a facility that groups comments into
// comment blocks.
class Comment_manager {
public:
  void save(const Location&, String);
  void reset();

  Comment_block* find(const Location& loc);

  Comment_block_list blocks;

private:
  void init(const Location&, String);
  void append(const Location&, String);
};

Comment_manager& comments();

} // namespace steve

#include <steve/Comment.ipp>

#endif
