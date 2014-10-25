
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
  case typename_type: return true;
  case unit_type: return true;
  case bool_type: return true;
  case nat_type: return true;
  case int_type: return true;
  case char_type: return true;
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
  default: break;
  }
  steve_unreachable(format("equivalence of unknown node '{}'", node_name(t)));
}

} // namespace steve