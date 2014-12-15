

#include <steve/Ast.hpp>
#include <steve/Language.hpp>

#include <iostream>

using namespace steve;

void
print_info(Node_kind t) {
  Writer w;
  w << pad(node_name(t), 24, ' ') << ' ' << pad(bin(t), 32, '0');
  std::cout << w.str() << '\n';
}


int
main() {
  Language lang;

  print_info(seq_node);
  // Types
  print_info(typename_type);
  print_info(unit_type);
  print_info(bool_type);
  print_info(nat_type);
  print_info(int_type);
  print_info(fn_type);
  print_info(bitfield_type);
  print_info(record_type);
  print_info(variant_type);
  print_info(enum_type);
  print_info(module_type);
  // Terms
  print_info(bool_term);
  print_info(int_term);
  print_info(fn_term);
  // Decls
  print_info(def_decl);
  print_info(import_decl);
}
