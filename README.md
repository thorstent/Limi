Limi (Language Inclusion Modulo Independence) 
=============================================

Limi (Language Inclusion Modulo Independence) is a template library that implements the basic antichain algorithm modulo an independence relation. The library takes as input two non-deterministic finite automata A and B with a shared alphabet and an independence relation I over the symbols of the alphabet. The independence relation determines for two symbols if they are independent. Two strings of the language are equivalent module independence if they are identical up to swapping independent symbols. The formal definition is given in our paper [1].

The usage of the library is explained in detail on the [project page](http://thorstent.github.io/Limi).

[1] Pavol Cerny, Edmund M. Clarke, Thomas A. Henzinger, Arjun Radhakrishna, Leonid Ryzhyk, Roopsha Samanta, Thorsten Tarrach. From Non-preemptive to Preemptive Scheduling using Synchronization Synthesis. In CAV 2015

Example: Timbuk
---------------

This example takes files in a modified version of the [Timbuk](http://www.irisa.fr/celtique/genet/timbuk/) format. The executable can be compiled using the Makefile and will be built in `build/buildr`. The only two arguments are the paths to the two automata that should be checked for language inclusion.

The main purpose of this example is to illustrate the usage of the Limi library. The code is documented to explain how Limi is used.

The input format is modified in two ways: 1) The arity of symbols must be either 0 or 1 (we do not support tree automata). Arity 0 indicates the outgoing transitions from the implicit (unnamed) initial state. 2) We introduced the optional Independence list, a list of symbols that are independent. The list taken to be symmetric, so there is no need to both specify (a b) and (b a). But it is not automatically made transitive.

Flex and Bison are optional dependencies. They are not required for the project to compile because the generated files are committed as part of this GIT repository. However, if the grammar is changed the files will not be regenerated without Flex and Bison installed.

The modified Timbuk input format (presentation taken from [here](http://www.fit.vutbr.cz/research/groups/verifit/tools/libvata/)):

	<file>             : 'Ops' <label_list> <automaton>
	
	<label_list>       : <label_decl> <label_decl> ... // a list of label declarations
	
	<label_decl>       : string ':' int // a symbol declaration (the name and the arity (0 or 1))
	
	<automaton>        : 'Automaton' string ['Independence' <independence_list>] 'States' <state_list> 'Final States' <state_list> 'Transitions' <transition_list>
	
	<independence_list>: '(' string string ')' // states that these two symbols are independent
	
	<state_list>       : <state> <state> ... // a list of states
	
	<state>            : string // the name of a state
	
	<transition_list>  : <transition> <transition> ... // a list of transitions
	
	<transition>       : <label> '(' <state> ',' <state> ',' ... ')' '->' <state> // a transition
	
	<label>            : string // the name of a label

An example is here:

	Ops a:0 b:1 c:1
	
	Automaton A
	Independence (a b)
	States q0 q1 q2
	Final States q2 
	Transitions
	a() -> q0
	b(q0) -> q1
	c(q1) -> q1
	c(q1) -> q2
	c(q2) -> q2

## Compiling on Windows

To compile on windows you need to install CMake and add it to the PATH (option during install). You also need the VS2013 C++ compiler. Then open the VS2013 x86 Native Tools command prompt and navigate to the folder where Limi is checked out and issue these commands:

	mkdir build
	cd build
	cmake -G "NMake Makefiles" ..\timbuk
	nmake

This results in `timbuk.exe`.