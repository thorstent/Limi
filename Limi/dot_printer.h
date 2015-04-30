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

#ifndef LIMI_DOT_PRINTER_H
#define LIMI_DOT_PRINTER_H
#include "automaton.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <deque>
#include <unordered_map>
#include <cassert>
#include "internal/helpers.h"
#include "generics.h"

namespace Limi {

/**
  * @brief Prints automata in dot format.
  * 
  * Only for small automata it makes sense to print them in dot format.
  * 
  * @tparam State The class of states.
  * @tparam Symbol The class of symbols.
  * @tparam Implementation The automata implementation class.
  * 
  */
template <class State, class Symbol,class Implementation>
class dot_printer {
  typedef std::unordered_set<State> State_set;
  typedef std::unordered_set<Symbol> Symbol_set;
  typedef automaton<State, Symbol, Implementation> Automaton;
    
  struct frontier_item {
    State s;
    Symbol_set sleep_set;
    explicit frontier_item(State s) : s(s) {}
    frontier_item(State s, const Symbol_set& sleep_set) : frontier_item(s)
    { this->sleep_set = sleep_set; }
  };
    

  public:
    
    /**
     * @brief Prints the automaton to the ostream out.
     * 
     * @param automaton The automaton to print
     * @param out The output stream to print to
     */
    void print_dot(const Automaton& automaton, std::ostream& out) {
      unsigned state_counter = 0; // give every state a unique number
      std::unordered_map<State, unsigned> state_id;
            
      out << "digraph automaton {" << std::endl;
      std::unordered_set<State> seen;
      std::deque<State> frontier;
      for (const auto& s : automaton.initial_states()) {
        frontier.push_back(s);
        state_id[s] = ++state_counter;
        out << "begin" << state_counter << " [shape=none,label=\"\"]" << std::endl;
        out << "begin" << state_counter << " -> " << state_counter << std::endl;
      }
      
      while (!frontier.empty()) {
        State next = frontier.front();
        frontier.pop_front();
        if (seen.find(next) == seen.end()) {
          seen.insert(next);
          auto next_it = state_id.find(next);
          assert(next_it != state_id.end());
          
          out << std::to_string(next_it->second) << " [label=\"" << automaton.state_printer()(next) << "\"";
          if (automaton.is_final_state(next))
            out << ",shape=doublecircle";
          out << "]" << std::endl;
          for (const Symbol& symbol : automaton.next_symbols(next)) {
            for (const State& succ : automaton.successors(next, symbol)) {
              // put id
              auto succ_it = state_id.find(succ);
              if (succ_it == state_id.end()) succ_it = state_id.insert(std::make_pair(succ, ++state_counter)).first;
              
              frontier.push_back(succ);
              out << std::to_string(next_it->second) << " -> " << std::to_string(succ_it->second) << " [label=\"" << automaton.symbol_printer()(symbol) << "\"]" << std::endl;
            }
          }
        }
      }
      
      out << "}" << std::endl;
    }

    /**
     * @brief Writes the automaton to a file.
     * 
     * @param automaton The automaton to print
     * @param filename Filename to write the automaton to. Will be overwritten if exists.
     */
    void print_dot(const automaton<State,Symbol,Implementation>& automaton, const std::string& filename) {
      std::ofstream myfile;
      myfile.open(filename);
      print_dot(automaton, myfile);
      myfile.close();
    }
  };

}

#endif // LIMI_DOT_PRINTER_H
