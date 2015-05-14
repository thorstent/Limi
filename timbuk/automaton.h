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

#ifndef TIMBUK_AUTOMATON_H
#define TIMBUK_AUTOMATON_H

#include <Limi/automaton.h>
#include "symbol_table.h"
#include "parsed_automaton.h"

namespace timbuk {

  /**
   * @brief An implementation of \ref Limi::automaton
   * 
   * Note that this class should not change its state (it should work as const).
   * This file acts as a bridge between the automaton we build during parsing and \ref Limi::antichain_algo.
   * Note how all function are declared as inline. The compiler will remove these function calls and
   * directly call the actual functions. 
   * 
   */
  class automaton : public Limi::automaton<state,symbol,automaton> {
public:
  automaton(const parsed_automaton& inner_automaton) : Limi::automaton<state,symbol,automaton>(false, false, true), inner_automaton(inner_automaton) {
    
  }
  
  inline bool int_is_final_state(const state& s) const { return inner_automaton.is_final(s); }
  
  inline void int_initial_states(State_set& states) const { states.insert(inner_automaton.initial().begin(), inner_automaton.initial().end()); }
  
  inline void int_successors(const state& s, const symbol& sigma, State_set& successors) const 
  { 
    inner_automaton.successors(s, sigma, successors);
  }
  
  inline void int_next_symbols(const state& s, Symbol_set& symbols) const { 
    inner_automaton.symbols(s, symbols);
  }
  
  // PRINTERS: We need to override these because the printers' constructors need arguments
  inline const Limi::printer_base<state>* int_state_printer() const { return new Limi::printer<state>(inner_automaton); }
  
  inline const Limi::printer_base<symbol>* int_symbol_printer() const { return new Limi::printer<symbol>(inner_automaton.get_symbol_table()); }
  
  inline bool int_is_epsilon(const symbol& symbol) const { return false; }
protected:
  // keep the inner automaton (the actual automaton)
  const parsed_automaton& inner_automaton;
};
}

#endif // TIMBUK_AUTOMATON_H
