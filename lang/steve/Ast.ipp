
namespace steve {

// -------------------------------------------------------------------------- //
// Node constructors

template<typename T, typename... Args>
  inline T* 
  make_expr(const Location& l, Type* t, Args&&... args) {
    T* r = new T(l, std::forward<Args>(args)...);
    r->type_ = t;
    return r;
  }


// -------------------------------------------------------------------------- //
// Streaming

inline
debug_node debug(Expr* e) { return {e}; }

} // namespace steve
