// $Id: bigint.hh,v 1.12 2009-01-24 15:14:46 kroening Exp $

// My own BigInt class, declaration.

#ifndef BIGINT_HH
#define BIGINT_HH

#include <cstdint>
#include <utility>

// This one is pretty simple but has a fair divide implementation.
// Though I'm not ambitious enough to do that FFT-like stuff.
//
// Desirable properties / design goals:
//
//  - Highly configurable, should run on chips from 8 to 64 bits word size.
//  - Is relatively small.
//  - Is thread safe.
//  - Does not significantly pollute the global namespace.
//
// Deliberate omissions:
//
//  - Support of the C++ iostream classes which I find obsolete.
//    (Of course there are conversions from/to strings.)
//  - Bitwise operations. A bitset is a different thing than a BigInt.
//
// Porting and configuration:
//
//  - Somehow resolve the problem when your compiler has no `bool' yet.
//  - Choose appropriate elementary types to use for onedig_t, twodig_t,
//    llong_t, and ullong_t. Some possible variants are #ifdef'd below.
//  - Decide what to do on division by zero (in implementation file.)
//  - In allocainc.h, do whatever necessary to provide the alloca() function.
//
// That should be all. I saw this running on:
//
//  - Linux/egcs-1.1.2 ... gcc-2.95.2
//  - Linux/Comeau C++ 4.2.42
//  - Linux/Fujitsu C++
//  - FreeBSD 3.1/gcc-2.7.2 ... 4.1/gcc-2.95.2, NetBSD 1.4.2/egcs-1.1.1
//  - FreeBSD 4.x/TenDRA 4.2-BETA
//  - AIX/xlC
//  - Sun Solaris 2.5 ... 8/Sparc/SunPRO CC 4.2 ... Forte 6.0
//  - Windows NT/i386/MS Visual-C 4.2 ... 6.0
//  - Windows NT/i386/Borland-C++ 5.0, 5.5 (also with Intel reference compiler)
//  - DEC-OSF1/DEC CXX 6.0
//  - DEC OpenVMS 6.2/VAX/DEC CXX 5.5 (without optimization)
//  - DEC OpenVMS 6.2 and 7.1/Alpha/DEC CXX 5.6
//
// Legal status:
//
// The copyright at this source is owned by Dirk Zoller,
// e-mail: duz@sol-3.de
//
// You may however use and modify this without restriction.


class BigInt
{
public:
  // Decide about and publish configuration details:

  // Choose digit type for best performance. Bigger is better as long
  // as there are machine instructions for multiplying and dividing on
  // twice the size of a digit, i.e. on twodig_t.
#if defined __GNUG__ || defined __alpha // || defined __TenDRA__
  // Or other true 64 bit CPU.
  typedef unsigned onedig_t;
  typedef unsigned long long twodig_t;
#elif defined _MSC_VER // || defined __BORLANDC__ (works but is slower)
  // Like GCC's long long this __int64 actually performs better than
  // unsigned. Though not as much as is the case with gcc.
  typedef unsigned onedig_t;
  typedef unsigned __int64 twodig_t;
#elif 1
  // Majority (currently) of 32 bit CPUs.
  typedef unsigned short onedig_t;
  typedef unsigned twodig_t;
#else
  // Works on 8/16 bit CPUs just as well.
  typedef unsigned char onedig_t;
  typedef unsigned short twodig_t;
#endif

  // Choose largest integral type to use. Must be >= twodig_t.
#if defined __GNUG__
  typedef long long llong_t;
  typedef unsigned long long ullong_t;
#elif defined _MSC_VER // || defined __BORLANDC__
  typedef __int64 llong_t;
  typedef unsigned __int64 ullong_t;
#else
  typedef long llong_t;
  typedef unsigned long ullong_t;
#endif

  // Maximum number of onedig_t digits which could also be represented
  // by an elementary type.
  enum
  {
    small = sizeof(ullong_t) / sizeof(onedig_t)
  };

private:
  unsigned size;   // Length of digit vector.
  unsigned length; // Used places in digit vector.
  onedig_t *digit; // Least significant first.
  bool positive;   // Signed magnitude representation.

