
#include <iostream>

#include <steve/Scope.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

namespace steve {

namespace {

// Return a textual representation of the scope kind.
String
scope_name(Scope_kind k) {
  switch (k) {
  case module_scope: return "module scope";
  case record_scope: return "record scope";
  case variant_scope: return "variant scope";
  case enum_scope: return "enum scope";
  case function_scope: return "function scope";
  case block_scope: return "block scope";
  }
  steve_unreachable("unkown scope kind");
}

} // namespace

// -------------------------------------------------------------------------- //
// Name comparison
//
// TODO: Generalize this for all expressions.

namespace {

template<typename T>
  inline bool 
  literal_less(const T* a, const T* b) { return a->first < b->first; }

} // namespace

std::size_t
Name_less::operator()(const Name* a, const Name* b) const {
  if (a->kind < b->kind)
    return true;
  if (b->kind < a->kind)
    return false;
  
  switch (a->kind) {
  case basic_id: return literal_less(as<Basic_id>(a), as<Basic_id>(b));
  default: break;
  }
  steve_unreachable(format("unhandled node '{}'", node_name(a)));
}

// -------------------------------------------------------------------------- //
// Scope

namespace {
// Try to overload the definition d, given the existing overload
// set ovl. Returns true on success and false on failure.
//
// TODO: This is going to be a big function. Move it into its own
// module.
//
// TODO: Enforce the rule that all overloads must have the same
// return type or kind. That is, a set of functions produces only
// values or types.
bool
overload(Overload& ovl, Decl* d) {
  error(d->loc) << format("redefinition of '{0}'", debug(d));
  for (Decl* d : ovl)
    note(d->loc) << format("previous definition is '{0}'", debug(d));
  return false;
}

// Make sure that we've initialized the scope correctly.
inline void
check_scope_and_context(Scope_kind k, Expr* c) {
  if (not c)
    return;
  switch (k) {
  case module_scope:
    // FIXME: What should this do?
    return;
  case record_scope:
    steve_assert(is<Record_type>(c), format("context '{}' is not a record type", debug(c)));
    break;
  case variant_scope:
    steve_assert(is<Variant_type>(c) || is<Variant_of_type>(c),
                 format("context '{}' is not a kind of variant", debug(c)));
    break;
  case enum_scope:
    steve_assert(is<Enum_type>(c), format("context '{}' is not a enum type", debug(c)));
    break;
  case function_scope:
    steve_assert(is<Fn>(c), format("context '{}' is not a function", debug(c)));
    break;
  case block_scope:
    // FIXME: Do something here...
    break;
  }
}

} // namespace

inline
Scope::Scope(Scope_kind k, Scope* p, Expr* c)
  : kind(k), parent(p), context(c) 
{
  check_scope_and_context(k, c);
}

// Return a declaration corresponding to the given name.
const Overload*
Scope::lookup(Name* n) {
  auto iter = find(n);
  if (iter != end())
    return &iter->second;
  
  // TODO: There might be some scope-specific lookup rules here.
  // For example, in class scope, we might search through base
  // classes if we had them.
  
  // Check the parent scope.
  if (parent)
    return parent->lookup(n);
  return nullptr;
}

// Insert the given declaration into this scope. If this is not the
// first declaration with the given name, then we need to determine
// if the definition can be overloaded.
const Overload*
Scope::declare(Name* n, Decl* d) {
  auto result = insert({n, {d}});
  Overload& ovl = result.first->second;
  if (not result.second) {
    if (overload(ovl, d))
      return &ovl;
    else
      return nullptr;
  }
  return &ovl;
}

namespace {

// Pointer to the scope stack.
Scope* stack_ = nullptr;

} // namespace

// -------------------------------------------------------------------------- //
// Declarations

// Lookup the given name in the current scope, returning its overload
// iset, if any.
const Overload*
lookup(Name* n) {
  steve_assert(stack_, "lookup on empty scope stack");
  return stack_->lookup(n);
}

// Lookup the given name, which is expected to refer to a single
// entity, in the given scope. If no such name exists, or the name
// refers to multiple entities, emit an error.
Decl*
lookup_single(Name* n) {
  if (const Overload* ovl = lookup(n)) {
    if (ovl->singleton())
      return ovl->front();
    error(n->loc) << format("'{}' refers to multiple entities:", debug(n));
    for (Decl* d : *ovl) {
      note(d->loc) << format("  - {}", debug(d));
    }
  } else {
    error(n->loc) << format("no matching declaration for '{}'", debug(n));
  }
  return nullptr;
}

// Declare the given definition in the current scope, returning its
// overload set.
const Overload*
declare(Name* n, Decl* d) {
  steve_assert(stack_, "declaration on empty scope stack");
  return stack_->declare(n, d);
}

// Declare the given definition in the the enclosing scope of the 
// current scope. This is used to leak a definiton ouside a 
// particular scope. Return the overload of the new declaration.
const Overload*
declare_outside(Decl* d) { 
  return stack_->parent->declare(get_name(d), d); 
}

// -------------------------------------------------------------------------- //
// Scope management

// Push a new scope of the given kind, associating the context, c.
Scope*
push_scope(Scope_kind k, Expr* c) {
  Scope* s = new Scope(k, stack_, c);
  stack_ = s;
  return stack_;
}

Scope*
pop_scope() {
  steve_assert(stack_, "pop on empty scope stack");
  Scope* s = stack_->parent;
  delete stack_;
  stack_ = s;
  return stack_;
}

// Returns the current scope, or null if the current stack is 
// uninitialized.
Scope* 
current_scope() { return stack_; }

// Returns the context associated with the current scope.
Expr*
current_context() { return current_scope()->context; }

Type* 
current_type() { return as<Type>(current_scope()->context); }

Record_type* 
current_record() { return as<Record_type>(current_scope()->context); }

Variant_type* 
current_variant() { return as<Variant_type>(current_scope()->context); }

Enum_type* 
current_enumeration() { return as<Enum_type>(current_scope()->context); }

} // namespace steve
