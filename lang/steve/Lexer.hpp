
#ifndef STEVE_LEXER_HPP
#define STEVE_LEXER_HPP

#include <steve/File.hpp>
#include <steve/Token.hpp>
#include <steve/Error.hpp>

// The lexer module implements the lexical analyzer for the steve
// programming language.
//
// TODO: Integrate the new diagnostics module into the lexer.

namespace steve {

// The lexer is responsible for decomposing a character stream into
// a token stream.
struct Lexer {
  using Iterator = std::string::const_iterator;

  Tokens operator()(File*);
  Tokens operator()(File*, const std::string&);
  Tokens operator()(File*, Iterator, Iterator);

  Iterator first;    // The current lex position
  Iterator last;     // The final lex position
  Location loc;      // The current source location
  Tokens toks;       // The current token list
  Diagnostics diags;
};

} // namespace steve

#include <steve/Lexer.ipp>

#endif
