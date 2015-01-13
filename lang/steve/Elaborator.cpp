
#include <steve/Elaborator.hpp>
#include <steve/Evaluator.hpp>
#include <steve/Syntax.hpp>
#include <steve/Ast.hpp>
#include <steve/Scope.hpp>
#include <steve/Type.hpp>
#include <steve/Conv.hpp>
#include <steve/Overload.hpp>
#include <steve/Variant.hpp>
#include <steve/Module.hpp>
#include <steve/Intrinsic.hpp>
#include <steve/Debug.hpp>

#include <iostream>

namespace steve {

namespace {

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

// Elaborate t as a reference to a parameter.
Parm*
elab_as_parm(Tree* t) {
  Expr* expr = elab_expr(t);
  if (not expr)
    return nullptr;
  Decl_id* id = as<Decl_id>(expr);
  if (not id) {
    error(t->loc) << format("'{}' does not name a declaration", debug(t));
    return nullptr;
  }
  Parm* parm = as<Parm>(id->decl());
  if (not parm) {
    error(t->loc) << format("'{}' does not name a parameter", debug(id));
  }
  return parm;
}


// Elaborate the given tree as a term.
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
  if (Decl_id *id = as<Decl_id>(e)) {
    if (Def* def = as<Def>(id->decl()))
      e = def->init();
    else if (Import* imp = as<Import>(id->decl()))
      e = imp->module();
  }

