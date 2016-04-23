/*
 * Copyright 2016, IST Austria
 *
 * This file is part of Limi.
 *
 * Limi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Limi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Limi.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIMI_INTERNAL_HASH_H
#define LIMI_INTERNAL_HASH_H

#include <ostream>
#include <utility>
#include "boost.h"

namespace std {
  template<class A, class B> struct hash<pair<A,B>> {
    inline size_t operator()(const pair<A,B>& val) const {
      size_t seed = 0;
      Limi::internal::hash_combine(seed, val.first);
      Limi::internal::hash_combine(seed, val.second);
      return seed;
    }
  };
  template<class A, class B> struct equal_to<pair<A,B>> {
    inline bool operator()(const pair<A,B>& a, const pair<A,B>& b) const {
      return equal_to<A>()(a.first,b.first) && equal_to<B>()(a.second, b.second);
    }
  };
}

#endif // LIMI_INTERNAL_HASH_H