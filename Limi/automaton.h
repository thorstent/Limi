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

#ifndef LIMI_AUTOMATON_H
#define LIMI_AUTOMATON_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <ostream> 
#include <algorithm>
#include <deque>
#include <iostream>
#include "generics.h"
#include "internal/hash.h"
#include "results.h"

namespace Limi {


/**
  * @brief Automata need to inherit from this class and implement certain methods.
  * 
  * We use the [curiously recurring template pattern (CRTP)](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
  * This means that the custom automaton must inherit from this class and pass itself as the Implementation template argument.
  * Furthermore none of the methods should be declared virtual (virtual function calls are too slow) and small functions should 
  * be declared inline if possible. Functions that need to be implemented by the deriving class start with `_int` and are marked as **Implement**.
  * 
  * @tparam State The state class that the automaton will use.
  * @tparam Symbol The symbol class.
  * @tparam Implementation The deriving class must pass its own name here.
  */
template <class State, class Symbol, class Implementation>
class automaton
{
public:
  /**
   * @brief Constructor
   * 
   * @param collapse_epsilon This class will automatically collapse epsilon transitions. That means that if the derived class returns a
   * successor for a specific state, this class will query for all epsilon successors of that state recursively and include them all in the
   * successor set. This is costly in terms of performance.
   * @param use_cache This class will keep a cache of successors for a specific state in a mutable hashmap. This means that the derived class 
   * will not be queried twice for the same successor. This can drastically improve performance if the successor computation is expensive.
   * If the successor computation is cheap it should not be used.
   * @param no_epsilon_produced This should be set to false if the automaton is known not to generate epsilon transitions. It implies collapse_epsilon
   * is set to false. This field is needed because \ref Limi::antichain_algo::antichain_algo() will reject any automaton B where collapse_epsilon
   * is false unless no_epsilon_produced is true.
   * 
   */
  automaton(bool collapse_epsilon = false, bool use_cache = true, bool no_epsilon_produced = false) : collapse_epsilon(collapse_epsilon && !no_epsilon_produced), use_cache(use_cache), no_epsilon_produced(no_epsilon_produced)  {}
  
  ~automaton() {
    delete state_printer_;
    delete symbol_printer_;
  }
  
  typedef std::unordered_set<State> State_set;
  typedef std::unordered_set<Symbol> Symbol_set;
  
  /***********************************************
   * Internal functions that need to be overriden by the deriving class
   **********************************************/
  
  /**
   * @brief **Optionally Implement** Returns a printer for states.
   * 
   * This function is called once the first time the printer is requested using \ref state_printer.
   * The default implementation will return a new instance of \ref Limi::printer \<State\>.
   * 
   * @return A pointer to the printer. The base class will take ownership of the pointer
   * and delete it when destructed.
   */
  inline printer_base<State>* int_state_printer() const { return new printer<State>(); }
  
  /**
   * @brief **Optionally Implement** Returns a printer for symbols.
   * 
   * This function is called once the first time the printer is requested using \ref symbol_printer.
   * The default implementation will return a new instance of \ref Limi::printer \<Symbol\>.
   * 
   * @return A pointer to the printer. The base class will take ownership of the pointer
   * and delete it when destructed.
   */
  inline printer_base<Symbol>* int_symbol_printer() const { return new printer<Symbol>(); }
  
  /**
   * @brief **Implement** This function determines if a specific state is final.
   * 
   * @param state The state to examine.
   * @return True if state is final, false otherwise.
   */
  bool int_is_final_state(const State& state) const;
  
  /**
   * @brief **Implement** Returns the initial states.
   * 
   * @param states A list of states where the initial states must be added.
   */
  void int_initial_states(State_set& states) const;
  
  /**
   * @brief **Implement** Returns the successor for a specific state.
   * 
   * @param state The state for which the successors should be determined.
   * @param sigma The symbol indicating the transition that should be followed.
   * @param successors The set where the successors should be added. The set need not be empty on function call.
   */
  void int_successors(const State& state, const Symbol& sigma, State_set& successors) const;
  
  /**
   * @brief **Optionally Implement** Returns the successor for a specific state.
   * 
   * @param state The state for which the successors should be determined.
   * @param sigma The symbol indicating the transition that should be followed.
   * @param history The history can be taken into account to determine possible successors
   * @param successors The set where the successors should be added. The set need not be empty on function call.
   */
  void int_successors_hist(const State& state, const Symbol& sigma, const std::shared_ptr<counterexample_chain<Symbol>>& history, State_set& successors) const {
    impl().int_successors(state, sigma, successors);
  }
  
