
#ifndef STEVE_PARSER_HPP
#define STEVE_PARSER_HPP

#include <steve/Error.hpp>
#include <steve/Token.hpp>

// The parser module implements the parser for the steve programming
// language. Note that the grammar is context sensitive, so semantic
// analysis is required at parse time.

namespace steve {

struct Tree;

// The parse kind specifies the entry point for the
// parser. By default, the entry point is a module.
//
// TODO: Allow more entry points into the parser. 
enum Parse_kind {
  postfix_parse,      // Parse an postfix-expression
  prefix_parse,       // Parse a prefix-expression
  expr_parse,         // Parse an expression
  top_parse,          // Parse a module
};

// The parser transforms a token stream into a parse tree. For
// this language, the parse tree is indistinguishable from the
// abstract syntax tree.
struct Parser {
  Tree* operator()(const Tokens&, Parse_kind = top_parse);
  Tree* operator()(Token_iterator, Token_iterator, Parse_kind = top_parse);

  Token_iterator first;
  Token_iterator last;
  Token_iterator current;
  Diagnostics    diags;
};

} // namespace steve

#endif
