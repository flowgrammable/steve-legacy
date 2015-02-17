
#ifndef STEVE_AST_HPP
#define STEVE_AST_HPP

#include <steve/Debug.hpp>
#include <steve/Node.hpp>
#include <steve/String.hpp>
#include <steve/Integer.hpp>
#include <steve/File.hpp>

#include <iostream>

namespace steve {

// FIXME: Decl-id should not exist. Identifiers should resolve
// during elaboration to their named declaraitons.

// Names
constexpr Node_kind basic_id          = make_name_node(1);  // id
constexpr Node_kind operator_id       = make_name_node(2);  // operator <op>
constexpr Node_kind scoped_id         = make_name_node(3);  // scope.id
constexpr Node_kind decl_id           = make_name_node(10); // x, referring to a decl
// Types
constexpr Node_kind typename_type     = make_type_node(1);  // typename
constexpr Node_kind unit_type         = make_type_node(2);  // unit
constexpr Node_kind bool_type         = make_type_node(3);  // bool
constexpr Node_kind nat_type          = make_type_node(4);  // nat
constexpr Node_kind int_type          = make_type_node(5);  // int
constexpr Node_kind char_type         = make_type_node(6);  // int
constexpr Node_kind fn_type           = make_type_node(7);  // (T*) -> T
constexpr Node_kind range_type        = make_type_node(8);  // .. T
constexpr Node_kind bitfield_type     = make_type_node(9);  // bitfield(T, N, B)
constexpr Node_kind record_type       = make_type_node(10); // record { ... }
constexpr Node_kind variant_type      = make_type_node(11); // variant { ... }
constexpr Node_kind dep_variant_type  = make_type_node(12); // variant(d) { ... }
constexpr Node_kind enum_type         = make_type_node(13); // enum { ... }
constexpr Node_kind array_type        = make_type_node(14); // T[n]
constexpr Node_kind dep_type          = make_type_node(30); // f(args) -> typename
constexpr Node_kind module_type       = make_type_node(50); // module
// Intrinsic types for networking
constexpr Node_kind net_str_type      = make_type_node(100);  // __net_str(n)
constexpr Node_kind net_seq_type      = make_type_node(101);  // __net_seq(t, p)
// Terms
constexpr Node_kind unit_term      = make_term_node(1);  // ()
constexpr Node_kind bool_term      = make_term_node(2);  // {true, false}
constexpr Node_kind int_term       = make_term_node(3);  // {0, 1, 2, ..., n}
constexpr Node_kind default_term   = make_term_node(4);  // default
constexpr Node_kind if_term        = make_term_node(5);  // if t1 then t2 else t3
// Misc terms
constexpr Node_kind fn_term        = make_term_node(21); // fn(p*)->t.e
constexpr Node_kind call_term      = make_term_node(22); // f(a*) -- TODO: Should be in values
constexpr Node_kind promo_term     = make_term_node(23); // t as N for an integral type
constexpr Node_kind pred_term      = make_term_node(24); // t as B for a boolean type
constexpr Node_kind range_term     = make_term_node(28); // t1 .. t2
constexpr Node_kind variant_term   = make_term_node(30); // <e:t> (value of a variant)
constexpr Node_kind unary_term     = make_term_node(40); // op t
constexpr Node_kind binary_term    = make_term_node(41); // t1 op t2
constexpr Node_kind builtin_term   = make_term_node(50); // <intrinsic>
// Statements
constexpr Node_kind block_stmt     = make_stmt_node(1); // { s1; s2; ...}
constexpr Node_kind return_stmt    = make_stmt_node(2); // return s;
constexpr Node_kind while_stmt     = make_stmt_node(3); // while (e) s
constexpr Node_kind continue_stmt  = make_stmt_node(4); // continue;
constexpr Node_kind break_stmt     = make_stmt_node(5); // break;
constexpr Node_kind switch_stmt    = make_stmt_node(6); // switch (e) s
constexpr Node_kind case_stmt      = make_stmt_node(7); // case e: s
// Declarations
constexpr Node_kind top_decl       = make_decl_node(1); // decl*
constexpr Node_kind def_decl       = make_decl_node(2); // def n : t = e
constexpr Node_kind parm_decl      = make_decl_node(3); // n : t = e
constexpr Node_kind field_decl     = make_decl_node(4); // n : t | c
constexpr Node_kind alt_decl       = make_decl_node(5); // n : t
constexpr Node_kind enum_decl      = make_decl_node(6); // n [= t] (in an enum)
constexpr Node_kind import_decl    = make_decl_node(7); // import n;
constexpr Node_kind using_decl     = make_decl_node(8); // using n;


// -------------------------------------------------------------------------- //
// Abstract terms

struct Expr;
struct Name;
struct Type;
struct Term;
struct Stmt;
struct Decl;

// Every phrase in the steve language is an expression. This type
// serves to distinguish utility nodes from phrases in the language.
//
// Note that there is no node class corresponding to this type.
// It is simply the union of all steve expression nodes.
//
// Every expression (except names) has an associated type. This is 
// computed and assigned during elaboration. 
//
// Additionally, each expression optionally tracks an originating 
// declaration, which is also assigned during elaboration. Note that the 
// originating declaration is always a definition, and is primarily used 
// to improve diagnostics by allowing details to be elided with a name.
//
// TODO: If a term or type is derived as the evaluation of a function,
// we should also track the originating calls. Right?
//
// TODO: Design a facility for creating programs that do not require
// elaboration to assign the type of a node. Note that this could easily
// be done by using the elaborator, and then asserting if the resulting
// diagnostics indicate failure.
//
// TODO: Allow every term and type (but not a name) to be constructed 
// with a type.
struct Expr : Node {
  using Node::Node;

