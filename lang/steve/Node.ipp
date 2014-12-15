
namespace steve {

// Declarations
struct Name;
struct Type;
struct Term;
struct Stmt;
struct Decl;
struct Tree;

// -------------------------------------------------------------------------- //
// Node interface

// Returns the kind of node.
inline Node_kind
kind(const Node* n) { return n->kind; }

// Returns the name of the node.
inline String
node_name(const Node* n) { return node_name(n->kind); }

// Returns true if the node is a name node.
inline bool
is_name(const Node* n) { return is_name_node(n->kind); }

// Returns true if the node is a type node.
inline bool
is_type(const Node* n) { return is_type_node(n->kind); }

// Returns true if the node is a declaration.
inline bool
is_decl(const Node* n) { return is_decl_node(n->kind); }

// Returns true if the node is a statement. Note that declarations
// are also statements.
inline bool
is_stmt(const Node* n) { return is_stmt_node(n->kind) || is_decl(n); }

// Returns true if the node is a term. Note that statements and
// declarations are also terms.
inline bool
is_term(const Node* n) { return is_term_node(n->kind) or is_stmt(n); }

// Returns true if n is a tree node.
inline bool
is_tree(const Node* n) { return is_tree_node(n->kind); }

namespace {

// Statically cast T to U if and only if Pred is satisfied.
template<typename U, typename T, typename P>
  struct as_if_fn {
    U* operator()(T* t) const { 
      P pred;
      return (t and pred(t)) ? static_cast<U*>(t) : nullptr; 
    }
    
    const U* operator()(const T* t) const { 
      P pred;
      return (t and pred(t)) ? static_cast<const U*>(t) : nullptr; 
    }
  };

// Returns true when the node has kind K.
template<Node_kind K>
  struct has_kind_fn {
    bool operator()(const Node* n) const { return n->kind == K; }
  };

// Returns true when the node is in the class defined by P.
template<bool (*P)(const Node*)>
  struct has_class_fn {
    bool operator()(const Node* n) const { return P(n); }
  };


// Defines the conversion from T ot U.  The general form tests for conversions
// specifically to a leaf type. That is, the conversion is admissable
// only when t has exactly the node kind of U.
//
// Partial specializations provide conversions to classes of nodes (e.g.,
// conversion from a Node to a Term).
//
// Note that the both T and and U may be const.
template<typename U, typename T>
  struct as_fn : as_if_fn<U, T, has_kind_fn<U::Kind>> { };

template<typename T>
  struct as_fn<Name, T> : as_if_fn<Name, T, has_class_fn<is_name>> { };

template<typename T>
  struct as_fn<Type, T> : as_if_fn<Type, T, has_class_fn<is_type>> { };

template<typename T>
  struct as_fn<Term, T> : as_if_fn<Term, T, has_class_fn<is_term>> { };

template<typename T>
  struct as_fn<Stmt, T> : as_if_fn<Stmt, T, has_class_fn<is_stmt>> { };

template<typename T>
  struct as_fn<Decl, T> : as_if_fn<Decl, T, has_class_fn<is_decl>> { };

template<typename T>
  struct as_fn<Tree, T> : as_if_fn<Tree, T, has_class_fn<is_tree>> { };

} // namespace

// Returns the node t dynamically converted to the node type U. If t does
// not have the dynamic type U, the resulting term is null.
template<typename U, typename T>
  inline U*
  as(T* t) { 
    as_fn<U, T> f;
    return f(t);
  }

template<typename U, typename T>
  inline const U*
  as(const T* t) { 
    as_fn<U, T> f;
    return f(t);
  }

// Returns true if node t has dynamic type U.
template<typename U, typename T>
  inline bool 
  is(const T* t) { return as<U>(t); }

} // namespace steve