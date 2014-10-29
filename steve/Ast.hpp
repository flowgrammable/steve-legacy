
#ifndef STEVE_AST_HPP
#define STEVE_AST_HPP

#include <steve/Debug.hpp>
#include <steve/Node.hpp>
#include <steve/String.hpp>
#include <steve/Integer.hpp>

#include <iostream>

namespace steve {

// Names
constexpr Node_kind basic_id       = make_name_node(1);  // id
constexpr Node_kind scoped_id      = make_name_node(2);  // scope.id
constexpr Node_kind decl_id        = make_name_node(10); // x, referring to a decl
// Types
constexpr Node_kind typename_type   = make_type_node(1);  // typename
constexpr Node_kind unit_type       = make_type_node(2);  // unit
constexpr Node_kind bool_type       = make_type_node(3);  // bool
constexpr Node_kind nat_type        = make_type_node(4);  // nat
constexpr Node_kind int_type        = make_type_node(5);  // int
constexpr Node_kind char_type       = make_type_node(6);  // int
constexpr Node_kind fn_type         = make_type_node(7);  // (T*) -> T
constexpr Node_kind range_type      = make_type_node(8);  // .. T
constexpr Node_kind bitfield_type   = make_type_node(9);  // bitfield(T, N, B)
constexpr Node_kind record_type     = make_type_node(10); // record { ... }
constexpr Node_kind variant_type    = make_type_node(11); // variant { ... }
constexpr Node_kind variant_of_type = make_type_node(12); // variant(T) { ... }
constexpr Node_kind enum_type       = make_type_node(13); // enum { ... }
constexpr Node_kind enum_of_type    = make_type_node(14); // enum(T) { ... }
constexpr Node_kind module_type     = make_type_node(16); // module
// Terms
constexpr Node_kind unit_term      = make_term_node(1);  // ()
constexpr Node_kind bool_term      = make_term_node(2);  // {true, false}
constexpr Node_kind int_term       = make_term_node(3);  // {0, 1, 2, ..., n}
// Misc terms
constexpr Node_kind block_term     = make_term_node(20); // { ... }
constexpr Node_kind fn_term        = make_term_node(21); // fn(p*)->t.e
constexpr Node_kind call_term      = make_term_node(22); // f(a*) -- TODO: Should be in values
constexpr Node_kind promo_term     = make_term_node(23); // t as N for an integral type
constexpr Node_kind pred_term      = make_term_node(24); // t as B for a boolean type
constexpr Node_kind range_term     = make_term_node(28); // t1 .. t2
// Arithmetic terms
constexpr Node_kind add_term       = make_term_node(30); // t1 + t2
constexpr Node_kind sub_term       = make_term_node(31); // t1 - t2
constexpr Node_kind mul_term       = make_term_node(32); // t1 * t2
constexpr Node_kind div_term       = make_term_node(33); // t1 / t2
constexpr Node_kind mod_term       = make_term_node(34); // t1 % t2
constexpr Node_kind neg_term       = make_term_node(35); // -t
// Bitwise terms
constexpr Node_kind band_term      = make_term_node(40); // t1 & t2
constexpr Node_kind bor_term       = make_term_node(41); // t1 | t2
constexpr Node_kind bxor_term      = make_term_node(42); // t1 ^ t2
constexpr Node_kind bnot_term      = make_term_node(43); // ~t
constexpr Node_kind lsh_term       = make_term_node(44); // t1 << t2
constexpr Node_kind rsh_term       = make_term_node(45); // t1 >> t2
// Relational terms
constexpr Node_kind eq_term        = make_term_node(50); // t1 == t2
constexpr Node_kind ne_term        = make_term_node(51); // t1 != t2
constexpr Node_kind lt_term        = make_term_node(52); // t1 < t2
constexpr Node_kind gt_term        = make_term_node(53); // t1 > t2
constexpr Node_kind le_term        = make_term_node(54); // t1 <= t2
constexpr Node_kind ge_term        = make_term_node(55); // t1 >= t2
// Logical terms
constexpr Node_kind imp_term       = make_term_node(60); // t1 -> t2
constexpr Node_kind or_term        = make_term_node(61); // t1 or t2
constexpr Node_kind and_term       = make_term_node(62); // t1 and t2
constexpr Node_kind not_term       = make_term_node(63); // not t

// Statements

// Declarations
constexpr Node_kind top_decl       = make_decl_node(1); // decl*
constexpr Node_kind def_decl       = make_decl_node(2); // def n : t = e
constexpr Node_kind parm_decl      = make_decl_node(3); // n : t = e
constexpr Node_kind field_decl     = make_decl_node(4); // n : t | c
constexpr Node_kind alt_decl       = make_decl_node(5); // n [if t] (in a variant)
constexpr Node_kind enum_decl      = make_decl_node(6); // n [= t] (in an enum)
constexpr Node_kind import_decl    = make_decl_node(7); // import n;

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

// A scoped id of the form 's.n' designates a declaration within 
// the scope, s.
struct Scoped_id : Name, Kind_of<scoped_id> {
  Scoped_id(Type* s, Name* n) 
    : Name(Kind), first(s), second(n) { }
  Scoped_id(const Location& l, Type* s, Name* n) 
    : Name(Kind, l), first(s), second(n) { }
  Type* first;
  Name* second;
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
  Decl_seq* first;
};

// A variant type of the for 'variant { as* }' where 'as' is a sequenc
// of alternatives. A variant type provides a mechanism for conveying
// a single value of a set of types, which can be unrapped at a later
// time. Each alternative defines a tag which constructs a wrapped
// version of the variant. For example:
//
//    def V:typename = variant { b:bool; n:nat;  z:int; }
//
// TODO: How do we construct variants of these types? Presumably through
// implicit conversion?
//
//    def v1:V = true; // v1 has value <b=true>
//    def v2:V = 0;    // v2 has value <z=0>
struct Variant_type : Type, Kind_of<variant_type> {
  Variant_type(Decl_seq* as) 
    : Type(Kind), first(as) { }
  Variant_type(const Location& l, Decl_seq* as) 
    : Type(Kind, l), first(as) { }