  Type* tr = nullptr; // The cached type or kind of the node.
  Decl* od = nullptr; // An originating declaration for the expression
};

// A name designates a declaration.
struct Name : Expr {
  using Expr::Expr;
};

// A type expression denotes a type, describing a set of values.
struct Type : Expr {
  using Expr::Expr;
};

// A term denotes a computable value.
struct Term : Expr {
  using Expr::Expr;
};

// A statement is a term whose result is computed and discarded.
struct Stmt : Term {
  using Term::Term;
};

// A declaration is a statement that introduces an entity into
// a declaration, typing, or logical context.
struct Decl : Stmt {
  using Stmt::Stmt;
};

// Helper types
using Expr_seq = Seq<Expr>;
using Type_seq = Seq<Type>;
using Term_seq = Seq<Term>;
using Stmt_seq = Seq<Stmt>;
using Decl_seq = Seq<Decl>;


// -------------------------------------------------------------------------- //
// Names

// A basic-id is a simple identifier that designates a declaration.
struct Basic_id : Name, Kind_of<basic_id> {
  Basic_id(String n) 
    : Name(Kind), first(n) { }
  Basic_id(const Location& l, String n) 
    : Name(Kind, l), first(n) { }

  String value() const { return first; }

  String first;
};

// An opeator-id is an identifier that designates an overloaded
// operator. The operator is rendered as a string.
//
// TODO: It might be better to enumerate the set of overloadable
// operators and define this in terms of those. Or maybe not...
struct Operator_id : Name, Kind_of<operator_id> {
  Operator_id(String n) 
    : Name(Kind), first(n) { }
  Operator_id(const Location& l, String n) 
    : Name(Kind, l), first(n) { }

  String op() const { return first; }

  String first;
};

// A name that refers to a declaration of some kind. During elaboration,
// names are linked to the declarations to which they refer. Note that
// the local name is kept for diagnostics.
struct Decl_id : Name, Kind_of<decl_id> {
  Decl_id(Name* n, Decl* d) 
    : Name(Kind), first(n), second(d) { }
  Decl_id(const Location& l, Name* n, Decl* d) 
    : Name(Kind, l), first(n), second(d) { }

  Name* name() const { return first; }
  Decl* decl() const { return second; }

