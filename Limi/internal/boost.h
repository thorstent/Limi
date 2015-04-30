// Copyright 2005-2009 Daniel James.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LIMI_INTERNAL_BOOST_H
#define LIMI_INTERNAL_BOOST_H

namespace Limi {
namespace internal {
  
  // functions below are copied from boost. See their copyright notice above.
  
  template <class T>
  inline void hash_combine(std::size_t& seed, T const& v)
  {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }
  
  template <class It>
  inline std::size_t hash_range(It first, It last)
  {
    std::size_t seed = 0;
    
    for(; first != last; ++first)
    {
      hash_combine(seed, *first);
    }
    
    return seed;
  }
  
  template <class It>
  inline void hash_range(std::size_t& seed, It first, It last)
  {
    for(; first != last; ++first)
    {
      hash_combine(seed, *first);
    }
  }
}
}

#endif // LIMI_INTERNAL_BOOST_H