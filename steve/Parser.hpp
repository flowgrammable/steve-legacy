
#ifndef STEVE_PARSER_HPP
#define STEVE_PARSER_HPP

#include <steve/Error.hpp>
#include <steve/Token.hpp>

// The parser module implements the parser for the steve programming
// language. Note that the grammar is context sensitive, so semantic
// analysis is required at parse time.

namespace steve {

struct Tree;

// The parser transforms a token stream into a parse tree. For
// this language, the parse tree is indistinguishable from the
// abstract syntax tree.
struct Parser {
  Tree* operator()(const Tokens&);
  Tree* operator()(Token_iterator, Token_iterator);

  Token_iterator first;
  Token_iterator last;
  Token_iterator current;
  Diagnostics    diags;
};

} // namespace steve

#endif
