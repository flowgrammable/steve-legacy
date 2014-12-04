
#ifndef OVERLOAD_HPP
#define OVERLOAD_HPP

#include <steve/Conv.hpp>
#include <steve/Error.hpp>
#include <steve/Debug.hpp>

#include <vector>

// This module provides facilities used in the declaration of and
// resolution of overloaded functions.

namespace steve {

struct Decl;
struct Overload;

// An overload candidate is a function declaration and a sequence of
// arguments that can be used to call that function.
struct Candidate {
  Candidate(Def*, Term*, Expr_seq*);

  explicit operator bool() const;

  Def*            def;    // The candidate definition
  Term*           fn;     // The candidate function
  Expr_seq*       args;   // The call arguments (possibly converted)
  Conversion_list convs;  // Conversions required to call the function
  Diagnostics     diags;  // Diagnostics resulting from the check
  bool            viable; // True if viable.
};

// A candidate list is a list of declarations that can be 
// targets of a function call.
using Candidate_list = std::vector<Candidate>;


// A resolution class contains the result of overload resolution.
//
// TODO: This should probably also store the initial list of
// candidates since we would probably want to diagnose the 
struct Resolution {
  enum Result {
    empty_res,
    unique_res,
    ambig_res
  } result;
  
  union Data {
    Data() : single(nullptr) { }
    Data(Decl* d) : single(d) { }
    Data(Candidate_list&& v) : multiple(std::move(v)) {} 
    ~Data() { }
    
    Decl*          single;
    Candidate_list multiple;
  } data;

  Resolution();
  Resolution(Resolution&&);
  Resolution(Decl*);
  Resolution(Candidate_list&&);
  ~Resolution();

  bool is_empty() const;
  bool is_unique() const;
  bool is_ambiguous() const;

  Decl* solution() const;
  const Candidate_list& solutions() const;
};

bool declare_overload(Overload&, Decl*);
Resolution resolve_call(Location, Overload&, Expr_seq*);
Resolution resolve_unary(Location, Overload&, Expr*);
Resolution resolve_binary(Location, Overload&, Expr*, Expr*);

} // namespace steve

#include <steve/Overload.ipp>

#endif

