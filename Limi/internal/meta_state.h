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

#ifndef LIMI_INTERNAL_META_STATE_H
#define LIMI_INTERNAL_META_STATE_H

#include <vector>
#include <ostream>
#include <memory>
#include "helpers.h"

namespace Limi {
  namespace internal {

// TODO make this more efficient by removing / adding on copy
/**
  * @brief A state in the meta-automaton
  * 
  * The state consists of a state of the inner automatan and two stacks of unmatched symbols.
  * 
  */
template <class StateB, class Symbol, class Independence = independence<Symbol>>
struct meta_state {
private:
  StateB inner_state_;
  std::vector<Symbol> early_;
  std::vector<Symbol> late_;
  size_t hash_ = 0;
public:
  typedef typename std::vector<Symbol>::const_iterator vector_iterator;
  meta_state(StateB inner_state) : inner_state_(inner_state), hash_(std::hash<StateB>()(inner_state)) {}
  
  size_t hash() const {
    return hash_;
  }
  
  /*virtual std::string to_string() const override {

  }*/
  
  inline StateB inner_state() const { return inner_state_; }
  
  inline void inner_state(StateB new_inner_state) {
    hash_ = hash_ ^ std::hash<StateB>()(inner_state_);
    inner_state_ = new_inner_state;
    hash_ = hash_ ^ std::hash<StateB>()(inner_state_);
  }
  
  inline const std::vector<Symbol>& early() const  { return early_; } 
  inline const std::vector<Symbol>& late() const { return late_; } 
  
  inline void add_early(const Symbol& symbol, const Independence& independence = Independence()) {
    hash_ = hash_ ^ std::hash<Symbol>()(symbol);
    // find position to insert
    int position;
    for(position = early_.size()-1; position>=0; --position) {
      if (symbol > early_[position] || !independence(symbol, early_[position])) 
        break;
    }
    early_.insert(early_.begin()+(position+1), symbol);
  }
  
  inline void add_late(const Symbol& symbol, const Independence& independence = Independence()) {
    hash_ = hash_ ^ ~std::hash<Symbol>()(symbol);
    int position;
    for(position = late_.size()-1; position>=0; --position) {
      if (symbol > late_[position] || !independence(symbol, late_[position])) 
        break;
    }
    late_.insert(late_.begin()+(position+1), symbol);
  }
  
  inline void erase_early(unsigned position) {
    hash_ = hash_ ^ std::hash<Symbol>()(early_[position]);
    early_.erase(early_.begin() + position); // trick because iterator is const
  }
  
  inline void erase_late(unsigned position) {
    hash_ = hash_ ^ ~std::hash<Symbol>()(late_[position]);
    late_.erase(late_.begin() + position);
  }
  
  inline unsigned size() {
    return early_.size();
  }
  
  inline bool operator==(const meta_state<StateB,Symbol,Independence>& other) const {
    if (!std::equal_to<StateB>()(inner_state(), other.inner_state()))
      return false;
    if (early().size() != other.early().size() || !std::equal(early().begin(), early().end(), other.early().begin(), std::equal_to<Symbol>()))
      return false;
    if (late().size() != other.late().size() || !std::equal(late().begin(), late().end(), other.late().begin(), std::equal_to<Symbol>()))
      return false;
    return true;
  }
};
  }

template<class InnerStateB, class Symbol, class Independence>
struct printer<std::shared_ptr<Limi::internal::meta_state<InnerStateB, Symbol,Independence>>> : printer_base<std::shared_ptr<Limi::internal::meta_state<InnerStateB, Symbol,Independence>>> {
  printer(const printer_base<InnerStateB>& printerS, const printer_base<Symbol>& printerSy) : printerS(printerS), printerSy(printerSy) {}
  virtual void print(const std::shared_ptr<Limi::internal::meta_state<InnerStateB, Symbol,Independence>>& state, std::ostream& out) const override {
    out << "(" << printerS(state->inner_state());
    
    out << ", ";
    internal::print_vector(state->early(), out, printerSy);
    out << ", ";
    internal::print_vector(state->late(), out, printerSy);
    out << ")";
  }
private:  
  const printer_base<InnerStateB>& printerS;
  const printer_base<Symbol>& printerSy;
};


}

namespace std {
  template<class StateB, class Symbol, class Independence> struct hash<Limi::internal::meta_state<StateB,Symbol,Independence>> {
    inline size_t operator()(const Limi::internal::meta_state<StateB,Symbol, Independence>& val) const {
      return val.hash();
    }
  };
  template<class StateB, class Symbol, class Independence> struct hash<shared_ptr<Limi::internal::meta_state<StateB,Symbol,Independence>>> {
    inline size_t operator()(const std::shared_ptr<Limi::internal::meta_state<StateB,Symbol, Independence>>& val) const {
      return hash<Limi::internal::meta_state<StateB,Symbol,Independence>>()(*val);
    }
  };
  
  template<class StateB, class Symbol, class Independence> struct equal_to<shared_ptr<Limi::internal::meta_state<StateB,Symbol,Independence>>> {
    inline bool operator()(const shared_ptr<Limi::internal::meta_state<StateB,Symbol, Independence>>& a, const shared_ptr<Limi::internal::meta_state<StateB,Symbol,Independence>>& b) const {
      return equal_to<Limi::internal::meta_state<StateB,Symbol,Independence>>()(*a,*b);
    }
  };
}


#endif // LIMI_INTERNAL_META_STATE_H