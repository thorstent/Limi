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

// make sure we compile every template in Limi so to catch syntax errors
// this file is not used for anything else. The linker will throw out this code.

#include "parsed_automaton.h"
#include "automaton.h"

#include <Limi/timbuk_printer.h>
#include <Limi/dot_printer.h>
#include <Limi/reachable.h>
#include <Limi/antichain_algo_ind.h>
#include <Limi/antichain_algo.h>
#include <Limi/list_automaton.h>

using namespace timbuk;

#define DEBUG_PRINTING 2

void all() {
  timbuk::symbol_table st;
  Limi::independence< timbuk::symbol > ind(st);
  Limi::dot_printer<timbuk::state,timbuk::symbol,timbuk::automaton> dp;
  parsed_automaton inner(st, "test");
  automaton aut(inner);
  Limi::explore<timbuk::state,timbuk::symbol,timbuk::automaton>(aut);
  Limi::timbuk_printer<timbuk::state,timbuk::symbol,timbuk::automaton> tp(ind);
  Limi::antichain_algo_ind<automaton, automaton> aai(aut,aut,2,ind);
  Limi::antichain_algo<automaton, automaton> aa(aut,aut);
  std::list<timbuk::symbol> sy_list;
  Limi::list_automaton<timbuk::symbol> list(sy_list.begin(), sy_list.end(), aut.symbol_printer());
}