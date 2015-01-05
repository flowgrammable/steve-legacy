
namespace steve {

inline Tokens
Lexer::operator()(File* f) {
  return (*this)(f, f->text());
}

inline Tokens
Lexer::operator()(File* f, const std::string& s) {
  return (*this)(f, s.begin(), s.end());
}

} // namespace steve
