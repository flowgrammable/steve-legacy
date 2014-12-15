
namespace steve {

inline Tokens
Lexer::operator()(const std::string& s) {
  return (*this)(s.begin(), s.end());
}

} // namespace steve