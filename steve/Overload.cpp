
#include <steve/Overload.hpp>
#include <steve/Scope.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

namespace steve {

// Returns true if t1 and t2 have equivalent parameter type lists.
bool
equivalent_parameters(Fn_type* t1, Fn_type* t2) {
  Type_seq* p1 = t1->parms();
  Type_seq* p2 = t2->parms();
  if (p1->size() != p2->size())
    return false;
  else
    return std::equal(p1->begin(), p1->end(), p2->begin(), is_same);
}

// Check if d1 can be overloaded with d2. Note that, d1 is the 
// candidate declaration and d2 is an existing declaration.
//
// Only function declarations can be overloaded (note that
// this includes type functions). However, functions whose signatures
// differ only in their result type cannot be overloaded.
//
// Also note that a builtin is technically a function although
// not derived from that class.
//
// NOTE: When called during elaboration of a parse tree, the
// initializer of d1 will not have been parsed when the entity
// is declared.
bool
check_overloadable(Decl* d1, Decl* d2) {
  // Determine overloading based on the types of d1 and d2.
  Fn_type* t1 = as<Fn_type>(get_type(d1));
  Fn_type* t2 = as<Fn_type>(get_type(d2));

  // Only functions can be overloaded.
  if (not t1) {
    error(d1->loc) << format("cannot overload '{}' becausse "
                             "it is not a function", 
                             debug(d1));
    return false;
  }

  // Redeclaration of a different kind.
  if (not t2) {
    error(d1->loc) << format("redefining '{}' as a different kind of symbol",
                             debug(d1));
    note(d2->loc) << format("  previous definition is '{}'", debug(d2));
    return false;
  }

  // Functions whose signatures differ only in their result types
  // cannot be overloaded.
  //
  // TODO: Allow exceptions in the case where a result type depends
  // on a deduced parameter.
  if (equivalent_parameters(t1, t2)) {
    if (is_same(t1->result(), t2->result())) {
      error(d1->loc) << format("redefinition of '{}'", debug(d1));
      note(d2->loc) << format("  previous definition is '{}'", debug(d2));
    } else {
      error(d1->loc) << format("cannot overload '{}' and '{}' because "
                               "they differ only their result types",
                               debug(d1), debug(d2));
    }
    return false;
  }

  return true;
}

// Returns true if d can be overloaded with the declarations
// in ovl. 
//
// Diagnostics are emitted if an overload is not possible.
bool
check_overloadable(Overload& ovl, Decl* d) {
  steve_assert(not ovl.empty(), "empty overload set");
  for (Decl* d1 : ovl) {
    if (not check_overloadable(d, d1))
      return false;
  }
  return true;
}

// Try to overload the definition d, given the existing overload
// set ovl. Returns true on success and false on failure.
//
// TODO: This is going to be a big function. Move it into its own
// module.
//
// TODO: Enforce the rule that all overloads must have the same
// return type or kind. That is, a set of functions produces only
// values or types.
bool
overload(Overload& ovl, Decl* d) {
  if (check_overloadable(ovl, d)) {
    ovl.push_back(d);
    return true;
  }
  return false;
}

} // namespace steve
