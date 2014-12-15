
#ifndef STEVE_SYNTAX_HPP
#define STEVE_SYNTAX_HPP

#include <steve/Node.hpp>
#include <steve/Token.hpp>

namespace steve {

// Expressions
constexpr Node_kind id_tree      = make_tree_node(1);  // identfiers
constexpr Node_kind lit_tree     = make_tree_node(2);  // literals
constexpr Node_kind call_tree    = make_tree_node(10); // e1(e*)
constexpr Node_kind index_tree   = make_tree_node(11); // e1[e2]
constexpr Node_kind dot_tree     = make_tree_node(12); // e1.e2
constexpr Node_kind range_tree   = make_tree_node(13); // e1..e2
constexpr Node_kind app_tree     = make_tree_node(14); // e1 e2
constexpr Node_kind unary_tree   = make_tree_node(15); // op e
constexpr Node_kind binary_tree  = make_tree_node(16); // e1 op e2
// Types
constexpr Node_kind record_tree  = make_tree_node(20); // record { ... }
constexpr Node_kind variant_tree = make_tree_node(21); // variant { ... }
constexpr Node_kind enum_tree    = make_tree_node(22); // enum { ... }
// Statements and declarations
constexpr Node_kind value_tree   = make_tree_node(30); // x : T (in def)
constexpr Node_kind parm_tree    = make_tree_node(31); // x : T (in parms)
constexpr Node_kind fn_tree      = make_tree_node(32); // f(p)->T (in def)
constexpr Node_kind def_tree     = make_tree_node(33); // def n = e
constexpr Node_kind field_tree   = make_tree_node(34); // x : T where (in record)
constexpr Node_kind pad_tree     = make_tree_node(35); // FIXME: remove?
constexpr Node_kind alt_tree     = make_tree_node(36); // e1 : e2 (in variant)
constexpr Node_kind import_tree  = make_tree_node(50); // import e
// Misc
constexpr Node_kind top_tree     = make_tree_node(100);

// The base class of all tree nodes.
struct Tree : Node { 
  using Node::Node; 
};

// Sequences of trees.
using Tree_seq = Seq<Tree>;


// An identifier.
struct Id_tree : Tree, Kind_of<id_tree> {
  Id_tree(const Token* k) 
    : Tree(Kind, k->loc), first(k) { }

  const Token* value() const { return first; }

  const Token* first;
};

// A literal.
struct Lit_tree : Tree, Kind_of<lit_tree> {
  Lit_tree(const Token* k)
    : Tree(Kind, k->loc), first(k) { }

  const Token* value() const { return first; }

  const Token* first;
};

// A call expression of the form 'f(as*)' where 'f' is a function
// and 'as*' is a sequence of arguments.
struct Call_tree : Tree, Kind_of<call_tree> {
  Call_tree(Tree* f, Tree_seq* as)
    : Tree(Kind, f->loc), first(f), second(as) { }

  Tree* fn() const { return first; }
  Tree_seq* args() const { return second; }

  Tree* first;
  Tree_seq* second;
};

// An index (or subscript) tree of the form `e1[e2]`.
struct Index_tree : Tree, Kind_of<index_tree> {
  Index_tree(Tree* e1, Tree* e2)
    : Tree(Kind, e1->loc), first(e1), second(e2) { }

  Tree* elem() const { return first; }
  Tree* index() const { return second; }

  Tree* first;
  Tree* second;
};

// A projection or access expression of the form `e1.e2`. These
// expressions are used to access a nested member `e2`, nested 
// within the scope of `e1`.
struct Dot_tree : Tree, Kind_of<dot_tree> {
  Dot_tree(Tree* e1, Tree* e2)
    : Tree(Kind, e1->loc), first(e1), second(e2) { }

  Tree* scope() const { return first; }
  Tree* member() const { return second; }

  Tree* first;
  Tree* second;
};

// An expression of the form `e1..e2`.
struct Range_tree : Tree, Kind_of<range_tree> {
  Range_tree(Tree* l, Tree* u)
    : Tree(Kind, l->loc), first(l), second(u) { }

  Tree* lower() const { return first; }
  Tree* upper() const { return second; }

  Tree* first;
  Tree* second;
};

// An application expression of the form 'e1 e2'. Application is
// distinct from function call, and primarily used to denote
// value construction (i.e., when e1 is a type and e2 is a term).
struct App_tree : Tree, Kind_of<app_tree> {
  App_tree(Tree* e1, Tree* e2)
    : Tree(Kind, e1->loc), first(e1), second(e2) { }

  Tree* fn() const { return first; }
  Tree* arg() const { return second; }

  Tree* first;
  Tree* second;
};

// An overloadable unary operator.
struct Unary_tree : Tree, Kind_of<unary_tree> {
  Unary_tree(const Token* k, Tree* t)
    : Tree(Kind, k->loc), first(k), second(t) { }

  const Token* op() const { return first; }
  Tree* arg() const { return second; }

  const Token* first;
  Tree* second;
};

// An overloadable binary operator.
struct Binary_tree : Tree, Kind_of<binary_tree> {
  Binary_tree(const Token* k, Tree* l, Tree* r)
    : Tree(Kind, l->loc), first(k), second(l), third(r) { }

