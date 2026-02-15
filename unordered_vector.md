# the unordered vector

a continous hash "map" that has no holes or toombstons , with  fast element itteration and acceptable lookup.




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



# rehash ellision:
    hash of N bits ,  we  have   m=(N/shift_index_node) hashes for  m
     levels , but the  next one requires a new hash , this means that
     if depth is passed in the function for hash regeneration it has
     its first shift_index_node bits zero always.




# find:
       goes down the tree to find the element,  if reached it compares the keys
   then returns


# insert:
    inserts based on hash in the tree , going down and putting it in the hash
   index of the tree , if the index had a leaf with different node we turn it
   into a node with the two different childen  , O(depth-tree) = O(logn)
   Amortized assuming a good hash function with good seed use .



# erase:
   removes  a key , using the reverse index to remove its  flat parent if its
   empty ,  then each removed element(leaf) or tree node  has the back of the
   vextor go in its place , using the reverse hash to change the leaf index in
   its parent ,   O(depth-tree) = O(logn)Amortized





# refrences:

