
#ifndef STEVE_ELABORATOR_HPP
#define STEVE_ELABORATOR_HPP

#include <steve/Error.hpp>

// The elaborator module is responsible for the elaboration of
// parse trees as syntax trees. The preferred interface is the
// Elaborator class, which accumulates its own diagnostics.
//
// Note that a number of supporing  functions are also made 
// accessible. This is largely due to the fact that elaboration is
// spread over a number of different implementaiton files.

namespace steve {

struct Tree;
struct Expr;

// The elaborator is responsible for transforming a parse tree into
// a fully elaborated and partially evaluated abstract syntax tree.
struct Elaborator {
  Expr* operator()(Tree*);

  Diagnostics diags;
};

} // namespace

#endif
