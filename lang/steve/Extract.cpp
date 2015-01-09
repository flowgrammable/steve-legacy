
#include <steve/Extract.hpp>
#include <steve/Ast.hpp>

#include <unordered_map>

namespace steve {

// -------------------------------------------------------------------------- //
// Visitor

void 
Visitor::visit(Expr* e) {
  switch (get_node_class(e->kind)) {
  case type_class: return visit(as<Type>(e));
  case term_class: return visit(as<Term>(e));
  case stmt_class: return visit(as<Stmt>(e));
  case decl_class: return visit(as<Decl>(e));
  default: steve_unreachable("unhandled node class");
  }
}

namespace {

inline void
no_such_visitor(Expr* e) {
  steve_unreachable(format("not defined visitor for '{}'", debug(e)));
}

} // namespace

void
Visitor::visit(Type* t) {
  switch (t->kind) {
  case typename_type: return visit(as<Typename_type>(t));
  case unit_type: return visit(as<Unit_type>(t));
  case bool_type: return visit(as<Bool_type>(t));
  case nat_type: return visit(as<Nat_type>(t));
  case int_type: return visit(as<Int_type>(t));
  case char_type: return visit(as<Char_type>(t));
  case fn_type: return visit(as<Fn_type>(t));
  case range_type: return visit(as<Range_type>(t));
  case bitfield_type: return visit(as<Bitfield_type>(t));
  case record_type: return visit(as<Record_type>(t));
  case variant_type: return visit(as<Variant_type>(t));
  case dep_variant_type: return visit(as<Dep_variant_type>(t));
  case enum_type: return visit(as<Enum_type>(t));
  case array_type: return visit(as<Array_type>(t));
  case dep_type: return visit(as<Dep_type>(t));
  case module_type: return visit(as<Module>(t));
  case net_str_type: return visit(as<Net_str_type>(t));
  case net_seq_type: return visit(as<Net_seq_type>(t));
  default: return no_such_visitor(t);
  }
}

void
Visitor::visit(Term* t) {
  if (Decl* d = as<Decl>(t))
    return visit(d);
  if (Stmt* s = as<Stmt>(t))
    return visit(s);
  switch (t->kind) {
  case unit_term: return visit(as<Unit>(t));
  case bool_term: return visit(as<Bool>(t));
  case int_term: return visit(as<Int>(t));
  case default_term: return visit(as<Default>(t));
  case block_term: return visit(as<Block>(t));
  case fn_term: return visit(as<Fn>(t));
  case call_term: return visit(as<Call>(t));
  case promo_term: return visit(as<Promo>(t));
  case pred_term: return visit(as<Pred>(t));
  case range_term: return visit(as<Range>(t));
  case variant_term: return visit(as<Variant>(t));
  case unary_term: return visit(as<Unary>(t));
  case binary_term: return visit(as<Binary>(t));
  case builtin_term: return visit(as<Builtin>(t));
  default: return no_such_visitor(t);
  }
}

void
Visitor::visit(Stmt* s) {
  if (Decl* d = as<Decl>(s))
    return visit(d);
  return no_such_visitor(s);
}

void
Visitor::visit(Decl* d) {
  switch (d->kind) {
  case top_decl: return visit(as<Top>(d));
  case def_decl: return visit(as<Def>(d));
  case parm_decl: return visit(as<Parm>(d));
  case field_decl: return visit(as<Field>(d));
  case alt_decl: return visit(as<Alt>(d));
  case enum_decl: return visit(as<Enum>(d));
  case import_decl: return visit(as<Import>(d));
  case using_decl: return visit(as<Using>(d));
  default: return no_such_visitor(d);
  }
}

// -------------------------------------------------------------------------- //
// Extractor facilities
//
// FIXME: This is a bit hacky... We'd really like to create these
// on the fly so we can pass configuration information to them. In
// other words, a factory.

namespace {

using Extractor_map = std::unordered_map<std::string, Extractor*>;

Extractor_map ex_ {
  {"list", new List_extractor(list_definitions)},
  {"list.var", new List_extractor(list_variables)},
  {"list.fn", new List_extractor(list_functions)},
  {"list.type", new List_extractor(list_types)},
  {"list.import", new List_extractor(list_imports)}
};

} // namespace

Extractor*
get_extractor(const std::string& name) {
  auto iter = ex_.find(name);
  if (iter == ex_.end())
    return nullptr;
  else
    return iter->second;
}


// -------------------------------------------------------------------------- //
// List extractor

void
List_extractor::operator()(Expr* e) { visit(e); }

void
List_extractor::visit(Module* m) {
  for (Decl* d : *m->decls())
    visit(d);
}

// TODO: Filter the extraction based on the user selection.
void
List_extractor::visit(Def* d) {
  std::cout << debug(d->name()) << '\n';
}

} // namespace steve
