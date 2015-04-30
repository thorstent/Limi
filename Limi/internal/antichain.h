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

#ifndef LIMI_INTERNAL_ANTICHAIN_H
#define LIMI_INTERNAL_ANTICHAIN_H

#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <ostream>
#include "../generics.h"
#include "helpers.h"

namespace Limi {
  /**
   * @brief Namespace for internal classes and functions.
   * 
   * These functions are generally not relevant to the users of the library.
   */
  namespace internal {
  
/**
 * @brief An Antichain of minimal elements
 * 
 * An antichain constains pairs of type (a,b), where a ∈ A and b ⊆ B. So a is a single element and b a set.
 * There is a relation defined between the elements (a1,b1) ⊑ (a2,b2) iff a1=a2 ∧ b1 ⊆ b2.
 * The antichain as an invariant that needs to be maintained by \ref add() as follows: ∀ a1,b1,a2,b2. ¬[(a1,b1) ⊑ (a2,b2)] ∧ ¬[(a2,b2) ⊑ (a1,b1)].
 * 
 * The antichain keeps for each pair (a,b) a dirty flag that tracks if this element is dirty. If it is then it is removed when the antichain_algo restarts.
 * 
 * @tparam A The type of elements A
 * @tparam B The type of elements B
 * 
 */
template <class A, class B, class HashA = std::hash<A>, class HashB = std::hash<B>, class CompareA = std::equal_to<A>, class CompareB = std::equal_to<B>>
class antichain
{
private:
  typedef std::unordered_set<B, HashB, CompareB> b_set;
  typedef std::shared_ptr<const b_set> pb_set;
  // the datastore contais a list of sets for each element A. The list corresponds to sets
  // of B's we saw with A. Further a dirty flag is stored for each set
  std::unordered_map<A,std::vector<std::pair<pb_set,bool>>, HashA, CompareA> datastore;
  
  /**
   * @brief Tests if set1 is a subset of set2
   */
  inline bool contained(const b_set& set1, const b_set& set2) const {
    for(auto it=set1.begin(); it!=set1.end(); it++) {
      if (set2.find(*it) == set2.end())
        return false;
    }
    return true;
  }
public:
  antichain() = default;
  
  /**
   * @brief Add to the antichain an element without checking if the invariant is preserved
   * 
   */
  inline void add_unchecked(const A& a, const pb_set& b, bool dirty = false) {
    datastore[a].push_back(std::make_pair(b, dirty));
  }
  
  
  /**
   * @brief Add element (a,b) to the antichain.
   * 
   * It preserve the antichain invariant by removing all a1,b1 (a,b) ⊑ (a1,b1)
   * and by not adding (a,b) if there is any a1,b1 (a1,b1) ⊑ (a,b)
   * 
   */
  void add(const A& a, const pb_set& b, bool dirty = false) {
    std::vector<std::pair<pb_set,bool>>& b_sets = datastore[a];
    bool found = false;
    for (auto it = b_sets.begin(); it != b_sets.end();) {
      // the smallest subset should stay in
      if (contained(*it->first, *b)) {
        found = true;
        break;
      }
      if (contained(*b, *it->first)) {
        it = b_sets.erase(it);
      } else {
        it++;
      }
    }
    if (!found)
      b_sets.push_back(std::make_pair(b,dirty));
  }
  
  /**
   * @brief Tests if the element (a,b) or a smaller element is already contained in the antichain
   * 
   * @returns True if there is any a1,b1 in the antichain, such that (a1,b1) ⊑ (a,b)
   */
  bool contains(const A& a, const pb_set& b) const {
    auto b_sets = datastore.find(a);
    if (b_sets == datastore.end())
      return false;
    for (auto it = b_sets->second.begin(); it != b_sets->second.end(); it++) {
      if (contained(*it->first, *b))
        return true;
    }
    return false;
  }
  
  /**
   * @brief Returns the size of the antichain.
   * 
   * @returns unsigned The number of different elements of A in the antichain.
   */
  inline unsigned size() const {
    return datastore.size();
  }
  
  /**
   * @brief Remove elements marked as dirty
   */
  void clean_dirty() {
    for(std::pair<const A,std::vector<std::pair<pb_set,bool>>>& ds : datastore) {
      ds.second.erase(std::remove_if( ds.second.begin(), ds.second.end(), [](std::pair<pb_set,bool> el) { return el.second; } ), ds.second.end());
    }
  }
  
  
  /**
   * @brief Print the antichain on the screen.
   * 
   * @param out The stream to print to
   * @param printerA The state printer for states of type A
   * @param printerB The state printer for states of type B
   */
  void print(std::ostream& out, const printer_base<A>& printerA = printer<A>(), const printer_base<B>& printerB = printer<B>()) const {
    for(const std::pair<A,std::vector<std::pair<pb_set,bool>>>& ds : datastore) {
      out << "For element " << printerA(ds.first) << std::endl;
      for (const std::pair<pb_set,bool>& set1 : ds.second) {
        out << "  ";
        print_set(*set1.first, out, printerB);
        if (set1.second) out << "_d";
        out << std::endl;
      }
    }
  }
};


}
}

#endif // LIMI_INTERNAL_ANTICHAIN_H