  // FIXME: Implement overlaod resolution!

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
  else
    return nullptr;
}


// -------------------------------------------------------------------------- //
// Elaboration of literals

// Return an elaborated unit literal. A unit literal has type unit.
Expr*
elab_unit(Lit_tree* t, const Token* k) {
  return make_expr<Unit>(t->loc, get_unit_type());
}

// Return an elaborated boolean literal. The boolean literals include
// the 'true' and 'false' tokens. A boolean literal has type 'bool'.
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
  case char_tok: return make_char_type(t->loc);
  case unit_tok: return elab_unit(t, k);
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

// Check that each argument matches the type of the corresponding 
// parameter. This returns a sequence of expressions, converting
// each argument to the parameter, or if no conversions are possible,
// nullptr.
Expr_seq*
check_arguments(Call_tree* t, Def* fn, Expr_seq* args, Type_seq* parms) {
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
    Type* parm = *first2;
    if (Expr* a = check_convertible(arg, parm))
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

// Elaborate the binding of a variant to its dependent descriminator,
// producing a new type.
//
// FIXME: This should go away.
Expr*
elab_variant_binding(Call_tree* t, Dep_variant_type* type) {
  /*
  Tree_seq* exprs = t->args();
  if (exprs->size() != 1) {
    if (exprs->size() == 0)
      error(t->loc) << format("too few arguments for descriminated "
                              "variant type '{}'", debug(type));
    else
      error(t->loc) << format("too many arguments for descriminated "
                              "variant type '{}'", debug(type));
    return nullptr;
  }

  // Elaborate the argument and convert it to the descriminator type.
  Expr* arg = elab_expr(exprs->front());
  if (not arg)
    return nullptr;
  arg = convert(arg, type->desc());
  if (not arg)
    return nullptr;
  arg = reduce(arg);
  if (not arg)
    return nullptr;
  return instantiate_variant(type, arg);
  */
  return nullptr;
}


// In some cases, we can apply arguments directly to types. The
// meaning depends on the type being called. For example:
//
//    def var : typename = variant(int) { 0 : nat; 1: int; }
//    def s : typename = record { n : int; v : var(n); };
//
// In the definition of `s`, the variant member `v` is bound
// to its discriminator `n`.
//
// FIXME: This should also go away.
Expr*
elab_type_call(Call_tree* t, Type* target) {
  if (Dep_variant_type* v = as<Dep_variant_type>(target))
    return elab_variant_binding(t, v);
  error (t->loc) << format("cannot call the type '{}'", debug(target));
  return nullptr;
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
  Def* def = nullptr; // Provides diagnostic context
  Term* fn = nullptr; // The expression actually called
  Fn_type* ft = nullptr; // The type of the fn

  // Unroll the target until we find a function.
  //
  // TODO: What other kinds of targets can we find here.
  if (Decl_id* ref = as<Decl_id>(tgt)) {
    def = as<Def>(ref->decl());
    if (def) {
      Expr* init = def->init();
      if (Term* f = as<Term>(init)) {
        fn = f;
        ft = as<Fn_type>(get_type(fn));
      } else if (Type* ty = as<Type>(init)) {
        // FIXME: This shouldn't be here...
        return elab_type_call(t, ty);
      }
    }
  }

  // Make sure we have a callable expression.
  if (not fn or not fn_type) {
    error(t->fn()->loc) << format("'{}' is not callable", debug(tgt));
    return nullptr;
  }

  // Check the arguments against the function parameters.
  args = check_arguments(t, def, args, ft->parms());
  if (not args)
    return nullptr;

  // Get the result type of the function; that's the type of
  // the expression.
  Type* type = ft->result();

  // TODO: If all of the arguments are constant, then evaluate
  // the result of the call.
  Call* call = make_expr<Call>(t->loc, type, fn, args);
  return call;
}

// -------------------------------------------------------------------------- //
// Elaboration of arrays and subscripting

// Form an array type. The argument `e` must be an integeral constant
// expression.
//
// TODO: If `e` is not constant, then this would be a dependent
// array type. This is similar to the notion of a variable length
// array, but different; dependent types in Steve are pairs that
// associate the dependent term with the type.
Type*
elab_array_type(Index_tree* t, Type* type, Expr* e) {
  Term* bound = as<Term>(e);
  if (not bound) {
    error(e->loc) << format("ill-formed array bound '{}'", debug(e));
    return nullptr;
  }

  // Array subscripts have type nat.
  bound = convert(bound, get_nat_type());
  if (not bound)
    return nullptr;

  // Array subscripts must be fully reduced.
  bound = reduce(bound);
  if (!is_value(bound)) {
    error(e->loc) << format("array bound '{}' is not constant", debug(e));
    return nullptr;
  }

  Type* kind = get_typename_type();
  return make_expr<Array_type>(t->loc, kind, type, bound);
}

Term*
elab_index_term(Index_tree* t, Term* term, Expr* e) {
  sorry(t->loc) << "array subscript not implemented";
  return nullptr;
}


// An expression of the form `e1[e2]` is:
//    - an array type `T[n]` when `e1` denotes a type `T` and `e2
//      is an integral expression `n`.
//    - a subscript operation when `e1` has type `T[N]` and
//      `e2` is an integral expression.
Expr*
elab_index(Index_tree* t) {
  Expr* e1 = elab_expr(t->elem());
  Expr* e2 = elab_expr(t->index());
  if (not e1 || not e2)
    return nullptr;

  if (Type* type = as<Type>(e1))
    return elab_array_type(t, type, e2);
  else if (Term* term = as<Term>(e1))
    return elab_index_term(t, term, e2);
  error(t->loc) << format("ill-formed index expression '{}'", debug(t));
  return nullptr;
}

// -------------------------------------------------------------------------- //
// Elaboration of projections
//
// The projection expression of the for `e1.e2` is a mechanism for 
// explicitly qualifying the lookup of `e2` within the scope `e1`.
// In general, `e1` is expected to be a compound type (e.g., record
// or module), and `e2` a member thereof.
//
// When a projection occurs as the target of an import-stmt, it
// is a module-id.

bool
defines_scope(Type* t) {
  switch (t->kind) {
  case enum_type:
  case record_type:
  case module_type:
    return true;
  default:
    return false;
  }
}

Expr*
elab_dot(Dot_tree* t) {
  Type* s = elab_type(t->scope());
  if (not s)
    return nullptr;
  if (not defines_scope(s)) {
    error(t->loc) << format("'{}' does not define a scope", debug(s)) << '\n';
    return nullptr;
  }

  Scope_guard scope(s);
  return elab_expr(t->member());
}


// -------------------------------------------------------------------------- //
// Elaboration of unary and binary expressions
//
// Unary and binary expressions are elaborated as unary and binary
// terms by looking up a function having the same name as the
// given operator.

// Create a new operator from the given token.
Name*
elab_operator(const Token* k) {
  return new Operator_id(k->loc, k->text);
}

Expr*
elab_unary(Unary_tree* t) {
  // Name* name = elab_operator(t->op());
  return get_bool_type(); // WRONG
}

Expr*
elab_binary(Binary_tree* t) {
  Expr* left = elab_expr(t->left());
  Expr* right = elab_expr(t->right());
  Name* name = elab_operator(t->op());
  if (not left or not right or not name)
    return nullptr;

  Overload* ovl = lookup(name);
  Resolution res = resolve_binary(t->loc, *ovl, left, right);

  // TODO: List candidates.
  if (not res.is_unique()) {
    error(t->loc) << format("no matching function for '{}' with arguments "
                            "{} and {}", 
                            debug(name), 
                            typed(left), 
                            typed(right));
    if (res.is_empty())
      note(t->loc) << "no viable candidates";
    else
      note(t->loc) << "resolution is ambiguous";
    return nullptr;
  }

  Decl* decl = res.solution();
  Term* fn = as<Term>(as<Def>(decl)->init()); // FIXME: Gross
  Type* type = get_type(fn);
  Type* result = as<Fn_type>(type)->result();

  return make_expr<Binary>(t->loc, result, fn, left, right);
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

// A record type is an aggregate type comprised of named fields.
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
  return make_expr<Record_type>(t->loc, kind, fields);
}

// Elaborate an unnamed field.
//
// TODO: Do we want a special node for unnamed fields? Note that
// code gen could use unnamed fields to align members (using alignas)
// rather than just dumping an unused member. Note that we could also
// detect this by its name.
Expr*
elab_unnamed_field(Field_tree* t) {
  if (t->prop()) {
    error(t->prop()->loc) << "an unnamed field cannot be constrained";
    return nullptr;
  }

  Type* type = elab_type(t->type());
  if (not type)
    return nullptr;

  // Build an internal name for the field.
  Record_type* record = current_record();
  std::stringstream ss;
  ss << "__unnamed" << record->field()->size() << "__";
  Name* name = new Basic_id(ss.str());

  return make_expr<Field>(t->loc, type, name, type, nullptr);
}

// Elaborate a field declaration. In general, a field has one of
// the following forms:
//
//    : 
//
// A field declaration of the form:
//
//    : t;
//
// is an unnamed field. It contributes to the layout of the class
// but is not an accessible member.
//
// TODO: Should we auto-complete the missing predicate with the
// trivial constraint "true" or "top" or something like that?
Expr*
elab_field(Field_tree* t) {
  if (not t->name())
    return elab_unnamed_field(t);

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
Decl*
elab_alt(Alt_tree* t, Type* var) {
  // Get the context for a descriminated variant type.
  Dep_variant_type* dep = as<Dep_variant_type>(var);

  // Elaborate the tag and type.
  Expr* tag = elab_alt_tag(t->tag(), dep);
  Type* type = elab_type(t->type());
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
  //
  // if (var)
  //   check_convertible(tag, var->desc());

  return make_expr<Alt>(t->loc, type, tag, type);
}

// Elaborate a sequence of variants nested within the variant type.
// Note that alternatives are never declared.
Expr*
elab_alternatives(Variant_tree* t, Type* var, Decl_seq* alts) {
  for (Tree *a : *t->alts()) {
    if (Decl* d = elab_alt(as<Alt_tree>(a), var))
      alts->push_back(d);
    else
      return nullptr;
  }
  return var;
}

// Elaborate a plain variant. A plain variant is a sequence of
// alternatives of the form `n:t` where `n` is an identifier
// and `t` is a type.
Expr*
elab_variant(Variant_tree* t) {
  // Stub ot the variant.
  Decl_seq* alts = new Decl_seq();
  Type* kind = get_typename_type();
  Type* var = make_expr<Variant_type>(t->loc, kind, alts);
  return elab_alternatives(t, var, alts);
}

// Elaborate a dependent variant. A dependent variant is
// a sequence of alternatives whose discriminators are defined
// in terms of a parameter `p`. Each alternative has the
// form `d:t` where `d` is a value of `p` and `t` is its
// corresponding type.
//
// The expression `d` must be one of:
//
//    `e` where `e` is a term that is convertible to 't'
//    `e1..e2` where `e1..e2` is a range of values of type `t`
//    `default`
Expr*
elab_dependent_variant(Variant_tree* t) {
  Parm* parm = elab_as_parm(t->arg());
  if (not parm)
    return nullptr;

  Decl_seq* alts = new Decl_seq();
  Type* kind = get_typename_type();
  Type* var = make_expr<Dep_variant_type>(t->loc, kind, parm, alts);
  return elab_alternatives(t, var, alts);
}


// Elaborate a variant declaration.
Expr*
elab_variant_type(Variant_tree* t) {
  if (t->arg())
    return elab_dependent_variant(t);
  else
    return elab_variant(t);
}

// -------------------------------------------------------------------------- //
// Elaboration of enumerated types

// The terms in a range constructor must be convertible to the base type. 
Expr*
elab_range_ctor(Range_tree* t, Enum_type* type) {
  Range* range = as<Range>(elab_term(t));
  if (not range)
    return nullptr;

  // The range type must be convertible to the enumeration's
  // base type.
  if (not check_convertible(range->lower(), type->base()))
    return nullptr;

  return range;
}

// For an expression of the form 'n = t', 'n' must be a basic-id
// and 't' must be a value whose type is convertible to the base
// type of the enumeration.
Expr*
elab_named_ctor(Binary_tree* t, Enum_type* type) {
  Name* name = elab_name(t->left());
  Expr* value = elab_expr(t->right());
  if (not name || not value)
    return nullptr;

  // Ensure that the value is convertible to the base type,
  // and partially evaluate the results.
  value = check_convertible(value, type->base());
  if (not value)
    return nullptr;
  value = check_constant(value);
  if (not value)
    return nullptr;

  Decl* decl = make_expr<Enum>(t->loc, type, name, value);
  if (declare(decl))
    return decl;
  return nullptr;
}

// Elaborate the constructor of an enum tree. Only two
// kinds of expressions are admissable constructors of an
// enum type: ranges and applications.
//
// TODO: Allow an id-tree to define a range-ctor when the base
// type is well-founded. The semantics are interesting.
Expr*
elab_enum_ctor(Tree* t, Enum_type* type) {
  if (Range_tree* r = as<Range_tree>(t))
    return elab_range_ctor(r, type);
  if (Binary_tree* b = as<Binary_tree>(t))
    if (b->op()->kind == equal_tok)
      return elab_named_ctor(b, type);

  // FIXME: This should probably be an assertion.
  error(t->loc) << format("invalid enumerator '{}'", debug(t));
  return nullptr;
}

// Elaborate the constructors of an enum type.
Expr*
elab_enum_ctors(Enum_tree* t, Enum_type* type, Expr_seq* ctors) {
  Scope_guard scope(enum_scope, type);
  for (Tree* c : *t->ctors()) {
    if (Expr* e = elab_enum_ctor(c, type))
      ctors->push_back(e);
    else
      return nullptr;
  }
  return type;  
}

// Elaborate a parse tree representing an enum type.
Expr*
elab_enum_type(Enum_tree* t) {
  // Elaborate the underlying type. If not given, then it is defined
  // to be int.
  Type* type = nullptr;
  if (t->base())
    type = elab_type(t->base());
  else
    type = get_int_type();
  if (not type)
    return nullptr;

  // Stub out the enumeration type.
  Type* kind = get_typename_type();
  Expr_seq* ctors = new Expr_seq();
  Enum_type* enu = make_expr<Enum_type>(t->loc, kind, type, ctors);
  return elab_enum_ctors(t, enu, ctors);
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

  // Compute the type of the function and update
  // the parsed function.
  Type* type = make_fn_type(parms, result);
  fn->tr = type;
  
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

// -------------------------------------------------------------------------- //
// Elaboration of imports
//
// An import declaration causes the loading of the named module. The
// module name is a nested name (a sequence of projections of identifiers)
// that identifies a module (either a directory or a Steve file) in the
// file system. For example:
//
//    import std.net.ofp;
//
// This causes the compiler to load the following modules:
//
//    ${STEVE_MODULE_PATH}/std           (a directory)   
//    ${STEVE_MODULE_PATH}/std/net       (a directory)
//    ${STEVE_MODULE_PATH}/std/ofp.steve (a file)
//
// Note that the resulting declaration adds only the outermost module
// to the current scope. That is, the `import` declaration above
// binds the name `std` to the module `${STEVE_MODULE_PATH}/std`.
//
// The recursive loading of modules also adds an import declaration
// to each loaded module. That is, `std` imports `net`, and `net`
// imports `ofp`. This allows lookup of nested names.
//
// Note that current will always point to the last module that
// was succesfully loaded.

bool
elab_module_id(Module*& first, Module*& current, Id_tree* t) {
  Module* mod = load_module(t->loc, current, t->value()->text);
  if (not mod)
    return false;
  current = mod;
  if (not first)
    first = current;
  return true;
}

Tree*
elab_nested_module_name(Module*& first, Module*& current, Dot_tree* t) {
  // Recursively load modules for nested names.  
  Tree* scope = t->scope();
  if (Dot_tree* dot = as<Dot_tree>(scope)) {
    scope = elab_nested_module_name(first, current, dot);
    if (not scope)
      return nullptr;
  }

  // Make sure that the left-most expression is an identifier.
  Id_tree* id = as<Id_tree>(scope);
  if (not id) {
    error(scope->loc) << format("ill-formed module-id '{}'", debug(scope));
    return nullptr;
  }

  // Load the enclosing module.
  if (not elab_module_id(first, current, id))
    return nullptr;
  
  // Return the right-most expression.
  return t->member();
}

bool
elab_module_name(Module*& first, Module*& current, Tree* t) {
  // Adjust the elaboration for nested module names.
  if (Dot_tree* dot = as<Dot_tree>(t))
    t = elab_nested_module_name(first, current, dot);
  if (not t)
    return false;

  // Elabarate the trailing id-expression.
  if (Id_tree* id = as<Id_tree>(t)) {
    if (not elab_module_id(first, current, id))
      return false;
  } else {
    error(t->loc) << format("ill-formed module-id '{}'", debug(t));
    return false;
  }
  return true;
}

Module*
elab_module_name(Tree* t) {
  Module* first = nullptr;
  Module* current = nullptr;

  if (not elab_module_name(first, current, t))
    return nullptr;
  else
    return first;
}

Expr*
elab_import(Import_tree* t) {
  // Name* n = elab_outermost_module_name(t->module());
  Module* m = elab_module_name(t->module());
  if (not m)
    return nullptr;

  // Declare the outermost imported module.
  Decl* imp = make_expr<Import>(t->loc, m, m->name(), m);
  declare(imp);
  return imp;
}


// ---------------------------------------------------------------------------//
// Elaboration of using declrations
//
// A using declaration adds the name of an element in a different module
// to the current scope. For example:
//
//    import std.net.ofp;
//    using std.net.ofp.uint;
//    def x : uint(8) = 3;
//
// The `using` declaration makes the definition of the `uint` type
// constructor available to the current scope.

// A using declaration names an element in another module.
Expr*
elab_using(Using_tree* t) {
  Expr* expr = elab_expr(t->name());
  if (not expr)
    return nullptr;
  Decl_id* id = as<Decl_id>(expr);
  if (not id) {
    error(t->loc) << format("using target '{}' does not refer to a "
                            "declaration", debug(expr));
    return nullptr;
  }

  Name* name = id->name();
  Decl* decl = id->decl();

  // Make sure that we can actually use this!
  if (not is<Def>(decl)) {
    error(t->loc) << format("using target '{}' is not a definition", debug(t));
    return nullptr;
  }

  declare(id->name(), id->decl());
  return make_expr<Using>(t->loc, get_type(decl), name, decl);
}


// ---------------------------------------------------------------------------//
// Elaboration of load expressions.
//
// A load expression is a reference to a declaration that is (probably)
// not imported. The primary use of load expressions is to query a
// program for properties of a declaration.
//
// Diagnostics are fully suppressed for this elaboration.

// The specific method of elaboration is a little odd. First, we try
// to elaborate the laod of a module. This will cause any module names
// to be resolved within the loaded name. If the name happens to refer
// to a module, we're done. If not, then we try to resolve the
// declaration as an id. Note that we have already loaded any modules
// occuring as part of the id.
Expr*
elab_load(Load_tree* t) {
  Diagnostics diags;
  Diagnostics_guard guard(diags);

  // Try to elaborate the name as a module. If we succeed,
  // the we don't need to do anything else.
  Module* first = nullptr;
  Module* current = nullptr;
  bool ok = elab_module_name(first, current, t->name());
  if (ok)
    return first;
  if (not first)
    return nullptr;

  // Otherwise, declare the first resolved module.
  Decl* imp = make_expr<Import>(t->loc, first, first->name(), first);
  declare(imp);

  // Now, try to resolve the name in the usual way.
  Expr* e = elab_expr(t->name());
  if (not e)
    return nullptr;

  // See through declaration.
  if (Decl_id* id = as<Decl_id>(e))
    return id->decl();
  else
    return e;
}


// ---------------------------------------------------------------------------//
// Miscellaneous expressions

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
elab_expr(Tree* t) {
  if (not t)
    return nullptr;

  switch (t->kind) {
  case id_tree: return elab_id(as<Id_tree>(t));
  case lit_tree: return elab_lit(as<Lit_tree>(t));
  case call_tree: return elab_call(as<Call_tree>(t));
  case index_tree: return elab_index(as<Index_tree>(t));
  case dot_tree: return elab_dot(as<Dot_tree>(t));
  case range_tree: return elab_range(as<Range_tree>(t));
  case unary_tree: return elab_unary(as<Unary_tree>(t));
  case binary_tree: return elab_binary(as<Binary_tree>(t));
  // Types
  case record_tree: return elab_record(as<Record_tree>(t));
  case variant_tree: return elab_variant_type(as<Variant_tree>(t));
  case enum_tree: return elab_enum_type(as<Enum_tree>(t));
  // Declarations
  case parm_tree: return elab_parm(as<Parm_tree>(t));
  case def_tree: return elab_def(as<Def_tree>(t));
  case field_tree: return elab_field(as<Field_tree>(t));
  case import_tree: return elab_import(as<Import_tree>(t));
  case using_tree: return elab_using(as<Using_tree>(t));
  case load_tree: return elab_load(as<Load_tree>(t));
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