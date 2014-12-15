
namespace steve {

// Returns the type of the expression e. Note that the type is null if
// e is a name or the type is uncomputed.
inline Type*
get_type(Expr* e) { return e->tr; }

} // namespace steve