  /**
   * @brief **Implement** Returns possible successor symbols for a state.
   * 
   * Note that it is possible to return symbols by this function and when later \ref int_successors() is queried
   * for a specific symbol it returns an empty list. This function must return a superset of possible successor symbols.
   * 
   * @param state The state for which successor symbols should be listed.
   * @param symbols A set of symbols, where the symbols on the outgoing edges of state should be added. Need not be empty when the function is called.
   */
  void int_next_symbols(const State& state, Symbol_set& symbols) const;
  
  /**
   * @brief **Implement** Determines if a symbol should be considered an epsilon transition.
   * 
   * If a symbol is an epsilon transition it means that this symbol will be collapsed and never
   * appear if collapse_epsilon is true. If collapse_epsilon is false the symbol will appear in the
   * counter-example produced by the language inclusion, but the right-hand side will not be advanced on
   * an epsilon transition.
   * 
   * @return Returns true if a symbol should be considered an epsilon transition.
   */
  bool int_is_epsilon(const Symbol& symbol) const;
  
  /***********************************************
   * Public functions
   **********************************************/

  /**
   * @brief This function determines if a specific state is final.
   * 
   * @param state The state to examine.
   * @return True if state is final, false otherwise.
   */
  inline bool is_final_state(const State& state) const { return impl().int_is_final_state(state); }
  
  /**
   * @brief Returns the initial states.
   * 
   * @param states A list of states where the initial states must be added.
   */
  inline void initial_states(State_set& states) const { impl().int_initial_states(states); explore_epsilon(states); }
  
  /**
   * @brief Returns the successor for a specific state.
   * 
   * @param state The state for which the successors should be determined.
   * @param sigma The symbol indicating the transition that should be followed.
   * @param successors1 The set where the successors should be added. The set need not be empty on function call.
   */
  void successors(const State& state, const Symbol& sigma, State_set& successors1) const {
    std::pair<State,Symbol> p = std::make_pair(state, sigma);
    if (use_cache) {
      auto it = successor_cache.find(p);
      if (it != successor_cache.end()) {
        successors1.insert(it->second.begin(), it->second.end());
        return;
      }
      State_set& successors = successor_cache[p];
      //State_set successors;
      impl().int_successors(state, sigma, successors);
      explore_epsilon(successors); 
      successors1.insert(successors.begin(), successors.end());
    } else {
      impl().int_successors(state, sigma, successors1);
      explore_epsilon(successors1); 
    }
  }
  
  /**
   * @brief Returns the successor for a specific state.
   * 
   * @param state The state for which the successors should be determined.
   * @param sigma The symbol indicating the transition that should be followed.
   * @param history The history can be taken into account to determine possible successors
   * @param successors The set where the successors should be added. The set need not be empty on function call.
   */
  void successors(const State& state, const Symbol& sigma, const std::shared_ptr<counterexample_chain<Symbol>>& history, State_set& successors) const {
    impl().int_successors_hist(state, sigma, history, successors);
    explore_epsilon(successors);    
  }
  
  /**
   * @brief Returns possible successor symbols for a state.
   * 
   * @param state The state for which successor symbols should be listed.
   * @param symbols A set of symbols, where the symbols on the outgoing edges of state should be added. Need not be empty when the function is called.
   */
  inline void next_symbols(const State& state, Symbol_set& symbols) const { impl().int_next_symbols(state, symbols); filter_epsilon(symbols); }
  
  /**
   * @brief Returns a printer for states.
   * 
   * The function is efficient and can be called often.
   * 
   * @return A reference to the printer.
   */
  inline const printer_base<State>& state_printer() const { 
    if (!state_printer_) state_printer_ = impl().int_state_printer();
    return *state_printer_;
  }
  
  /**
   * @brief Returns a printer for symbols.
   * 
   * The function is efficient and can be called often.
   * 
   * @return A reference to the printer.
   */
  inline const printer_base<Symbol>& symbol_printer() const { 
    if (!symbol_printer_) symbol_printer_ = impl().int_symbol_printer();
    return *symbol_printer_; 
    
  }
  
  /**
   * @brief Determines if a symbol should be considered an epsilon transition.
   * 
   * If a symbol is an epsilon transition it means that this symbol will be collapsed and never
   * appear if collapse_epsilon is true. If collapse_epsilon is false the symbol will appear in the
   * counter-example produced by the language inclusion, but the right-hand side will not be advanced on
   * an epsilon transition.
   * 
   * @return Returns true if a symbol should be considered an epsilon transition.
   */
  inline bool is_epsilon(const Symbol& symbol) const { return impl().int_is_epsilon(symbol); }
  