  Decl_seq* alts() const { return first; }

  Decl_seq* first;
};

// A variant-of type of the form 'variant of t { as* }' where 't' is
// the descriminator type and 'as*' is a sequence of alternatives. In
// a variant-of type, the alternative tags are given as named values
// of the descriminator type (often enumerators). The 'default' label
// matches any other tag not explicitly given in the list of 
// alternatives. For example:
//
//    def V:typenae = variant(nat) { 0:bool; 1:nat; 2:int; default:unit; }
//
// TODO: The following is all wild speculation.
//
// When constructing a variant of this type, a descriminator must be
// given. For example:
//
//    def v1:V(0) = true; // has value <0=true>
//    def v2:V = 0;       // deduced value <2=0>
//    def v3:V = nat 0;   // deduced value <1=0>
//
//    var x : nat; // dynamic value
//    var v4:V(x); // OK: dependent type???
//
//    record R { 
//      type : nat; 
//      value : V(type); // OK: 
//    }
struct Variant_of_type : Type, Kind_of<variant_of_type> {
  Variant_of_type(Type* t, Decl_seq* as) 
    : Type(Kind), first(t), second(as) { }
  Variant_of_type(const Location& l, Type* t, Decl_seq* as) 
    : Type(Kind, l), first(t), second(as) { }
  
  Type* desc() const { return first; }
  Decl_seq* alts() const { return second; }
  
  Type* first;
  Decl_seq* second;
};


// The enum type of the form 'enum { es* } represents a set of values 
// where 'es*' is a list of constructors of those values. For example
//
//    def Day = enum { Mon, Tue, Wed, Thur, Fri, Sat, Sun }
//
// Enumerations of this type are closely related to variants. In fact,
// the underlying type of this enumeration is a variant where each
// constructor is mapped to a tagged unit value.
//
// Enumerations can also define recursive data types.
//
//    def Expr = enum { Plus {Expr, Expr}, Minus {Expr, Expr}, ... }
//
// When recursively defined, the underlying type is a recursive type.
//
// TODO: Implement recursive data types.
//
// Note that unlike the enum-of type, the underlying type of this 
// enum is defined by its constructors. It is either a variant or a
// recursive data type.
struct Enum_type : Type, Kind_of<enum_type> {
  Enum_type(Expr_seq* cs) 
    : Type(Kind), first(cs) { }
  Enum_type(const Location& l, Expr_seq* cs) 
    : Type(Kind, l), first(cs) { }

