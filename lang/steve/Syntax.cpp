
#include <steve/Syntax.hpp>
#include <steve/Print.hpp>
#include <steve/Debug.hpp>

namespace steve {

void
init_trees() {
  // Terms
  init_node(id_tree, "id-tree");
  init_node(lit_tree, "lit-tree");
  init_node(brace_tree, "brace-tree");
  init_node(call_tree, "call-tree");
  init_node(index_tree, "index-tree");
  init_node(dot_tree, "dot-tree");
  init_node(range_tree, "range-tree");
  init_node(app_tree, "app-tree");
  init_node(unary_tree, "unary-tree");
  init_node(binary_tree, "binary-tree");
  init_node(if_tree, "if-tree");
  // Types
  init_node(record_tree, "record-tree");
  init_node(variant_tree, "variant-tree");
  init_node(enum_tree, "enum-tree");
  // Statements
  init_node(block_tree, "block-tree");
  init_node(return_tree, "return-tree");
  init_node(break_tree, "break-tree");
  init_node(cont_tree, "cont-tree");
  init_node(while_tree, "while-tree");
  init_node(switch_tree, "switch-tree");
  init_node(load_tree, "load-tree");
  // Declarations
  init_node(value_tree, "value-tree");
  init_node(parm_tree, "parm-tree");
  init_node(fn_tree, "fn-tree");
  init_node(def_tree, "def-tree");
  init_node(field_tree, "field-tree");
  init_node(alt_tree, "alt-tree");
  init_node(import_tree, "import-tree");
  init_node(using_tree, "using-tree");
  // Misc
  init_node(top_tree, "top-tree");
}

// FIXME: A whole lot of this stuff is common in both Ast.cpp and
// here. In fact, it's practically verbatim. It would not be amiss
// to move much of this out to Node.cpp.

namespace {

struct sexpr {
 sexpr(Printer& p, const char* n) 
    : printer(p) 
    {
      print(printer, '(');
      print(printer, n);
      print_space(printer);
    }

  sexpr(Printer& p, String s)
    : sexpr(p, s.data()) { }

  template<typename T>
    sexpr(Printer& p, T* n) 
      : sexpr(p, node_name(n)) { }

  ~sexpr() { print(printer, ')'); }

  Printer& printer;
};


void debug_print(Printer&, Tree*);

void
debug_print(Printer& p, const Token* k) {
  print(p, k->text);
}

struct debug_printer {
  template<typename T>
    void operator()(Printer& p, T t) const { debug_print(p, t); }
};

// Top-level match printing sequences-of-T.
template<typename T>
  void 
  debug_print(Printer& p, Seq<T>* s) {
    print(p, '(');
    print_separated(p, s, debug_printer(), space_separator());
    print(p, ')');
  }

// Note that t->first is a token.
template<typename T>
  inline void
  debug_terminal(Printer& p, T* t) {
    sexpr s(p, t);
    debug_print(p, t->first);
  }

template<typename T>
  inline void
  debug_nullary(Printer& p, T* t) { sexpr s(p, t); }

template<typename T>
  inline void
  debug_unary(Printer& p, T* t) {
    sexpr s(p, t);
    debug_print(p, t->first);
  }

template<typename T>
  void
  debug_binary(Printer& p, T* t) {
    sexpr s(p, t);
    debug_print(p, t->first);
    print_space(p);
    debug_print(p, t->second);
  }

template<typename T>
  inline void
  debug_ternary(Printer& p, T* t) {
    sexpr s(p, t);
    debug_print(p, t->first);
    print_space(p);
    debug_print(p, t->second);
    print_space(p);
    debug_print(p, t->third);
  }


void
debug_unary(Printer& p, Unary_tree* t) {
    sexpr s(p, t->op()->text);
    debug_print(p, t->arg());
}

void
debug_binary(Printer& p, Binary_tree* t) {
    sexpr s(p, t->op()->text);
    debug_print(p, t->left());
    print_space(p);
    debug_print(p, t->right());
}

void
debug_top(Printer& p, Top_tree* t) {
  sexpr s(p, t);
  indent(p);
  print_newline(p);
  for (Tree* s : *t->first) {
    print_indent(p);
    debug_print(p, s);
    print_newline(p);
  }
  undent(p);
  print_indent(p);
}

void
debug_print(Printer& p, Tree* t) {
  if (not t) {
    print(p, "()");
    return;
  }
  
  switch (t->kind) {
  // Terms
  case id_tree: return debug_terminal(p, as<Id_tree>(t));
  case lit_tree: return debug_terminal(p, as<Lit_tree>(t));
  case brace_tree: return debug_unary(p, as<Brace_tree>(t));
  case call_tree: return debug_binary(p, as<Call_tree>(t));
  case index_tree: return debug_binary(p, as<Index_tree>(t));
  case app_tree: return debug_binary(p, as<App_tree>(t));
  case dot_tree: return debug_binary(p, as<Dot_tree>(t));
  case range_tree: return debug_binary(p, as<Range_tree>(t));
  case unary_tree: return debug_unary(p, as<Unary_tree>(t));
  case binary_tree: return debug_binary(p, as<Binary_tree>(t));
  case if_tree: return debug_ternary(p, as<If_tree>(t));
  // Types
  case record_tree: return debug_unary(p, as<Record_tree>(t));
  case variant_tree: return debug_binary(p, as<Variant_tree>(t));
  case enum_tree: return debug_binary(p, as<Enum_tree>(t));
  // Statements
  case return_tree: return debug_unary(p, as<Return_tree>(t));
  case break_tree: return debug_nullary(p, as<Break_tree>(t));
  case cont_tree: return debug_nullary(p, as<Cont_tree>(t));
  case while_tree: return debug_binary(p, as<While_tree>(t));
  case switch_tree: return debug_binary(p, as<Switch_tree>(t));
  case case_tree: return debug_binary(p, as<Case_tree>(t));
  case load_tree: return debug_unary(p, as<Load_tree>(t));
  // Declarations
  case value_tree: return debug_binary(p, as<Value_tree>(t));
  case parm_tree: return debug_ternary(p, as<Parm_tree>(t));
  case fn_tree: return debug_ternary(p, as<Fn_tree>(t));
  case def_tree: return debug_binary(p, as<Def_tree>(t));
  case field_tree: return debug_ternary(p, as<Field_tree>(t));
  case alt_tree: return debug_binary(p, as<Alt_tree>(t));
  case import_tree: return debug_unary(p, as<Import_tree>(t));
  case using_tree: return debug_unary(p, as<Using_tree>(t));
  // Misc
  case top_tree: return debug_top(p, as<Top_tree>(t));
  // Unhandled
  default:
    steve_unreachable("unhandled parse tree");
  }
}

} // namespace

std::ostream& 
operator<<(std::ostream& os, debug_tree d) {
  Printer p(os);
  debug_print(p, d.e);
  return os;
}

} // namespace steve
