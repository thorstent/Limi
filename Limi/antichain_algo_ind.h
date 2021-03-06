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

#ifndef LIMI_ANTICHAINALGO_IND_H
#define LIMI_ANTICHAINALGO_IND_H

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
#include "internal/meta_automaton.h"

/**
 * @brief The main namespace of the library.
 * 
 */
namespace Limi {

/**
  * @brief The antichain algorithm compares two automata for language inclusion module an independence relation.
  * 
  * Language inclusion up to an independence relation is undecidable in general. Therefore this class
  * implements a bounded version where a stack is used to match symbols that may occure in the future.
  * For low stack sizes the algorithm works faster but may produce spurious counter-examples. The bound 
  * can be increased to eliminate those. See the main file in the timbuk example to see how to deal
  * with spurious counter-examples.
  * 
  * The class already accepts the automata as constructor arguments and therefore cannot be reused for more
  * than one language inclusion query. The run() function runs until a counter-example is produced. This 
  * counter-example can be spurious though. Run can be called again to produce another counter-example.
  * In addition increase_bound() can be called to increase the bound. A subsequent call to the run() function
  * will yield a counter-example for the higher bound. This process is incremental.
  * 
  * 
  * @tparam ImplementationA The implementation class of Automaton A
  * @tparam InnerImplementationB The implementation class of Automaton B
  * @tparam Independence The independence relation (defaults to \ref Limi::independence). Instead of giving an
  * empty independence relation (where nothing is independent) it would be more efficient to use the class 
  * \ref Limi::antichain_algo.
  * 
  */
template <class ImplementationA, class InnerImplementationB, class Independence = independence<typename ImplementationA::Symbol_>>
class antichain_algo_ind
{
  using StateA = typename ImplementationA::State_;
  using InnerStateB = typename InnerImplementationB::State_;
  using Symbol = typename ImplementationA::Symbol_;
  
  using ImplementationB = internal::meta_automaton<InnerImplementationB, Independence>;
  using StateB = typename ImplementationB::StateI;
  using StateA_vector = std::vector<StateA>;
  using StateB_set = std::unordered_set<StateB>;
  using StateBI_set = std::shared_ptr<const StateB_set>;
  using Symbol_set = std::unordered_set<Symbol>;
  using Symbol_vector = std::vector<Symbol>;
  using AutomatonA = automaton<StateA, Symbol, ImplementationA>;
  using InnerAutomatonB = automaton<InnerStateB, Symbol, InnerImplementationB>;
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
    
    bool dirty = false;
    pcounter_chain cex_chain;
  };
  
  using pair_antichain = internal::antichain<StateA, StateB>;
  
  void prune(std::shared_ptr<StateB_set>& b, StateBI_set& un_pruned, unsigned k, bool dirty) {
    for(auto it = b->begin(); it!=b->end(); ) {
      if ((**it).size() > k) {
        if (!un_pruned && !dirty) {
          un_pruned = std::make_shared<StateB_set>(*b);
        }
        it = b->erase(it);
      } else {
        ++it;
      }
    }
  }
  
  void remove_dirty(std::deque<pair>& frontier) {
    std::deque<pair> newp;
    while (!frontier.empty()) {
      auto& e = frontier.front();
      if (!e.dirty) {
        newp.push_back(e);
      }
      frontier.pop_front();
    }
    frontier = newp;
  }
  
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
  ImplementationB b_;
  const AutomatonB& b = b_;
  pair_antichain antichain;
  
  unsigned bound = 2;  // bound of the algorithm
  const Independence& independence_;
  
  std::deque<pair> before_dirty;
  std::deque<pair> frontier = initial_states(a,b);    
public:
  
  /**
    * @brief Constructor that initialises the language inclusion algorithm.
    * 
    * @param a The automaton a
    * @param ib The automaton b
    * @param initial_bound The starting bound 
    * @param independence The independence (if there is no default constructor)
    * 
    */
  antichain_algo_ind(const AutomatonA& a, const InnerAutomatonB& ib, unsigned initial_bound = 2, const Independence& independence = Independence()) :
    a(a), b_(ib, independence), bound(initial_bound), independence_(independence) {
  }
  
  /**
    * @brief Returns the current bound.
    * 
    */
  unsigned get_bound() {
    return bound;
  }
  
  /**
    * @brief Incleases the bound of the language inclusion check. 
    * 
    * This causes a partial restart of the language inclusion check the next time run()
    * is called and run may find the same counter-example again.
    */
  void increase_bound(unsigned new_bound) {
    if (new_bound < bound) throw std::logic_error("New bound smaller than old bound.");
    if (new_bound == bound) return;
    bound = new_bound;
    antichain.clean_dirty();
    
    remove_dirty(frontier);
    for (auto& e : before_dirty) {
      if (!antichain.contains(e.a, e.b)) {
        frontier.push_back(e);
        antichain.add(e.a, e.b, false);
      }
    }
    before_dirty.clear();
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
    result.max_bound = bound;
    
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
        if (current.dirty)
          result.bound_hit = true;
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
          prune(states_b1, unpruned, bound, current.dirty);
          states_b = states_b1;
        }
        
        for (StateA state_a : states_a) {
          antichain_algo_ind::pair next(state_a, states_b, current.cex_chain, sigma);
          next.dirty = current.dirty || unpruned;
          if (unpruned) before_dirty.push_back(antichain_algo_ind::pair(state_a, unpruned, current.cex_chain, sigma));
          
          if (!antichain.contains(next.a, next.b)) {
            antichain.add(next.a, next.b, next.dirty);
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

#endif // LIMI_ANTICHAINALGO_IND_H
