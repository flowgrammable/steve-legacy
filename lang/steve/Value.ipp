
namespace steve {

// -------------------------------------------------------------------------- //
// Values

namespace {

// After initializing the kind, copy v into self. Note that we
// only need to in-place allocate the non-trivial members of
// the union.
inline void
copy_data(Value_kind k, Value_data& self, const Value_data& d) {
  switch (k) {
  case unit_value:
    self.u = d.u;
    break;
  case bool_value: 
    self.b = d.b; 
    break;
  case function_value:
    self.f = d.f;
    break;
  case integer_value: 
    new (&self.n) Integer(d.n); 
    break;
  case type_value: 
    self.t = d.t;
    break;
  }
}

// After initializing the kind, copy v into self. Note that we
// only need to in-place allocate the non-trivial members of
// the union, and that trivial data members don't move; they
// just copy.
inline void
move_data(Value_kind k, Value_data& self, Value_data&& d) {
  switch (k) {
  case unit_value:
    self.u = d.u;
    break;
  case bool_value: 
    self.b = d.b; 
    break;
  case function_value:
    self.f = d.f;
    break;
  case integer_value: 
    new (&self.n) Integer(std::move(d.n)); 
    break;
  case type_value:
    self.t = d.t;
    break;
  }
}

// Deallocate any owned resources. Note that this only applies
// to the non-trivial members of the union.
inline void
clear_data(Value_kind k, Value_data& self) {
  if (k == integer_value)
    self.n.~Integer();
}

} // namespace

inline
Value::Value(const Value& v) 
  : kind(v.kind)
{ copy_data(kind, data, v.data); }

inline Value&
Value::operator=(const Value& v) {
  if (this == &v)
    return *this;
  clear_data(kind, data);
  kind = v.kind;
  copy_data(kind, data, v.data);
  return *this;
}

inline
Value::Value(Value&& v)
  : kind(v.kind) 
{ move_data(kind, data, std::move(v.data)); }

inline Value&
Value::operator=(Value&& v) {
  if (this == &v)
    return *this;
  clear_data(kind, data);
  kind = v.kind;
  move_data(kind, data, std::move(v.data));
  return *this;
}

inline
Value::Value(unit_t u)
  : kind(unit_value), data(u) { }

inline
Value::Value(bool b)
  : kind(bool_value), data(b) { }

inline
Value::Value(Integer n)
  : kind(integer_value), data(n) { }

inline
Value::Value(Fn* f)
  : kind(function_value), data(f) { }

inline
Value::Value(Type* t)
  : kind(type_value), data(t) { }

inline
Value::~Value() { clear_data(kind, data); }

namespace {

inline void
check_value_kind(const Value& v, Value_kind k) {
  steve_assert(v.kind == k, "requesting a value with the wrong kind");
}

} // namespace

inline unit_t
Value::as_unit() const {
  check_value_kind(*this, unit_value);
  return data.u;
}

inline bool
Value::as_bool() const {
  check_value_kind(*this, bool_value);
  return data.b;
}

inline const Integer&
Value::as_integer() const {
  check_value_kind(*this, integer_value);
  return data.n;
}

inline Fn*
Value::as_function() const {
  check_value_kind(*this, function_value);
  return data.f;
}

inline Type*
Value::as_type() const {
  check_value_kind(*this, type_value);
  return data.t;
}

// Returns true if v is a unit value.
inline bool
is_unit(const Value& v) { return v.kind == unit_value; }

// Returns true if v is a boolean value.
inline bool
is_bool(const Value& v) { return v.kind == bool_value; }

// Returns true if v is an integer value.
inline bool
is_integer(const Value& v) { return v.kind == integer_value; }

// Returns true if v is a function.
inline bool
is_function(const Value& v) { return v.kind == function_value; }

// Returns true if v is a type.
inline bool
is_type(const Value& v) { return v.kind == type_value; }


namespace {

// Copy data into self. Note that we need to inplace construct the
// non-trivial members.
inline void 
copy_data(bool p, Eval_data& self, const Eval_data& x) {
  if (p)
    self.e = x.e;
  else
    new (&self.v) Value(x.v);
}

// Move data into self. Note that we need to inplace construct the
// non-trivial members.
inline void
move_data(bool p, Eval_data& self, const Eval_data&& x) {
  if (p)
    self.e = x.e;
  else
    new (&self.v) Value(std::move(x.v));
}

// Explicitly destroy any non-trivial members.
inline void
clear_data(bool p, Eval_data& self) {
  if (not p)
    self.v.~Value();
}

} // namespace

inline
Eval::Eval(const Eval& x)
  : partial(x.partial)
{ copy_data(partial, data, x.data); }

inline Eval& 
Eval::operator=(const Eval& x) {
  if (this != &x) {
    clear_data(partial, data);
    partial = x.partial;
    copy_data(partial, data, x.data);
  }
  return *this;
}

inline
Eval::Eval(Eval&& x)
  : partial(x.partial)
{ move_data(partial, data, std::move(x.data)); }

inline Eval& 
Eval::operator=(Eval&& x) {
  if (this != &x) {
    clear_data(partial, data);
    partial = x.partial;
    move_data(partial, data, std::move(x.data));
  }
  return *this;
}

inline 
Eval::Eval(const Value& v)
  : partial(false), data(v) { }

inline 
Eval::Eval(Expr* e)
  : partial(true), data(e) { }

inline const Value&
Eval::as_value() const { 
  steve_assert(not partial, "requesting a value from a partial eval");
  return data.v; 
}

inline Expr*
Eval::as_expr() const {
  steve_assert(partial, "requesting an expression for a value");
  return data.e;
}


// Returns true if an evaluation holds a normal form.
inline bool 
is_value(const Eval& v) { return not v.partial; }

// Returns true if an evaluation hodls an un-evaluated expression.
inline bool
is_partial(const Eval& v) { return v.partial; }

} // namespace steve
