
namespace steve {

inline
Token::Token(Token_kind k, String t)
  : loc(), kind(k), text(t) { }

inline
Token::Token(Location l, Token_kind k, String t)
  : loc(l), kind(k), text(t) { }


// -------------------------------------------------------------------------- //
// Operations

// Returns the token's kind.
inline Token_kind
kind(const Token& t) { return t.kind; }

// Returns the token's kind.
inline Token_kind
kind(const Token* t) { return t->kind; }


// -------------------------------------------------------------------------- //
// Printing

template<typename T>
  inline Requires<is_token<T>(), debug_token<T>>
  debug(const T& tok) { return debug_token<T>{tok}; }

template<typename T>
  inline Requires<is_token<T>(), debug_token<T>>
  debug(const T* tok) { return debug_token<T>{*tok}; }

template<typename T>
  inline Requires<is_token<T>(), quote_token<T>>
  quote(const T& tok) { return quote_token<T>{tok}; }

template<typename T>
  inline Requires<is_token<T>(), quote_token<T>>
  quote(const T* tok) { return quote_token<T>{*tok}; }

// Quoted output for tokens.

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Token& tok) {
    return os << tok.text;
  }

template<typename C, typename T, typename K>
  inline std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const debug_token<K>& k) {
    os << '[';
    os << token_name(k.tok.kind);
    if (token::is_typed(k.tok.kind))
      os << ":" << k.tok.text;
     os << ']';
     return os;
  }

template<typename C, typename T, typename K>
  inline std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const quote_token<K>& k) {
    return os << '\'' << k.tok.text << '\'';
  }

} // namespace steve