  // Create or resize this.
  inline void allocate(unsigned digits);
  inline void reallocate(unsigned digits);
  inline void resize(unsigned digits);

  // Adjust length (e.g. after subtraction).
  inline void adjust();

  // Assign to this.
  void assign(llong_t);
  void assign(ullong_t);

  // Aux methods, only for internal use.
  inline int ucompare(BigInt const &) const;
  void add(onedig_t const *, unsigned, bool);
  void mul(onedig_t const *, unsigned, bool);

  // Auxiliary constructor used for temporary or static BigInt.
  // Sets size=0 which indicates that ~BigInt must not delete[].
  inline BigInt(onedig_t *, unsigned, bool);

public:
  ~BigInt();

  BigInt();
  BigInt(int);
  BigInt(unsigned);

  BigInt(long signed int x);
  BigInt(long unsigned int x);

  BigInt(llong_t);
  BigInt(ullong_t);
  BigInt(BigInt const &);
  BigInt(BigInt &&);
  BigInt(char const *, onedig_t = 10);

  BigInt &operator=(BigInt const &);
  BigInt &operator=(BigInt &&);

  // Input conversion from text.

  char const *scan_on(char const *, onedig_t = 10);
  char const *scan(char const *, onedig_t = 10);

  // Output conversion into text.

  // Return an upper bound for the number of digits the textual
  // representation of this might have.
  unsigned digits(onedig_t = 10) const;

  // Convert into string, right adjusted in field of specified width.
  // Returns pointer to start of number or NULL if field too small.
  char *as_string(char *, unsigned, onedig_t = 10) const;

  // Convert to/from a binary representation.

  // Write and read in a compact byte-wise binary form. Effectively
  // print in base 256 with the most significant digit first. Also
  // read back from such a representation. Return success.
  bool dump(unsigned char *, unsigned) const;
  void load(unsigned char const *, unsigned);

  // Conversions to elementary types.

  bool is_int64() const;
  bool is_uint64() const
  {
    return length <= small;
  }

  // disabled by DK: makes operators ambigous
  //operator llong_t() const;
  //operator ullong_t() const;
  uint64_t to_uint64() const;
  int64_t to_int64() const;

#ifndef bool
  // Like int: non-zero is true. Equivalent to !is_zero().
  //operator bool() const			{ return length != 0; }
#endif

  // All comparisions are done with these primitives.

  int compare(llong_t) const;
  int compare(ullong_t) const;
  int compare(BigInt const &) const;

  // Eliminate need for explicit casts when comparing.

  int compare(long n) const
  {
    return compare(static_cast<llong_t>(n));
  }
  int compare(unsigned long n) const
  {
    return compare(static_cast<ullong_t>(n));
  }
  int compare(int n) const
  {
    return compare(static_cast<llong_t>(n));
  }
  int compare(unsigned n) const
  {
    return compare(static_cast<ullong_t>(n));
  }

  // Tests. These are faster than comparing with 0 or of course %2.

  bool is_zero() const
  {
    return length == 0;
  }
  bool is_positive() const
  {
    return positive;
  }
  bool is_negative() const
  {
    return !positive;
  }
  bool is_odd() const
  {
    return length != 0 && digit[0] & 1;
  }
  bool is_even() const
  {
    return !is_odd();
  }

  // Arithmetic.

  BigInt &negate()
  {
    if(!is_zero())
      positive = !positive;
    return *this;
  }

  friend BigInt operator-(BigInt b)
  {
    b.negate();
    return b;
  }

#define IN_PLACE_OPERATOR(TYPE)                                                \
  BigInt &operator+=(TYPE);                                              \
  BigInt &operator-=(TYPE);                                              \
  BigInt &operator*=(TYPE);                                              \
  BigInt &operator/=(TYPE);                                              \
  BigInt &operator%=(TYPE);

