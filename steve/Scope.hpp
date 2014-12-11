
#ifndef STEVE_SCOPE_HPP
#define STEVE_SCOPE_HPP

#include <iosfwd>
#include <map>
#include <vector>

#include <steve/Ast.hpp>
#include <steve/Decl.hpp>

namespace steve {

// The scope kind defines the various kinds of scopes within the
// steve language.
enum Scope_kind {
  builtin_scope,
  module_scope,
  record_scope,
  variant_scope,
  enum_scope,
  function_scope,
  block_scope
};

// An overload set is a list of definitions sharing the same name,
// but having different types and definitions.
struct Overload : std::vector<Decl*> {
  using std::vector<Decl*>::vector;

  bool singleton() const;
};

// A The hash name function generates a hash of a name.
struct Name_less {
  std::size_t operator()(const Name*, const Name*) const;
};

// The Scope class defines a mapping from names to declarations.
// Each scope has a kind, which defines the semantic properties
// of that scope, and a parent scope. Note that the top-level
// scope has no parent.
//
// A scope's context is the declaration (if any) that introduces
// the scope (e.g., a function, record, variant, etc).
struct Scope : std::map<Name*, Overload, Name_less> {
public:
  Scope(Scope_kind, Scope*);
  Scope(Scope_kind, Scope*, Expr*);

  Overload* lookup(Name*);
  Overload* declare(Name*, Decl*);
  
  Scope_kind kind;
  Scope* parent;
  Expr* context;
};


// -------------------------------------------------------------------------- //
// Declarations

Overload* lookup(Name*);
Overload* declare(Name*, Decl*);
Overload* declare(Decl*);
Overload* declare_outside(Decl*);
Decl* lookup_single(Name*);


// -------------------------------------------------------------------------- //
// Scope management

Scope* push_scope(Scope_kind k, Expr* = nullptr);
Scope* push_scope(Type*);
Scope* pop_scope();
Scope* current_scope();
Expr* current_context();
Type* current_type();
Record_type* current_record();
Variant_type* current_variant();
Enum_type* current_enumeration();


// -------------------------------------------------------------------------- //
// Scope guard

// The scope guard is an RAII helper class that provides scope-based
// pushing and popping of scopes.
struct Scope_guard {
  Scope_guard(Type*);
  Scope_guard(Scope_kind, Expr* = nullptr);
  ~Scope_guard();
};


// -------------------------------------------------------------------------- //
// Debugging support

struct debug_scope { Scope* s; };
struct debug_overload { Overload* o; };

inline debug_scope
debug(Scope* s) { return {s}; }

inline debug_overload
debug(Overload* o) { return {o}; }

std::ostream& operator<<(std::ostream&, debug_scope);
std::ostream& operator<<(std::ostream&, debug_overload);

} // namespace steve

#include <steve/Scope.ipp>

#endif