  const Token* op() const { return first; }
  Tree* left() const { return second; }
  Tree* right() const { return third; }

  const Token* first;
  Tree* second;
  Tree* third;
};


// A record type of the form 'record { f* }' where 'f*' is a sequence
// of fields.
struct Record_tree : Tree, Kind_of<record_tree> {
  Record_tree(const Token* k, Tree_seq* fs)
    : Tree(Kind, k->loc), first(fs) { }

  Tree_seq* fields() const { return first; }

  Tree_seq* first;
};

// A variant type of the form 'variant(T) { c* }' where 'T' is an
// (optional) descriminator type and 'c*' is a sequence of alternatives.
struct Variant_tree : Tree, Kind_of<variant_tree> {
  Variant_tree(const Token* k, Tree* d, Tree_seq* as)
    : Tree(Kind, k->loc), first(d), second(as) { }

  Tree* desc() const { return first; }
  Tree_seq* alts() const { return second; }

  Tree* first;
  Tree_seq* second;
};

// An enumeration type of the form 'record { v* }' where 'v*' is
// a sequence of values (possibly having names).
struct Enum_tree : Tree, Kind_of<enum_tree> {
  Enum_tree(const Token* k, Tree* t, Tree_seq* es)
    : Tree(Kind, k->loc), first(t), second(es) { }

  Tree* base() const { return first; }
  Tree_seq* ctors() const { return second; }

  Tree* first;
  Tree_seq* second;
};

// A value declarator of the form 'n : t'.
struct Value_tree : Tree, Kind_of<value_tree> {
  Value_tree(Tree* n, Tree* t)
    : Tree(Kind, n->loc), first(n), second(t) { }

  Tree* name() const { return first; }
  Tree* type() const { return second; }

  Tree* first;
  Tree* second;
};

// A parameter declaration of the form 'n : t = e'.
struct Parm_tree : Tree, Kind_of<parm_tree> {
  Parm_tree(Tree* n, Tree* t, Tree* e)
    : Tree(Kind, n->loc), first(n), second(t) { }

  Tree* name() const { return first; }
  Tree* type() const { return second; }
  Tree* def() const { return third; }

  Tree* first;
  Tree* second;
  Tree* third;
};

// A function declarator of the form 'n (p*) -> t'.
struct Fn_tree : Tree, Kind_of<fn_tree> {
  Fn_tree(Tree* n, Tree_seq* ps, Tree* t)
    : Tree(Kind, n->loc), first(n), second(ps), third(t) { }

  Tree* name() const { return first; }
  Tree_seq* parms() const { return second; }
  Tree* result() const { return third; }

  Tree*     first;
  Tree_seq* second;
  Tree*     third;
};

// A definition.
struct Def_tree : Tree, Kind_of<def_tree> {
  Def_tree(const Token* k, Tree* d, Tree* e)
    : Tree(Kind, k->loc), first(d), second(e) { }

  Tree* decl() const { return first; }
  Tree* init() const  { return second; }

  Tree* first;
  Tree* second;
};

// A field of the form 'n : t | c'
struct Field_tree : Tree, Kind_of<field_tree> {
  Field_tree(Tree* n, Tree* t, Tree* c)
    : Tree(Kind, n->loc), first(n), second(t), third(c) { }

  Tree* name() const { return first; }
  Tree* type() const { return second; }
  Tree* prop() const { return third; }

  Tree* first;
  Tree* second;
  Tree* third;
};

// An padding field of the form ': t'. This is kept separate
// from values since it has neither a name nor a constraint.
struct Pad_tree : Tree, Kind_of<pad_tree> {
  Pad_tree(const Token* k, Tree* t)
    : Tree(Kind, k->loc), first(t) { }

  Tree* type() const { return first; }

  Tree* first;
};

// An alternative in a variant type of the form 't : T' where 't'
// is the term or tag of the field and 'T' is its corresponding type.
struct Alt_tree : Tree, Kind_of<alt_tree> {
  Alt_tree(Tree* t1, Tree* t2)
    : Tree(Kind, t1->loc), first(t1), second(t2) { }

  Tree* tag() const { return first; }
  Tree* type() const { return second; }

  Tree* first;
  Tree* second;
};

// A declaration of the form `import e`.
struct Import_tree : Tree, Kind_of<import_tree> {
  Import_tree(const Token* k, Tree* t)
    : Tree(Kind, k->loc), first(t) { }

  Tree* module() const { return first; }

  Tree* first;
};

// Represents a top-level sequence of declarations.
struct Top_tree : Tree, Kind_of<top_tree> {
  Top_tree(Tree_seq* ds)
    : Tree(Kind), first(ds) { }
  Tree_seq* first;
};


// -------------------------------------------------------------------------- //
// Printing

// Streaming
struct debug_tree { Tree* e; };
debug_tree debug(Tree*);

std::ostream& operator<<(std::ostream&, debug_tree);

} // namespace steve

#include <steve/Syntax.ipp>

#endif
