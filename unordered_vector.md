# the unordered vector

a continous hash "map" that has no holes or toombstons , with  fast element iteration and acceptable lookup.




# definitions:

 - a reverse index :
  if we have `vec[i] = j;`
  then ,  `rev[j] =i;`


 - reverse index in tree:
  but because our `vec[i]` is placed  in a flat_tree it  is `flat_tree[k][r]` , i
  being `(k<<shift_index_node)+r`


 -  flat_tree node indexes are:
  0. the value 0 is null child.
  1. positive values P correspond to leaf node values[P-1]
  2. negative values N correspond to child node flat_tree[~N]


- members:
```c++
  using childern_node_t = std::array<intlen_t, (1ull << (  uintlen_t(1)<< branching_power_v ) )>;

  std::vector<childern_node_t> m_flat_tree{};
  std::vector<value_t> m_values{};
  std::vector<key_t> m_keys{};
  std::vector<intlen_t> m_reverse_tree_indexies{};
  std::vector<intlen_t> m_reverse_value_indexies{};
  hash_fn_t hash_fn{};
```



-  rehash ellision:
 
      hash of N bits ,  we  have   m=(N/shift_index_node) hashes for  m
     levels , but the  next one requires a new hash , this means that
     if depth is passed in the function for hash regeneration it has
     its first shift_index_node bits zero always.




- find:
       goes down the tree to find the element,  if reached it compares the keys
   then returns ,O(depth-tree) = O(logn)Amortized


- insert:
    inserts based on hash in the tree , going down and putting it in the hash
   index of the tree , if the index had a leaf with different node we turn it
   into a node with the two different childen  , O(depth-tree) = O(logn)
   Amortized assuming a good hash function with good seed use .



- erase:
   removes  a key , using the reverse index to remove its  flat parent if its
   empty ,  then each removed element(leaf) or tree node  has the back of the
   vextor go in its place , using the reverse hash to change the leaf index in
   its parent ,   O(depth-tree) = O(logn)Amortized




- my   benchmark on potato pc:

clang version 21.1.1 (https://github.com/llvm/llvm-project 5a86dc996c26299de63effc927075dcbfb924167)
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /home/mjz/LLVM-21.1.1-Linux-X64/bin

/String/tests$ ~/LLVM-21.1.1-Linux-X64/bin/clang++ -std=c++26 -O3   -g -fprebuilt-module-path=build  -fconstexpr-steps=55555555 -fexperimental-library  -march=native   -Werror -Wall -Wextra  -Walloca -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdeprecated-copy-dtor -Wdouble-promotion -Wenum-conversion -Wfloat-equal -Wformat-signedness -Wformat=2 -Wmismatched-tags  -Wmultichar -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wrange-loop-construct -Wshadow -Wuninitialized -Wvla -Wwrite-strings -Wsign-conversion -Wdelete-non-virtual-dtor  -ferror-limit=10        -stdlib=libstdc++    -lstdc++    -o  uvec uvec.cpp

/String/tests$ ./uvec
[baseline:4480900ns]
[insert:321527297.00000017ns]
[find_good:142235100.00000007ns]
[erase:115435700.00000006ns]
[find_mix:119566699.00000006ns]
[find_bad:60268000.00000003ns]
[iterate_load:1099800ns]
[std_insert:475399299.00000025ns]
[std_find_good:187802500.0000001ns]
[std_erase:142239100.00000007ns]
[std_find_mix:159960399.00000008ns]
[std_find_bad:89687900.00000005ns]
[std_iterate_load:38746900.00000002ns]


# refrences:

benchmark code:
https://github.com/Mjz86/String/blob/main/tests/uvec.cpp

my impl :
https://github.com/Mjz86/String/blob/main/mjz_lib/unordered_vector.hpp


https://en.cppreference.com/w/cpp/container/unordered_map

https://en.cppreference.com/w/cpp/container/flat_map
