
namespace steve {

// -------------------------------------------------------------------------- //
// Expected value

template<typename T>
  inline
  Expected<T>::Expected(Expected&& x)
    : valid_(x.valid_) 
  {
    if (valid_)
      new (&state_.value) value_type(std::move(x.state_.value));
    else
      state_.error = x.state_.error;
  }

template<typename T>
  inline
  Expected<T>::Expected(const Expected& x)
    : valid_(x.valid_) 
  {
    if (valid_)
      new (&state_.value) value_type(x.state_.value);
    else
      state_.error = x.state_.error;
  }

template<typename T>
  inline
  Expected<T>::Expected(const value_type& x)
    : valid_(true), state_(x) { }

template<typename T>
  inline
  Expected<T>::Expected(value_type&& x)
    : valid_(true), state_(std::move(x)) { }

template<typename T>
  inline
  Expected<T>::Expected(error_type c)
    : valid_(false), state_(c) { }

template<typename T>
  inline
  Expected<T>::~Expected() {
    if (valid_)
      state_.value.~T();
  }

// Evaluates to true when this object has a value and false when
// it is an error.
template<typename T>
  inline
  Expected<T>::operator bool() const { return valid_; }

// Returns the expected value.
template<typename T>
  inline auto
  Expected<T>::get() const -> const value_type& {
    steve_assert(valid_, "expected value is invalid");
    return state_.value;
  }

// Returns the error code when invalid.
template<typename T>
  inline auto
  Expected<T>::error() const -> error_type {
    steve_assert(not valid_, "expected value is valid");
    return state_.error;
  }

// -------------------------------------------------------------------------- //
// Diagnostics

inline
Diagnostics_guard::Diagnostics_guard()
  : saved(current_diagnostics())
  , current(nullptr)
{ reset_diagnostics(); }

inline
Diagnostics_guard::Diagnostics_guard(Diagnostics& diags)
  : saved(current_diagnostics())
  , current(&diags)
{ use_diagnostics(diags); }

inline
Diagnostics_guard::~Diagnostics_guard() {
  if (saved)
    use_diagnostics(*saved);
  else
    reset_diagnostics();
}


template<typename C, typename T>
  inline std::basic_ostream<C, T>& 
  operator<<(std::basic_ostream<C, T>& os, const Diagnostics& ds) {
    emit(os, ds);
    return os;
  }

} // namespace steve