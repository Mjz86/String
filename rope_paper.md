# Considering Fragmentation: A Custom Rope Implementation

## Abstract:

This paper presents a custom rope data structure based on a semi-immutable string and a lazy generator. It aims to reduce fragmentation while maintaining sufficient memory efficiency. The rope is designed to be fully usable in a `constexpr` context, although not generally as a `constexpr` variable. (For simplicity, this paper primarily discusses 64-bit little-endian architectures, but the concepts are applicable to others.)

## Introduction:

While ropes may not be a common tool for most developers, they are essential in specific applications. This paper documents the design and considerations behind my rope implementation, even though it's still a work in progress. The goal is to provide a useful reference.

## Implementation Details:

The following conceptual code illustrates the core ideas. It's simplified for clarity:

```c++

struct elem_meta_t/* conceptual type   */{
// used to search our way to the data in a cache friendly way, also , this only needs B/8 cache lines  and the searching would be easy mostly,  just a linear or binary search , although  linear search is showed to be more performant .
 // most significant bits 
 
 // we can prove that ( numbers with  decimal point , M , K , N being non negative integrers ) :
 // `N.99999999< K.x `  if  and only if `N< K `  and `K.x < M`  if  and only if `K < M`  
 // so  , we know  that if we think of the most significant digits as the integer component,  and past that as the non integer component,  we could  do comparisons on the data , without  doing an and operation. 
// both of thoes proofs correspond to each  of the search strategies:
 // for searching  for an index's  logical elements, we  do  `temp=(index<<8 )| 0xFF` or `temp=(index<<8 )` ( 0xff being like .9999 )  , 
 // then we search for the last  elem_meta  that holds  true for "temp<elem_meta" or "elem_meta<temp"( theres no need to etract the bit fields ) ,
 // this search  can be preformed by a simd comparison operation , after we got the simd mask ,  we get the last  bit's index that was either one or zero for the search that we wanted  , because  B is less than 65 , we can put all the masks in a 64-bit unsigned integer  , then we can use std::count(r/l)_(zero/one) ( botwise not , std::experimental::find_first_set, std::experimental::find_last_set if using std::experimental::simd) to get the first position .
 // so , therfore , the search and compare operation can be performed  in about B/8+1 instructions ( note that the bits that correspond to non active elements  would  be removed by bit shifts ) .
 // this is also a huge win because  a lot of access to the node first needs to search for indexies. 

 
 size_t index_of_end : 56;
 // enum is not usable because this is a bit field
// because the sso buffer is a trivial type ,the destructed state for the children object is the sso state with length of 0 ( the index_of_end is the same as the previous index_of_end )
size_t type : 2;
// each of these corresponds to one in children_storage , at first , the indexes are like views::iota , but when N elements are removed from the middle,  everything after it shifts left  by N and the removed N elements are placed at the end in an unspecified order, this algorithm is achieved by 2 std::ranges::reverse operations, and as a result these insexies are always  unique and between 0 and B ,because this is the map of logical to physical indexes. 
  // the reason for the storage being mapped to indexies in non linear order is that  the lazy objects need a virtual call for object movement,  and accessing all  B cache lines is cumbersome,  so , the indexies are moved around and stuff to simulate object movement, because  std::ranges::reverse  often uses vectorized instructions,  and because this region is at the beginning  of the node ( aligned to the max node alignment,  likely 64) , and because it only needs at most 7 cache lines , then its very easy to move these elements around.
 // even if std::ranges::reverse doesnt use simd , the avoidance of virtual calls and many cache accesses is still a huge win , especially considering that the meta data is already loaded in cache when we did our linear search.
 // note that std::ranges::reverse is used for all of the elem_meta_t, not just the 6 bits , this does 2 things at once,  first : each physical index remains unique,  second,  the implace_vector operations essentially are just insert and delete in the children.
// least significant bits 
 size_t physical_index:6;
};
// important node is that we *cannot* make the node size dynamic,  because that would mess up the data layout in many different ways , and would significantly hurt simd potential and bit feild layouts.
struct node  {
// the reason for not using the elem_meta_t directly is because the MSVC ( and presumably other compilers )  std::ranges::reverse  only uses simd for the basic integral or pointer types , but the data can be used by std::bit_cast .
 /*elem_meta_t*/  size_t elem_meta[/* logical indexies*/8*ceil(B/8.0) /* to ensure that no padding is between this , and the children_storage, this , and the alignment helps make this a better simd optimized array*/ ];// implicitly aligned by alignas(sizeof(elem)) 
 alignas(sizeof(elem))  children_storage[/* physical indexies*/B];// all the algorithms work with logical indexes,  physical indexes are not much relevant, this can be thought of as a pre allocated region,  the when we do the double-reverse operations,  we allocate or deallocate the selected elements,  but the allocation and deallocation are not really from memory,  but from the empty elements , we dont really care that these are not in linear order , because  all of them are in this node , and each of them is one cache line , so theres really not much benefit , especially considering how much mutation operations move these around .
// To avoid trashing cache lines, each node has a padded member that holds
// the reference count, the allocator  can be placed here because its not accessed until destruction,  and that means it can be shared. 
  alignas(std::hardware_destructive_interference_size) size_t reference_count;
  bool is_threaded;
  allocator node_alloc;
};

struct node_ref {
  node *object;
  size_t  offset; // Easier substringing could be achieved with an offset, but it's
  // avoided due to potential fragmentation, 
//  it would be beneficial to support it tho , even if the normal substring algorithm doesn't use it , because  sometimes a fast_substring may be needed,  and its veey cheap,  because all the data to make the string shrink is in the root , and node traversal is not performed. 
// the Drawbacks of this speed in substring is going to be reflected in the n<=m assumptions,  because in the fast case, this may not hold ,
// n<=m isnt an invariant , but a larger m means a larger h , and therfore,  losses in the tree access time , 
// but in the case where the tree would not be concatenated again after the fast substring , it might be better not to touch the nodes .
// a good approach might be to use the fast option if  2^(ceil_log2(b)×h+6) <n , and if not , rebalance the tree,  this would cap the fragmentation , while also help increase rope sharing and substring speed.
  size_t length;
  size_t elem_count;// a std::span<elem_index_map_t> can be made by this and the object pointer.
  size_t tree_height; // Helps in the concatenation algorithm to identify the
                     // tree depth at which concatenation should occur.
};

// This struct is not padded because the SSO buffer is the largest member.
union elem {
    char sso_buffer[64];
    mjz::string string;
    Lazy lazy;
    node_ref node_ref;
  };


// Similar to std::any, but the vtable includes a subrange iteration
// function that calls a slice view functor and an allocator reference
// alongside the "void*".  Effectively:
// `void iterate (*)( const Lazy&obj, size_t real_offset,size_t real_length,void(*)(void* state ,string_view slice) callback,void* state);`
// A trick with an empty base class `void_struct_t` emulates the  "`void*`"
struct Lazy {
 //size_t length;// can be derived by index_of_end information.
  size_t offset;// only stored , added to the virtual offset to get the real_offset
 struct lazy_info {
const lazy_Vtable *vtable;
  union lazy_storage; // Size of 48bytes of inline storage for generators
  };
};

struct root {
  elem head;
  size_t length: 56;
 size_t encoding : 5;
  size_t is_threaded : 1;
  size_t type : 2;// enum is not usable because this is a bit field
  allocator alloc;
};
```

