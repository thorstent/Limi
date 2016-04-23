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

#ifndef TIMBUK_SYMBOL_TABLE_H
#define TIMBUK_SYMBOL_TABLE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <Limi/generics.h>
#include <Limi/internal/hash.h>
#include <cstdint>

namespace timbuk {
  /**
   * @brief The symbol is a thin wrapper around a 32-bit integer
   * 
   */
  struct symbol {
    uint32_t s;
    inline symbol(uint32_t s) : s(s) {}
    inline operator uint32_t() const { return s;}
    /**
     * @brief Equality for the symbol needs to be defined 
     * 
     * Implicitly defines equal_to<symbol>
     */
    inline bool operator== (const symbol& other) const { return s==other.s; }
  };
}

namespace std {
  /**
   * @brief Hash function for the symbol needs to be defined
   * 
   */
  template<> struct hash<timbuk::symbol> {
    inline size_t operator()(const timbuk::symbol& val) const {
      return hash<uint32_t>()(static_cast<uint32_t>(val.s));
    }
  };
}

namespace timbuk {
  
  /**
   * @brief The symbol table provides a mapping from intergers and names.
   *
   * Note that we use integers because it improves performance.
   * We need a seperate class for the symbols because the symbols need to match between the two automata
   * (as in the same name needs to map to the same integer).
   * 
   */
  class symbol_table
{
  std::vector<std::string> symbols_;
  std::unordered_map<std::string, symbol> lookup_;
  std::unordered_set<std::pair<symbol, symbol>> independence_;
public:
  symbol add_symbol(std::string name);
  std::string lookup(symbol s) const;
  symbol find(std::string name) const;
  void add_independence(std::string name1, std::string name2);
  /**
   * @brief Tests if two symbols are independent
   */
  inline bool independent(const symbol a, const symbol b) const {
    // the test simply looks this up in a list of all independent symbols
    return (independence_.find(std::make_pair(a,b))!=independence_.end());
  }
  bool independence_empty();
};
}

namespace Limi {
  /**
   * @brief Provide a printer for symbols (look up their name in the symbol table)
   * 
   */
  template<> struct printer<timbuk::symbol> : printer_base<timbuk::symbol> {
    printer(const timbuk::symbol_table& symbol_table) : symbol_table_(symbol_table) {}
    virtual void print(const timbuk::symbol& symbol, std::ostream& out) const override {
      out << symbol_table_.lookup(symbol);
    }
  private:
    const timbuk::symbol_table& symbol_table_;
  };
  
  /**
   * @brief The independence relation requires the symbol table.
   * 
   */
  template<> struct independence<timbuk::symbol> {
    independence(const timbuk::symbol_table& symbol_table) : symbol_table_(symbol_table) {}
    inline bool operator()(const timbuk::symbol& a, const timbuk::symbol& b) const {
      return symbol_table_.independent(a,b);
    }
  private:
    const timbuk::symbol_table& symbol_table_;
  };
}

#endif // TIMBUK_SYMBOL_TABLE_H
