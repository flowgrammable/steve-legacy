
#include <steve/Elaborator.hpp>
#include <steve/Evaluator.hpp>
#include <steve/Syntax.hpp>
#include <steve/Ast.hpp>
#include <steve/Scope.hpp>
#include <steve/Type.hpp>
#include <steve/Conv.hpp>
#include <steve/Intrinsic.hpp>
#include <steve/Debug.hpp>

#include <iostream>

namespace steve {

namespace {

// -------------------------------------------------------------------------- //
// Diagnostic helper
//
// We define the following print manipulators to support diagnostics
//
//    typed(e) -- prints "e (of type T)"
//    ...


struct typed_printer { Expr* e; };

inline typed_printer 
typed(Expr* e) { return {e}; }

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, typed_printer p) {
    Expr* e = p.e;
    Type* t = get_type(p.e);
    return os << format("'{}' (of type '{}')", debug(e), debug(t));
  }


// -------------------------------------------------------------------------- //
// Elaborate as kind
//
// The following functions are used to elaborate a tree as a specific
// kind of term. 

Expr* elab_expr(Tree*);

// Elaborate the parse tree as a node of kind T.
template<typename T>
  inline T*
  elaborate_as(Tree* t, const char* kind) {
    Expr* e = elab_expr(t);
    if (not e)
      return nullptr;
    if (T* d = as<T>(e))
      return d;

    error(t->loc) << format("expected a {} but got '{}'", kind, debug(e));
    return nullptr;
  }

// Elaborate the parse tree as a declaration. If the tree is not a
// declaration, behavior is undefined.
Decl* 
elab_decl(Tree* t) { return elaborate_as<Decl>(t, "declaration"); }

// Elabor
Term*
elab_term(Tree* t) { return elaborate_as<Term>(t, "term"); }

// Elaborate the parse tree as a type. If the tree is not a type,
// behavior is undefined.
Type*
elab_type(Tree* t) { 
  Expr* e = elab_expr(t);
  if (not e)
    return nullptr;

  // See through declaration references.
  if (Decl_id *id = as<Decl_id>(e))
    if (Def* def = as<Def>(id->decl()))
      e = def->init();

  // If the elaboration refers to a type function, then evaluate it.
  if (Call *call = as<Call>(e)) {
    if (not is_same(get_type(call), get_typename_type())) {
      error(call->loc) << format("'{}' does not yield a type", debug(call));
      return nullptr;
    }
    e = reduce(e);
  }
  
  // Now check to see if we've gotten a type.
  if (Type* r = as<Type>(e))
    return r;
  
  error(t->loc) << format("expected a type but got '{}'", debug(e));
  return nullptr;
}

// Return a new basic id.
Name*
elab_name(Id_tree* t) {
  return new Basic_id(t->loc, as_identifier(*t->value()));
}

// Return a declaration name from given parse tree. Behavior is
// undefined if t is not Id_tree.
Name*
elab_name(Tree* t) {
  if (Id_tree* n = as<Id_tree>(t))
    return elab_name(n);
  steve_unreachable(format("{}: invalid declaration name '{}'", t->loc, debug(t)));
}


// -------------------------------------------------------------------------- //
// Elaboration of names

// Elaborate an identifier. This returns the overload set corresponding
// to the name.
//
//     lookup(G, n) = n : T
//    --------------------- E-id
//          G |- n : T
//
// TODO: Allow lookup to return a set of declarations. Note that the
// type of an overload set the set of types returned identified by the
// name.
Expr*
elab_id(Id_tree* t) {
  Name* n = elab_name(t);
  if (not n)
    return nullptr;

  // Lookup the name.
  //
  // TODO: Change to lookup(n) when we want to support overloading.
  if (Decl* d = lookup_single(n))
    return make_expr<Decl_id>(t->loc, get_type(d), n, d);
  
  error(t->loc) << format("no matching declaration for '{}'", debug(n)) << '\n';
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration of literals

// Return an elaborated boolean literal.
//
//    --------------- E-bool
//    G |- {B} : bool
//
// Here, [B] denotes the set of boolean values.
Expr*
elab_bool(Lit_tree* t, const Token* k) {
  return make_expr<Bool>(t->loc, get_bool_type(), (k->text == "true"));
}

// Returns an elaborated integer literal.
//
//    -------------- E-int
//    G |- {N} : int
//
// Here, {N} denotes the set of integer values.
Expr*
elab_int(Lit_tree* t, const Token* k) {
  return make_expr<Int>(t->loc, get_int_type(), as_integer(*k));
}

template<typename F>
  Expr*
  elab_intrinsic(Tree* t, F fn) {
    Decl* decl = fn();
    Name* name = get_name(decl);
    Type* type = get_type(decl);
    return make_expr<Decl_id>(t->loc, type, name, decl);
  }

// Elaborate a literal. The type of a literal depends on its token.
// In the rules for elaborating literals
//
//    ------------------- E-type
//    G |- {T} : typename
//
// The term {T} denotes the set of ground types in the language.
//
//    ------------------------------------------------- E-bitfield-type
//    G |- bitfield : (typename, nat, bool) -> typename
//
// There is a special elaboration rule for the bitfield type since
// it is a builtin type constructor.
//
// Note that there are no tokens of type nat. They are all integers
// by default.
Expr*
elab_lit(Lit_tree* t) {
  const Token* k = t->value();
  switch (k->kind) {
  case typename_tok: return make_typename_type(t->loc);
  case bool_tok: return make_bool_type(t->loc);
  case nat_tok: return make_nat_type(t->loc);
  case int_tok: return make_int_type(t->loc);
  case bitfield_tok: return elab_intrinsic(t, get_bitfield_ctor);
  case boolean_literal_tok: return elab_bool(t, k);
  case binary_literal_tok: return elab_int(t, k);
  case octal_literal_tok: return elab_int(t, k);
  case decimal_literal_tok: return elab_int(t, k);
  case hexadecimal_literal_tok: return elab_int(t, k);
  default: break;
  }
  steve_unreachable(format("{}: elaboration failure of '{}'", t->loc, debug(t)));
}


// -------------------------------------------------------------------------- //
// Elaboration of function calls

// Elaborate the function arguments.
//
// TODO: Partially evaluate each argument.
Expr_seq*
elab_args(Tree_seq* t) {
  Expr_seq* args = new Expr_seq();
  for (Tree* p : *t) {
    // TODO: Evaluate the argument before appending it.
    if (Expr* a = elab_expr(p))
      args->push_back(a);
    else
      return nullptr;
  }
  return args;
}

// Returns the type of e1 and e2 if they have the same type.
//
// TODO: The diagnostic could be better.
Type*
check_same_type(Expr* e1, Expr* e2) {
  Type* t1 = get_type(e1);
  Type* t2 = get_type(e2);
  if (is_same(t1, t2))
    return t1;
  error(e2->loc) << format("{} does not have the same type as {}",
                           typed(e2), 
                           typed(e1));
  return nullptr;
}

// Check that that the expression e can be converted to t, possbily
// returning a conversion.
Expr*
check_convertible(Expr* e, Type* t) {
  if (Expr* c = convert(e, t))
    return c;
  error(e->loc) << format("no known conversion from {} to '{}'", 
                          typed(e), 
                          debug(t));
  return nullptr;
}

// Check that the expression is a constant, that it can be fully
// reduced to a value. This returns the constant evaluation if so,
// and nullptr otherwise. Diagnostics are emitted when not constant.
Expr*
check_constant(Expr* e) {
  Eval v = eval(e);
  if (is_value(v))
    return to_expr(v, e);
  error(e->loc) << format("'{}' is not a constant expression", debug(e));
  return nullptr;
}

// Check that expr is a boolean term.
Term*
check_boolean(Term* e) {
  Type* t = get_type(e);
  if (is_same(t, get_bool_type())) 
    return e;
  error(e->loc) << format("'{}' is not a boolean term", debug(e));
  return nullptr;
}

// Check that each argument matches the corresponding parameter, rewriting
// the argument, possibly to have conversions.
//
// TODO: Allow conversions.
Expr_seq*
check_arguments(Call_tree* t, Def* fn, Expr_seq* args, Decl_seq* parms) {
  auto first1 = args->begin();
  auto last1 = args->end();
  auto first2 = parms->begin();
  auto last2 = parms->end();

  // Check each argument. If it doesn't match, then skip it so we
  // can diagnose all failures.
  //
  // TODO: Are there any kinds of parameters that consume multiple
  // arguments? E.g., variadic or sized parameter packs? Not yet,
  // but conceivably. Note that this would affect the check(s) below.
  Expr_seq* result = new Expr_seq();
  while (first1 != last1 and first2 != last2) {
    Expr* arg = *first1;
    Expr* parm = *first2;
    if (Expr* a = check_convertible(arg, get_type(parm)))
      result->push_back(a);
    ++first1;
    ++first2;
  }

  // If we didn't elaborate all of the args, then bail.
  if (result->size() != args->size())
    return nullptr;

  // Too few arguments were given for the function.
  //
  // TODO: This is where would would instantiate default arguments.
  if (first1 == last1 and first2 != last2) {
    error(t->loc) << format("too few arguments for '{}'", debug(fn->name()));
    return nullptr;
  }

  // Too many arguments were given for the function.
  //
  // TODO: Unless the last argument is variadic, then all remaining
  // arguments would be placed into an argument pack.
  if (first2 == last2 and first1 != last1) {
    error(t->loc) << format("too many arguments for '{}'", debug(fn->name()));
    return nullptr;
  }

  return result;  
}

// Elaborate a function call. The argument types must agree with
// the parameter types of the given function.
//
//
//    G |- lookup(f) = f(pi : Ti) -> T   
//              G |- ai : Ti
//    --------------------------------- E-call
//             G |- f(a*) : T
//
// This is approximately right. Note that if all arguments are
// constant, then the elaboration of the function is also its
// evaluation.
//
// TODO: Support overload resolution.
//
// TODO: Support conversion of function arguments (yikes!).
Expr*
elab_call(Call_tree* t) {
  // Elaborate the target, returning a function declaration or
  // an overload set.
  Expr* tgt = elab_expr(t->fn());
  if (not tgt)
    return nullptr;

  // Elaborate the arguments.
  Expr_seq* args = elab_args(t->args());
  if (not args)
    return nullptr;

  // TODO: This is the single declaration case. Implement overload
  // resolution here. Actually, refactor the single case into a
  // special case of overload resolution. 
  Def* def = nullptr;   // Provides diagnostic context
  Fn* fn = nullptr; // The expression actually called

  // Unroll the target until we find a function.
  //
  // TODO: What other kinds of targets can we find here.
  if (Decl_id* ref = as<Decl_id>(tgt)) {
    def = as<Def>(ref->decl());
    if (def)
      fn = as<Fn>(def->init());
  }

  // Make sure we have a callable expression.
  if (not fn) {
    error(t->fn()->loc) << format("'{}' is not callable", debug(tgt));
    return nullptr;
  }

  // Check the arguments against the function parameters.
  args = check_arguments(t, def, args, fn->parms());
  if (not args)
    return nullptr;

  // Get the result type of the function; that's the type of
  // the expression.
  Type* type = fn->result();

  // TODO: If all of the arguments are constant, then evaluate
  // the result of the call.
  Call* call = make_expr<Call>(t->loc, type, fn, args);
  return call;
}


// -------------------------------------------------------------------------- //
// Elaboration of ranges

// Elaborate a range term. The type of a range term is the unified
// type of both its subterms.
//
//    G |- t1 : T   G |- t2 : T
//    ------------------------- T-range
//        G |- t1..t2 : T
Expr*
elab_range(Range_tree* t) {
  Term* t1 = elab_term(t->lower());
  Term* t2 = elab_term(t->upper());

  if (not t1 or not t2)
    return nullptr;

  // Determine the type of the range.
  Type* kind = get_typename_type();
  Type* type = check_same_type(t1, t2);
  if (not type)
    return nullptr;
  type = make_expr<Range_type>(no_location, kind, type);

  return make_expr<Range>(t->loc, type, t1, t2);
}


// -------------------------------------------------------------------------- //
// Elaboration of record types

// Elaborate a record type. 
//
// TODO: Document the elaboration rules.
Expr*
elab_record(Record_tree* t) {
  Type* kind = get_typename_type();
  Decl_seq* fields = new Decl_seq();
  Type* rec = make_expr<Record_type>(t->loc, kind, fields);

  // Push the record scope and elaborate each field in turn.
  Scope_guard scope(record_scope, rec);
  for (Tree *f : *t->fields()) {
    if (Decl* d = as<Decl>(elab_expr(f)))
      fields->push_back(d);
    else
      return nullptr;
  }
  return make_expr<Record_type>(t->loc, get_typename_type(), fields);
}

// Elaborate a field declaration.
//
// TODO: Should we auto-complete the missing predicate with the
// trivial constraint "true" or "top" or something like that?
Expr*
elab_field(Field_tree* t) {
  Name* name = elab_name(t->name());
  Type* type = elab_type(t->type());
  Term* pred = elab_term(t->prop());

  // Note that the predicate is optional. However, if it wasn't
  // given and elaboration failed, don't proceed.
  if (not name or not type)
    return nullptr;
  if (t->prop() and not pred)
    return nullptr;

  // The predicate needs to have type bool. Diagnose the failure
  // but proceed as if it were unconstrained.
  if (pred)
    pred = check_boolean(pred);

  Decl* d = make_expr<Field>(t->loc, type, name, type, pred);
  if (declare(d))
    return d;
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration of variants
//
// TODO: Don't recursively elaboate variant terms as ordinary
// expresions. It's easier (and clearer) if we define our own
// loop over the alternatives in a variant parse tree.


// Elaborate an alternative tag in the context of the given
// variant. When v is non-null, the tag is elaborated as a tag
// of a variant-of type. Otherwise, the tag is a new name in
// this scope.
Expr*
elab_alt_tag(Tree* t, Type* v) {
  if (v)
    return elab_expr(t);
  else
    return elab_name(t);
}

// Elaborate the declaration of an alternative.
//
// Note that an alternative is not a declaration in the usual sense.
// It is simply a term that associates a tag and a type.
//
// FIXME: In the elaboration of alternatives in a variant-of type,
// they tend to be of the form 'e:T'. Here, 'e' is effectively
// interpreted as an implied condition, generally of the form
// 'x == e', where 'x' is the implicit descriminator. If 'e' has
// the form 'e1..e2', then the corresponding tag condition is
// 'x in e1..e2'. This should be made more explicit.
//
// Also, is there a need to access the implicit descriminator? If so,
// then it should be given a special name (this?).
Expr*
elab_alt(Alt_tree* t) {
  // Get the context for a variant-of type.
  Variant_of_type* var = as<Variant_of_type>(current_context());

  // Elaborate the tag and type.
  Expr* tag = elab_alt_tag(t->tag(), var);
  Type* type = elab_type(t->type());

  // Stop analyzing if we didn't get everything.
  if (not tag or not type)
    return nullptr;

  // If declaring alternatives in a variant-of type, check that
  // the tag is convertible to the descriminator type.
  //
  // TODO: This won't work for range terms., For example:
  //
  //    variant(int) { 0..10:X; }
  //
  // Here, 0..10 does not have type int, although its bounds do
  // This should be well-formed. Also, the tag 'default' would
  // need to be made admissable.
  if (var)
    check_convertible(tag, var->desc());

  return make_expr<Alt>(t->loc, type, tag, type);
}

// Elaborate a sequence of alternatives for the variant or variant-of
// type var.
Expr*
elab_alts(Variant_tree* t, Type* var, Decl_seq* alts) {
  Scope_guard scope(variant_scope, var);
  for (Tree *a : *t->alts()) {
    if (Decl* d = as<Decl>(elab_expr(a)))
      alts->push_back(d);
    else
      return nullptr;
  }
  return var;
}

// Elaborate the variant type.
//
//    for each ai in as G |- ai : Ti
//    ------------------------------ T-variant
//    G |- variant { as } : typename
Expr*
elab_variant(Variant_tree* t) {
  // Stub ot the variant.
  Decl_seq* alts = new Decl_seq();
  Type* kind = get_typename_type();
  Type* var = make_expr<Variant_type>(t->loc, kind, alts);
  return elab_alts(t, var, alts);
}

// Elaborate the variant-of type.
//
//    G |- t : T   for each ai in as G |- ai : Ti
//    ------------------------------------------- T-variant-of
//         G |- variant(t) { as } : typename
Expr*
elab_variant_of(Variant_tree* t) {
  Type* type = elab_type(t->desc());
  if (not type)
    return nullptr;

  // Stub out the variant.
  Decl_seq* alts = new Decl_seq();
  Type* kind = get_typename_type();
  Type* var = make_expr<Variant_of_type>(t->loc, kind, type, alts);
  return elab_alts(t, var, alts);
}


// Elaborate a variant declaration.
Expr*
elab_variant_type(Variant_tree* t) {
  if (t->desc())
    return elab_variant_of(t);
  else
    return elab_variant(t);
}

// -------------------------------------------------------------------------- //
// Elaboration of enum types
//
// TODO: Implement me.

Expr*
elab_enum(Enum_tree* t) {
  sorry(t->loc) << "enumeration type not implemented" << '\n';
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration of enum-of types

// In an enum-of-T, the terms in a range constructor must be convertible
// to the base type. Note that we require convertibility instead of 
// equivalence to make the language more expressive.
Expr*
elab_range_ctor(Range_tree* t, Enum_of_type* enu) {
  Range* range = as<Range>(elab_term(t));
  if (not range)
    return nullptr;

  // The range type must be convertible to the enumeration's
  // base type.
  if (not check_convertible(range->lower(), enu->base()))
    return nullptr;

  return range;
}

// For an expression of the form 'n = t', 'n' must be a basic-id
// and 't' must be a value whose type is convertible to the base
// type of the enumeration.
//
// Note that enumerator is declared in the enclosing scope of
// the enumeration type.
Expr*
elab_named_ctor(Binary_tree* t, Enum_of_type* enu) {
  Name* name = elab_name(t->left());
  Expr* value = elab_expr(t->right());

  if (not name || not value)
    return nullptr;

  // Ensure that the value is convertible to the base type,
  // and partially evaluate the results.
  value = check_convertible(value, enu->base());
  if (not value)
    return nullptr;
  value = check_constant(value);
  if (not value)
    return nullptr;

  Decl* decl = make_expr<Enum>(t->loc, enu, name, value);
  if (declare(decl))
    return decl;
  return nullptr;
}

// Elaborate the constructor of an enum-of tree. Only two
// kinds of expressions are admissable constructors of an
// enum-of type: ranges and applications.
//
// TODO: Allow an id-tree to define a range-ctor when the base
// type is well-founded. The semantics are interesting.
Expr*
elab_enum_of_ctor(Tree* t, Enum_of_type* enu) {
  if (Range_tree* r = as<Range_tree>(t))
    return elab_range_ctor(r, enu);
  if (Binary_tree* b = as<Binary_tree>(t))
    if (b->op()->kind == equal_tok)
      return elab_named_ctor(b, enu);

  // FIXME: This should probably be an assertion.
  error(t->loc) << format("invalid enumerator '{}'", debug(t));
  return nullptr;
}

// Elaborate the constructors of an enum-of type.
Expr*
elab_enum_of_ctors(Enum_tree* t, Enum_of_type* enu, Expr_seq* ctors) {
  for (Tree* c : *t->ctors()) {
    if (Expr* e = elab_enum_of_ctor(c, enu))
      ctors->push_back(e);
    else
      return nullptr;
  }
  return enu;  
}

// Elaborate a parse tree representing an enum-of type.
Expr*
elab_enum_of(Enum_tree* t) {
  Type* type = elab_type(t->base());
  if (not type)
    return nullptr;

  // Stub out the enumeration type.
  Type* kind = get_typename_type();
  Expr_seq* ctors = new Expr_seq();
  Enum_of_type* enu = make_expr<Enum_of_type>(t->loc, kind, type, ctors);
  return elab_enum_of_ctors(t, enu, ctors);
}

// Elaborate a parse tree that represents either an enum type
// or an enum-of type.
Expr*
elab_enum_type(Enum_tree* t) {
  if (t->base())
    return elab_enum_of(t);
  else
    return elab_enum(t);
}


// -------------------------------------------------------------------------- //
// Elaboration of constants
//
// Note that the initializer is provided, making it par

// Elaboborate a constant definition.
//
//       G, n : T |- e : T
//    ------------------------ E-const
//    G |- (def n : T = e) : T
Expr*
elab_const(Value_tree* t, Tree* e) {
  Name* name = elab_name(t->name());
  Type* type = elab_type(t->type());

  if (not name or not type)
    return nullptr;

  // Emit the declaration before elaborating the initializer.
  Def* d = make_expr<Def>(t->loc, type, name, type, nullptr);
  if (not declare(d))
    return d;

  // Check that the initializer is convertible to the declared
  // type, and then partiually evaluate it. Update the definition 
  // wth the initializer.
  Expr* init = elab_expr(e);
  if (not init)
    return nullptr;
  init = check_convertible(init, type);
  if (not init)
    return nullptr;
  d->third = reduce(init);

  // Bind the initializer to its definition.
  d->third->od = d;

  return d;
}


// -------------------------------------------------------------------------- //
// Elaboration of functions

// Elaborate a parameter.
//
//     G  |- e : T
//    -------------- T-parm
//    G |- n : T = e
//
// Note that the default argument does not rely on the typing
// of the parameter.
//
// TODO: Implement default arguments. Also, the argument is only
// required to be convertible to result type.
Expr*
elab_parm(Parm_tree* t) {
  Name* name = elab_name(t->name());
  Type* type = elab_type(t->type());

  // Don't proceed if we didn't get the right stuff.
  if (not name or not type)
    return nullptr;

  // Declare the parameter.
  Parm* d = make_expr<Parm>(t->loc, type, name, type, nullptr);
  if (declare(d))
    return d;
  return nullptr;
}

// Elaborate a sequence of parameters.
Decl_seq*
elab_parms(Fn_tree* t, Decl_seq* parms) {
  for (Tree* p : *t->parms()) {
    if (Decl* d = elab_decl(p))
      parms->push_back(d);
    else
      return nullptr;
  }
  return parms;
}

// Elaborate a function definition. Note that the function
// declartor syntax:
//
//    def f(pi : Ti)->T = e;
//
// is equivalent to the long-form syntax:
//
//    def f : (Ti)->T = \(pi : Ti).e
//
Expr*
elab_fn(Fn_tree* t, Tree* e) {
  // Elaborate the name.
  Name* name = elab_name(t->name());
  if (not name)
    return nullptr;

  // Stub out the function.
  Decl_seq* parms = new Decl_seq();
  Fn* fn = make_expr<Fn>(t->loc, nullptr, parms, nullptr, nullptr);

  // Enter function scope and elaborate the parameters.
  Scope_guard scope(function_scope, fn);
  if (not elab_parms(t, parms))
    return nullptr;

  // Now, elaborate the type (the type name may refer to parameter
  // names, just like a late-binding return type in C++).
  Type* result = elab_type(t->result());
  if (not result)
    return nullptr;
  fn->second = result;

  // Compute the type of the function.
  Type* type = get_fn_type(parms, result);
  
  // Emit the declaration of the (as-of-yet) incomplete function
  // into the enclosing scope (we pushed the function paramter
  // scope above). We declare this before elaborating the initializer
  // so that the function can be defined recursively.
  Def* d = make_expr<Def>(t->loc, type, name, type, fn);
  if (not declare_outside(d))
    return nullptr;

  // Check that the function body converts to the result
  // type, and then partially evaluate it. Update the function
  // with its body.
  Expr* body = elab_expr(e);
  if (not body)
    return nullptr;
  body = check_convertible(body, result);
  if (not body)
    return nullptr;
  fn->third = reduce(body);
  return d;
}


// -------------------------------------------------------------------------- //
// Elaboration of definitions

// A definition declares either a constant or a function. The two 
// elaboration rules are given below.
//
//     G, n : T |- e : T
//    ------------------- T-def-const
//    G |- def n : T = e
//
//    G, n(pi)->T : (Ti)->T |- e : T
//    ------------------------------- T-def-fn
//         G |- def n(pi)->T = e
//
// Note that the typing of the initializer is handled by elab_const
// and elab_fn.
//
// TODO: Weaken the same-type requirement to allow for conversion of
// the initializer to the returned type (e.g., nat to int?).
Expr*
elab_def(Def_tree* t) {
  if (Value_tree* v = as<Value_tree>(t->decl()))
    return elab_const(v, t->init());
  if (Fn_tree* f = as<Fn_tree>(t->decl()))
    return elab_fn(f, t->init());
  steve_unreachable(format("{}: elaboration failure", t->loc));
}

// Elaborate each declaration in turn. Note that declarations are
// never replaced. The elaboration operates in-place.
Expr*
elab_top(Top_tree* t) {
  Scope_guard scope(module_scope);
  Decl_seq* ds = new Decl_seq();
  for (Tree* d1 : *t->first) {
    if (Decl* d2 = elab_decl(d1))
      ds->push_back(d2);
    else
      return nullptr;
  }
  return new Top(ds);
}

Expr*
elab_unimplemented(Tree* t) {
  steve_unreachable(format("unimplemented elaboration of '{}'", debug(t)));
}

Expr*
elab_expr(Tree* t) {
  if (not t)
    return nullptr;

  switch (t->kind) {
  case id_tree: return elab_id(as<Id_tree>(t));
  case lit_tree: return elab_lit(as<Lit_tree>(t));
  case call_tree: return elab_call(as<Call_tree>(t));
  case range_tree: return elab_range(as<Range_tree>(t));
  case unary_tree: return elab_unimplemented(t);
  case binary_tree: return elab_unimplemented(t);
  // Types
  case record_tree: return elab_record(as<Record_tree>(t));
  case variant_tree: return elab_variant_type(as<Variant_tree>(t));
  case enum_tree: return elab_enum_type(as<Enum_tree>(t));
  // Declarations
  case parm_tree: return elab_parm(as<Parm_tree>(t));
  case def_tree: return elab_def(as<Def_tree>(t));

  case field_tree: return elab_field(as<Field_tree>(t));
  case pad_tree: return elab_unimplemented(t);
  case alt_tree: return elab_alt(as<Alt_tree>(t));
  case nv_tree: return elab_unimplemented(t);
    break;
  // Misc
  case top_tree: return elab_top(as<Top_tree>(t));
  }
  steve_unreachable(format("elaborating unknown node '{}'", node_name(t)));
}

} // namespace

Expr*
Elaborator::operator()(Tree* t) {
  use_diagnostics(diags);
  return elab_expr(t);
}


} // namespace steve