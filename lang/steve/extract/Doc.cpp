
#include <steve/extract/Doc.hpp>
#include <steve/Ast.hpp>

#include <iostream>

namespace steve {

namespace {

// Emit documentation for the given definition.
void
document_def(Def* def) {
  std::cout << debug(def) << '\n';
}

} // namespace

void
extract_docs(Expr* e)
{
  switch (e->kind) {
  case def_decl: return document_def(as<Def>(e));
  default:
    steve_unreachable(format("cannot document '{}'", debug(e)));
  }
}

} // namespace steve
