
namespace steve {

// -------------------------------------------------------------------------- //
// Candidate

// Initialize the candidate with the given candidate definition,
// function, and sequence of arguments.
inline
Candidate::Candidate(Def* d, Term* f, Expr_seq* a)
  : def(d), fn(f), args(a), diags(), viable(false) { }

// Contextually converts to true if the candidate is viable.
inline
Candidate::operator bool() const { return viable; }


// -------------------------------------------------------------------------- //
// Resolution

inline
Resolution::Resolution()
  : result(empty_res), data() { }

inline
Resolution::Resolution(Resolution&& x)
  : result(x.result)
{
  if (result == ambig_res)
    data.multiple = std::move(x.data.multiple);
  else
    data.single = x.data.single;
}

inline
Resolution::Resolution(Decl* d)
  : result(unique_res), data(d) { }

inline
Resolution::Resolution(Candidate_list&& c)
  : result(ambig_res), data(std::move(c)) { }

inline
Resolution::~Resolution() {
  if (result == ambig_res)
    data.multiple.~Candidate_list();
}

// Returns true if overload resolution produced no solutions.
inline bool 
Resolution::is_empty() const { return result == empty_res; }

// Returns true if overload resolution produced exactly one solution.
inline bool 
Resolution::is_unique() const { return result == unique_res; }

// Returns true if overload resolution produced multiple solutions.
inline bool 
Resolution::is_ambiguous() const { return result == ambig_res; }

inline Decl* 
Resolution::solution() const {
  steve_assert(is_unique(), "resolution is not unique");
  return data.single;
}

inline const Candidate_list&
Resolution::solutions() const { 
  steve_assert(is_ambiguous(), "resolution is not ambiguous");
  return data.multiple;
}

} // namespace steve