  /**
   * @brief Determines if one of the states is final.
   * 
   * @param states A set of states.
   * @return True iff any of the states is a final state.
   */
  inline bool is_final_state(const State_set& states) const {
    for (State s:states) {
      if (is_final_state(s)) 
        return true;
    }
    return false;
  }
  
  /**
   * @brief Finds successors for a set of states.
   * 
   * @param states The starting states
   * @param sigma The symbol we are looking for a successor for
   * @return A set of successor states
   */
  inline State_set successors(const State_set& states, const Symbol& sigma) const
  {
    State_set result;
    successors(states, sigma, result);
    return result;
  }
  
  /**
   * @brief Returns successors for a set of states.
   * 
   * @param states The starting states
   * @param sigma The symbol we are looking for a successor for
   * @param successors1 Successors are added to this set.
   */
  inline void successors(const State_set& states, const Symbol& sigma, State_set& successors1) const
  {
    for (State state : states) {
      successors(state, sigma, successors1);
    }
  }
  
  /**
   * @brief Returns a set of initial states.
   * 
   * @return The set of initial states.
   */
  inline State_set initial_states() const { 
    State_set result;
    initial_states(result);
    return result;
  }
  
  /**
   * @brief Returns successors for a state
   * 
   * @param state The state.
   * @param sigma The symbol for successors.
   * @return The successors of state for symbol sigma.
   */
  inline State_set successors(const State& state, const Symbol& sigma) const { 
    State_set result;
    successors(state, sigma, result);
    return result;
  }
  
  /**
   * @brief Returns possible successor symbols of a state.
   * 
   * @param state The state.
   * @return The symbols on the outgoing edges of state
   */
  inline Symbol_set next_symbols(const State& state) const {
    Symbol_set result;
    next_symbols(state, result);
    return result;
  }
  
  /**
   * @brief If true during exploration of the next state the epsilons will be fully explored.
   * If false then epsilon transitions may be returned.
   * 
   */
  bool collapse_epsilon;
  
  /**
   * @brief Allow caching of successor relationships.
   *
   * Can be switched of while the automaton is in use. That will result in the cache no longer being used (neither read nor written).
   * 
   */
  mutable bool use_cache;
  
  /**
   * @brief Indicates that the implementation of next_symbols(const State&,Symbol_set&) const will never produce epsilon transitions.
   * 
   */
  bool no_epsilon_produced;
private:
  mutable const printer_base<State>* state_printer_ = nullptr;
  mutable const printer_base<Symbol>* symbol_printer_ = nullptr;
  
  inline Implementation& impl() {
    return *static_cast<Implementation*>(this);
  }
  inline const Implementation& impl() const {
    return *static_cast<const Implementation*>(this);
  }

  mutable std::unordered_map<std::pair<State,Symbol>,State_set> successor_cache;
  
  void filter_epsilon(Symbol_set& symbols) const {
    if (collapse_epsilon) {
      for (auto it = symbols.begin(); it!=symbols.end(); ) {
        if (impl().int_is_epsilon(*it))
          it = symbols.erase(it);
        else
          ++it;
      }
    }
  }
  
  void explore_epsilon(State_set& states) const {
    if (!collapse_epsilon) return;
    State_set seen;
    std::deque<State> frontier;
    frontier.insert(frontier.begin(), states.begin(), states.end());
    states.clear();
    while(!frontier.empty()) {
      const State& s = frontier.front();
      //state_printer()(s, std::cout); std::cout << std::endl;

      Symbol_set next_symbols;
      impl().int_next_symbols(s, next_symbols);
      State_set succs;
      bool first = true;
      if (impl().int_is_final_state(s)) {
        first = false;
        states.insert(s);
      }
      for (const Symbol& sy : next_symbols) {
        if (impl().int_is_epsilon(sy)) {
          impl().int_successors(s, sy, succs);
        } else if (first) { // if there is at least one non-epsilon transition we add this state
          first = false;
          states.insert(s);
        }
      }
      for (const State& st : succs) {
        if (seen.insert(st).second) {
          frontier.push_back(st);
        }
      }
      frontier.pop_front();
    }
    //std::cout << std::endl;
  }
};
}

#endif // LIMI_AUTOMATON_H