  Name* first;
  Decl* second;
};



// -------------------------------------------------------------------------- //
// Types

// The typename kind denotes the type of a type.
struct Typename_type : Type, Kind_of<typename_type> {
  Typename_type() 
    : Type(Kind) { }
  Typename_type(const Location& l) 
    : Type(Kind, l) { }
};

// The unit type describes a set with a single, unspecified value.
struct Unit_type : Type, Kind_of<unit_type> {
  Unit_type() 
    : Type(Kind) { }
  Unit_type(const Location& l) 
    : Type(Kind, l) { }
};

// The bool type represents the boolean values true and false.
struct Bool_type : Type, Kind_of<bool_type> {
  Bool_type() 
    : Type(Kind) { }
  Bool_type(const Location& l) 
    : Type(Kind, l) { }
};

// The nat type represents the set of unsigned integer values that
// are most efficiently represented on the host architecture. This
// is the same as "unsigned int" for C and C++.
struct Nat_type : Type, Kind_of<nat_type> {
  Nat_type() 
    : Type(Kind) { }
  Nat_type(const Location& l) 
    : Type(nat_type, l) { }
};

// The int type represents the set of signed integer values that
// are most efficiently represented on the host architecture. This
// is the same as "int" for C and C++.
struct Int_type : Type, Kind_of<int_type> {
  Int_type() 
    : Type(Kind) { }
  Int_type(const Location& l) 
    : Type(Kind, l) { }
};

// The character type represents a set of character values.
//
// FIXME: What are the actual properties of this type's representation?
// Is it an 8-bit character? 16-bit? System-specific? How would
// I define 16 or 32 bit characters? Like this: bitfield(char, 16)?
struct Char_type : Type, Kind_of<char_type> {
  Char_type() 
    : Type(Kind) { }
  Char_type(const Location& l)
    : Type(Kind, l) { }
};

// The function type (t1, t2, ...) -> t represents the type of a 
// function mapping from input types t1, t2, ... to the output
// type t.
struct Fn_type : Type, Kind_of<fn_type> {
  Fn_type(Type_seq* ts, Type* t)
    : Type(Kind), first(ts), second(t) { }
  Fn_type(const Location& l, Type_seq* ts, Type* t)
    : Type(Kind, l), first(ts), second(t) { }

  Type_seq* parms() const { return first; }
  Type* result() const { return second; }

  Type_seq* first;
  Type*     second;
};

// The range type '..T' is the type of a range term t1..t2.
struct Range_type : Type, Kind_of<range_type> {
  Range_type(Type* t)
    : Type(Kind), first(t) { }
  Range_type(const Location& l, Type* t)
    : Type(Kind, l), first(t) { }

  Type* type() const { return first; }

  Type* first;
};

// The bitfield type of the form 'bits(b, n, o)' where 'b' is the base
// type (one of bool, nat, int, or char), 'n' is the number of bits 
// allocated to the bitfield, and o is a boolean flag determining
// the byte-ordering of the bitfield representation.
//
// FIXME: Rename to binary.
struct Bitfield_type : Type, Kind_of<bitfield_type> {
  Bitfield_type(Type* b, Term* n, Term* o) 
    : Type(Kind), first(b), second(n), third(o) { }
  Bitfield_type(const Location& l, Type* b, Term* n, Term* o) 
    : Type(Kind, l), first(b), second(n), third(o) { }

  Type* type() const { return first; }
  Term* width() const { return second; }
  Term* order() const { return third; }

  Type* first;
  Term* second;
  Term* third;
};

// A record type of the form 'record { f* }' where 'f*' is a sequence
// of fields. 
struct Record_type : Type, Kind_of<record_type> {
  Record_type(Decl_seq* f) 
    : Type(Kind), first(f) { }
  Record_type(const Location& l, Decl_seq* f) 
    : Type(Kind, l), first(f) { }

  Decl_seq* field() const { return first; }

  Decl_seq* first;
};

// A variant type of the for `variant { a }` where `a` is a sequence
// of alternatives. A variant type provides a mechanism for conveying
// a single value of a set of types, which can be unwrapped at a later
// time. Each alternative defines a tag which constructs a wrapped
// version of the variant. For example:
//
//    def V:typename = variant { b:bool; n:nat;  z:int; }
//
//    def v1:V = true; // v1 has value <b=true>
//    def v2:V = 0;    // v2 has value <z=0>
struct Variant_type : Type, Kind_of<variant_type> {
  Variant_type(Decl_seq* as) 
    : Type(Kind), first(as) { }
  Variant_type(const Location& l, Decl_seq* as) 
    : Type(Kind, l), first(as) { }

  Decl_seq* vars() const { return first; }

