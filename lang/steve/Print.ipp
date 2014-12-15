
namespace steve {

inline
Printer::Printer(std::ostream& s)
  : os(s)
  , indent(0)
{ }

inline Printer&
indent(Printer& p) { 
  ++p.indent; 
  return p;
}

inline Printer&
undent(Printer& p) { 
  --p.indent; 
  return p;
}

// Print a intercalated sequence of nodes. Here, S is an specialization
// of Seq<T*>, P is the type of a printing function, and F is the
// type of a separating function.
//
// TODO: Improve docs.
template<typename S, typename P, typename F>
  inline void 
  print_separated(Printer& p, S* seq, P print, F sep) {
    if (not seq)
      return;
    auto iter = seq->begin();
    auto end = seq->end();
    while (iter != end) {
      print(p, *iter);
      if (std::next(iter) != end)
        sep(p);
      ++iter;
    }
  }

inline void
space_separator::operator()(Printer& p) const { 
  print_space(p);
}

inline void
comma_separator::operator()(Printer& p) const { 
  print(p, ',');
  print_space(p);
}

} // namespace steve