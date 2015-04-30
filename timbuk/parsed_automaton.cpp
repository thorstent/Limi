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

#include "parsed_automaton.h"

#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <fstream>

using namespace timbuk;

using namespace std;


extern int yylineno;

bool parse_error = false;
/**
 * @brief Called by the parser if a parse error is encountered
 */
void yyerror(timbuk::parsed_automaton& in, char const* err) {
  cerr << "Parse error in " << in.filename << " at line " << yylineno << endl;
  cerr << err << endl;
  parse_error = true;
}


int yyparse(parsed_automaton&);
extern FILE* yyin;

/**
 * @brief Parses a timbuk files
 * 
 * @param symbol_table The symbol table must be identical between automata
 * @param filename The file to parse
 */
parsed_automaton::parsed_automaton(symbol_table& symbol_table, const std::string& filename) : filename(filename), st(symbol_table), initial_(state_set{ 0 })
{
  yyin = fopen(filename.c_str(), "r" );
  if (yyin == 0) {
    throw std::runtime_error("File " + filename + " not accessible");
  }
  add_state("initial");
  /**
   * @brief Call the parser and give it an instance of this class to fill it.
   * 
   */
  int res = yyparse(*this);
  if (res!=0 || parse_error)
    throw runtime_error("Parse error");
  fclose(yyin);
    
}
// *********************************
// remaining functions are called by the parser

state parsed_automaton::add_state(const string& name)
{
  if (lookup_.find(name)!=lookup_.end())
    throw runtime_error("State " + name + " duplicate");
  names.push_back(name);
  successors_.push_back(successor_set());
  symbols_.push_back(symbol_set());
  final_.push_back(false);
  state s = names.size()-1;
  lookup_.insert(make_pair(name, s));
  return s;
}

symbol parsed_automaton::add_symbol(const string& name)
{
  return st.add_symbol(name);
}

void parsed_automaton::mark_final(string s)
{
  state st = find(s);
  mark_final(st);
}

void parsed_automaton::mark_final(state s)
{
  final_[s] = true;
}

state parsed_automaton::find(string name)
{
  auto it = lookup_.find(name);
  if (it==lookup_.end())
    throw range_error("State " + name + " not found");
  return it->second;
}

void parsed_automaton::add_successor(state s, symbol transition_symbol, state successor)
{
  successors_[s][transition_symbol].insert(successor);
  symbols_[s].insert(transition_symbol);
}

void parsed_automaton::add_successor(string s, string transition_symbol, string successor)
{
  //cout << s << " - " << transition_symbol << endl;
  add_successor(find(s), st.find(transition_symbol), find(successor));
}

void parsed_automaton::add_successor(string transition_symbol, string successor)
{
  add_successor(0, st.find(transition_symbol), find(successor));
}

symbol_table& parsed_automaton::get_symbol_table()
{
  return st;
}

const symbol_table& parsed_automaton::get_symbol_table() const
{
  return st;
}

void parsed_automaton::add_independence(const string& name1, const string& name2)
{
  st.add_independence(name1, name2);
}
