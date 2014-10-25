
#ifndef STEVE_INTEGER_HPP
#define STEVE_INTEGER_HPP

#include <gmp.h>

#include <steve/String.hpp>
#include <steve/Memory.hpp>

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

  Integer& neg();
  Integer& abs();

  // Observers
  int bits() const;
  int base() const;
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

// Streaming
std::ostream& operator<<(std::ostream&, const Integer&);

} // namespace steve

#include <steve/Integer.ipp>

#endif