# thread-safety:
the thread-safety grantees of the whole tree is achieved when all the nodes are using thread-safe allocators ( the `is_threaded` property is more of an indicator for the thread-safety of the allocator resource , if all of them are true , then the rope is thread-safe on all const operations ,but if mutation operations are involved,  then synchronization would be necessary.)
the most notable example of why thread-safety is important in the rope is for asynchronous file reads and writes, 
a  rope that  is written to the file asynchronously.
and , the partial copy of the rope can still be modified in another thread.

* effectively:

 a single rope object ( having  same address ) should not be modified in parallel.
 
 any constant rope object can be used on any threads  if the thread-safety flags are all true.
 
a single rope object cannot be both modified and read from.
 
but two distinct rope objects (having different addresses) can , if the thread-safety flags in both of them are all true.

## Fragmentation:

The degree of fragmentation in the rope is influenced by the randomness of user access patterns. If a user modifies a position `i`, and another modification occurs at `j` within a distance of at least 56, then `i` and `j` likely reside in the same block. Initially, the rope has minimal fragmentation (m < 2). Even if the rope is initialized with a gigabyte of data, extensive modifications across almost the entire dataset would be required to reach the worst-case fragmentation of m = n/64. This is a demanding task, even for continuous arrays. Rope users are unlikely to modify an entire gigabyte. However, `for_range` can reduce fragmentation back to 1. Furthermore, `for_range` can be used to reduce fragmentation of a specific range before iteration, creating a continuous slice without modifying the data directly.

