
#ifndef STEVE_EXTRACT_HPP
#define STEVE_EXTRACT_HPP

#include <steve/String.hpp>

#include <cstdint>

namespace steve {

class Expr;
class Type; 
class Term;
class Stmt;
class Decl;

// Fundamental types
struct Typename_type;
struct Unit_type;
struct Bool_type;
struct Nat_type;
struct Int_type;
struct Char_type;
// Composite types
struct Fn_type;
struct Range_type;
struct Bitfield_type;
struct Record_type;
struct Variant_type;
struct Dep_variant_type;
struct Enum_type;
struct Array_type;
struct Dep_type;
struct Module;
// Intrinsic type functions
struct Net_str_type;
struct Net_seq_type;
// Expressions
struct Unit;
struct Bool;
struct Int;
struct Default;
struct Block;
struct Fn;
struct Builtin;
struct Call;
struct Promo;
struct Pred;
struct Range;
struct Variant;
struct Unary;
struct Binary;
// Declarations
struct Top;
struct Def;
struct Parm;
struct Field;
struct Alt;
struct Enum;
struct Import;
struct Using;

// The Visitor class provides a conventional visitor for the
// abstract syntax of the Steve Programming Language.
//
// TODO: This should really be in the AST file.
struct Visitor {
  void visit(Expr*);

  // Types
  virtual void visit(Type*);
  virtual void visit(Typename_type*) { }
  virtual void visit(Unit_type*) { }
  virtual void visit(Bool_type*) { }
  virtual void visit(Nat_type*) { }
  virtual void visit(Int_type*) { }
  virtual void visit(Char_type*) { }
  virtual void visit(Fn_type*) { }
  virtual void visit(Range_type*) { }
  virtual void visit(Bitfield_type*) { }
  virtual void visit(Record_type*) { }
  virtual void visit(Variant_type*) { }
  virtual void visit(Dep_variant_type*) { }
  virtual void visit(Enum_type*) { }
  virtual void visit(Array_type*) { }
  virtual void visit(Dep_type*) { }
  virtual void visit(Module*) { }
  virtual void visit(Net_str_type*) { }
  virtual void visit(Net_seq_type*) { }
  // Terms
  virtual void visit(Term*);
  virtual void visit(Unit*) { }
  virtual void visit(Bool*) { }
  virtual void visit(Int*) { }
  virtual void visit(Default*) { }
  virtual void visit(Block*) { }
  virtual void visit(Fn*) { }
  virtual void visit(Builtin*) { }
  virtual void visit(Call*) { }
  virtual void visit(Promo*) { }
  virtual void visit(Pred*) { }
  virtual void visit(Range*) { }
  virtual void visit(Variant*) { }
  virtual void visit(Unary*) { }
  virtual void visit(Binary*) { }
  // Statements
  virtual void visit(Stmt*);
  // Declarations
  virtual void visit(Decl*);
  virtual void visit(Top*) { }
  virtual void visit(Def*) { }
  virtual void visit(Parm*) { }
  virtual void visit(Field*) { }
  virtual void visit(Alt*) { }
  virtual void visit(Enum*) { }
  virtual void visit(Import*) { }
  virtual void visit(Using*) { }
};

// An extractor is a function that recursively visits elements
// of a module to generate a fragment of a program. For example,
// An extractor could be used to enumerate top-level types in
// a module, or to generate a C-struct or C++-class for those types,
// or to extract its associated documentation.
struct Extractor {
  virtual void operator()(Expr*) = 0;
};

Extractor* get_extractor(const std::string&);

// -------------------------------------------------------------------------- //
// Lists
//
// TODO: Define a query language that can be used kind of like
// a 'select' statement from SQL. For example what if I want to
// list only type functions, or only type variables, or only non-type
// variables, or all variables whose type is T? 

using List_extraction = std::uint32_t;
// Type/value declarations
static constexpr List_extraction list_variables   = 0x01;
static constexpr List_extraction list_functions   = 0x02;
static constexpr List_extraction list_types       = 0x04;
static constexpr List_extraction list_definitions = 0x07;
// Lexical declarations
static constexpr List_extraction list_imports     = 0x10;

// The declaration extrator enumerates a list of type declarations
// in a module.
struct List_extractor : Extractor, Visitor {
  List_extractor(List_extraction);

  void operator()(Expr*);

  using Visitor::visit;
  void visit(Module*);
  void visit(Def*);

  List_extraction what;
};

} // namespace steve

#include <steve/Extract.ipp>

#endif
