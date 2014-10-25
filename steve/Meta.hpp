
#ifndef STEVE_META_HPP
#define STEVE_META_HPP

#include <type_traits>

// This module provides various metaprogramming facilities.

namespace steve {

template<bool B, typename T>
  using Requires = typename std::enable_if<B, T>::type;

} // namespace steve

#endif
