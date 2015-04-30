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

#ifndef LIMI_DATASTRUCTURES_GENERICS_H
#define LIMI_DATASTRUCTURES_GENERICS_H

#include <ostream>
#include <functional>
#include <sstream>


typedef std::function<void(std::ostream&)> printable;

namespace std {
inline std::ostream& operator<<(std::ostream& os, const printable& p) {
  p(os);
  return os;
}

inline std::stringstream& operator<<(std::stringstream& os, const printable& p) {
  p(os);
  return os;
}
}

namespace Limi {

/**
 * @brief The template for the independence relation.
 * 
 * Ideally your class should specialise this template for the desired independence relation.
 * The specialised class must implement the () operator that returns true if two elements are independent.
 * 
 * @tparam Key The type of the symbols in the independence relation.
 */
template< class Key >
struct independence;


/**
 * @brief This class simply states that no symbols are independent.
 * 
 * This class can be used when an independence relation is expected.
 * For performance reasons it should not be used with \ref antichain_algo_ind,
 * instead use \ref antichain_algo.
 * 
 * @tparam Key The type of the symbols in the independence relation.
 */
template< class Key >
struct no_independence {
  inline bool operator()(const Key& a, const Key& b) const {
    return false;
  }
};

/**
 * @brief The printer base is what custom printers need to inherit from.
 *
 * Derived classes need to override the print method using the virtual keyword.
 * For printer_base the << operater is implemented. 
 * 
 * @tparam Key The class of the items that should be printed.
 */
template< class Key >
struct printer_base {
  virtual ~printer_base() {}
  inline printable operator()(const Key& item) const {
    return [&](std::ostream& o) {print(item, o);};
  }
  
  /**
   * @brief Prints an item to the out stream.
   * 
   * This class needs to be overridden.
   */
  virtual void print(const Key& item, std::ostream& out) const = 0;
};

/**
 * @brief The default implementation of \ref Limi::printer_base.
 *
 * It uses the << operator of Key. 
 * This template can be specialised for your State and Symbol classes if desired.
 * 
 * @tparam Key The class of the items that should be printed. 
 */
template< class Key >
struct printer : public printer_base<Key> {
  virtual void print(const Key& item, std::ostream& out) const override {
    out << item;
  }
};

}

#endif // LIMI_DATASTRUCTURES_GENERICS_H