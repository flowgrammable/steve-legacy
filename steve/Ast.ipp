
namespace steve {

// -------------------------------------------------------------------------- //
// Node

// Initialize the node as having kind k.
inline
Node::Node(Node_kind k)
  : kind(k), loc(no_location) { }

// Initialize the node with kind k and at location l.
inline
Node::Node(Node_kind k, const Location& l)
  : kind(k), loc(l) { }


// -------------------------------------------------------------------------- //
// Seq<T>

// Initialize an empty sequence of nodes.
template<typename T>
  inline
  Seq<T>::Seq() 
    : Node(seq_node), Base_() { }

// Initialize a sequence of nodes with those given in list.
template<typename T>
  inline
  Seq<T>::Seq(std::initializer_list<T*> list) 
    : Node(seq_node), Base_(list) { }

// Initizlize a sequence of n nodes, each having the given value.
// If the given value is omitted, it is taken to be nullptr.
template<typename T>
  inline
  Seq<T>::Seq(std::size_t n, T* t)
    : Node(seq_node), Base_(n, t) { }



// -------------------------------------------------------------------------- //
// Node constructors

template<typename T, typename... Args>
  inline T* 
  make_expr(const Location& l, Type* t, Args&&... args) {
    T* r = new T(l, std::forward<Args>(args)...);
    r->tr = t;
    return r;
  }


// -------------------------------------------------------------------------- //
// Streaming

inline
debug_node debug(Expr* e) { return {e}; }

} // namespace steve
