
#include <iostream>
#include <fstream>
#include <iterator>

#include <steve/Language.hpp>
#include <steve/Lexer.hpp>
#include <steve/Parser.hpp>
#include <steve/Elaborator.hpp>

using namespace steve;

int 
main() {
  Language lang;

  // Read the input text into a file.
  using Iter = std::istreambuf_iterator<char>;
  std::string text(Iter(std::cin), Iter());

  Lexer lex;
  Tokens toks = lex(text);
  for (const Token& k : toks)
    std::cout << debug(k) << ' ';
  std::cout << '\n';
  return 0;

  Parser parse;
  Tree* pt = parse(toks);
  if (not parse.diags.empty()) {
    std::cerr << "got errors\n" << parse.diags;
    return -1;
  }

  Elaborator elab;
  Expr* ast = elab(pt);
  if (not elab.diags.empty()) {
    std::cerr << "got typing errors\n" << elab.diags;
    return -1;
  }

  std::cout << debug(ast) << '\n';
}
