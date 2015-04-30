Overview                         {#mainpage}
============

Limi (Language Inclusion Modulo Independence) is a template library that implements the basic antichain algorithm modulo an independence relation. The library takes as input two non-deterministic finite automata (NFA) A and B with a shared alphabet and an independence relation I over the symbols of the alphabet. The independence relation determines for two symbols if they are independent. Two strings of the language are equivalent module independence if they are identical up to swapping independent symbols. The formal definition is given in our paper [1].

[1] Pavol Cerny, Edmund M. Clarke, Thomas A. Henzinger, Arjun Radhakrishna, Leonid Ryzhyk, Roopsha Samanta, Thorsten Tarrach. From Non-preemptive to Preemptive Scheduling using Synchronization Synthesis. In CAV 2015


Usage of the library
====================

The library consists of two namespaces: \ref Limi and \ref Limi::internal. The latter namespace contains support functions that are not directly used by a consumer of the library. The library is known to compile with GCC 4.8, Clang 3.5 and VS2013. The flag `-std=c++11` must be specified on compilation. To use the library Limi it needs to be added to the include path.

The first step is to create an NFA. As is well known an automaton consists of a set of states, an alphabet of symbols, a set of initial states, a set of final states and a transition function.

States and symbols
------------------

States and symbols are represented by one class each and are specified as template arguments. Since they have to fulfil the same conditions I will only talk about states. They need to fulfil:

- The state *should* be small (either an unsigned int, a (shared) pointer or a struct with few small members).
- For the state class `std::hash<state>` *must* be implemented correctly (if it is not provided by the STL)
- For the state class `std::equal_to<state>` *must* be implemented correctly. Note that if your class provides operator= std::equal_to is automatically defined. However, be very careful with pointers as their definition of equality is pointer equality, which is not what we want. **If pointers are to be used it is crucial to implement std::equal_to**
- hash and equal_to *should* be very fast and possibly declared inline

Automata
--------

Automata inherit from the \ref Limi::automaton class (see documentary of that class to learn about the exact methods that need to be implemented). The automaton is a state-less class where all methods must be declared const.

Printers
--------

State and symbol classes are not expected to override the << operator. Instead the automaton class must be able to produce printer classes for states and symbols. The idea is that states and symbols do not hold all the information to meaningfully present them as a readable string. For example the state may be just a number and the printer holds a reference to a translation table. (See also \ref Limi::printer_base)

Language inclusion
------------------

The language inclusion algorithm is in the \ref Limi::antichain_algo class and is documented there. The algorithm accepts two automata A and B and tests if A is a subset of B. This means that for every work that A accepts there must be also an accepting run in B. If language inclusion does not hold a counter-example is produces. This is a word accepted by A, but not by B.

Language inclusion up to an independence relation is undecidable in general. Therefore this class
implements a bounded version where a stack is used to match symbols that may occur in the future.
For low stack sizes the algorithm works faster but may produce spurious counter-examples. The bound 
can be increased to eliminate those. See the main file in the timbuk example to see how to deal
with spurious counter-examples.

Epsilon transitions
-------------------

Certain transitions may be considered epsilon transitions in your automata. The reason for using epsilon transitions is not non-determinism (the automaton should not produce those if possible), but because certain transitions may have an effect for the state, but may be ignored during the language inclusion test.

The correct semantics is as follows:
Only the automaton A in the language inclusion algorithm may produce epsilon transitions. These become part of the counter-example trace. When an epsilon transition is encountered it is simply added to the counter-example chain and there is no attempt to match to any transition in the B automaton.
The B automaton must never produce epsilon transitions. This can be easily accomplished by setting collapse_epsilon to true in the constructor \ref Limi::automaton::automaton. As a matter of fact the antichain algorithm will enforce that for the B automaton either collapse_epsilon or no_epsilon_produced must be true. 

Other useful classes
--------------------

The \ref Limi::dot_printer class can print automata in the dot format (only useful if the automaton is very small). The \ref Limi::timbuk_printer can print automata in the timbuk format. \ref Limi::list_automaton is a special class that creates an automaton out of a symbol list. The automaton accepts exactly the word made out of the list of symbols. This can be useful to test if a trace is spurious (see the documentation of \ref Limi::antichain_algo).

Debug printing
--------------

The library recognises a special macro `DEBUG_PRINTING` that causes additional printing to `cout`. To activate debug printing put `#define DEBUG_PRINTING 2` before including any Limi headers. This will set the verbosity level to 2. In production code this macro should not be set (it is only for debugging the algorithm or the input to it).

Example
=======

In the timbuk folder of the repository there is an example implementation that reads automata in the timbuk format and uses Limi to decide language inclusion. The cause is commented and the timbuk format described in the readme.md file.

Other questions
===============

Can the library be used by other languages (C, Java, C#)?
---------------------------------------------------------

For Java and C# an extensive wrapper needs to be provided that represents the automata needed and then this wrapper would be called from the higher language. This involves a lot of work. It would be crucial to keep the native/managed transitions to a minimum as they are very expensive.

Is it better to use the command-line interface instead of the library
---------------------------------------------------------------------

The command line interface (timbuk example) has one major drawback: All data structures need to be expanded in advance. This is in general not a problem because the antichain algorithm would anyhow explore the whole automaton. However, in case of the independence relation this is a significant slowdown: Mostly independence can be decided directly from the symbol classes, but in the timbuk format the relation needs to be fully expanded and during the run of the algorithm every query of the relation involves an expensive lookup in a hashset.