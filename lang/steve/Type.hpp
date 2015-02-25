
#ifndef STEVE_TYPE_HPP
#define STEVE_TYPE_HPP

#include <steve/Ast.hpp>

// This module provides facilities for querying properties of types.

namespace steve {

// Builtins. Return global instances of these types. These
// are used internally when not parsed from expressions.
Type* get_typename_type();
Type* get_unit_type();
Type* get_bool_type();
Type* get_nat_type();
Type* get_int_type();
Type* get_char_type();

// Type expressions. Create an instance of a built in type as
// it occurs in an expression (i.e. having some source location).
Type* make_typename_type(const Location&);
Type* make_unit_type(const Location&);
Type* make_bool_type(const Location&);
Type* make_nat_type(const Location&);
Type* make_int_type(const Location&);
Type* make_char_type(const Location&);

// TODO: Clean this up?
Type* make_fn_type(Decl_seq*, Type*);

// Type categories
bool is_typename_type(Type*);
bool is_boolean_type(Type*);
bool is_integral_type(Type*);
bool is_type_constructor_type(Type*);

// Type queries
Type* type(Expr*);
Fn_type* type(Fn*);

Integer size_in_bits(Type*);

} // namespace steve

#include <steve/Type.ipp>

#endif