  Decl_seq* first;
};

// A dependent variant type has the form `variant(d) { ... }` where 'd' 
// is a parameter whose values discriminate the variants of the type.
// For example:
//
//    def V(n : nat) -> typename = variant(n) { 
//        0 : bool; 
//        1 : nat; 
//        2 : int; 
//        default : unit; 
//    }
//
// Each variant within the type has the form `e:t` where `e` is some
// value of the variant's dependent parameter.
//
// When constructing a variant of this type, a discriminator must be
// given or deduced. For example:
//
//    def v1:V(0) = true; // has value <0=true>
//    def v2:V = 0;       // deduced value <2=0>
//    def v3:V = nat 0;   // deduced value <1=0>
//
// Note that binding a dependent variant to a field of a record is
// also allowed.
//
//     record R { 
//       type : nat; 
//       value : V(type); // OK: value discriminated by type
//     }
//
// Here, `value` has dependent variant type. That is, the discriminator
// depends on the value of `type`. All operations on `value` are
// require that value as input.
//
// TODO: What kinds of types can we use as the discriminator?
struct Dep_variant_type : Type, Kind_of<dep_variant_type>  {
   Dep_variant_type(Decl* t, Decl_seq* as) 
     : Type(Kind), first(t), second(as) { }
   Dep_variant_type(const Location& l, Decl* t, Decl_seq* as) 
    : Type(Kind, l), first(t), second(as) { }
  
  Decl* arg() const { return first; }
  Decl_seq* alts() const { return second; }
  
  Decl* first;
  Decl_seq* second;
};

// The enum-of type of the form 'enum (b) { cs* }' represents a set 
// of values of some specified underlying type 'b' (also called the 
// base type) whose constructors are given by the list 'cs*'.  For 
// example:
//
//    def Color = record { value : array(uint(8), 3); }
//
//    def Named_color = enum(Color) { 
//      Red   = {0xff, 0, 0},
//      Green = {0, 0xff, 0},
//      Blue  = {0, 0, 0xff}
//    }
//
// Here, the Named_color type is a refinement of Color whose values
// are only those with named constructors. An enumeration of a specific
// type can also include ranges of such values.
//
// In addition to named constructors, the enumeration may also include
// ranges of values.
//
//    def Port = { 
//      0 .. 0xffef, 
//      Forward_port   = 0xff01, 
//      Broadcast_port = 0xff02 
//    }
//
// Range constructors are unnamed, but allow the construction of Port
// values within that range (e.g., 'Port 5' constructs a port with
// value 5.
//
// By default, the underlying type is int.
struct Enum_type : Type, Kind_of<enum_type> {
  Enum_type(Type* b, Expr_seq* cs) 
    : Type(Kind), first(b), second(cs) { }
  Enum_type(const Location& l, Type* b, Expr_seq* cs) 
    : Type(Kind, l), first(b), second(cs) { }

  Type* base() const { return first; }
  Expr_seq* ctors() const { return second; }

  Type* first;
  Expr_seq* second;
};

// An array type is the type of a fixed-length sequence of values and
// has the form `T[N]` where `T` is a type and `N` is an integer value.
//
// Note that array types can compound: `T[M][N]` is an `N` dimensional
// array whose elements are arrays of `M` elements of `T`.
struct Array_type : Type, Kind_of<array_type> {
  Array_type(Type* t, Term* n)
    : Type(Kind), first(t), second(n) { }
  Array_type(const Location& l, Type* t, Term* n)
    : Type(Kind, l), first(t), second(n) { }

  Type* elem() const { return first; }
  Term* bound() const { return second; }

  Type* first;
  Term* second;
};

// A dependent type is a type function that associates a sequence
// of runtime values with a type. This allows certain properties
// of those types to be deferred until runtime. For example:
//
//    def Vec : typename = record {
//      size : nat;
//      elems : array(double, size);
//    }
//
// Here, the type of elems depends on whatever value size is
// given during initialization.
//
// Dependencies are resolved as needed. This generally happens during
// code generation. In the case of code generation for C++, the incomplete
// array type and the size would be passed as a pair to every function
// to which elems is passed as an argument.
//
// Note that the dependent type is the function definition that
// computes the completed typed.
struct Dep_type : Type, Kind_of<dep_type> {
  Dep_type(Term* t, Expr_seq* a)
    : Type(Kind), first(t), second(a) { }
  Dep_type(const Location& l, Term* t, Expr_seq* a)
    : Type(Kind, l), first(t), second(a) { }

  Term* fn() const { return first; }
  Expr_seq* args() const { return second; }

  Term* first;
  Expr_seq* second;
};

// A module is a type that contains a sequence of declarations. Each module
// defines its own distinct type.
//
// TODO: Consider having a different sequence of declarations for module
// imports. 
struct Module : Type, Kind_of<module_type> {
  Module(const Path& p, Name* n, Decl_seq* e)
    : Type(Kind), path_(p), first(n), second(e) { }
  Module(const Location& l, const Path& p, Name* n, Decl_seq* e)
    : Type(Kind, l), path_(p), first(n), second(e) { }

