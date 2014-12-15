
#ifndef STEVE_INTEGER_HPP
#define STEVE_INTEGER_HPP

// TODO: We need a fixed precision integer whose bounds can be set
// dynamically. This may rule out Boost.Multiprecision since its
// bounds are static.

#include <steve/String.hpp>
#include <steve/Memory.hpp>
#include <steve/Debug.hpp>

#include <gmp.h>

namespace steve {

// The Integer class represents arbitrary integer values.
class Integer {
public:
  // Default constructor
  //
  // FIXME: Make a default constructor that takes a width specification.
  Integer();

  // Copy semantics
  Integer(const Integer&);
  Integer& operator=(const Integer&);

  // Value initialization.
  Integer(long, int = 10);
  Integer(String, int = 10);

  // Destructor
  ~Integer();

  // Arithmetic compound assignmnt operators.
  Integer& operator+=(const Integer&);
  Integer& operator-=(const Integer&);
  Integer& operator*=(const Integer&);
  Integer& operator/=(const Integer&);
  Integer& operator%=(const Integer&);

  Integer& operator&=(const Integer&);
  Integer& operator|=(const Integer&);
  Integer& operator^=(const Integer&);

  Integer& operator<<=(const Integer&);
  Integer& operator>>=(const Integer&);

  Integer& neg();
  Integer& abs();
  Integer& comp();

  // Observers
  int sign() const;
  bool is_positive() const;
  bool is_negative() const;
  bool is_nonpositive() const;
  bool is_nonnegative() const;

  int bits() const;
  int base() const;
  std::uintmax_t getu() const;
  std::intmax_t gets() const;
  const mpz_t& data() const;

private:
  mpz_t value_;
  int   base_;
};

// Equality
bool operator==(const Integer&, const Integer&);
bool operator!=(const Integer&, const Integer&);

// Ordering
bool operator<(const Integer&, const Integer&);
bool operator>(const Integer&, const Integer&);
bool operator<=(const Integer&, const Integer&);
bool operator>=(const Integer&, const Integer&);

// Arithmetic
Integer operator+(const Integer&, const Integer&);
Integer operator-(const Integer&, const Integer&);
Integer operator*(const Integer&, const Integer&);
Integer operator/(const Integer&, const Integer&);
Integer operator%(const Integer&, const Integer&);
Integer operator-(const Integer&);
Integer operator+(const Integer&);

Integer operator&(const Integer&, const Integer&);
Integer operator|(const Integer&, const Integer&);
Integer operator^(const Integer&, const Integer&);
Integer operator~(const Integer&);

Integer operator<<(const Integer&, const Integer&);
Integer operator>>(const Integer&, const Integer&);

// Streaming
std::ostream& operator<<(std::ostream&, const Integer&);

} // namespace steve

#include <steve/Integer.ipp>

#endif
