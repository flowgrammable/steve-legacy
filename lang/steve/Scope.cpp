
#include <iostream>

#include <steve/Scope.hpp>
#include <steve/Overload.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

namespace steve {

// Return a textual representation of the scope kind.
const char*
scope_name(Scope_kind k) {
  switch (k) {
  case builtin_scope: return "builtin";
  case module_scope: return "module";
  case record_scope: return "record";
  case variant_scope: return "variant";
  case enum_scope: return "enum";
  case function_scope: return "function";
  case block_scope: return "block";
  }
  steve_unreachable("unkown scope kind");
}

// Return a textual representation of the scope kind.
const char*
scope_name(const Scope* s) {
  return scope_name(s->kind);
}

// -------------------------------------------------------------------------- //
// Name comparison
//
// TODO: Generalize this for all expressions.

namespace {

template<typename T>
  inline bool 
  literal_less(const T* a, const T* b) { 
    return a->first < b->first; 
  }

} // namespace

std::size_t
Name_less::operator()(const Name* a, const Name* b) const {
  if (a->kind < b->kind)
    return true;
  if (b->kind < a->kind)
    return false;
  
  switch (a->kind) {
  case basic_id: return literal_less(as<Basic_id>(a), as<Basic_id>(b));
  case operator_id: return literal_less(as<Operator_id>(a), as<Operator_id>(b));
  default: break;
  }
  steve_unreachable(format("unhandled node '{}'", node_name(a)));
}

// -------------------------------------------------------------------------- //
// Scope

namespace {

// Make sure that we've initialized the scope correctly.
inline void
check_scope_and_context(Scope_kind k, Expr* c) {
  if (not c)
    return;
  switch (k) {
  case builtin_scope:
  case module_scope:
    // FIXME: What should this do?
    return;
  case record_scope:
    steve_assert(is<Record_type>(c), 
                 format("context '{}' is not a record type", debug(c)));
    break;
  case variant_scope:
    steve_assert(is<Variant_type>(c) || is<Dep_variant_type>(c),
                 format("context '{}' is not a kind of variant", debug(c)));
    break;
  case enum_scope:
    steve_assert(is<Enum_type>(c), 
                 format("context '{}' is not a enum type", debug(c)));
    break;
  case function_scope:
    steve_assert(is<Fn>(c), 
                 format("context '{}' is not a function", debug(c)));
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
Overload*
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
Overload*
Scope::declare(Name* n, Decl* d) {
  auto result = insert({n, {d}});
  Overload& ovl = result.first->second;
  
  // If this didn't succeed, ovl is non-empty, so we need to
  // determine if we can add d to that set.
  if (not result.second) {
    if (declare_overload(ovl, d))
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
Overload*
lookup(Name* n) {
  steve_assert(stack_, "lookup on empty scope stack");
  return stack_->lookup(n);
}

// Lookup the given name, which is expected to refer to a single
// entity, in the given scope. If no such name exists, or the name
// refers to multiple entities, emit an error.
Decl*
lookup_single(Name* n) {
  if (Overload* ovl = lookup(n)) {
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
Overload*
declare(Name* n, Decl* d) {
  steve_assert(stack_, "declaration on empty scope stack");
  return stack_->declare(n, d);
}

// Declare the given definition in the the enclosing scope of the 
// current scope. This is used to leak a definiton ouside a 
// particular scope. Return the overload of the new declaration.
Overload*
declare_outside(Decl* d) { 
  return stack_->parent->declare(name(d), d); 
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

// Add the sequence of declarations to the current scope.
inline void
declare(Decl_seq* decls) {
  for (Decl* d : *decls)
    declare(d);
}

// Add all declarations (among the sequence of expressions) to
// the current scope.
inline void
declare(Expr_seq* exprs) {
  for (Expr* e : *exprs) {
    if (Decl* d = as<Decl>(e))
      declare(d);
  }
}


// FIXME: This is not terribly efficient because we have to re-do all
// of the re-declaration work. It would be better if each scoped type
// saved it's corresponding scope, we could just push that.
Scope*
push_module_scope(Module* m) {
  Scope* s = push_scope(module_scope, m);
  declare(m->decls());
  return s;
}

Scope*
push_enum_scope(Enum_type* e) {
  Scope* s = push_scope(enum_scope, e);
  declare(e->ctors());
  return s;
}

// Push the declarations corresponding to the given type onto the
// scope stack.
Scope*
push_scope(Type* t) {
  switch (t->kind) {
  case module_type:
    return push_module_scope(as<Module>(t));
  case record_type:
    steve_unreachable("unimplemented: push record scope");
  case enum_type:
    return push_enum_scope(as<Enum_type>(t));
  default:
    steve_unreachable(format("cannot push non-aggregate type '{}'", debug(t)));
  }
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

// FIXME: These may not be right. If we allow the definition of
// member functions, then we might want to walk upwards to the
// innermost record scope, for example

Type* 
current_type() { return as<Type>(current_scope()->context); }

Record_type* 
current_record() { return as<Record_type>(current_scope()->context); }

Variant_type* 
current_variant() { return as<Variant_type>(current_scope()->context); }

Enum_type* 
current_enumeration() { return as<Enum_type>(current_scope()->context); }

// Return the current function. This will search outwards to the
// innermost function scope.
Fn*
current_function() {
  Scope* s = current_scope();
  while (s->kind != function_scope) {
    s = s->parent;
  }
  return s ? as<Fn>(s->context) : nullptr;
}

// -------------------------------------------------------------------------- //
// Debugging support
//
// FIXME: Use the pretty printer to manage indentation.

std::ostream&
operator<<(std::ostream& os, debug_scope d) {
  Scope* s = d.s;
  for (auto& x : *s)
    os << debug(x.first) << " : " << debug(&x.second) << '\n';
  return os;
}

std::ostream&
operator<<(std::ostream& os, debug_overload d) {
  Overload* o = d.o;
  if (o->singleton()) {
    os << debug(o->front());
  } else {
    os << "{\n";
    for (auto* x : *o)
      os << "  " << debug(x) << '\n';
    os << "}";
  }
  return os;
}

} // namespace steve
