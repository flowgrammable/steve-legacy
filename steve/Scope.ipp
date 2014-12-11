
namespace steve {

// -------------------------------------------------------------------------- //
// Overload sets

inline bool
Overload::singleton() const { return size() == 1; }


// -------------------------------------------------------------------------- //
// Scope

inline
Scope::Scope(Scope_kind k, Scope* p)
  : kind(k), parent(p), context(nullptr) { }

// -------------------------------------------------------------------------- //
// Declarations

inline Overload* 
declare(Decl* d) { return declare(get_name(d), d); }

// -------------------------------------------------------------------------- //
// Scope guaard

inline
Scope_guard::Scope_guard(Scope_kind k, Expr* e) {
  push_scope(k, e);
}

inline
Scope_guard::Scope_guard(Type* t) {
  push_scope(t);
}

inline
Scope_guard::~Scope_guard() {
  pop_scope();
}

} // namespace steve
