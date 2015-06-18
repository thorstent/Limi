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

#ifndef LIMI_LIST_AUTOMATON_H
#define LIMI_LIST_AUTOMATON_H

#include <memory>

#include "automaton.h"
#include "internal/helpers.h"
#include <vector>
#include <iostream>
#include <cassert>

namespace Limi {
  

/**
  * @brief This automaton represents accepts exactly one word.
  * 
  * This class is derived from the Limi::automaton class and represents an automaton that accepts
  * exactly one word that is specified as a list of symbols.
  * 
  * @tparam Symbol the type of symbols in the word.
  * 
  */
template <class Symbol>
class list_automaton : public Limi::automaton<unsigned,Symbol,list_automaton<Symbol>> {
protected:
  typedef typename Limi::automaton<unsigned,Symbol,list_automaton<Symbol>>::State_set State_set;
  typedef typename Limi::automaton<unsigned,Symbol,list_automaton<Symbol>>::Symbol_set Symbol_set;
  
protected:
  std::vector<Symbol> symbol_list;
  const printer_base<Symbol>& symbol_printer_;
  printer<unsigned> state_printer_;
public:
  /**
   * @brief Constructs an automaton that accepts a specified word.
   * 
   * The word is specified by iterators and copied to an internal vector. The trace must be epsilon-free.
   * 
   * @param first An iterator to the first symbol of the word
   * @param last An iterator to the end of the word
   * @param symbol_printer The printer for the symbols of the word
   * 
   */
  template <class InputIterator>
  list_automaton(InputIterator first, InputIterator last, const printer_base<Symbol>& symbol_printer = printer<Symbol>()) : Limi::automaton<unsigned,Symbol,list_automaton<Symbol>>(false, false), symbol_printer_(symbol_printer) {
    symbol_list.insert(symbol_list.begin(), first, last);
  }
  
  bool int_is_final_state(const unsigned& state) const { return state >= symbol_list.size(); }
  
  void int_initial_states(State_set& states) const { states.insert(0); }
  
  void int_successors(const unsigned& state, const Symbol& sigma, State_set& successors) const 
  { 
    if (state < symbol_list.size() && std::equal_to<Symbol>()(symbol_list[state], sigma)) 
      successors.insert(state+1);
  }
  
  void int_next_symbols(const unsigned& state, Symbol_set& symbols) const { 
    if (state < symbol_list.size())
      symbols.insert(symbol_list[state]);
  }
  
  inline const printer_base<Symbol>& symbol_printer() const { return symbol_printer_; }
  
  inline bool int_is_epsilon(const Symbol& symbol) const { return false; }
  
};
}

#endif