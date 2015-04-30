/*
 * Copyright 2015, IST Austria
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

#ifndef LIMI_INTERNAL_HELPERS_H
#define LIMI_INTERNAL_HELPERS_H

#include <unordered_set>
#include <vector>
#include <list>
#include <ostream>
#include "../generics.h"

namespace Limi {
namespace internal {
  template <class Key, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>>
  void print_set(const std::unordered_set<Key, Hash, Pred>& set1, std::ostream& out, const printer_base<Key>& printerK = printer<Key>()) {
    out << "{ ";
    for (auto it = set1.begin(); it!=set1.end();) {
      out << printerK(*it);
      ++it;
      if (it!=set1.end()) {
        out << ",";
      } else {
        break;
      }
    }
    out << " }";
  }
  
  template <class Key>
  void print_vector(const std::vector<Key>& vector1, std::ostream& out, const printer_base<Key>& printerK = printer<Key>()) {
    out << "[";
    for (auto it = vector1.begin(); it!=vector1.end();) {
      out << printerK(*it);
      ++it;
      if (it!=vector1.end()) {
        out << ", ";
      } else {
        break;
      }
    }
    out << "]";
  }
  
  template <class Key>
  void print_list(const std::list<Key>& vector1, std::ostream& out, const printer_base<Key>& printerK = printer<Key>()) {
    out << "[";
    for (auto it = vector1.begin(); it!=vector1.end();) {
      printerK(*it, out);
      ++it;
      if (it!=vector1.end()) {
        out << ", ";
      } else {
        break;
      }
    }
    out << "]";
  }
  
  template <class Items>
  void print_array(const Items* items, unsigned length, std::ostream& out, const printer_base<Items>& printerK = printer<Items>()) {
    out << "[";
    for (unsigned i = 0; i<length; i++) {
      printerK(items[i], out);
      if (i < length-1) {
        out << ", ";
      }
    }
    out << "]";
  }
  
  template <class Keys, class Hash, class Pred, class Alloc>
  void set_remove(std::unordered_set<Keys,Hash,Pred,Alloc>& set1, const std::unordered_set<Keys,Hash,Pred,Alloc>& set2) {
    for (const auto& item : set2) {
      auto it = set1.find(item);
      if (it!=set1.end())
        set1.erase(it);
    }
  }
  
  template <class Keys, class Hash, class Pred, class Alloc>
  void set_intersect(std::unordered_set<Keys,Hash,Pred,Alloc>& set1, const std::unordered_set<Keys,Hash,Pred,Alloc>& set2) {
    for (auto it = set1.begin(); it!=set1.end(); ) {
      if (set2.find(*it)==set2.end())
        it = set1.erase(it);
      else
        ++it;
    }
  }
  
  template <class Keys, class Hash, class Pred, class Alloc>
  bool set_intersection_empty(const std::unordered_set<Keys,Hash,Pred,Alloc>& set1, const std::unordered_set<Keys,Hash,Pred,Alloc>& set2) {
    for (const auto& item : set1) {
      if (set2.find(item)!=set2.end())
        return false;
    }
    return true;
  }
  
  template <class Keys, class Hash, class Pred, class Alloc>
  void set_union(std::unordered_set<Keys,Hash,Pred,Alloc>& set1, const std::unordered_set<Keys,Hash,Pred,Alloc>& set2) {
    for (const auto& item : set2) {
      set1.insert(item);
    }
  }
}
}

#endif // LIMI_INTERNAL_HELPERS_H