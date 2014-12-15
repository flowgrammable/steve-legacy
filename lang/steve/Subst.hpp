
#ifndef STEVE_SUBST_HPP
#define STEVE_SUBST_HPP

#include <steve/Ast.hpp>

#include <map>

namespace steve {

// A substitution is a mapping between declarations (generally parameters)
// and an expression with which they are being replaced.
struct Subst : std::map<Decl*, Expr*> {
  using std::map<Decl*, Expr*>::map;

  Subst(Decl_seq*, Expr_seq*);

  Expr* get(Decl*) const;
};

Expr* subst(Expr*, const Subst&);

} // namespace steve

#endif