  const Path path() const { return path_; }
  Name* name() const { return first; }
  Decl_seq* decls() const { return second; }
  
  Path path_;
  Name* first;
  Decl_seq* second;
};


// -------------------------------------------------------------------------- //
// Intrinsic networking types
//
// The following types and type constructors are used to support the
// definition of various networking protocols. They should be library
// supported, but for now, they are internal.

// The network string type is a fixed-sized, null-terminated string.
struct Net_str_type : Type, Kind_of<net_str_type> {
  Net_str_type(Term* n)
    : Type(Kind), first(n) { }
  Net_str_type(const Location& l, Term* n)
    : Type(Kind, l), first(n) { }

  Term* size() const { return first; }

  Term* first;
};

// The sequence type constructor defines a sequence of elements of
// some type t. A predicate function determines the terminator for
// the sequence.
struct Net_seq_type : Type, Kind_of<net_seq_type> {
  Net_seq_type(Type* t, Term* p)
    : Type(Kind), first(t), second(p) { }
  Net_seq_type(const Location& l, Type* t, Term* p)
    : Type(Kind, l), first(t), second(p) { }

  Type* type() const { return first; }
  Term* pred() const { return second; }

  Type* first;
  Term* second;
};


// -------------------------------------------------------------------------- //
// Terms


// The unit_t type and its sole enumerator unit denote the value
// and constructor of a unit value.
enum unit_t { unit };

// The unit literal.
struct Unit : Term, Kind_of<unit_term> {
  Unit() 
    : Term(Kind) { }
  Unit(const Location& l) 
    : Term(Kind, l) { }

  unit_t value() const { return unit; }
};

// A boolean literal.
struct Bool : Term, Kind_of<bool_term> {
  Bool(bool b) 
    : Term(Kind), first(b) { }
  Bool(const Location& l, bool b) 
    : Term(Kind, l), first(b) { }

  bool value() const { return first; }

  bool first;
};

// An integer literal.
struct Int : Term, Kind_of<int_term> {
  Int(Integer n) 
    : Term(Kind), first(n) { }
  Int(const Location& l, Integer n) 
    : Term(Kind, l), first(n) { }

  const Integer& value() const { return first; }

  Integer first;
};

// The default term is a placeholder for an expression to
// instantiated by various language rules or some other synthesis
// mechanism.
struct Default : Term, Kind_of<default_term> {
  Default() 
    : Term(Kind) { }
  Default(const Location& l) 
    : Term(Kind, l) { }
};

// An anonymous function of the form \(p*)->t.e where p* is
// a sequence of parameters, t is the result type and e is its
// definition.
struct Fn : Term, Kind_of<fn_term> {
  Fn(Decl_seq* p, Type* t, Expr* e) 
    : Term(Kind), first(p), second(t), third(e) { }
  Fn(const Location& l, Decl_seq* p, Type* t, Expr* e) 
    : Term(Kind, l), first(p), second(t), third(e) { }

  Decl_seq* parms() const { return first; }
  Type* result() const { return second; }
  Expr* body() const { return third; }

  Decl_seq* first;
  Type* second;
  Expr* third;
};

// A builtin function represented as an expression.
struct Builtin : Term, Kind_of<builtin_term> {
  using Unary = Expr* (*)(Expr*);
  using Binary = Expr* (*)(Expr*, Expr*);
  using Ternary = Expr* (*)(Expr*, Expr*, Expr*);
  
  union Fn {
    Fn(Unary f) : f1(f) { }
    Fn(Binary f) : f2(f) { }
    Fn(Ternary f) : f3(f) { }
    
    Unary f1;
    Binary f2;
    Ternary f3;
  };

  Builtin(int a, Fn f)
    : Term(Kind), first(a), second(f) { }
  Builtin(Unary f)
    : Term(Kind), first(1), second(f) { }
  Builtin(Binary f)
    : Term(Kind), first(2), second(f) { }
  Builtin(Ternary f)
    : Term(Kind), first(3), second(f) { }

  int arity() const { return first; }
  Fn fn() const { return second; }

