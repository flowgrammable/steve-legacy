
#include <iostream>

#include <steve/Token.hpp>

using namespace steve;

void
print_info(Token_kind t) {
  Writer w;
  w << pad(token_name(t), 24, ' ') << ' ' << pad(bin(t), 32, '0');
  std::cout << w.str() << '\n';
}

namespace steve {
  extern void init_tokens();
} // namespace steve

int
main() {
  init_tokens();
  print_info(lparen_tok);
  print_info(rparen_tok);
  print_info(lbrace_tok);
  print_info(rbrace_tok);
  print_info(lbracket_tok);
  print_info(rbracket_tok);
  print_info(semicolon_tok);
  print_info(question_tok);
  print_info(colon_tok);
  print_info(comma_tok);
  print_info(dot_tok);
  print_info(arrow_tok);
  print_info(plus_tok);
  print_info(minus_tok);
  print_info(star_tok);
  print_info(slash_tok);
  print_info(percent_tok);
  print_info(caret_tok);
  print_info(ampersand_tok);
  print_info(pipe_tok);
  print_info(tilde_tok);
  print_info(equal_tok);
  print_info(equal_equal_tok);
  print_info(bang_equal_tok);
  print_info(langle_tok);
  print_info(langle_equal_tok);
  print_info(langle_langle_tok);
  print_info(rangle_tok);
  print_info(rangle_equal_tok);
  print_info(rangle_rangle_tok);
  print_info(import_tok);
  print_info(export_tok);
  print_info(def_tok);
  print_info(record_tok);
  print_info(variant_tok);
  print_info(enum_tok);
  print_info(int_tok);
  print_info(nat_tok);
  print_info(typename_tok);
  print_info(and_tok);
  print_info(or_tok);
  print_info(not_tok);
  print_info(if_tok);
  print_info(else_tok);
  print_info(identifier_tok);
  print_info(binary_literal_tok);
  print_info(octal_literal_tok);
  print_info(decimal_literal_tok);
  print_info(hexadecimal_literal_tok);
  print_info(boolean_literal_tok);
}
