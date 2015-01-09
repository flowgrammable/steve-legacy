
namespace steve {

inline
Comment::Comment(const Location& l, String s)
  : tok(l, comment_tok, s) { }

inline String
Comment::text() const { return tok.text; }

inline const Location&
Comment::location() const { return tok.loc; }

inline const Location&
Comment_block::first_location() const { return front().location(); }

inline const Location&
Comment_block::last_location() const { return back().location(); }

} // namespace steve
