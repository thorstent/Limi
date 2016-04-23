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

#ifndef LIMI_INTERNAL_META_AUTOMATON_H
#define LIMI_INTERNAL_META_AUTOMATON_H

#include <memory>

#include "../automaton.h"
#include "meta_state.h"
#include "helpers.h"
#include <iostream>
#include <cassert>
#include <stdexcept>

namespace Limi {
  namespace internal {
  
  
  /**
   * @brief This automaton presents an automaton up to an independence relation
   * 
   * This automaton consists of states that stack symbols not yet matched. The details of the algorithm are outlined in the paper.
   * 
   */
template <class InnerImplementationB, class Independence = independence<typename InnerImplementationB::Symbol_>>
class meta_automaton : public Limi::automaton<std::shared_ptr<meta_state<typename InnerImplementationB::State_, typename InnerImplementationB::Symbol_, Independence>>,typename InnerImplementationB::Symbol_,meta_automaton<InnerImplementationB, Independence>> {
  using InnerStateB = typename InnerImplementationB::State_;
  using Symbol = typename InnerImplementationB::Symbol_;
  typedef meta_state<InnerStateB, Symbol, Independence> StateB;
public:
  typedef std::shared_ptr<StateB> StateI;
private:
  typedef typename Limi::automaton<StateI,Symbol,meta_automaton<InnerImplementationB, Independence>>::State_vector State_vector;
  typedef typename Limi::automaton<StateI,Symbol,meta_automaton<InnerImplementationB, Independence>>::Symbol_vector Symbol_vector;
  
  typedef automaton<InnerStateB, Symbol, InnerImplementationB>  InnerAutomatonB;
public:
    
  meta_automaton(const InnerAutomatonB& automaton, const Independence& independence = Independence()) :
  Limi::automaton<std::shared_ptr<meta_state<InnerStateB, Symbol, Independence>>,Symbol,meta_automaton<InnerImplementationB, Independence>>(false, true),
  inner(automaton), independence_(independence) {
    if (!automaton.collapse_epsilon && !automaton.no_epsilon_produced) {
      throw std::logic_error("For the automaton B in the language inclusion algorithm either collapse_epsilon must be true or no_epsilon_produced");
    }
  }
  
  bool int_is_final_state(const StateI& state) const {
    return (state->early().size()==0 && state->late().size()==0 && inner.is_final_state(state->inner_state()));
  }
  
  void int_initial_states(State_vector& states) const {
    std::vector<InnerStateB> is;
    inner.initial_states(is);
    for (const auto& i:is) {
      states.push_back(std::make_shared<StateB>(i));
    }
  }
  
private:
  
  /**
   * @brief Checks if the symbol is independent with all elements in the vector.
   * 
   * @param begin The beginning iterator
   * @param end The end.
   * @return bool -1 if not found, -2 if not independent, otherwise the position
   */
  int check_independence(typename StateB::vector_iterator begin, const typename StateB::vector_iterator& end, const Symbol& symbol) const {
    int counter = 0;
    for (; begin!=end; ++begin) {
      if (std::equal_to<Symbol>()(*begin, symbol)) return counter;
      if (!independence_(*begin, symbol)) return -2;
      ++counter;
    }
    return -1;
  }
  
  
  StateI successor(const StateI& state, const Symbol& sigmaA, const Symbol& sigmaB) const {
    // check if the element we are about to add to early (sigmaB) can commute with everything in late. In the case it does not, then this state is dead
    // if the element is already in late than it only needs to commute with all elements up to that
    // furthermore we remove the element in late
    int pos_early, pos_late;
    if ((pos_late = check_independence(state->late().begin(), state->late().end(), sigmaB)) == -2) return StateI();

    // make a copy
    StateI newst = std::make_shared<StateB>(*state);
    // now delete if needed (and add otherwise)
    if (pos_late!=-1)
      newst->erase_late(pos_late);
    else
      newst->add_early(sigmaB, independence_);
    
    // now do it for the other one
    if ((pos_early = check_independence(newst->early().begin(), newst->early().end(), sigmaA)) == -2) return StateI();
    if (-1!=pos_early)
      newst->erase_early(pos_early);
    else
      newst->add_late(sigmaA, independence_);
    assert(newst->early().size() == newst->late().size());
    //print_state(newst, std::cout); std::cout << std::endl;
    return newst;
  }
  
public:
  
  void int_successors(const StateI& state, const Symbol& sigmaA, State_vector& successors) const {
    for (const Symbol& sigmaB : inner.next_symbols(state->inner_state())) {
      //print_state(state, std::cout); std::cout << std::endl;
      //std::cout << sigmaA << std::endl;
      //std::cout << sigmaB << std::endl;
      StateI succ = successor(state, sigmaA, sigmaB);
      if (succ) {
        typename InnerAutomatonB::State_vector succs;
        inner.successors(state->inner_state(),sigmaB, succs);
        for(auto it = succs.begin(); it!=succs.end(); ++it) {
          // make a copy if this is more than once needed
          if (it!=succs.begin())
            succ = std::make_shared<StateB>(*succ);
          succ->inner_state(*it);
          
          successors.push_back(succ);
        }
        
      }
    }
  }
  
  inline void int_next_symbols(const StateI& state, Symbol_vector& symbols) const {
    throw std::logic_error("The meta-automaton cannot produce a set of next symbols");
  }
  
  inline printer_base<StateI>* int_state_printer() const { return new printer<StateI>(inner.state_printer(), inner.symbol_printer()); }
  
  inline const printer_base<Symbol>& symbol_printer() const { return inner.symbol_printer(); }
  
  inline bool int_is_epsilon(const Symbol& symbol) const { return inner.is_epsilon(symbol); }
  
private:
  const InnerAutomatonB& inner;
  const Independence& independence_;
};

  }
}

#endif //LIMI_INTERNAL_META_AUTOMATON_H