
#ifndef STEVE_CONV_HPP
#define STEVE_CONV_HPP

#include <steve/Ast.hpp>

#include <vector>

namespace steve {

struct Expr;
struct Type;

Expr* convert(Expr*, Type*);
Term* convert(Term*, Type*);

// A ranked conversion is a numeric quantity that describes the
// conversions applied to an expression.
using Conversion = int;

// A conversion list is a list of ranked conversions for a sequence
// of expressions.
using Conversion_list = std::vector<int>;

Conversion_list rank_conversions(Expr_seq*);
Conversion rank_conversion(Expr*);

} // namespace steve

#endif
