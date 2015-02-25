
namespace steve {

// Return the context in which d is declared.
inline Expr*
context(Decl* d) { return d->cxt_; }

} // namespace steve