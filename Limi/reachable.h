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

#ifndef LIMI_REACHABLE_H
#define LIMI_REACHABLE_H
#include "automaton.h"

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <deque>
#include <unordered_map>

namespace Limi {

/**
  * @brief This function fully explores an automaton and returns a set of all reachable states.
  * 
  * @tparam State The type of states
  * @tparam Symbol The type of symbols
  * @tparam Implementation The implementation type of the automaton
  * 
  * @param automaton The object of the automaton to explore.
  * 
  * @returns A set of all reachable states.
  * 
  */
template <class State, class Symbol,class Implementation>
std::unordered_set<State> explore(const automaton<State,Symbol,Implementation>& automaton) {
  std::unordered_set<State> seen;
  std::deque<State> frontier;
  for (const auto& s : automaton.initial_states()) {
    frontier.push_back(s);
   }
  while (!frontier.empty()) {
    State next = frontier.front();
    frontier.pop_front();
    if (seen.find(next) == seen.end()) {
      seen.insert(next);
      for (const Symbol& symbol : automaton.next_symbols(next)) {
        for (const State& succ : automaton.successors(next, symbol)) {
          frontier.push_back(succ);
        }
      }
    }
  }
  
  return seen;
}

}

#endif // LIMI_REACHABLE_H
