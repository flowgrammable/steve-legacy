
#ifndef STEVE_NODE_HPP
#define STEVE_NODE_HPP

#include <steve/Memory.hpp>
#include <steve/String.hpp>
#include <steve/Location.hpp>

#include <cstdint>
#include <vector>

// This module is defines facilities used in a variety of different
// tree structures including the abstract syntax tree, the parse tree
// and diagnostic trees.

namespace steve {

// -------------------------------------------------------------------------- //
// Node classification

// The node kind is a 32-bit integer value of the form:
//
//    kkkkkkkk NNNNNNNN NNNNNNNN NNNNNNNN
//
// Where the high-order k bits designate the class of the term, and
// the next 6 bits are various flags used for internal bookkeeping.
// These are currently reserved.
using Node_kind = std::uint32_t;

// The ndoe class characterizes the node. 
using Node_class = std::uint32_t;
constexpr Node_class util_class = 0; // General purpose nodes
constexpr Node_class name_class = 1; // Names
constexpr Node_class type_class = 2; // Types
constexpr Node_class term_class = 3; // Terms
constexpr Node_class stmt_class = 4; // Statements
constexpr Node_class decl_class = 5; // Declarations
constexpr Node_class tree_class = 6; // Syntax trees

// Return a node kind for the given class.
constexpr Node_kind
make_node_class(Node_class k) { return k << 24; }

// Returns the node class.
constexpr Node_class
get_node_class(Node_kind k) { return k >> 24; }

// Return the node id, but without the class.
constexpr Node_kind
get_node_id(Node_kind k) { return k & 0x00ffffff; }

// Returns true if the node is a utility node.
constexpr bool
is_util_node(Node_kind k) { return get_node_class(k) == util_class; }

// Returns true if the node is a name.
constexpr bool
is_name_node(Node_kind k) { return get_node_class(k) == name_class; }

// Returns true if the node is a type.
constexpr bool
is_type_node(Node_kind k) { return get_node_class(k) == type_class; }

// Returns true if the node is a term.
constexpr bool
is_term_node(Node_kind k) { return get_node_class(k) >= term_class; }

// Returns true if the node is a statement.
constexpr bool
is_stmt_node(Node_kind k) { return get_node_class(k) >= stmt_class; }

// Returns true if the node is a declaration.
constexpr bool
is_decl_node(Node_kind k) { return get_node_class(k) == decl_class; }

// Returns true if the node is a parse tree.
constexpr bool
is_tree_node(Node_kind k) { return get_node_class(k) == tree_class; }


// Create a utility node kind.
constexpr Node_kind 
make_util_node(std::uint32_t n) { return n; }

// Create a name node with the given id.
constexpr Node_kind
make_name_node(std::uint32_t n) { return make_node_class(name_class) | n; }

// Create a type node with the given id.
constexpr Node_kind 
make_type_node(std::uint32_t n) { return make_node_class(type_class) | n; }

// Create a term node with the given id.
constexpr Node_kind 
make_term_node(std::uint32_t n) { return make_node_class(term_class) | n; }

// Create a statement node with the given id.
constexpr Node_kind
make_stmt_node(std::uint32_t n) { return make_node_class(stmt_class) | n; }

// Create a declaration node with the given id.
constexpr Node_kind
make_decl_node(std::uint32_t n) { return make_node_class(decl_class) | n; }

// Create a tree node with the given id.
constexpr Node_kind
make_tree_node(std::uint32_t n) { return make_node_class(tree_class) | n; }

// Return the name of the node.
String node_name(Node_kind);

// Set the name of the node.
void node_name(Node_kind, const char*);


// -------------------------------------------------------------------------- //
// Core node categories

// Common nodes
constexpr Node_kind seq_node       = make_util_node(1);



// -------------------------------------------------------------------------- //
// Common nodes

// The Node class is the base class for an abstract syntax tree
// hierarchy. This  class provides common facilities for working 
// with expressions in programming languages. In particular, it 
// provides garbage collection facilities and expression 
// classification services.
//
// Garbage collection is provided automatically through the overloaded
// new operator. Once dynamically allocated, deleting a node is
// almost never needed.
//
// Term classification is provided by the kind member variable. This
// must be initalized by the constructors of the derived class.
struct Node : Gc {
  Node(Node&&) = delete;
  Node(const Node&) = delete;
  Node& operator=(Node&&) = delete;
  Node& operator=(const Node&) = delete;

  Node(Node_kind);
  Node(Node_kind, const Location&);

  Node_kind kind;
  Location  loc;
};

// A helper class for derived Node implementations. This defines
// a static constexpr member, Kind, that defines the static type
// of the node.
template<Node_kind K>
  struct Kind_of {
    static constexpr Node_kind Kind = K;
  };

// The Seq class provides a facility for aggregating a sequence
// of nodes. This class also provides the same interface as
// std::vector<T*> where T is the type of aggregated node.
//
// Note that T must be derived from Node.
template<typename T>
  struct Seq : Node, std::vector<T*, Gc_allocator<T*>> {
    using Base_ = std::vector<T*, Gc_allocator<T*>>;

    Seq();
    Seq(std::initializer_list<T*>);
    Seq(std::size_t, T* = nullptr);
  };


// -------------------------------------------------------------------------- //
// Operations

void init_node(Node_kind k, const char*);

Node_kind kind(const Node*);
String node_name(const Node*);

bool is_name(const Node*);
bool is_type(const Node*);
bool is_term(const Node*);
bool is_stmt(const Node*);
bool is_decl(const Node*);

template<typename U, typename T> U* as(T* t);
template<typename U, typename T> const U* as(const T* t);
template<typename U, typename T> bool is(const T* t);

} // naemespace steve

#include <steve/Node.ipp>

#endif