the fragmentation of the rope is measured by m,
and the approximate maximum for it is ` 2^(h×ceil_log2(b))` .

by using fast substring,  the ratio of fragmentation , `2^(h×ceil_log2(b)) /n ` will increase , because h was unchanged but n decreased
but , on the other hand ,all the data is shared, so we didn't really allocate anything in return.


### effects of node combining:

the sso buffer is a non null terminated ( the length information is in the `index_of_end`) 64 byte ( most likely a cache line , because of the alignment and size)

the operation is preformed in this order:

if a non sso node has length less than or equal to size if the sso buffer,  then the node would be transformed into a sso node .

for every ajason children from left to right:

if two sso children have combined length  less than or equal to size of   sso buffer , then they will be combined into a single SSO node. 
else if two sso children are ajason to each other,  the data in the right one would be shifted to the left one until the left one is full ,(  because, of the previous check , this would result in a full sso node at the left , and a non empty right sso node) (len left=64 bytes) (1 memcpy,  1 memmove) 
else if two children are ajason to each other and the combined length of them is less than or equal to size of  sso buffer times 2, then they will be turned into sso 2 sso children , the left one being full.anr the right one being non empty.
else do nothing. 

( this is also removing nodes , and  it might look like std remove if , while doing so)

this is preformed   `elem_count-1` times , in a scanning swipe ,
( note that  because we know that the node was in a combined state before  any mutation operations , we can save on checking the operations  that we know would do nothing)

and it packs all of the data to the left part , 
the only time data is not packed happens in the condition that the combined length of two children  exceeds 64×2=128 bytes , and in that case , its already low fragmentation  ( the non sso node would be at least  65bytes , if the left is full , and at least  127bytes if the left has only one byte).

with this way of packing the data , we know  that a node  has at least  (a-1)×64+1 bytes ,  so m would  be less than n/63 in the worse case of all sso children. 

this is still an experimental algorithm,  and there might be better ways or threasholds, 

one way to solve this issue is to check for the condition  that if the node has total length,  less than B×64 , after the operation,  then it would collapse  into a single  continuous  `mjz::string`,  for this reason,  the root inline leaf  can be modified and used up to B×64×2 bytes , and any data less than that wouldn't use the tree structure,
because  of this , if we attempt to modify  a generator leaf  root inline leaf , then if the resulting string  dose not exceed B×64×2 , then the generator would collapse into a single continuous `mjz::string` in that operation.

