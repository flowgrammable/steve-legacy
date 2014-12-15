

#include <steve/Language.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Intrinsic.hpp>
#include <steve/Debug.hpp>

#include <iostream>

namespace steve {

namespace {
// Language initialization flag.
bool init_ = false;

void
init_lang() {
  steve_assert(not init_, "language already initialized");
  if (not init_)
    init_ = true;
}

} // namespace

extern void init_tokens();
extern void init_trees();
extern void init_exprs();
extern void init_types();
extern void init_intrinsics();

Language::Language() 
  : Scope_guard(builtin_scope)
{
  init_lang();
  init_tokens();
  init_trees();
  init_exprs();
  init_types();
  init_intrinsics();
}

Language::~Language() { }

} // namespace steve
