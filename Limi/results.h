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

#ifndef LIMI_RESULTS_H
#define LIMI_RESULTS_H

#include <unordered_set>
#include <list>
#include <algorithm>
#include "generics.h"

namespace Limi {

/**
  * @brief Represents a list of symbols.
  * 
  * This type builds a single-linked list of symbols. This can save memory if different heads 
  * point to the identical tails of the list. It is used to track the path through the automaton
  * so that a counter-example can be returned.
  * 
  * @tparam Symbol The type of symbols in the list
  * 
  */
template<class Symbol>
struct counterexample_chain {
  Symbol current;
  std::shared_ptr<counterexample_chain> parent;
  
  counterexample_chain(const Symbol& current, const std::shared_ptr<counterexample_chain>& parent) :
  current(current), parent(parent) {  }
  
  std::list<Symbol> to_list() const {
    std::list<Symbol> result;
    const counterexample_chain* c = this;
    while (c) {
      result.push_front(c->current);
      c = c->parent.get();
    }
    return result;
  }
  
  std::vector<Symbol> to_vector() const {
    std::vector<Symbol> result;
    const counterexample_chain* c = this;
    while (c) {
      result.push_back(c->current);
      c = c->parent.get();
    }
    std::reverse(result.begin(), result.end());
    return result;
  }

private:
};


/**
 * @brief The result of the language inclusion test.
 * 
 */
template <class Symbol>
struct inclusion_result {
  /**
   * @brief True if automaton A is included in automaton B.
   * 
   * A result true can always be trusted. A negative result *can* be false-negative
   * if \ref bound_hit is true.
   * 
   */
  bool included = false;
  /**
   * @brief Was the bound hit during automata exploration.
   * 
   * If the bound was hit then a false-negative result may be returned.
   * This means that if bound_hit is true and \ref included is false further
   * checks are needed. See the timbuk example to understand how to proceed in 
   * that case.
   * 
   */
  bool bound_hit = false;
  /**
   * @brief A counter-example if applicable.
   * 
   * Only used by \ref Limi::antichain_algo_ind
   * 
   * Only if \ref included is false. The vector represents a word accepted by A
   * and not accepted by B up to bounded independence.
   * 
   */
  std::vector<Symbol> counter_example;
  
  /**
   * @brief Filters the result trace
   * 
   * This is used to remove symbols from the trace that are not of interest.
   * Does nothing if the inclusion result is true.
   * 
   * @param to_remove A function that returns true if a symbol should be filtered out (removed)
   * from the trace.
   */
  
  void filter_trace(std::function<bool(Symbol)> to_remove) {
    counter_example.erase( std::remove_if(counter_example.begin(), counter_example.end(), to_remove), std::end(counter_example));
  }
    
  /**
   * @brief The maximum bound used to obtain this inclusion result.
   * 
   * Only used by \ref Limi::antichain_algo_ind
   * 
   */
  unsigned max_bound = 0;
    
  /**
   * @brief Print this result.
   * 
   * @param stream The stream to print to.
   * @param symbol_printer The printer to print out the counter-example if any.
   */
  void print_long(std::ostream& stream, const printer_base<Symbol>& symbol_printer) {
    if (included)
      stream << "Included" << std::endl;
    else {
      stream << "Not Included";
      if (bound_hit) stream << "; bound";
      stream << std::endl;
      for (const auto& s : counter_example) {
        stream << symbol_printer(s);
        //s.symbol->instr_id->printPretty(lout, nullptr, pp);
        stream << std::endl;
      }
    }
  }
};


}

#endif //LIMI_RESULTS_H