  Expr_seq* ctors() const { return first; }

  Expr_seq* first;
};


// The enum-of type of the form 'enum (b) { cs* }' represents a set 
// of values of some specified underlying type 'b' (also called the 
// base type) whose constructors are given by the list 'cs*'.  For 
// example:
//
//    def Color = record { r, g, b : Uint8 }
//
//    def Named_color = enum(Color) { 
//      Red   {0xff, 0, 0},
//      Green {0, 0xff, 0},
//      Blue  {0, 0, 0xff}
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
//      Forward_port 0xff01, 
//      Broadcast_port 0xff02 
//    }
//
// Range constructors are unnamed, but allow the construction of Port
// values within that range (e.g., 'Port 5' constructs a port with
// value 5.
struct Enum_of_type : Type, Kind_of<enum_of_type> {
  Enum_of_type(Type* b, Expr_seq* cs) 
    : Type(Kind), first(b), second(cs) { }
  Enum_of_type(const Location& l, Type* b, Expr_seq* cs) 
    : Type(Kind, l), first(b), second(cs) { }

  Type* base() const { return first; }
  Expr_seq* ctors() const { return second; }

  Type* first;
  Expr_seq* second;
};


// The module type represents the type of an imported module. 
//
// TODO: Implement me!
struct Module_type : Type, Kind_of<module_type> {
  Module_type() : Type(module_type) { }
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

// A compound expression of the form '{ s* }' where 's*' are a 
// sequence of statements.
struct Block : Term, Kind_of<block_term> {
  Block(Stmt_seq* ss)
    : Term(Kind), first(ss) { }
  Block(const Location& l, Stmt_seq* ss)
    : Term(Kind, l), first(ss) { }

  Stmt_seq* stmts() const { return first; }

  Stmt_seq* first;
};

// An anonymous function of the form \(p*)->t.e where p* is
// a sequence of parameters, t is the result type and e is its
// definition.
//
// The is_intrinsic method returns true when the function
// is a built-in function. This is the case when the definition
// is null.
struct Fn : Term, Kind_of<fn_term> {
  Fn(Decl_seq* p, Type* t, Expr* e) 
    : Term(Kind), first(p), second(t), third(e) { }
  Fn(const Location& l, Decl_seq* p, Type* t, Expr* e) 
    : Term(Kind, l), first(p), second(t), third(e) { }

  Decl_seq* parms() const { return first; }
  Type* result() const { return second; }
  Expr* body() const { return third; }

  // FIXME: This is not right... There needs to be a better
  // test for these kinds of things.
  bool is_intrinsic() const { return not third; }

