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

#include "symbol_table.h"

using namespace timbuk;
using namespace std;

// *************************
// All the function are called by the parser.

symbol symbol_table::add_symbol(std::string name)
{
  auto it = lookup_.find(name);
  if (it==lookup_.end()) {
    symbol s = symbols_.size();
    symbols_.push_back(name);
    lookup_.insert(make_pair(name, s));
    return s;
  } else 
    return it->second;
}

string symbol_table::lookup(symbol s) const
{
  return symbols_.at(s);
}

symbol symbol_table::find(string name) const
{
  auto it = lookup_.find(name);
  if (it==lookup_.end())
    throw range_error("Symbol " + name + " not found");
  return it->second;
}

void symbol_table::add_independence(string name1, string name2)
{
  auto p1 = make_pair(find(name1),find(name2));
  auto p2 = make_pair(find(name2),find(name1));
  independence_.insert(p1);
  independence_.insert(p2);
}

bool symbol_table::independence_empty()
{
  return independence_.empty();
}