  int first;
  Fn second;
};


// A function call of the form `f(a*)` where `f` is the target
// function (or overload set) and 'a*' is a sequence of arguments.
// In either case, note that the target of a call is a declaration.
//
// Note that a call target can also be a type function.
struct Call : Term, Kind_of<call_term> {
  Call(Term* f, Expr_seq* as)
    : Term(Kind), first(f), second(as) { }
  Call(const Location& l, Term* f, Expr_seq* as)
    : Term(Kind, l), first(f), second(as) { }

  Term* fn() const { return first; }
  Expr_seq* args() const { return second; }

  Term* first;
  Expr_seq* second;
};

// An integer promotion from one type to another. 
struct Promo : Term, Kind_of<promo_term> {
  Promo(Expr* e, Type* t)
    : Term(Kind), first(e), second(t) { }
  Promo(const Location& l, Expr* e, Type* t)
    : Term(Kind, l), first(e), second(t) { }

  Expr* expr() const { return first; }
  Type* type() const { return second; }

  Expr* first;
  Type* second;
};

// An evaluation of a term as a boolean value. Note that the resulting
// boolean type can also be a boolean bitfield.
struct Pred : Term, Kind_of<pred_term> {
  Pred(Expr* e, Type* t)
    : Term(Kind), first(e), second(t) { }
  Pred(const Location& l, Expr* e, Type* t)
    : Term(Kind, l), first(e), second(t) { }

  Expr* expr() const { return first; }
  Type* type() const { return second; }

  Expr* first;
  Type* second;
};


// A range of the form 't1..t2'. Note that t1 and t2 must be 
// constant values
struct Range : Term, Kind_of<range_term> {
  Range(Term* t1, Term* t2)
    : Term(Kind), first(t1), second(t2) { }
  Range(const Location& l, Term* t1, Term* t2)
    : Term(Kind, l), first(t1), second(t2) { }

  Term* lower() const { return first; }
  Term* upper() const { return second; }

  Term* first;
  Term* second;
};

// A variant term binds together a labeling expression and
// a value of some type. These are the values of variant types.
struct Variant : Term, Kind_of<variant_term> {
  Variant(Expr* e, Term* v)
    : Term(Kind), first(e), second(v) { }
  Variant(const Location& l, Expr* e, Term* v)
    : Term(Kind), first(e), second(v) { }

  Expr* label() const { return first; }
  Term* value() const { return second; }

  Expr* first;
  Term* second;
};

// A unary term represents a unary expression whose operator is
// overloaded. Note that arguments are expressions, allowing
// for operations on types.
struct Unary : Term, Kind_of<unary_term> {
  Unary(Decl* op, Expr* t) 
    : Term(Kind), first(op), second(t) { }
  Unary(const Location& l, Decl* op, Expr* t) 
    : Term(Kind, l), first(op), second(t) { }

  Decl* op() const { return first; }
  Expr* arg() const { return second; }

  Decl* first;
  Expr* second;
};

// A binary term represents a binary expression whose operator is
// overloaded. Note that arguments are expressions, allowing
// for operations on types.
struct Binary : Term, Kind_of<binary_term> {
  Binary(Term* f, Expr* t1, Expr* t2) 
    : Term(Kind), first(f), second(t1), third(t2) { }
  Binary(const Location& l, Term* f, Expr* t1, Expr* t2) 
    : Term(Kind, l), first(f), second(t1), third(t2) { }

  Term* fn() const { return first; }
  Expr* left() const { return second; }
  Expr* right() const { return third; }

  Term* first;
  Expr* second;
  Expr* third;
};

// A term of the form `e1 if t else e2`.
//
// The term `t` shall be be convertible to `bool`, and types of `e1` and 
// `e2` shall be the same.
//
// If the converted term `t` is `true`, then its evaluation of this
// term is the value of `e1`, otherwise it is the value of `e2`.
//
// TODO: Relax the same-type constraints on these terms.
struct If : Term, Kind_of<if_term> {
  If(Term* c, Expr* t, Expr* f)
    : Term(Kind), first(c), second(t), third(f) { }
  If(const Location& l, Term* c, Expr* t, Expr* f)
    : Term(Kind, l), first(c), second(t), third(f) { }

  Term* cond() const { return first; }
  Expr* pass() const { return second; }
  Expr* fail() const { return third; }