  IN_PLACE_OPERATOR(const BigInt &)
  IN_PLACE_OPERATOR(llong_t)
  IN_PLACE_OPERATOR(ullong_t)
#undef IN_PLACE_OPERATOR

#define OVERLOAD_IN_PLACE_OPERATOR(FROM, TO)                                   \
  BigInt &operator+=(FROM x)                                                   \
  {                                                                            \
    return operator+=(static_cast<TO>(x));                                     \
  }                                                                            \
  BigInt &operator-=(FROM x)                                                   \
  {                                                                            \
    return operator-=(static_cast<TO>(x));                                     \
  }                                                                            \
  BigInt &operator*=(FROM x)                                                   \
  {                                                                            \
    return operator*=(static_cast<TO>(x));                                     \
  }                                                                            \
  BigInt &operator/=(FROM x)                                                   \
  {                                                                            \
    return operator/=(static_cast<TO>(x));                                     \
  }                                                                            \
  BigInt &operator%=(FROM x)                                                   \
  {                                                                            \
    return operator%=(static_cast<TO>(x));                                     \
  }

  OVERLOAD_IN_PLACE_OPERATOR(long, llong_t)
  OVERLOAD_IN_PLACE_OPERATOR(unsigned long, ullong_t)
  OVERLOAD_IN_PLACE_OPERATOR(int, llong_t)
  OVERLOAD_IN_PLACE_OPERATOR(unsigned, ullong_t)
#undef OVERLOAD_IN_PLACE_OPERATOR

  BigInt &operator++()
  {
    return operator+=(1);
  } // preincrement
  BigInt &operator--()
  {
    return operator-=(1);
  } // predecrement

  static void
  div(BigInt const &, BigInt const &, BigInt &quot, BigInt &rem);

  // Returns the largest x such that 2^x <= abs() or 0 if input is 0
  // Not part of original BigInt.
  unsigned floorPow2() const;

  // Not part of original BigInt.
  static BigInt power2(unsigned n)
  {
    BigInt b;
    b.setPower2(n);
    return b;
  }

  // Not part of original BigInt.
  static BigInt power2m1(unsigned n)
  {
    BigInt b = power2(n);
    --b;
    return b;
  }

  void swap(BigInt &other)
  {
    std::swap(other.size, size);
    std::swap(other.length, length);
    std::swap(other.digit, digit);
    std::swap(other.positive, positive);
  }

private:
  // Sets the number to the power of two given by the exponent
  // Not part of original BigInt.
  void setPower2(unsigned exponent);
};

// Binary arithmetic operators

inline BigInt operator+(const BigInt &lhs, const BigInt &rhs)
{
  return BigInt(lhs) += rhs;
}
inline BigInt operator-(const BigInt &lhs, const BigInt &rhs)
{
  return BigInt(lhs) -= rhs;
}
inline BigInt operator*(const BigInt &lhs, const BigInt &rhs)
{
  return BigInt(lhs) *= rhs;
}
inline BigInt operator/(const BigInt &lhs, const BigInt &rhs)
{
  return BigInt(lhs) /= rhs;
}
inline BigInt operator%(const BigInt &lhs, const BigInt &rhs)
{
  return BigInt(lhs) %= rhs;
}

// Because the operators `+` and `*` are associative, we can do fast math, no
// matter which side the BigInt is on.  For the rest of the operators, which
// are non-associative, we can only get speedups if the primitive type is on
// the RHS.
#define BINARY_ARITHMETIC_OPERATORS(OTHER)                                     \
  inline BigInt operator+(const BigInt &lhs, OTHER rhs)                        \
  {                                                                            \
    return BigInt(lhs) += rhs;                                                 \
  }                                                                            \
  inline BigInt operator+(OTHER lhs, const BigInt &rhs)                        \
  {                                                                            \
    return BigInt(rhs) += lhs;                                                 \
  }                                                                            \
  inline BigInt operator*(const BigInt &lhs, OTHER rhs)                        \
  {                                                                            \
    return BigInt(lhs) *= rhs;                                                 \
  }                                                                            \
  inline BigInt operator*(OTHER lhs, const BigInt &rhs)                        \
  {                                                                            \
    return BigInt(rhs) *= lhs;                                                 \
  }                                                                            \
  inline BigInt operator-(const BigInt &lhs, OTHER rhs)                        \
  {                                                                            \
    return BigInt(lhs) -= rhs;                                                 \
  }                                                                            \
  inline BigInt operator/(const BigInt &lhs, OTHER rhs)                        \
  {                                                                            \
    return BigInt(lhs) /= rhs;                                                 \
  }                                                                            \
  inline BigInt operator%(const BigInt &lhs, OTHER rhs)                        \
  {                                                                            \
    return BigInt(lhs) %= rhs;                                                 \
  }

