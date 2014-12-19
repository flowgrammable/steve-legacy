
#include <iostream>
#include <unordered_map>

#include <steve/Ast.hpp>
#include <steve/Decl.hpp>
#include <steve/Print.hpp>
#include <steve/Debug.hpp>

namespace steve {

void
init_exprs() {
  // Util
  init_node(seq_node, "seq");
  // Names
  init_node(basic_id, "basic-id");
  init_node(operator_id, "operator-id");
  init_node(scoped_id, "scoped-id");
  init_node(decl_id, "decl-id");
  // Types
  init_node(typename_type, "typename-type");
  init_node(unit_type, "unit-type");
  init_node(bool_type, "bool-type");
  init_node(nat_type, "nat-type");
  init_node(int_type, "int-type");
  init_node(char_type, "char-type");
  init_node(fn_type, "fn-type");
  init_node(range_type, "range-type");
  init_node(bitfield_type, "bitfield-type");
  init_node(record_type, "record-type");
  init_node(variant_type, "variant-type");
  init_node(dep_variant_type, "dep-variant-type");
  init_node(enum_type, "enum-type");
  init_node(array_type, "array-type");
  init_node(dep_type, "dep-type");
  init_node(module_type, "module-type");
  // Terms
  init_node(unit_term, "unit");
  init_node(bool_term, "bool");
  init_node(int_term, "int");
  init_node(default_term, "default");
  init_node(fn_term, "fn");
  init_node(call_term, "call");
  init_node(promo_term, "promo");
  init_node(pred_term, "pred");
  init_node(range_term, "range");
  init_node(variant_term, "variant");
  init_node(unary_term, "unary");
  init_node(binary_term, "binary");
  init_node(builtin_term, "builtin");
  // Decls
  init_node(top_decl, "top-decl");
  init_node(def_decl, "def-decl");
  init_node(parm_decl, "parm-decl");
  init_node(field_decl, "field-decl");
  init_node(alt_decl, "alt-decl");
  init_node(enum_decl, "enum-decl");
  init_node(import_decl, "import-decl");
}


// -------------------------------------------------------------------------- //
// Debug printing
//
// Dump an AST as an sexpr.

template<typename T>
  void debug_print(Printer&, Seq<T>*);

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


namespace {

struct sexpr {
  sexpr(Printer& p, String name) 
  : printer(p) 
  {
    print(printer, '(');
    print(printer, name);
    print_space(printer);
  }

  ~sexpr() { print(printer, ')'); }

  Printer& printer;
};


template<typename T>
  inline void
  debug_terminal(Printer& p, T* e) { print(p, e->first); }

template<typename T>
  inline void
  debug_unary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
  }

template<typename T>
  inline void
  debug_binary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
    print_space(p);
    debug_print(p, e->second);
  }

template<typename T>
  inline void
  debug_ternary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
    print_space(p);
    debug_print(p, e->second);
    print_space(p);
    debug_print(p, e->third);
  }

template<typename T>
  inline void
  debug_quaternary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
    print_space(p);
    debug_print(p, e->second);
    print_space(p);
    debug_print(p, e->third);
    print_space(p);
    debug_print(p, e->fourth);
  }

// TODO: Actually render a scoped name.
void
debug_scoped_id(Printer& p, Scoped_id* e) {
  sexpr s(p, "scoped-id");
}

// Print a reference to a declaration.
template<typename T>
  void
  debug_id(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->name());
  }

void
debug_operator(Printer& p, Operator_id* e) {
  sexpr s(p, node_name(e));
  print(p, e->op());
}

template<typename T>
  inline void
  debug_ref(Printer& p, T* t) {
    sexpr s(p, "ref");
    debug_print(p, decl_name(t->first));
  }

void
debug_fn(Printer& p, Fn* e) {
  debug_ternary(p, e);
  // if (e->is_intrinsic()) {
  //   sexpr s(p, "intrinsic");
  //   debug_print(p, e->first);
  //   print_space(p);
  //   debug_print(p, e->second);
  //   print_space(p);
  //   print(p, "<intrinsic>");
  // } else {
  //   debug_ternary(p, e);
  // }
}

