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

#ifndef TIMBUK_PARSED_AUTOMATON_H
#define TIMBUK_PARSED_AUTOMATON_H

#include <string>
#include <vector>
#include <cassert>
#include <unordered_set>
#include "symbol_table.h"
#include <Limi/generics.h>


namespace timbuk {
  /**
   * @brief The state of automata is a 32 bit integer wrapped in a struct.
   * 
   */
  struct state {
    uint32_t s;
    inline state(uint32_t s) : s(s) {}
    inline operator uint32_t() const { return s;}
    /**
     * @brief Equality for states need to be defined
     * 
     * Implicitly defines equal_to<state>
     */
    inline bool operator== (const state& other) const { return s==other.s; }
  };
}

namespace std {
  /**
   * @brief The hash function needs to be defined for the state.
   * 
   */
  template<> struct hash<timbuk::state> {
    inline size_t operator()(const timbuk::state& val) const {
      return hash<uint32_t>()(static_cast<uint32_t>(val.s));
    }
  };
}


namespace timbuk {
  /**
   * @brief This class invokes the parser and holds the automaton.
   * 
   * States are represented 32-bit integers.
   * 
   */
  class parsed_automaton
{
public:    
  using state_vector = std::vector<state>;
  using successor_vector = std::unordered_map<symbol,state_vector>;
  using symbol_vector = std::vector<symbol>;
      
  parsed_automaton(symbol_table& symbol_table, const std::string& filename);
  
  std::string automaton_name;
  
  state find(std::string name);
  
  state add_state(const std::string& name);
  symbol add_symbol(const std::string& name);
  void add_independence(const std::string& name1, const std::string& name2);
  
  void add_successor(std::string s, std::string transition_symbol, std::string successor);
  void add_successor(std::string symbol, std::string successor);
  void add_successor(state s, symbol symbol, state successor);
  void mark_final(std::string s);
  void mark_final(state s);
  
  inline const std::string name(state s) const { return names[s]; }
  inline void symbols(state s, symbol_vector& symbols) const { symbols.insert(symbols.end(), symbols_[s].begin(), symbols_[s].end()); }
  inline void successors(state s, symbol sigma, state_vector& successors) const { 
    const successor_vector& ss = successors_[s]; 
    auto it = ss.find(sigma);
    if (it == ss.end()) return;
    successors.insert(successors.end(), it->second.begin(), it->second.end());
  }
  inline bool is_final(state s) const { return final_[s]; }
  
  inline const state_vector& initial() const { return initial_; }
  
  symbol_table& get_symbol_table();
  const symbol_table& get_symbol_table() const;
  const std::string filename;
private:
  symbol_table& st;
  // a map from state to name (state is the index)
  std::vector<std::string> names;
  // reverse map from name to state
  std::unordered_map<std::string, state> lookup_;
  // list of successors for the state (vector index)
  std::vector<successor_vector> successors_;
  // list of successors symbols (on the edges to the successors)
  std::vector<symbol_vector> symbols_;
  // true if it is a final state
  std::vector<bool> final_;
  // set of initial states
  state_vector initial_;
};
}
  
namespace Limi {
  /**
   * @brief The printer looks up the name of the state and prints it.
   * 
   * This is more efficient since keeping a reference to the name as part of
   * the \ref state struct would **double** its size!
   * 
   */
  template<> struct printer<timbuk::state> : printer_base<timbuk::state> {
    printer(const timbuk::parsed_automaton& automaton) : automaton_(automaton) {}
    virtual void print(const timbuk::state& state, std::ostream& out) const override {
      out << automaton_.name(state);
    }
  private:
    const timbuk::parsed_automaton& automaton_;
  };
}


#endif // TIMBUK_PARSED_AUTOMATON_H