BINARY_ARITHMETIC_OPERATORS(BigInt::llong_t)
BINARY_ARITHMETIC_OPERATORS(BigInt::ullong_t)
BINARY_ARITHMETIC_OPERATORS(long)
BINARY_ARITHMETIC_OPERATORS(unsigned long)
BINARY_ARITHMETIC_OPERATORS(int)
BINARY_ARITHMETIC_OPERATORS(unsigned)
#undef BINARY_ARITHMETIC_OPERATORS

// Binary comparison operators

inline bool operator<(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) < 0;
}
inline bool operator>(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) > 0;
}
inline bool operator<=(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) <= 0;
}
inline bool operator>=(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) >= 0;
}
inline bool operator==(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) == 0;
}
inline bool operator!=(const BigInt &lhs, const BigInt &rhs)
{
  return lhs.compare(rhs) != 0;
}

// These operators are all associative, so we can define them all for
// primitives on the LHS and RHS.
#define COMPARISON_OPERATORS(OTHER)                                            \
  inline bool operator<(const BigInt &lhs, OTHER rhs)                          \
  {                                                                            \
    return lhs.compare(rhs) < 0;                                               \
  }                                                                            \
  inline bool operator>(const BigInt &lhs, OTHER rhs)                          \
  {                                                                            \
    return lhs.compare(rhs) > 0;                                               \
  }                                                                            \
  inline bool operator<=(const BigInt &lhs, OTHER rhs)                         \
  {                                                                            \
    return lhs.compare(rhs) <= 0;                                              \
  }                                                                            \
  inline bool operator>=(const BigInt &lhs, OTHER rhs)                         \
  {                                                                            \
    return lhs.compare(rhs) >= 0;                                              \
  }                                                                            \
  inline bool operator==(const BigInt &lhs, OTHER rhs)                         \
  {                                                                            \
    return lhs.compare(rhs) == 0;                                              \
  }                                                                            \
  inline bool operator!=(const BigInt &lhs, OTHER rhs)                         \
  {                                                                            \
    return lhs.compare(rhs) != 0;                                              \
  }                                                                            \
  inline bool operator<(OTHER lhs, const BigInt &rhs)                          \
  {                                                                            \
    return -rhs.compare(lhs) < 0;                                              \
  }                                                                            \
  inline bool operator>(OTHER lhs, const BigInt &rhs)                          \
  {                                                                            \
    return -rhs.compare(lhs) > 0;                                              \
  }                                                                            \
  inline bool operator<=(OTHER lhs, const BigInt &rhs)                         \
  {                                                                            \
    return -rhs.compare(lhs) <= 0;                                             \
  }                                                                            \
  inline bool operator>=(OTHER lhs, const BigInt &rhs)                         \
  {                                                                            \
    return -rhs.compare(lhs) >= 0;                                             \
  }                                                                            \
  inline bool operator==(OTHER lhs, const BigInt &rhs)                         \
  {                                                                            \
    return -rhs.compare(lhs) == 0;                                             \
  }                                                                            \
  inline bool operator!=(OTHER lhs, const BigInt &rhs)                         \
  {                                                                            \
    return -rhs.compare(lhs) != 0;                                             \
  }

COMPARISON_OPERATORS(BigInt::llong_t)
COMPARISON_OPERATORS(BigInt::ullong_t)
COMPARISON_OPERATORS(long)
COMPARISON_OPERATORS(unsigned long)
COMPARISON_OPERATORS(int)
COMPARISON_OPERATORS(unsigned)
#undef COMPARISON_OPERATORS

#endif //ndef BIGINT_HH
