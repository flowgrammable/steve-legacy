
#include <steve/Ast.hpp>
#include <steve/Debug.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Type equivalence

namespace {

struct same_fn {
  template<typename T>
    bool operator()(T* t, T* u) const { return is_same(t, u); }
};

template<typename T>
  inline bool
  is_same(Seq<T>* t, Seq<T>* u) {
    if (t->size() != u->size())
      return false;
    return std::equal(t->begin(), t->end(), u->begin(), same_fn{});
  }

inline bool
is_same(bool t, bool u) { return t == u; }

inline bool
is_same(const Integer& t, const Integer& u) { return t == u; }

inline bool
is_same(String t, String u) { return t == u; }

template<typename T>
  inline bool 
  same_unary(T* t, T* u) {
    return is_same(t->first, u->first);
  }

template<typename T>
  inline bool 
  same_binary(T* t, T* u) {
    return same_unary(t, u) and is_same(t->second, u->second);
  }

template<typename T>
  inline bool 
  same_ternary_type(T* t, T* u) {
    return same_binary(t, u) and is_same(t->third, u->third);
  }

} // namespace


bool
is_same(Expr* t, Expr* u) {
  // If both expressions are null, they are the same. If one is 
  // null and the other is not, they are different.
  if (not t) {
    if (not u)
      return true;
    else
      return false;
  } else {
    if (not u)
      return false;
  }

  // Different kinds? Different expressions.
  if (t->kind != u->kind)
    return false;
  
  // Both types are the same. Compare structurally.
  switch (t->kind) {
  // Names
  case basic_id: 
    return same_unary(as<Basic_id>(t), as<Basic_id>(u));
  // Types
  case typename_type: 
  case unit_type:
  case bool_type:
  case nat_type:
  case int_type:
  case char_type:
    return true;
  case fn_type: 
    return same_binary(as<Fn_type>(t), as<Fn_type>(u));
  case range_type: 
    return same_unary(as<Range_type>(t), as<Range_type>(u));
  case bitfield_type: 
    return same_ternary_type(as<Bitfield_type>(t), as<Bitfield_type>(u));
  case record_type:
    return same_unary(as<Record_type>(t), as<Record_type>(u));
  case variant_type:
    return same_unary(as<Variant_type>(t), as<Variant_type>(u));
  case variant_of_type:
    return same_binary(as<Variant_of_type>(t), as<Variant_of_type>(u));
  case enum_type:
    return same_unary(as<Enum_type>(t), as<Enum_type>(u));
  case enum_of_type:
    return same_binary(as<Enum_of_type>(t), as<Enum_of_type>(u));
  case module_type:
    break; // FIXME: Make this work.

  // Terms
  case unit_term: 
    return true;
  case bool_term: 
    return same_unary(as<Bool>(t), as<Bool>(u));
  case int_term: 
    return same_unary(as<Int>(t), as<Int>(u));

  // Declarations
  case enum_decl:
    return same_binary(as<Enum>(t), as<Enum>(u));

  default: break;
  }
  steve_unreachable(format("equivalence of unknown node '{}'", node_name(t)));
}

} // namespace steve