  Term* first;
  Expr* second;
  Expr* third;
};

// -------------------------------------------------------------------------- //
// Statement

// A compound expression of the form `{ s1; s2; ... }` where each
// `si` is a statement. Note that the sequence of statements may
// be empty.
struct Block : Stmt, Kind_of<block_stmt> {
  Block(Stmt_seq* s)
    : Stmt(Kind), first(s) { }
  Block(const Location& l, Stmt_seq* s)
    : Stmt(Kind, l), first(s) { }

  Stmt_seq* stmts() const { return first; }

  Stmt_seq* first;
};


// A return statement of the form `return e` where `e` is an 
// expression. A return statement shall appear only within the
// block scope of the definition of a function `f`.
//
// The return value `e` shall be convertible to the return type
// of `f`.
struct Return : Stmt, Kind_of<return_stmt> {
  Return(Expr* t)
    : Stmt(Kind), first(t) { }
  Return(const Location& l, Expr* t)
    : Stmt(Kind, l), first(t) { }
  
  Expr* value() const { return first; }

  Expr* first;
};

struct While : Stmt, Kind_of<while_stmt> {
  While(Term* c, Stmt* b)
    : Stmt(Kind), first(c), second(b) { }
  While(const Location& l, Term* c, Stmt* b)
    : Stmt(Kind, l), first(c), second(b) { }

  Term* cond() const { return first; }
  Stmt* body() const { return second; }

  Term* first;
  Stmt* second;
};

struct Break : Stmt, Kind_of<break_stmt> {
  Break()
    : Stmt(Kind) { }
  Break(const Location& l)
    : Stmt(Kind, l) { }
};

struct Continue : Stmt, Kind_of<continue_stmt> {
  Continue()
    : Stmt(Kind) { }
  Continue(const Location& l)
    : Stmt(Kind, l) { }
};

// A switch statement has the form `switch (e) s` where `e` is an
// expression and `s` is a statement.
//
// TODO: Should we allow the full range of expression in C/C++, or
// should we restrict this to a list of cases?
struct Switch : Stmt, Kind_of<switch_stmt> {
  Switch(Expr* t, Stmt* b)
    : Stmt(Kind), first(t), second(b) { }
  Switch(const Location& l, Expr* t, Stmt* b)
    : Stmt(Kind, l), first(t), second(b) { }

  Expr* value() const { return first; }
  Stmt* body() const { return second; }

  Expr* first;
  Stmt* second;
};

// Within a switch statement `switch(e1)`, a case statement
// of the form `case e2:e3` associates the value `e3` with a 
// case value `e2`.
//
// The case value `e2` shall be convertible to the switch 
// expression `e1`.
struct Case : Stmt, Kind_of<case_stmt> {
  Case(Expr* t, Expr* b)
    : Stmt(Kind), first(t), second(b) { }
  Case(const Location& l, Expr* t, Expr* b)
    : Stmt(Kind, l), first(t), second(b) { }

  Expr* match() const { return first; }
  Expr* result() const { return first; }

  Expr* first;
  Expr* second;
};

// -------------------------------------------------------------------------- //
// Declarations
//
// TODO: Every declaration can have a constraint.

// The top-level declaration sequence of a Steve program.
//
// TODO: Should we replace top with a module? So that every program
// is inherently a module. This seems like a good idea. Of course,
// that means that every program would inherently be a type, which
// seems a bit odd...
struct Top : Decl, Kind_of<top_decl> {
  Top(Decl_seq* ds) 
    : Decl(Kind), first(ds) { }

  Decl_seq* decls() const { return first; }

  Decl_seq* first;
};

// A term of the form def n : t = e where n is a name, t is
// a type, and e is its initializer.
struct Def : Decl, Kind_of<def_decl> {
  Def(Name* n, Type* t, Expr* e) 
    : Decl(Kind), first(n), second(t), third(e) { }
  Def(const Location& l, Name* n, Type* t, Expr* e) 
    : Decl(Kind, l), first(n), second(t), third(e) { }
  
  Name* name() const { return first; }
  Type* type() const { return second; }
  Expr* init() const { return third; }

  Name* first;
  Type* second;
  Expr* third;
};

// A parameter the form 'n : t = e' where 'n' is the parameter
// name, 't' is its type, and 'e' is its default argument. Note that
// the default argument may be omitted. 
struct Parm : Decl, Kind_of<parm_decl> {
  Parm(Name* n, Type* t, Expr* e) 
    : Decl(Kind), first(n), second(t), third(e)  { }
  Parm(const Location& l, Name* n, Type* t, Expr* e) 
    : Decl(Kind, l), first(n), second(t), third(e)  { }