because of this , we know that each non leaf child has at least B×64+1 bytes , and because  the leafs can only be found at the bottom of the tree , we know that each node contains at least  a^(h-1)×B×64 bytes ( h starts at 1 , this doesn't apply to leaves with h=0)

so , this might be a very effective way of reducing fragmentation,  without this , the grantee would be at least  a^(h-1)×(a-1)×64 bytes (h starts at 1 , this doesn't apply to leaves with h=0),
which is still good , but it doesn't apply to the root , and its not very good to have half full stuff , so this isn't choosen .

also , the benefit of collapses into `mjz::string` is that this string type is highly optimized for continuous storage,  and has the power of cow,sso , and substring sharing , and although the only useful part in the context of ropes is substring sharing , the other parts help in making it a go to option.

this operation is always reduces fragmentation and may reduce tree hight,  because the same information is now stored in half the space as before , and its trivial to work with. 

because of this , most of the time ,  indexes near each other result in the same cache line.


as the algorithm evolves , there might be ways of combining that are helpful,   but 


this effectively Defragments all of the node ( not all of the tree) 
so that all the ajason sso sections are full 


### common operations and the corresponding optimizations:

  
 - Defragmentation algorithms:
 all the operations get  speed up from m reducing , so this is a huge win.
 
- copy-on-write (COW) and substring sharing:
  substring sharing both in nodes and leaves  is essential for the low time complexity .
 
 - small slice optimization(SSO):
  drastically reduces TLB misses when accessing a node .
  drastically reduces allocations
  works really well in small slices scattered over large continuous sections, and also is very cheap to combine these , especially because both memcpy, memmove and std::ranges::reverse  are often simd optimized. 
 
 - physical to logical index mapping:
  this is very efficient in reducing the number of cache lines being accessed  , and it reduces them by a factor of 8 ,
  it also eliminates any function calls that might have been used for object moment.
  the final touch is that with the help of fast range algorithms,  this is simd optimized and might do the work of 8 functions calls in one simd instruction , drastically reducing the constant factors for mutation operations on the rope.

- the range functions:

these are both user-friendly,  and help reduce m , therfore increasing the overall structure performance 

 - cached Iteration :
 
 avoids multiple tree traversals , and is also very cache friendly,  because the iterator cache hits are almost always real cache hits , and the cost if the misses is amortized among the hits , just like how most cpus like working with data.

- node and root collapse:

from the previous paper on `mjz::string`, ive shown how much more performant continuous storage can be ,  this builds opon that , and tries to achieve as much continuous storage as possible,  without  increasing the time complexity or reducing performance,  in the case of B=56 for example,  the continuous  string in the root node would be at most 2 × 4killo byte pages ( or 3 , partially used pages)  ,  working with 2 pages is relatively quick for the cpu , because all of it can be fit into L1 cache in most places,  so , we dont really lose much  with putting a threashold,  and the wins are very good, 
because almost all modorn cpus are optimized for continuous storage.

- simd search  instead of binary search  and bit feilds:
instead of using bit feilds , by shifting the data manually and adjusting the layout,  the search algorithm can be performed with minimal overhead in a linear search .
the index search is a crucial factor in constant time operations,  by using simd , this factor is reduced drastically. 

- lazy generators :
while moving or copying  these objects can be annoying,  they help the rope to better optimize for many use cases .

## Invariants:
( the limit on B is beacuse  of the bit fields,  but B=56 , making a 4kb =1 physical page node is good enough for a limit)
* also  `2<2a<B<65`.

The index of the string end serves as the key to the tree structure. All (a,b)-tree invariants must hold. Additionally, the following optimization invariants are enforced:

- If two adjacent children have a combined size smaller than the SSO buffer, they must be combined into a single SSO leaf.
- If there is only one leaf, the tree height is 0, and no node exists.
- Leaves (except SSO leaves and the root inline leaf) cannot be directly modified (i.e., characters cannot be changed or appended via direct buffer access). However, substring operations are permitted.

### B?

 the choice of B is going to effect:
 
 - node size  :
 
  more children increase node size, 
  each child adds 64+8bytes (padding not considered , but each node has max of 56×2bytes of padding and min of 56)


- sso potential :

 more children  means that more  ajason children can be combined ,
 and considering the 64 sso for each child ,  the more the  inline children,  the fragmented spread the data will be ,
 and less L3 cache misses and TLB misses ( translation  lookup table for virtual addresses to physical addresses is usually in 4kb chunks,  so , almost all the sso node data , would have a single physical place)
 
 - hight :
  the hight would decrease by -log(multipier) 
 
 - balancing constant  :
   as each node gets bigger , more stuff would need to be moved around for node balancing ,
  and lazy nodes are very annoying in this regard,  because they cant be moved by memmove, 
  this is because the lazy nodes have virtual move and copy functions that move them from one lazy sso to the other.
   but this is not generally a problem because trivial generators could use the indicator that they are trivially copyable.
   and in general,  operations on arrays tend to be fast
   
   
 - Allocator performance :
 
 allocators might be better for particular sizes.
 if nodes are too large,  they may not fit in a single page .
 also , bigger nodes will increase memory consumption  when the rope root node exeds the sso requirement. 

## definition of substring and concatenate:

- note that a node copy is just sharing it , its O(1).
- note : if at any point two ajason children or a node had less size than its corresponding sso , we collapse them O(B)+O(iteration(B))+O(1+)=O(B+)=O(1+).

### node destroy definition :

we ( either unsharing or dealocating) the unused children O(1+).

### substring:

- case of no length: destroy the whole rope , set to sso.
- case of a leaf : we know that sharing substings is O(1) , an SSO memmove is O(B)=O(1) , a lazy substring is a copy of generator and offset and length change O(1)+O(gen-cpy)=O(1) because generators should be cheap to copy.
- case of a node:
  we search for the new begin and end pair , and we find the range of children that the substring needs. O(logB)=O(1).
  - if there was only one child left , we correct our offset, we make that child the new root and destroy the old root and do a substring on the new root O(h-1 +).
  - if there was more :
    we ownerize the current node O(1)( either a reference check or an unshare-destroy copy).
    we destroy each unused node O(1+).
    we reposition the nodes and their keys ( the indexes may need to change) O(B)=O(1).
    we ownerize the first and last child O(1).( if these children's length is not changed, we do not ownerize).
    we make sure that the first and last child have more than A children ( not A itself) ( merge or steal , or collapse to root) O(1).
    we do a a substring on the first and last children with the right index adjustment( if the length is unchanged we do nothing ) O(h-1).

### concatenation of two nodes O(new_h):

rhs=right hand side.
lhs=left hand side.
ohs= opposite hand side.
chs=current hand side.

- case of two roots with same hight:
  merge these (hight may increase). O(1).
- else:
  choose the taller rope as chs.
  make sure chs has less nodes than B ( maybe by split and increasing the hight ) and ownerize it O(B)=O(1).
  lets call the ohs child of chs , C.
- if ohs has same hight as C :
  we insert ohs at the position of C child of chs O(B)=O(1).
- else ( we know it has less hight):
  make sure C has less nodes than B ( by split ) and ownerise it O(B)=O(1).
  we concatenate C( C is going to change ) with ohs in the correct order. O(new_h-1).

## Results of Said Properties:

String operations can be performed using substringing, concatenation, and creation. This rope implementation achieves the property that its big O complexity is independent of the string length for operations other than iterating.

- `h` is the height of the tree. This represents the number of levels from the root to a leaf. Since it's a balanced (a,b)-tree, h is O(log_a(m)).
- `m` is the number of slices in the rope. This is _not_ the same as the number of characters. The design prioritizes keeping 'm' much smaller than 'n'.
- `n` is the number of characters in the rope.
- `k` is the iteration length/number of chunks iterated over.

Given the string constraints, the height `h` is  bounded by a small constant (e.g., 50) ( unless `fast_substring` is used without care), and in practice, `h` is often less than 4. Therefore, O(h) ≈ O(1).
 
 ( heres an overview: `h=log_a(m=n/64=2^(56-6))=50*log_a(2) `, `a` being typically more than 2, so  `h<50` .,  for example  if we want a node to as big as a page , the maximum  `b` is 56 , so maximum `a` is 27 and the maximum `h` would be 11, and that is impractical , in practice, h is mostly less than 4 for this case)

The (+) indicates the amortized cost of deallocating unused tree segments, with a worst-case cost of O(m). However, this cost is paid during allocation.

Approximate Time Complexity for various operations:

- Construction:

  - From empty: O(1) (Create a root node with an empty SSO string.)
  - From copy: O(1) (Shallow copy of the tree structure, thanks to COW.)
  - From generator: O(1) (Wrap the generator in a root node. The generation process is lazy.)
  - From `mjz::string`: O(1) (Wrap the string in a root node; sharing is O(1).)

- Destruction: O(1+) (Amortized constant time). Most of the time, destruction is just decrementing reference counts. But, occasionally, we need to reclaim memory.

- Concatenation: O(h) ≈ O(1) (Rebalancing the tree after joining two ropes. Because h is almost constant)

- Substring: O(h+) ≈ O(1+) (Mostly constant time due to the balanced tree and amortized rebalancing. We are basically finding the start and end slices and decrementing unused ones )

- Insertion/Deletion of a Rope Segment: O(h1 + h2 +) ≈ O(1+) (Similar to concatenation, mostly constant time to find the insertion point)

- Indexing (Accessing a Character by Index): O(h) ≈ O(1) (Walking down the tree to find the correct slice). This is a logarithmic operation but h is a relatively small constant.

- Iteration: (Important note: ALL iterators are `const` due to COW limitations in my string. But I have `for_range` and other functions to allow mutations, similar to how my `mjz::string` works.)

  - Simple Iterators (index and object): O(h×k) ≈ O(k) These iterators are lightweight but less efficient because they need to traverse the tree for each character. But in reality, h is constant.
  - Specialized Iterators (for `std::ranges`): O(h+k) They cache the nearby area `[i, i+h*g)` (G is an optional parameter for iteration creation and g is a value that is G unless bounds are not enough, the default value for G would be a  template pram) (if in bounds, if not, the storage is shrinked or shifted ). The previous cached index `i` is stored to compare and see if the current index is in the cache. If not, the cache is updated. If the iteration is smooth, then we save many lazy calls and cache misses, because the cache size is a multiple  of h , if we do a smooth iteration with a optional g value,  the number of access would be `k*c/(g*h)` ( c is a semi-constant  factor determining the randomness of access)  which if we multiply with `O(h)` for single access  , it would be `O(h*k*c/(g*h))=O((c/g)*k)=O(k)`

- `for_each_slice` (Function called for each slice - doesn't change the rope): O(k+h) .


- `for_range` (Mutable Reference - CAN change the rope): O(h+k) time, Memory O(k). This is the important one for in-place modifications. It create a continuous `mjz` string and apply the function, then it inserts the new string to the list of strings.

 note : string  given from `for_range` is passed with two additional arguments `offset` and `length` ,  these arguments show where the actual range that we choose is , this is because  `for_range`  may be used on an already continuous region, and for this reason  , the continuous region in given fully , and the offset is specified for users to know what to do , note that all of the continuous region may be modified  , but the only granteed available region  is the range input requested  by `for_range`.
 

  More explanation: This makes each of the characters go first to a continuous `mjz` string buffer with reserved size of k, then calls the function, then inserts that buffer into the appropriate position, potentially reducing fragmentation for free. This also allows the API to provide a continuous mutable string to the function, improving its performance and user experience. Use with caution: if you have a one-gigabyte file and use this on all of it, you need 2 gigabytes of memory in the middle of the function (buffer + rope), but the end result would be a more continuous rope, reducing fragmentation. It's a trade-off. However, this isn't usually a problem because users would typically only modify small sections (= small k). Also, a continuous mutable string is easier to work with, and the user can read and modify batches together into a nice continuous chuck, while being easy to use and arguably faster over the long run. This also reduces fragmentation after the operation has completed, therefore, it's both a user-friendly and cache-friendly thing.
 also: the state of the rope is unspecified  once the string is in the `for_range`  call back , this is because  some optimizations may be possible  that are not possible in the `pop_range`  case .

- `pop_range` ( mut operation) :  makes the specified segmentat continuous  as if by calling `for_range` , then  returning the section,  while also not inserting back the data.
  note : the temporary returned by this function  shares less  data with the string in the rope via substring sharing , because unlike  `get_mut_range`,  this changes the ropes data representation. 

- `get_mut_range` ( mut operations,  use this insted if *this is the only accessed reference to the rope object) : return a  string , that is as if it was the temporary string in `for_range` , and as if `for_range`  was performed but the content was unchanged.
 note : the temporary returned by this function  shares the data with the string in the rope via substring sharing , and this operation  does not change the data represented by the rope , but reorders it , so its effects are like `get_const_range` , while also being  a logical mutation operation.


- `get_const_range` ( const operations, only use when thread-safety is necessary, and mutations cause race conditions) : return a  string , that is as if it was the temporary string in `for_range`, but the rope is unchanged , ( if the user doesn't want to use lambdas , or has an asynchronous function , then using this and insert would be practically equivalent)
 note: this is the least optimal way to call `for_range` , because  the previous  tree memory could not be reclaimed at all , and for cases of almost continuous strings that would only need some few push back or push fronts to be usable , this would need to copy it all , and wouldn't be able to share it out without modification. 



## Benefits and Trade-offs:

### Benefits:

- **Reduced Fragmentation:** Node combining and copy-on-write strategies help minimize memory fragmentation, especially with frequent modifications to large strings. In most cases, the hot sections are SSO buffers, which simplify mutations.

- **Memory Efficiency:** Copy-on-write semantics and small string optimization reduce memory consumption by sharing data and avoiding unnecessary copies.

- **Efficient Substring / concatenate  Operations:** Substring operations are efficient because they create a new rope that shares the underlying data with the original rope. The main string shares substrings, which enables more efficient substringing for the rope too. the concatenations are often just  insertions of a tree head into another tree or sub tree as a node .

- **Lazy Evaluation:** Lazy generators defer the cost of string generation, improving performance when the string data is not immediately needed.

- **`constexpr`-Friendly Design:** The implementation aims to be `constexpr`-friendly, enabling compile-time string manipulation where possible. Custom allocators support this goal.

- **Future Unicode Support:** Support will follow when the main string implementation supports Unicode.

### Trade-offs:

- **Constant-Time Overhead:** There is constant-time overhead associated with accessing and manipulating the rope due to tree traversal and COW management.

- **Mutable Iteration Limitations:** The COW implementation makes mutable iteration complex, but `for_range` addresses this.

## Usability:

A good approach for implementing undo/redo functionality is to maintain a vector of ropes.
Each modification can be stored as a new rope at the end of the vector.
Thanks to COW, this is manageable.  
For synchronization in file operations, a COW copy of a rope can be passed to be written to a file, or a lazily evaluated immutable view can be obtained for a constant file.
Data can be generated on-the-fly. For example,
if the value is hard to compute (e.g., decoding a message encrypted with AES),
a generator with a mutex and a mutable sub-rope initialized with an internal generator as storage can be used.
The sub-rope is materialized using `for_crange` (basically `for_range` but provides a constant reference rather than a mutable one, which is more efficient in some cases and also reduces fragmentation) every time a subrange is needed.  
This ensures that the algorithm only runs once per block, saving many decodings.
The rope API is designed to be similar to the main string API, without assuming continuous representations.
The rope does not need to be a tree for small strings.  

## Conclusion

This paper has presented a custom rope implementation designed to address the challenges of manipulating large strings efficiently. By combining a balanced (a,b)-tree, copy-on-write semantics, small string optimization, and lazy evaluation, this implementation aims to minimize memory fragmentation, reduce memory consumption, and provide efficient substring operations.

## Note


While the implementation is still under development and not yet open source, I would appreciate your feedback. Also, sorry if the markdown is hard to read. This paper is located at:

[https://github.com/Mjz86/String/blob/main/rope_paper.md](https://github.com/Mjz86/String/blob/main/rope_paper.md)

You may give feedback in:

[https://github.com/Mjz86/String/issues](https://github.com/Mjz86/String/issues)

## References and Inspirations:

My continuous string:

[https://github.com/Mjz86/String/blob/main/README.md](https://github.com/Mjz86/String/blob/main/README.md)

Learn about (a,b) trees and b trees :

[https://youtube.com/watch?v=lifFgyB77zc&si=IPNlqVVdr0nU-n_G](https://youtube.com/watch?v=lifFgyB77zc&si=IPNlqVVdr0nU-n_G)

[https://www.youtube.com/watch?v=K1a2Bk8NrYQ](https://www.youtube.com/watch?v=K1a2Bk8NrYQ)

Learn about the cache:

[https://www.youtube.com/watch?v=dFIqNZ8VbRY](https://www.youtube.com/watch?v=dFIqNZ8VbRY)

Lazy evaluation |Ruby Conference 2007 Ropes： An Alternative to Ruby's Strings by Eric Ivancich :

[https://www.youtube.com/watch?v=5Xt6qN269Uo](https://www.youtube.com/watch?v=5Xt6qN269Uo)

Amortized analysis:

[https://www.youtube.com/watch?v=3MpzavN3Mco](https://www.youtube.com/watch?v=3MpzavN3Mco)

Rope data structure |strings:

[https://www.youtube.com/watch?v=NinWEPPrkDQ](https://www.youtube.com/watch?v=NinWEPPrkDQ)

[https://en.m.wikipedia.org/wiki/Rope\_(data_structure)](<https://en.m.wikipedia.org/wiki/Rope_(data_structure)>)

[https://www.cs.tufts.edu/comp/150FP/archive/hans-boehm/ropes.pdf](https://www.cs.tufts.edu/comp/150FP/archive/hans-boehm/ropes.pdf)

Standard any:

[https://en.cppreference.com/w/cpp/utility/any](https://en.cppreference.com/w/cpp/utility/any)

String view usage:

[https://www.youtube.com/watch?v=PEvkBmuMIr8](https://www.youtube.com/watch?v=PEvkBmuMIr8)

std::basic_string:

[https://en.cppreference.com/w/cpp/string/basic_string](https://en.cppreference.com/w/cpp/string/basic_string)

std::basic_string_view:

[https://en.cppreference.com/w/cpp/string/basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

CppCon 2017： Barbara Geller & Ansel Sermersheim “Unicode Strings： Why the Implementation Matters” :

[https://www.youtube.com/watch?v=ysh2B6ZgNXk](https://www.youtube.com/watch?v=ysh2B6ZgNXk)

CppCon 2016： Nicholas Ormrod “The strange details of std：：string at Facebook＂ :

[https://www.youtube.com/watch?v=kPR8h4-qZdk](https://www.youtube.com/watch?v=kPR8h4-qZdk)

Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik, Morgan McGuire CppCon 22 :

[https://www.youtube.com/watch?v=fglXeSWGVDc](https://www.youtube.com/watch?v=fglXeSWGVDc)

CppCon 2018： Victor Ciura “Enough string_view to Hang Ourselves” :

[https://www.youtube.com/watch?v=xwP4YCP_0q0](https://www.youtube.com/watch?v=xwP4YCP_0q0)

Postmodern immutable data structures :

[https://www.youtube.com/watch?v=sPhpelUfu8Q](https://www.youtube.com/watch?v=sPhpelUfu8Q)
 
strings 1 - Why COW is ungood for std string\strings 1 - Why COW is ungood for std string :

[https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755](https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755)
