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

#include "automaton.h"
#include "automaton.h"

// set verbosity to 2 (if desired)
//# define DEBUG_PRINTING 2

#include <Limi/antichain_algo_ind.h>
#include <Limi/antichain_algo.h>
#include <Limi/dot_printer.h>
#include <Limi/list_automaton.h>

#include <chrono>

#include <string>
#include <iomanip>
#include <stdexcept>

using namespace std;

int main_wrapped(int argc, const char **argv);
Limi::inclusion_result<timbuk::symbol> compare_no_independence(const timbuk::automaton& a, const timbuk::automaton& b);
Limi::inclusion_result<timbuk::symbol> compare_with_independence(const timbuk::automaton& a, const timbuk::automaton& b, const timbuk::symbol_table& st);

// some arbitrary value indicating 
const unsigned max_bound = 10;
const unsigned initial_bound = 2; // 1 or 2 is usually a good start

/**
 * @brief Wraps main and catches any execptions thrown.
 */
int main(int argc, const char **argv) {
  try {
    return main_wrapped(argc, argv);
  } catch (std::exception e) {
    cerr << "Exception thrown: " << e.what() << endl;
    return 2;
  }
}

/**
 * @brief Actual main function
 */
int main_wrapped(int argc, const char **argv) {
  if (argc < 3) {
    cerr << "Two arguments are needed: The two automata to compare." << endl;
    return 1;
  }
  string filename(argv[1]);
  string filename2(argv[2]);
  timbuk::symbol_table st;
  cout << "Parsing" << endl;
  timbuk::parsed_automaton aut(st,filename);
  timbuk::parsed_automaton aut2(st,filename2);
  
  timbuk::automaton auti(aut);
  timbuk::automaton auti2(aut2);
  
  // these commands would print out a dot file of the parsed automaton
  //ofstream out("automaton.dot");
  //Limi::print_dot(auti, out);
  //out.close();
  
  cout << "Language inclusion check..." << endl;
  
  // time measuring stuff
  auto start = chrono::steady_clock::now();
  
  Limi::inclusion_result<timbuk::symbol> result;
  // if the independence relation is empty use the faster classic antichain algorithm algorithm 
  if (st.independence_empty()) {
    result = compare_no_independence(auti, auti2);
  } else {
    result = compare_with_independence(auti, auti2, st);
  }

  auto stop = chrono::steady_clock::now();
  
  // print the trace of the language inclusion
  result.print_long(cout, auti.symbol_printer());
  
  chrono::milliseconds passed = std::chrono::duration_cast<chrono::milliseconds>(stop - start);
  cout << "TIME: " << std::setprecision(3) << std::fixed << (double)passed.count()/1000 << " s" << endl;
}

/**
 * @brief Runs the algorithm without independence relation. Faster if no independence relation is required.
 */
Limi::inclusion_result<timbuk::symbol> compare_no_independence(const timbuk::automaton& a, const timbuk::automaton& b) {
  auto algo = Limi::antichain_algo<timbuk::automaton,timbuk::automaton>(a, b);
  return algo.run();
}

/**
 * @brief Runs the language inclusion algorithm with independence relation
 * 
 * @return Guarantees that the trace is not spurious
 */
Limi::inclusion_result<timbuk::symbol> compare_with_independence(const timbuk::automaton& a, const timbuk::automaton& b, const timbuk::symbol_table& st) {
  auto algo = Limi::antichain_algo_ind<timbuk::automaton,timbuk::automaton>(a, b, initial_bound, Limi::independence<timbuk::symbol>(st));
  // limit the loop to some arbitrary boundary you can fix
  // in general the algorithm may diverge
  while (algo.get_bound() < max_bound) {
    auto result = algo.run(); // run the algorithm
    if (result.included)
      // a positive result is always correct
      return result;
    // now let's see what the negative result is
    if (!result.bound_hit)
      // if the bound was not hit, the result is also guaranteed to be correct
      return result;
    // now we know that a counter-example was produced that hit the bound.
    // we need to check if the example is suprious (we could be lucky and the example is valid even though it hit the bound)
    // the check is very cheap because we use the list_automaton
    {
      Limi::list_automaton<timbuk::symbol> ctex_automaton(result.counter_example.begin(), result.counter_example.end(), a.symbol_printer());
      // we use a starting bound of the length of the counter-example
      Limi::antichain_algo_ind<Limi::list_automaton<timbuk::symbol>,timbuk::automaton> algo_check(ctex_automaton, b, result.counter_example.size(),  Limi::independence<timbuk::symbol>(st));
      auto check_res = algo_check.run();
      assert (!check_res.bound_hit); // this should never happen because the bound has the length of the trace
      if (!check_res.included)
        return result; // we indeed found a counter-example 
      // otherwise continue with the loop by increasing the bound
    }
    // increase the bound so that we can get rid of the spurious counter-example
    algo.increase_bound(algo.get_bound()+1);
  }
  // no valid solution was found
  throw runtime_error("No valid answer found up to max_bound");
}