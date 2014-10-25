
#ifndef STEVE_CONV_HPP
#define STEVE_CONV_HPP

namespace steve {

struct Expr;
struct Type;

Expr* convert(Expr*, Type*);

} // namespace steve

#endif