  Decl_seq* first;
  Type* second;
  Expr* third;
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

// A unary term has a single sub-term.
struct Unary_term : Term {
  Unary_term(Node_kind k, Term* t1) 
    : Term(k), first(t1) { }
  Unary_term(Node_kind k, const Location& l, Term* t1) 
    : Term(k, l), first(t1) { }
  Term* first;
};

// A binary term has two sub-terms.
struct Binary_term : Term {
  Binary_term(Node_kind k, Term* t1, Term* t2) 
    : Term(k), first(t1), second(t2) { }
  Binary_term(Node_kind k, const Location& l, Term* t1, Term* t2) 
    : Term(k, l), first(t1), second(t2) { }
  Term* first;
  Term* second;
};

// Addition
struct Add : Binary_term, Kind_of<add_term> { 
  Add(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Add(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Subtraction
struct Sub : Binary_term, Kind_of<sub_term> { 
  Sub(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Sub(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Multiplication
struct Mul : Binary_term, Kind_of<mul_term> { 
  Mul(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Mul(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Division
struct Div : Binary_term, Kind_of<div_term> { 
  Div(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Div(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Modulus
struct Mod : Binary_term, Kind_of<mod_term> { 
  Mod(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Mod(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Logical not
struct Neg : Unary_term, Kind_of<neg_term> {
  Neg(Term* t1) 
    : Unary_term(Kind, t1) { }
  Neg(const Location& l, Term* t1) 
    : Unary_term(Kind, l, t1) { }
};

// Bitwise and
struct Band : Binary_term, Kind_of<band_term> { 
  Band(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Band(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Bitwise or
struct Bor : Binary_term, Kind_of<bor_term> { 
  Bor(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Bor(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Bitwise xor
struct Bxor : Binary_term, Kind_of<bxor_term> { 
  Bxor(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Bxor(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Bitwise not (one's complement)
struct Bnot : Unary_term, Kind_of<bnot_term> {
  Bnot(Term* t1) 
    : Unary_term(Kind, t1) { }
  Bnot(const Location& l, Term* t1) 
    : Unary_term(Kind, l, t1) { }
};

// Left shift
struct Lsh : Binary_term, Kind_of<lsh_term> { 
  Lsh(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Lsh(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Right shift
struct Rsh : Binary_term, Kind_of<rsh_term> { 
  Rsh(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Rsh(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Equality relation
struct Eq : Binary_term, Kind_of<eq_term> {
  Eq(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Eq(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Distinction relation
struct Ne : Binary_term, Kind_of<ne_term> {
  Ne(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Ne(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Less than relation
struct Lt : Binary_term, Kind_of<lt_term> {
  Lt(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Lt(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Greater than relation
struct Gt : Binary_term, Kind_of<gt_term> {
  Gt(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Gt(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Less than or equal to relation
struct Le : Binary_term, Kind_of<le_term> {
  Le(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Le(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Greater than or equal to relation
struct Ge : Binary_term, Kind_of<ge_term> {
  Ge(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Ge(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Logical if (material implication).
struct Imp : Binary_term, Kind_of<imp_term> {
  Imp(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Imp(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Logical and
struct And : Binary_term, Kind_of<and_term> {
  And(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  And(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Logical or
struct Or : Binary_term, Kind_of<or_term> { 
  Or(Term* t1, Term* t2) 
    : Binary_term(Kind, t1, t2) { }
  Or(const Location& l, Term* t1, Term* t2) 
    : Binary_term(Kind, l, t1, t2) { }
};

// Logical not
struct Not : Unary_term, Kind_of<not_term> {
  Not(Term* t1) 
    : Unary_term(Kind, t1) { }
  Not(const Location& l, Term* t1) 
    : Unary_term(Kind, l, t1) { }
};

// -------------------------------------------------------------------------- //
// Declarations
//
// TODO: Every declaration can have a constraint.

// The top-level declaration sequence of a steve program.
struct Top : Decl, Kind_of<top_decl> {
  Top(Decl_seq* ds) 
    : Decl(Kind), first(ds) { }
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

// An enumerator of the form 'n e' within an enum-of type. 
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

// An import declaration of the form 'import n' introduces the named
// module into the current context.
struct Import : Decl, Kind_of<import_decl> {
  Import(Name* n) 
    : Decl(Kind), first(n) { }
  Import(const Location& l, Name* n) 
    : Decl(Kind, l), first(n) { }
  Name* first;
};


// -------------------------------------------------------------------------- //
// Expression construction.

template<typename T, typename... Args>
  T* make_expr(const Location&, Type*, Args&&...);

// -------------------------------------------------------------------------- //
// Queries

bool is_same(Expr*, Expr*);
bool is_less(Expr*, Expr*);

// -------------------------------------------------------------------------- //
// Printing

// Debug printing
class Printer;
void debug_print(Printer&, Expr*);

// Streaming
struct debug_node { Expr* e; };
debug_node debug(Expr*);

std::ostream& operator<<(std::ostream&, debug_node);

} // namespace steve

#include "Ast.ipp"

#endif