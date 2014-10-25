
#include <steve/Subst.hpp>
#include <steve/Intrinsic.hpp>
#include <steve/Debug.hpp>

namespace steve {

template<typename C, typename T>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const Subst& s) {
    for (auto&& p : s)
      os << debug(p.first) << " ~> " << debug(p.second) << '\n';
    return os;
  }


// -------------------------------------------------------------------------- //
// Substitution

// Initialize the substitution with the mapping of a sequence of
// arguments to parameters.
Subst::Subst(Decl_seq* parms, Expr_seq* args) {
  steve_assert(parms->size() == args->size(), "parameter/argument mismatch");
  auto first1 = parms->begin();
  auto first2 = args->begin();
  while (first1 != parms->end()) {
    insert({*first1, *first2});
    ++first1;
    ++first2;
  }
}

Expr*
Subst::get(Decl* d) const {
  auto iter = find(d);
  if (iter != end())
    return iter->second;
  else
    return iter->first;
}

// -------------------------------------------------------------------------- //
// Substitution rules

namespace {

// Substitute through the intrinsic function.
Expr*
subst_intrinsic(Intrinsic* e, const Subst& s) {
  return e->subst(s);
}

// Substitute into a function of the form \(p*).e.
//
//    [s]\(p*).e = \(p*).[s]e
//
// FIXME: Do I need to substitute into the paramter list?
Expr*
subst_fn(Fn* e, const Subst& s) {
  if (e->is_intrinsic())
    return subst_intrinsic(as<Intrinsic>(e), s);
  else
    return subst(e->body(), s);
  return e;
}

// Substitute into a call expression.
//
//    [s]f(a*) = [s](f) ([s]ai)
//
// FIXME: Do I really substitute into the target? Maybe, maybe not.
Expr*
subst_call(Call* e, const Subst& sub) {
  std::cout << "UH... " << debug(e) << '\n';
  std::cout <<"SUBST\n" << sub << '\n';
  return e;
}

} // namespace


// Replace, in the expression e, all those declarations in the
// the substitution s that are mapped to terms.
Expr* 
subst(Expr* e, const Subst& s) {
  switch (e->kind) {
  case fn_term: return subst_fn(as<Fn>(e), s);
  case call_term: return subst_call(as<Call>(e), s);
  default:
    steve_unreachable(format("undefined substitution '{}'", node_name(e)));
  }
  return e;
}

} // namespace steve
