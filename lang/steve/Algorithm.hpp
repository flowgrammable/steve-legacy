
#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <algorithm>

// This module provides a collection of generic algorithms used
// throughout the Steve compiler.

namespace steve {

// -------------------------------------------------------------------------- //
// Partial orders
//
// A comparator for partial orders is a 3-way comparison of type
// (T, T) -> int. The resulting values of such a comparison are:
//
//    cmp(a, b) == 1 iff a is greater than b
//    cmp(a, b) == -1 if b is greater than a
//    cmp(a, b) == 0 if neither is better than the other
//
// When the result is 0, we say that the comparison of a and b is
// indeterminate. This could imply that a and b are incomparable or
// that they are equivalent.


// -------------------------------------------------------------------------- //
// Best element

// A subroutine of best(), advance the current best iterator.
// If iter is better than best, then iter becomes the new
// best iterator. Otherwise, if neither is better, then reset
// the best iterator to the one past iter. 
//
// In either case, iter is advanced to one past best. However,
// if best is past the end, then iter is also past the end.
template<typename I>
  inline void
  advance_best(I& best, I& iter, I last, bool better) {
    if (better)
      best = iter;
    else {
      best = next(iter);
      if (best == last)
        iter == last;
    }
    iter = next(best);
  }

// Suggest a candidate that may be better than all others in the
// range of elements [first, last) partially ordered by cmp.
//
// The resulting element (if not equal to last) is greater
// than all of the elements reachable from that element.
template<typename I, typename C>
  I 
  suggest_best(I first, I last, C cmp) {
    I best = first;
    I iter = next(best);

    // In the first pass, suggest a candidate that may be better
    // than all others.
    while (iter != last) {
      int diff = cmp(*best, *iter);
      if (diff == 1) {
        ++iter;
      } else {
        if (diff == 0) {
          best = std::next(iter);
          if (best == last)
            return last;
        } else {
          best = iter;
        }
        iter = std::next(best);
      }
    }

    return best;
  }

// Returns true if last compares greater than each element x in the
// range [first, last). Note that last shall not be past the end.
template<typename I, typename C>
  bool 
  is_best(I first, I last, C cmp) {
    return all_of(first, last, [=](I iter){ 
      return cmp(*last, *iter) == 1;}
    );
  }

// Try to find a best element in the range of elements [first, last) 
// partially ordered by the 3-way comparator cmp.
//
// TODO: This implementation could be optimized by removing comparisons.
template<typename I, typename C>
  bool 
  best(I first, I last, C cmp) {
    I best = suggest_best(first, last, cmp);
    if (best == last)
      return last;
    return is_best(first, best);
  }


} // namespace steve

#endif
