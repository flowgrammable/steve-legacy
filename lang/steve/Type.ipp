
namespace steve {

// Returns the type of the expression e. Note that the type is null if
// e is a name or the type is uncomputed.
inline Type*
type(Expr* e) { return e->type_; }

} // namespace steve
