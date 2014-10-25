
namespace steve {

template<typename C, typename T>
  inline std::basic_ostream<C, T>& 
  operator<<(std::basic_ostream<C, T>& os, const Diagnostics& ds) {
    emit(os, ds);
    return os;
  }

} // namespace steve