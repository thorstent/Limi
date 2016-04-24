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

#ifndef LIMI_TIMBUK_PRINTER_H
#define LIMI_TIMBUK_PRINTER_H
#include "automaton.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <deque>
#include <unordered_map>
#include <cassert>
#include "internal/hash.h"
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace Limi {

/**
  * @brief Prints automata in timbuk format.
  * 
  * If there is independence relation is empty (returns false for all queries) the resulting automaton will be compatible 
  * with other language inclusion tools that can read timbuk files.
  * 
  * Printing an automaton -> parsing it again with the timbuk parser -> and printing it again will not yield
  * a valid automaton. This is a known behaviour because the init symbols would appear twice.
  * 
  * @tparam State The class of states.
  * @tparam Symbol The class of symbols.
  * @tparam Implementation The automata implementation class.
  * 
  */  
template <class State, class Symbol,class Implementation, class Independence = independence<Symbol>>
class timbuk_printer {    
  const Independence& independence_;
public:
  timbuk_printer(const Independence& independence = Independence()) : independence_(independence) {
    
  }
  
  /**
   * @brief Prints the automaton
   * 
   * The states of the automaton will simply be numbered as they are encountered. The symbols need to be printed in a
   * predictable way because if a pair of automata is fed to another language inclusion tool the symbol set needs to be 
   * identical.
   * 
   * @param automaton The automaton to print
   * @param out The stream to print to
   * @param symbol_printer The printer for symbols. This may be different from the normal symbol_printer and must only produce the symbols [a-zA-Z0-9].
   * @param name The name for the automaton.
   */
  void print_timbuk(const automaton<State,Symbol,Implementation>& automaton, std::ostream& out, const printer_base<Symbol>& symbol_printer, const std::string& name = "A") {
    std::unordered_map<State, unsigned> state_id;
    std::unordered_set<Symbol> symbols;
    std::unordered_multimap<std::pair<unsigned, Symbol>, unsigned> transitions; // (state, symbol), state
    std::unordered_set<unsigned> final_state_ids;
    std::unordered_set<unsigned> initial_state_ids;

    std::unordered_set<State> seen;
    std::deque<State> frontier;
    unsigned state_counter = 0; // give every state a unique id
    for (const auto& s : automaton.initial_states()) {
      frontier.push_back(s);
      state_id[s] = ++state_counter;
      initial_state_ids.insert(state_id[s]);
    }
    
#ifdef DEBUG_PRINTING
    unsigned loop_counter = 0;
#endif
    while (!frontier.empty()) {
#ifdef DEBUG_PRINTING
      if (DEBUG_PRINTING>=2 && ++loop_counter % 1000 == 0) debug << loop_counter << " rounds; A states: " << state_id.size() << std::endl;
#endif
      
      State next = frontier.front();
      frontier.pop_front();
      if (seen.find(next) == seen.end()) {
        seen.insert(next);
        auto next_it = state_id.find(next);
        assert(next_it != state_id.end());
        
        if (automaton.is_final_state(next))
          final_state_ids.insert(next_it->second);
        for (const Symbol& symbol : automaton.next_symbols(next)) {
          symbols.insert(symbol);
          for (const State& succ : automaton.successors(next, symbol)) {
            // put id
            auto succ_it = state_id.find(succ);
            if (succ_it == state_id.end()) succ_it = state_id.insert(std::make_pair(succ, ++state_counter)).first;
            
            frontier.push_back(succ);
            transitions.insert(std::make_pair(std::make_pair(next_it->second,symbol),succ_it->second));
          }
        }
      }
    }
        
    
    // do the printing
    out << "Ops init:1 ";
    for (const auto& symbol : symbols) {
      out << symbol_printer(symbol) << ":2 ";
      // check if the symbol is printed correctly
      std::stringstream sym_ss;
      sym_ss << symbol_printer(symbol);
      std::string sym_str = sym_ss.str();
      if (sym_str=="init")
        throw std::logic_error("No symbol may be called init. Are you trying to print an automaton previously printed and parsed by timbuk.");
      for (char c : sym_str) {
        if (!isalnum(c))
          throw std::logic_error("Symbols must only consist of alphanumeric characters. This symbol violates this: " + sym_str);
      }
    }
    out << std::endl;
    
    // independence relation
    std::unordered_set<std::pair<Symbol,Symbol>> independence_set;
    for (auto it = symbols.begin(); it!=symbols.end(); ++it) {
      auto it2 = it;
      for (++it2; it2!=symbols.end(); ++it2) {
        if (independence_(*it, *it2))
          independence_set.insert(std::make_pair(*it,*it2));
      }
    }
    
    if (!independence_set.empty()) {
      out << "Independence ";
      for (const auto& p : independence_set) {
        out << "(" << symbol_printer(p.first) << " " << symbol_printer(p.second) << ") ";
      }
      out << std::endl;
    }
    
    out << "Automaton " << name << std::endl;
    
    out << "States ";
    for (const auto& state : state_id) {
      out << "st" << name << state.second << " ";
    }
    out << std::endl;
    
    out << "Final States ";
    for (const auto& state : final_state_ids) {
      out << "st" << name << state << " ";
    }
    out << std::endl;
    
    out << "Transitions" << std::endl;
    for (const auto& state : initial_state_ids) {
      out << "init -> st" << name << state << std::endl;
    }
    for (const auto& trans : transitions) {
      out << symbol_printer(trans.first.second) << "(st" << name << trans.first.first << ") -> st" << name << trans.second << std::endl;
    }
  }
  
  /**
   * 
   * \overload 
   */
  void print_timbuk(const automaton<State,Symbol,Implementation>& automaton, const std::string& filename, const printer_base<Symbol>& symbol_printer, const std::string& name = "A") {
    std::ofstream myfile;
    myfile.open(filename);
    print_timbuk(automaton, myfile, symbol_printer, name);
    myfile.close();
  }
}; 
}

#endif // LIMI_TIMBUK_PRINTER_H
