
#include <steve/Overload.hpp>
#include <steve/Algorithm.hpp>
#include <steve/Scope.hpp>
#include <steve/Ast.hpp>
#include <steve/Type.hpp>
#include <steve/Conv.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

#include <iostream>

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
    error(d1->loc) << format("cannot overload '{}' because "
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

// Try to overload the declaration d, given the existing overload
// set ovl. Returns true on success and false on failure.
bool
declare_overload(Overload& ovl, Decl* d) {
  if (check_overloadable(ovl, d)) {
    ovl.push_back(d);
    return true;
  }
  return false;
}

// -------------------------------------------------------------------------- //
// Overload resultion

// If def defines a function, return that.
Term*
get_function_candidate(Def* def) {
  return as<Fn>(def->init());
}

// If def defines a builtin function, return that.
Term*
get_builtin_candidate(Def* def) {
  if (Builtin* b = as<Builtin>(def->init()))
    if (as<Fn_type>(get_type(b)))
      return b;
  return nullptr;
}

// Build the candidate list. A candidate is a function declaration or
// builtin function declaration.
//
// TODO: Ultimately, overload resolution chooses a function. It
// may, in the future, be possible that we have some declarations
// for which we go in search of other functions to call (e.g.,
// constructors of a type).
Candidate_list
gather_candidates(Location loc, Overload& ovl, Expr_seq* args) { 
  Candidate_list cands;
  for (Decl* d : ovl) {
    if (Def* def = as<Def>(d)) {
      if (Term* f = get_function_candidate(def))
        cands.emplace_back(def, f, args);
      else if (Term* f = get_builtin_candidate(def))
        cands.emplace_back(def, f, args);
      else
        ; // This is not a candidate.
    } else {
      // This is also not a candidate; It's probably an error.
    }
  }
  return cands;
}


// Match a sequence of arguments against a sequence of parameters.
// The arguments do not match if the number of arguments is not
// the same as the number of parameters.
//
// An argument A matches a parameter type P if and only if A can be 
// converted to P.
//
// The result of matching arguments against parameters is a new
// sequence of arguments, possibly with conversions.
Expr_seq*
match_arguments(Location loc, Def* fn, Expr_seq* args, Type_seq* parms) {
  auto first1 = args->begin();
  auto last1 = args->end();
  auto first2 = parms->begin();
  auto last2 = parms->end();

  // Check each argument. If it doesn't match, then skip it so we
  // can diagnose all failures.
  //
  // TODO: Are there any kinds of parameters that consume multiple
  // arguments? E.g., variadic or sized parameter packs? Not yet,
  // but conceivably. Note that this would affect the check(s) below.
  Expr_seq* result = new Expr_seq();
  while (first1 != last1 and first2 != last2) {
    Expr* arg = *first1;
    Type* parm = *first2;
    if (Expr* a = convert(arg, parm))
      result->push_back(a);
    ++first1;
    ++first2;
  }

  // If we didn't elaborate all of the args, then bail.
  if (result->size() != args->size())
    return nullptr;

  // Too few arguments were given for the function.
  //
  // TODO: This is where would would instantiate default arguments.
  if (first1 == last1 and first2 != last2) {
    error(loc) << format("too few arguments for '{}'", debug(fn->name()));
    return nullptr;
  }

  // Too many arguments were given for the function.
  //
  // TODO: Unless the last argument is variadic, then all remaining
  // arguments would be placed into an argument pack.
  if (first2 == last2 and first1 != last1) {
    error(loc) << format("too many arguments for '{}'", debug(fn->name()));
    return nullptr;
  }

  return result;  
}

// A funtion F is viable if and only if the sequence of arguments
// matches the types of its parameters.
//
// Note that diagnostics are suppressed (but saved) during overload 
// resolution. Each candidate saves any diagnostics that were
// generated during its analysis.
Candidate
viable_candidate(Location loc, Candidate& c, Expr_seq* args) {
  Diagnostics_guard guard(c.diags);

  Fn_type* ft = as<Fn_type>(get_type(c.fn));
  steve_assert(ft, "candidate with non-function type");
  
  // If the arguments match, update the candidate with the new
  // arguments and mark the candidate viable.
  args = match_arguments(loc, c.def, args, ft->parms());
  if (args) {
    c.args = args;
    c.convs = rank_conversions(args);
    c.viable = true;
  }

  return c;
}

// Remove, from the candidate set, all those declarations that are
// not viable. 
Candidate_list
viable_candidates(Location loc, Candidate_list& cands, Expr_seq* args) {
  Candidate_list viable;
  for (Candidate& c1 : cands) {
    if (Candidate c2 = viable_candidate(loc, c1, args))
      viable.push_back(std::move(c1));
  }
  return viable;
}

// A 3-way comparison of ranked conversions. This returns 1 if a is
// better than b, -1 if b is better than a, and 0 otherwise.
//
// Currently conversions are ranked simply as integer values. However,
// this may, in the future, expand to C++-like conversion sequences.
int
better_conversion(int a, int b) {
  int result = 0;
  if (not a) ++result;
  if (not b) --result;
  return result;    
}

// A 3-way comparison over the conversion given conversion sequences.
//
// FIXME: Cache the conversion sequences when matching arguments.
int
better_conversions(const Conversion_list& a, const Conversion_list& b) {
  int result = 0;
  for (std::size_t i = 0; i < a.size(); ++i)
    result += better_conversion(a[i], b[i]); // ???
  return result;
}

// Rank the conversion sequences of the candidates a and b, returning
// 1 if a has better conversion sequence than b, -1 if the opposite
// is true, and 0 if neither is better than the other.
int
better_candidate(const Candidate& a, const Candidate& b) {
  return better_conversions(a.convs, b.convs);
}


// Determine which of the viable candidates is the best. This comparison
// is based on the relationship between arguments and their corresponding
// parameters.
Candidate_list
best_candidates(Candidate_list& viable) {
  Candidate_list bests;

  // Find a candidate that ranks better than all of its successor
  // candidates, but not its predecessors.
  auto first = viable.begin();
  auto last = viable.end();
  auto best = suggest_best(first, last, better_candidate);

  // FIXME: This is not especially good. What I'd really like to do
  // is partitiion the set of candidates into those that are categorically
  // better than all others, but not better than each other.
  if (best == last) {
    return viable;
  }

  // Seed the candidate list.
  bests.push_back(*best);

  // Make sure that the selected candoidate is actually better than
  // all preceding candidates. If there are any incomparable candidates
  // save those. Otherwise, if there is a better candidate, then
  // there is no best, and we can just return.
  for (auto iter = first; iter != best; ++iter) {
    int n = better_candidate(*best, *iter);
    if (n == 0)
      bests.push_back(*iter);
    if (n == -1)
      return Candidate_list{};
  }

  return bests;
}

// Perform overload resolution by finding a unique declaration in
// the overload set that can be called with the given sequence
// of arguments.
//
// TODO: The results should include all candidates and their viability.
// In fact, it might actually be better to use linked lists and then
// resplice viable and non-viable candidates.
Resolution
resolve_call(Location loc, Overload& ovl, Expr_seq* args) {
  Candidate_list cands = gather_candidates(loc, ovl, args);
  Candidate_list viable = viable_candidates(loc, cands, args);
  Candidate_list best = best_candidates(viable);

  if (best.empty())
    return {};
  if (best.size() == 1)
    return {best.front().def};
  else
    return {std::move(best)};
}

Resolution
resolve_unary(Location loc, Overload& ovl, Expr* arg) {
  return resolve_call(loc, ovl, new Expr_seq {arg});
}

Resolution
resolve_binary(Location loc, Overload& ovl, Expr* arg1, Expr* arg2) {
  return resolve_call(loc, ovl, new Expr_seq {arg1, arg2});
}

} // namespace steve
