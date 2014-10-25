
#include <steve/Integer.hpp>
#include <steve/Debug.hpp>

namespace steve {

// Consruct an integer with the value in s in base b. Behavior is undefined
// if s does not represent an integer in base b.
Integer::Integer(String s, int b) 
  : base_(b)
{
  if (mpz_init_set_str(value_, s.data(), base_) == -1)
    steve_unreachable(format("invalid integer representation '{}' in base {}", s, b));
}

namespace {

inline std::size_t
get_buffer_size(const mpz_t& z, int b) {
  // add 1 for a null terminator
  std::size_t r = mpz_sizeinbase(z, b) + 1; 
  
  // add in the formatting characters.
  switch (b) {
  case 2: return r + 2;
  case 8: return r + 1;
  case 10: return r + 0;
  case 16: return r + 2;
  }
  steve_unreachable(format("unsupported integer base {}", b));
}

} // namespace

// Streaming
std::ostream&
operator<<(std::ostream& os, const Integer& z) {
  int  base = z.base();
  std::size_t n = get_buffer_size(z.data(), base);
  std::unique_ptr<char[]> buf(new char[n]);
  switch (base) {
    case 2:
      gmp_snprintf(buf.get(), n + 2, "0b%Zo", z.data());
      break;
    case 8:
      gmp_snprintf(buf.get(), n + 1, "0%Zo", z.data());
      break;
    case 10:
      gmp_snprintf(buf.get(), n, "%Zd", z.data());
      break;
    case 16:
      gmp_snprintf(buf.get(), n + 2, "0x%Zx", z.data());
      break;
  }
  return os << buf.get(); 
}

} // namespace steve