  Name* name() const { return first; }
  Type* type() const { return second; }
  Expr* def() const { return third; }

  Name* first;
  Type* second;
  Expr* third;
};

// A field in a record, having the form 'n : t | c' where 'n' is
// the field name, 't' is its type and 'c' is a constraint on its
// use. Note that 'c' must be satisfied for the program to be
// well-formed.
//
// TODO: Allow default initializers?
//
// TODO: Allow member types?
struct Field : Decl, Kind_of<field_decl> {
  Field(Name* n, Type* t, Term* e) 
    : Decl(Kind), first(n), second(t), third(e)  { }
  Field(const Location& l, Name* n, Type* t, Term* e) 
    : Decl(Kind, l), first(n), second(t), third(e)  { }

  Name* name() const { return first; }
  Type* type() const { return second; }
  Term* prop() const { return third; }

  Name* first;
  Type* second;
  Term* third;
};

// An alternative of the form 'e : t' where 'e' is expression
// that tags the alternative and 't' is its corresponding type.
struct Alt : Decl, Kind_of<alt_decl> {
  Alt(Expr* e, Type* t)
    : Decl(Kind), first(e), second(t) { }
  Alt(const Location& l, Expr* e, Type* t)
    : Decl(Kind, l), first(e), second(t) { }

  Expr* tag() const { return first; }
  Type* type() const { return second; }

  Expr* first;
  Type* second;
};

// An enumerator of the form 'n = e' within an enum type. 
// Here, 'n' is the name of the enumerator and 'e' the associated
// expression.
//
// The type of the enumerator is define to be the enumeration
// type in which the enumerator is declared.
struct Enum : Decl, Kind_of<enum_decl> {
  Enum(Name* n, Expr* v)
    : Decl(Kind), first(n), second(v) { }
  Enum(const Location& l, Name* n, Expr* v)
    : Decl(Kind, l), first(n), second(v) { }

  Name* name() const { return first; }
  Expr* value() const { return second; }
  
  Name* first;
  Expr* second;
};

// An import declaration of the form `import n` introduces the named
// module into the current context. The import declaration associates
// that name with the loaded module type.
//
// Note that in declarations of the form `import m1.m2`, the
// import declaration contains only `import m1. The nested module
// type of `m1` contains the declaration `import m2`.
struct Import : Decl, Kind_of<import_decl> {
  Import(Name* n, Type* m) 
    : Decl(Kind), first(n), second(m) { }
  Import(const Location& l, Name* n, Type* m) 
    : Decl(Kind, l), first(n), second(m) { }

  Name* name() const { return first; }
  Type* module() const { return second; }

  Name* first;
  Type* second;
};

// A using declaration of the form `using n` binds the name of
// the declaration into the current scope.
struct Using : Decl, Kind_of<using_decl> {
  Using(Name* n, Decl* d)
    : Decl(Kind), first(n), second(d) { }
  Using(const Location& l, Name* n, Decl* d)
    : Decl(Kind, l), first(n), second(d) { }

  Name* name() const { return first; }
  Decl* decl() const { return second; }

  Name* first;
  Decl* second;
};

// -------------------------------------------------------------------------- //
// Expression construction.

template<typename T, typename... Args>
  T* make_expr(const Location&, Type*, Args&&...);

// -------------------------------------------------------------------------- //
// Queries

bool is_same(Expr*, Expr*);
bool is_less(Expr*, Expr*);

Type* get_type(Expr*);

// -------------------------------------------------------------------------- //
// Printing

// Debug printing
class Printer;
void debug_print(Printer&, Expr*);

// Streaming
struct debug_node { Expr* e; };
debug_node debug(Expr*);

std::ostream& operator<<(std::ostream&, debug_node);


// We define the following print manipulators to support diagnostics
//
//    typed(e) -- prints "e (of type T)"
//    ...

struct typed_printer { Expr* e; };

inline typed_printer 
typed(Expr* e) { return {e}; }

// FIXME: This should call out to the pretty printer, not the
// debug printer.
template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, typed_printer p) {
    Expr* e = p.e;
    Type* t = get_type(p.e);
    return os << format("'{}' (of type '{}')", debug(e), debug(t));
  }

} // namespace steve

#include "Ast.ipp"

#endif
