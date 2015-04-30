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

%error-verbose
%locations
%expect 0

%code requires {
  #include <stdio.h>
  #include <string>
  #include "parsed_automaton.h"
  
}

%union
{
  std::string *sval;
  int val;
};

%code {
  int yylex (void);
  void yyerror (timbuk::parsed_automaton&, char const *);
}


%parse-param {timbuk::parsed_automaton& aut}

%token OPS AUTOMATON STATES FINALSTATES TRANSITIONS INDEPENDENCE
%token COLON LPAREN RPAREN ARROW
%token <val> NUMBER
%token <sval> NAME

%start File
%%
File:
OPS Symbollist
Independencesection
AUTOMATON NAME
STATES Statelist
FINALSTATES Finalstatelist
TRANSITIONS Transitionlist
{ /*aut.automaton_name(*$5);*/ delete $5; }
;

Independencesection:
| INDEPENDENCE Independencelist
;

Independencelist:
| Independencelist LPAREN NAME NAME RPAREN
{ aut.add_independence(*$3,*$4); delete $3; delete $4; }
;

Symbollist:
| Symbollist NAME COLON NUMBER 
{ aut.add_symbol(*$2); delete $2; }
;

Statelist:
| Statelist NAME
{ aut.add_state(*$2); delete $2; }
;

Finalstatelist:
| Finalstatelist NAME  
{ aut.mark_final(*$2); delete $2; }
;

Transitionlist:
| Transitionlist Transition
;

Transition:
NAME ARROW NAME
{ aut.add_successor(*$1, *$3); delete $1; delete $3; }
| NAME LPAREN RPAREN ARROW NAME 
{ aut.add_successor(*$1, *$5); delete $1; delete $5; }
| NAME LPAREN NAME RPAREN ARROW NAME
{ aut.add_successor(*$3, *$1, *$6); delete $1; delete $3; delete $6; }
;