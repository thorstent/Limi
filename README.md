Limi (Language Inclusion Modulo Independence) 
=============================================

Limi (Language Inclusion Modulo Independence) is a template library that implements the basic antichain algorithm [1] modulo an independence relation. The library takes as input two non-deterministic finite automata A and B with a shared alphabet and an independence relation *I* over the symbols of the alphabet. The independence relation determines for two symbols if they are independent. Two strings of the language are equivalent module independence if they are identical up to swapping independent symbols. The formal definition is given in our paper [2].

The usage of the library is explained in detail on the [project page](http://thorstent.github.io/Limi).

[1] Martin De Wulf, Laurent Doyen, Thomas A. Henzinger, Jean-François Raskin. Antichains: A New Algorithm for Checking Universality of Finite Automata. In CAV 2006

[2] Pavol Cerny, Edmund M. Clarke, Thomas A. Henzinger, Arjun Radhakrishna, Leonid Ryzhyk, Roopsha Samanta, Thorsten Tarrach. From Non-preemptive to Preemptive Scheduling using Synchronization Synthesis. In CAV 2015

Features
--------

* Efficient implementation of the basic antichain algorithm for NFAs
* Accepts an independence relation *I* for language inclusion testing up to *I*
* Automata are not povided explicitly, but in C++ code (by deriving from the automata base class)

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

Comparison to other tools
=========================

The website [languageinclusion.org](http://languageinclusion.org/) lists a number of language inclusion tools:
[HKC](http://perso.ens-lyon.fr/damien.pous/hknt/) is written in OCaml and it uses a technique based on bisimulation modulo congruence. Apart from language inclusion HKC can also test equivalence.
[libvata](https://github.com/ondrik/libvata) is written in C++ and can also handle tree automata.
[RABIT](http://languageinclusion.org/doku.php?id=tools) is written in Java and can also handle omega automata.

Benchmarks
----------

We know of two benchmarks involving these tools for different sets of automata.

**Neither of these two benchmarks is complete in any way.**

### Benchmark 1

For this performance comparisons we used the benchmark suite from [Lukas Holik's page](http://www.fit.vutbr.cz/~holik/pub/ARMCautomata.tar.gz). We compare the performance of the standard language inclusion question over two NFAs as this is the functionality all tools have in common. All tools but RABIT can process the timbuk language as input.

From the benchmark suite we used the `Bakery4pBinEnc-FbOneOne-Nondet-Partial` folder comparing automata 1000 to 1084
as they are sufficiently large to yield meaningful results. Each iteration of the test compares automaton `armcNFA_inclTest_n` with `armcNFA_inclTest_n+1`. For example both tools are called with `armcNFA_inclTest_1000` and `armcNFA_inclTest_1001` as arguments. In the next iteration they are called with `armcNFA_inclTest_1001` and `armcNFA_inclTest_1002` as arguments. 

We compare HKC 1.0 (switches `-bkd -incl`) and libvata (switches `-t incl`) and present the times reported by the tools themselves.

In total we tested 83 language inclusion question of which 41 were answered with yes, the rest with no.
All times are reported by the tools themselves (not the Unix time command).

|  Tool   |  Mean  | Median      | Min     |  Max     |
| --------|-------:|------------:|--------:|---------:|
|  HKC    |  1.82s |  1.84s      |  1.59s  |   2.14s  |
| libvata |  0.25s |  0.22s      |  0.10s  |   0.43s  |
|  Limi   |  0.03s |  0.01s      |  0.01s  |   0.10s  |

Therefore, we tested the Limi and libvata on the two harder instances from the benchmark set:

For `Bakery5PUnrEnc-FbOneOne-Nondet-Partial/armcNFA_inclTest_42` and `_44` libvata requires 1.32s and Limi 0.125s.
For `IBakery5PUnrEnc-FbOneOne-Nondet-Partial/armcNFA_inclTest_42` and `_44` libvata requires 5.65s and Limi 3.04s.
`IBakery5PUnrEnc-FbOneOne-Nondet-Partial/armcNFA_inclTest_44` has 6000 states and 70000 transitions.

### Benchmark 2

This benchmark was conducted by Richard Mayr. It uses automata that require large antichains to solve.

| Benchmark | HKC  | libvata | Limi  | RABIT |
|-----------|-----:|--------:|------:|------:|
| t0 ⊆ t1   | 5.2s | 429s    | 2606s | 23.3s |
| t3 ⊆ t2   | 5.7s | 444s    | 1146s | 22.8s |
| t4 ⊆ t5   | 2.7s | 236s    | 1851s | 27.2s |
| t5 ⊆ t4   | 2.9s | 309s    | 1006s | 24.1s |
| t7 ⊆ t6   | 2.8s | 300s    |  777s | 23.6s |
| t9 ⊆ t8   | 4.3s | 396s    | 1786s | 27.4s |