// Decls

template<typename T>
  void
  debug_constructor(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
    print_space(p);
    debug_print(p, e->second);
  }

template<typename T>
  void
  debug_print_nested(Printer& p, Seq<T>* seq) {
    for (auto* s : *seq) {
      print_indent(p);
      debug_print(p, s);
      print_newline(p);
    }
  }

template<typename T>
  void
  debug_nested_unary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    indent(p);
    print_newline(p);
    debug_print_nested(p, e->first);
    undent(p);
    print_indent(p);
  }


template<typename T>
  void
  debug_nested_binary(Printer& p, T* e) {
    sexpr s(p, node_name(e));
    debug_print(p, e->first);
    indent(p);
    print_newline(p);
    debug_print_nested(p, e->second);
    undent(p);
    print_indent(p);
  }

void
debug_module(Printer& p, Module* m) {
  print(p, format("<module {}>", m->path().c_str()));
}

} // namespace

void
debug_print(Printer& p, Expr* e) {
  if (not e) {
    print(p, "()");
    return;
  }
  
  switch (e->kind) {
  // Names
  case basic_id: return debug_terminal(p, as<Basic_id>(e));
  case operator_id: return debug_operator(p, as<Operator_id>(e));
  case scoped_id: return debug_scoped_id(p, as<Scoped_id>(e));
  case decl_id: return debug_id(p, as<Decl_id>(e));
  // Types
  case typename_type: return print(p, "typename");
  case unit_type: return print(p, "unit");
  case bool_type: return print(p, "bool");
  case nat_type: return print(p, "nat");
  case int_type: return print(p, "int");
  case char_type: return print(p, "char");
  case bitfield_type: return debug_ternary(p, as<Bitfield_type>(e));
  case fn_type: return debug_binary(p, as<Fn_type>(e));
  case range_type: return debug_unary(p, as<Range_type>(e));
  case record_type: return debug_unary(p, as<Record_type>(e));
  case variant_type: return debug_unary(p, as<Variant_type>(e));
  case dep_variant_type: return debug_nested_binary(p, as<Dep_variant_type>(e));
  case enum_type: return debug_nested_binary(p, as<Enum_type>(e));
  case array_type: return debug_binary(p, as<Array_type>(e));
  case dep_type: return debug_binary(p, as<Dep_type>(e));
  case module_type: return debug_module(p, as<Module>(e));
  // Terms
  case unit_term: return print(p, "<unit>");
  case bool_term: return debug_terminal(p, as<Bool>(e));
  case int_term: return debug_terminal(p, as<Int>(e));
  case default_term: return print(p, "default");
  case fn_term: return debug_fn(p, as<Fn>(e));
  case call_term: return debug_binary(p, as<Call>(e));
  case promo_term: return debug_binary(p, as<Promo>(e));
  case pred_term: return debug_binary(p, as<Pred>(e));
  case range_term: return debug_binary(p, as<Range>(e));
  case variant_term: return debug_binary(p, as<Variant>(e));
  case unary_term: return debug_binary(p, as<Unary>(e));
  case binary_term: return debug_ternary(p, as<Binary>(e));
  case builtin_term: return print(p, "<builtin>");
  // Statements
  // Declarations
  case top_decl: return debug_nested_unary(p, as<Top>(e));
  case def_decl: return debug_ternary(p, as<Def>(e));
  case parm_decl: return debug_ternary(p, as<Parm>(e));
  case field_decl: return debug_ternary(p, as<Field>(e));
  case alt_decl: return debug_binary(p, as<Alt>(e));
  case enum_decl: return debug_constructor(p, as<Enum>(e));
  case import_decl: return debug_unary(p, as<Import>(e));
  // Unhandled
  default:
    // FIXME: Make a print formatter that gives complete information
    // about a node, including its class and id.
    steve_unreachable(format("debugging unhandled expression '{}'", node_name(e)));
  }
}

std::ostream& 
operator<<(std::ostream& os, debug_node d) {
  Printer p(os);
  debug_print(p, d.e);
  return os;
}

} // namespace steve
