#ifndef STEVE_MEMORY_HPP
#define STEVE_MEMORY_HPP

#include <memory>

// Boehm GC

namespace steve {

struct Gc { };

template<typename T>
  using Gc_allocator = std::allocator<T>;

} // namespace steve

#if 0

// Force GC-related code to be scoped to boehmgc.
// FIXME: Older versions may not install this.
// #define GC_NAMESPACE 1

// Most compilers do this these days.
#define GC_OPERATOR_NEW_ARRAY 1

#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

namespace steve {

// The garbage collector class provides the allocation hook
// for collectable types.
using Gc = gc;

// The garbage-collecting allocator is needed to ensure that
// elements of containers are allocated using the garbage
// collector.
template<typename T>
  using Gc_allocator = gc_allocator<T>;

} // namespace steve

#endif

#endif
