# The MCC compiler execution scheduler:


## abstract :
in this document i specify the reason and inner workings of my scheduler,
to give a Turing complete SEMA deterministic execution.


# THE RULE:
for each external or internal dependency D in phase Q of a node N in phase P , the D not depend on N , or if it does , the phase Q must be lower than P, otherwise the program is ill-formed.

 * note:
 file read makes a dependency of that file , file write makes the file the dependent.



#  the foundation:

 1. Kahn's Algorithm extended:
  for any node N with  array of Ds as dependencies , its in degree is :
  `in_deg = |Ds| - actives`.
  to decrement it we increase `actives`.

 2. Phase space `S=(C,T)`:
  for any node N with  array of Ds as dependencies,  the N's Phase space S's C (`current`) must be equal or lower than all D's `C`es.
  `|actives|=  for all Ds count_of( N.T <= D.C )`. 


##  state space 'finite' state machine:

 * note:
 `OMEGA` is a state where reaching it or going past it is equivalent, it is bigger than all phases states available .  

 1. errors :
 violation of a rule in the graph with make `N.s == (OMEGA,T)` where `T < OMEGA `.

 2. complete:
`N.s == (OMEGA,OMEGA)` is the complete state. 

 3. triggered:
 `S=(C,T)` where `T<=C` ,  to reduce edge cases my implementation makes T equal to C to signify that T is no longer the trigger.

 4. asleep:
 `S=(C,T)` where `C<T<OMEGA`.



 # algorithm:

 1. `asleep -> triggered` happens when in degree reaches zero:
 event E is fired, E belongs to wave W2, we assert that E only depends on events who's wave W1 is less than W2. 
 see the implementation.

 2. `triggered -> (triggered|asleep|complete)` happens when calling `defuse`:
 updates the node to match the invariants, if any dependent in degree reaches 0 they get triggered in next wave.  
 if our in degree was still zero we re trigger in next wave.
 if we are complete or asleep we don't re trigger ourselves.
 
 
 3. `any -> error` :
 invariant was broken.



## practical use case of `S=(C,T)`:

- given the code :

```txt

[:meta_expantion:]

let a=b;

```

for name lookup to complete in the node `b`, `meta_expantion` must be evaluated,

`b`'s trigger however is `name_lookup` , but we can do something ,

let b depend on `[:meta_expantion:]` and let `[::]` sat that the trigger T is `code_gen` , but current C is `none`,

now b depends on `meta_expantion` to execute , and b will never lookup prematurely.





lookated at :
https://github.com/Mjz86/String/blob/main/mjz_lib/graph/scheduler.md

references:


https://github.com/Mjz86/String/blob/main/mjz_lib/graph/deps.hpp


https://www.geeksforgeeks.org/dsa/topological-sorting-indegree-based-solution/
