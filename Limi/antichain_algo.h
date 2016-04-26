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

#ifndef LIMI_ANTICHAINALGO_H
#define LIMI_ANTICHAINALGO_H

#include "automaton.h"
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <queue>
#include <memory>
#include <iostream>
#include "internal/antichain.h"
#include "results.h"
#include "internal/helpers.h"

/**
 * @brief The main namespace of the library.
 * 
 */
namespace Limi {

/**
  * @brief The classic antichain algorithm compares two automata for language inclusion (no independence relation)
  * 
  * The class already accepts the automata as constructor arguments and therefore cannot be reused for more
  * than one language inclusion query. The run() function runs until a counter-example is produced.
  * Run can be called again to produce another counter-example.
  * 
  * The algorithm is faster than \ref antichain_algo_ind and is guaranteed to terminate.
  * 
  * @tparam ImplementationA The implementation class of Automaton A
  * @tparam ImplementationB The implementation class of Automaton B
  * 
  */
template <class ImplementationA, class ImplementationB>
class antichain_algo
{
  using StateA = typename ImplementationA::State_;
  using Symbol = typename ImplementationA::Symbol_;
  using StateB = typename ImplementationB::State_;
  
  using StateA_vector = std::vector<StateA>;
  using StateB_set = std::unordered_set<StateB>;
  using StateBI_set = std::shared_ptr<const StateB_set>;
  using Symbol_set = std::unordered_set<Symbol>;
  using Symbol_vector = std::vector<Symbol>;
  using AutomatonA = automaton<StateA, Symbol, ImplementationA>;
  using AutomatonB = automaton<StateB, Symbol, ImplementationB>;
  
  using counter_chain = counterexample_chain<Symbol>;
  using pcounter_chain = std::shared_ptr<counter_chain>;
        
  struct pair {
    StateA a;
    StateBI_set b;
    pair(StateA a, StateBI_set b) : a(a), b(b), cex_chain(nullptr) {}
    pair(StateA a, StateBI_set b, const pcounter_chain& parent, const Symbol& sym) : pair(a,b)
    {
      cex_chain = std::make_shared<counter_chain>(sym, parent);
    }
    pcounter_chain cex_chain;
  };
  
  using pair_antichain = internal::antichain<StateA, StateB>;
  
  std::deque<pair> initial_states(const AutomatonA& a, const AutomatonB& b) {
    std::shared_ptr<StateB_set> states_b = std::make_shared<StateB_set>();
    b.initial_states(*states_b);
    std::deque< pair > result;
    for(StateA state_a : a.initial_states()) {
      pair p(state_a, states_b);
      result.push_back(p);
      antichain.add_unchecked(state_a, states_b, false);
    }
    return result;
  }
  
  const AutomatonA& a;
  const AutomatonB& b;
  pair_antichain antichain;
      
  std::deque<pair> frontier = initial_states(a,b);
  
public:
  
  /**
    * @brief Constructor that initialises the language inclusion algorithm.
    * 
    * @param a The automaton a
    * @param b The automaton b. The b automaton must not produce any epsilon transitions.
    * 
    */
  antichain_algo(const AutomatonA& a, const AutomatonB& b) :
    a(a), b(b) {
      if (!b.collapse_epsilon && !b.no_epsilon_produced) {
        throw std::logic_error("For the automaton B in the language inclusion algorithm either collapse_epsilon must be true or no_epsilon_produced");
      }
    }
  
  
  /**
    * @brief Run the language inclusion.
    * 
    * Can be called several time to obtain several counter-examples.
    * 
    * @return Language inclusion result and a counter-example trace (if applicable)
    */
  inclusion_result<Symbol> run()
  {
    inclusion_result<Symbol> result;
    result.included = true;
    result.bound_hit = false;
    
#ifdef DEBUG_PRINTING
    unsigned loop_counter = 0;
    unsigned transitions = 0;
#endif
    while (frontier.size() > 0) {
#ifdef DEBUG_PRINTING
      if (DEBUG_PRINTING>=2 && loop_counter % 1000 == 0) std::cout << loop_counter << " rounds; A states: " << antichain.size() << std::endl;
#endif
      pair current = frontier.front();
      frontier.pop_front();
      
      Symbol_vector next_symbols;     
      a.next_symbols(current.a, next_symbols);
      
#ifdef DEBUG_PRINTING
      ++ loop_counter;
#endif
      if ((a.is_final_state(current.a) && !b.is_final_state(*current.b))) {
        result.counter_example = current.cex_chain->to_vector();
        result.included = false;
        break;
      }           
      
#ifdef DEBUG_PRINTING        
      if (DEBUG_PRINTING>=3) {
        std::cout << "Next pair: ";
        std::cout << a.state_printer()(current.a) << " - ";
        internal::print_set(*current.b, std::cout, b.state_printer());
        std::cout << std::endl;
      }    
#endif

      for (Symbol sigma : next_symbols) {
#ifdef DEBUG_PRINTING
        ++transitions;
        if (DEBUG_PRINTING>=4) {
          std::cout << "Symbol: ";
          std::cout << a.symbol_printer()(sigma);
          std::cout << std::endl;
        }
#endif
        StateA_vector states_a = a.successors(current.a, sigma);
        StateBI_set unpruned;
        StateBI_set states_b;
        if (a.is_epsilon(sigma)) states_b=current.b; else {
          auto states_b1 = std::make_shared<StateB_set>();
          b.successors(*current.b, sigma, *states_b1);
          states_b = states_b1;
        }
                
        for (StateA state_a : states_a) {
          antichain_algo::pair next(state_a, states_b, current.cex_chain, sigma);
          
          if (!antichain.contains(next.a, next.b)) {
            antichain.add(next.a, next.b, false);
          
            frontier.push_front(std::move(next));
          }
        }
        
      }
      
    }
    
#ifdef DEBUG_PRINTING
    if (DEBUG_PRINTING>=1) std::cout << loop_counter << " rounds; seen states: " << antichain.size() << "; transitions: " << transitions << std::endl;
    
    if (DEBUG_PRINTING >= 4) {
      antichain.print(std::cout, a.state_printer(), b.state_printer());
    }
#endif
    
    return result;
  }

};
  
  
}

#endif // LIMI_ANTICHAINALGO_H
