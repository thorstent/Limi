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
 
 
%option noyywrap
%option nounput
%option yylineno
%option noinput
%option never-interactive
%option nounistd

%{
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "timbuk.y.hh"
#include <string>

#define STOKEN( x ) ( new std::string( x ) )
%}

string  \"[^\n"]+\"

ws      [ \t]+
nl      [\n]+


alpha   [A-Za-z]
dig     [0-9]
doublecolon "::"
dot "."
name    ({alpha}|{dig})({alpha}|{dig}|[_]|{dot})*
number  {dig}+

%%

{ws}            { /* ignore */ }
{nl}            { /* ignore */ }
"#".*           { /* ignore comment */ }

(?i:"Ops")           { return OPS; }
(?i:"Automaton")     { return AUTOMATON; }
(?i:"Final States")  { return FINALSTATES; }
(?i:"States")        { return STATES; }
(?i:"Transitions")   { return TRANSITIONS; }
(?i:"Independence")   { return INDEPENDENCE; }
"("             { return LPAREN; }
")"             { return RPAREN; }
":"             { return COLON; }
"->"            { return ARROW; }

{number}        { yylval.val = atoi(yytext); return NUMBER; }
{name}          { yylval.sval = new std::string(yytext, strlen(yytext));
                  return(NAME); } 
                  