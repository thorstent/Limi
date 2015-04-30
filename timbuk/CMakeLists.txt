cmake_minimum_required(VERSION 2.8)

project(timbuk)

find_package(FLEX 2.5)
find_package(BISON 2.4.3)

if (${FLEX_FOUND})
flex_target(timbuk_lexer timbuk.l ${CMAKE_CURRENT_SOURCE_DIR}/generated/timbuk.l.cc)
else (${FLEX_FOUND})
message(WARNING "Flex was not found. Compilation should still work, but lexer is not regenerated if you change timbuk.l")
endif (${FLEX_FOUND})
if (${BISON_FOUND})
bison_target(timbuk_parser timbuk.y ${CMAKE_CURRENT_SOURCE_DIR}/generated/timbuk.y.cc)
else (${BISON_FOUND})
message(WARNING "Bison was not found. Compilation should still work, but parser is not regenerated if you change timbuk.y")
endif (${BISON_FOUND})
if (${FLEX_FOUND} AND ${BISON_FOUND})
add_flex_bison_dependency(timbuk_lexer timbuk_parser)
endif (${FLEX_FOUND} AND ${BISON_FOUND})

add_compile_options(-std=c++11)

include_directories(".." ".")

add_executable(timbuk parsed_automaton.cpp symbol_table.cpp ${CMAKE_CURRENT_SOURCE_DIR}/generated/timbuk.l.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/generated/timbuk.y.cc main.cpp compile_all.cpp) 


