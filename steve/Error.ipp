
namespace steve {

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