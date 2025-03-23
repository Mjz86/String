
# Considering Fragmentation: A Custom Rope Implementation

## Abstract:

This paper presents a custom rope data structure based on a semi-immutable string and a lazy generator. It aims to reduce fragmentation while maintaining sufficient memory efficiency. The rope is designed to be fully usable in a `constexpr` context, although not generally as a `constexpr` variable. (For simplicity, this paper primarily discusses 64-bit little-endian architectures, but the concepts are applicable to others.)

## Introduction:

While ropes may not be a common tool for most developers, they are essential in specific applications. This paper documents the design and considerations behind my rope implementation, even though it's still a work in progress. The goal is to provide a useful reference.

## Implementation Details:

The following conceptual code illustrates the core ideas. It's simplified for clarity:

```c++
// To avoid trashing cache lines, each node has a padded base that holds
// the reference count.  Since it's padded anyway, redundant members
// are added for convenience during destruction.
struct alignas(std::hardware_destructive_interference_size)
node_shared_cache {
  size_t reference_count;
  std::span<elem> elems;
  size_t length;
  allocator my_alloc;
  bool is_threaded;
  bool is_sso;
};

struct node : node_shared_cache {
  union {
    elem children[B];
    //  B×64 bytes
    //  For B=15, the node's SSO segment would be 960 bytes.
    //  Rope slice progression: root(48) -> slice(56) -> node(960) -> more.
    //  The maximum m would be approximately n/28, and the minimum would be 1.
    //  'm' depends only on the number of operations, not 'n', so fragmentation is generally low.
    char sso_buffer[sizeof(children)];
  };
};

struct node_ref {
  node *object;
  // size_t  offset;  Easier substringing could be achieved with an offset, but it's
  // avoided due to potential fragmentation.
  size_t length;
  size_t elem_count;
  size_t tree_hight; // Helps in the concatenation algorithm to identify the
                     // tree depth at which concatenation should occur.
  allocator node_alloc;
  bool is_threaded;
  bool is_sso;
};

// This struct is not padded because the SSO buffer is the largest member.
struct alignas(64) elem {
  size_t index_of_end : 62; // SSO length can be calculated using this and the previous index.
  size_t type : 2;
  union {
    char sso_buffer[56];
    mjz::string string;
    Lazy lazy;
    node_ref node_ref;
  };
};

// Similar to std::any, but the vtable includes a subrange iteration
// function that calls a slice view functor and an allocator reference
// alongside the "void*".  Effectively:
// `void iterate ( const Lazy&obj, std::function<void( string_view slice)> callback);`
// A trick with an empty base class `void_struct_t` emulates `void*`
struct Lazy {
  size_t length;
  size_t offset;
  lazy_Vtable *vtable;
  union lazy_storage; // Size of max(64-32,16)
};

struct root {
  allocator alloc;
  size_t encoding : 3;
  size_t is_threaded : 1;
  size_t type : 2;
  size_t length : 58;
  union {
    char sso_buffer[48];
    mjz::string leaf;
    Lazy lazy;
    node_ref node_ref;
  };
};
```

## Fragmentation:

The degree of fragmentation in the rope is influenced by the randomness of user access patterns.  If a user modifies a position `i`, and another modification occurs at `j` within a distance of at least 56, then `i` and `j` likely reside in the same block.  Initially, the rope has minimal fragmentation (m < 2). Even if the rope is initialized with a gigabyte of data, extensive modifications across almost the entire dataset would be required to reach the worst-case fragmentation of m = n/28.  This is a demanding task, even for continuous arrays.  Rope users are unlikely to modify an entire gigabyte. However,  `for_range` can reduce fragmentation back to 1.  Furthermore, `for_range` can be used to reduce fragmentation of a specific range before iteration, creating a continuous slice without modifying the data directly.

## Invariants:

The index of the string end serves as the key to the tree structure. All (a,b)-tree invariants must hold. Additionally, the following optimization invariants are enforced:

*   If two adjacent non sso children have a combined size smaller than the SSO buffer, they must be combined into a single SSO leaf.
*   If there is only one leaf, the tree height is 0, and no node exists.
*   Leaves (except SSO leaves and the root inline leaf) cannot be directly modified (i.e., characters cannot be changed or appended via direct buffer access). However, substring operations are permitted.

## definition of substring and concatenate:
* note that a node copy is just sharing it , its O(1).
* note : if at any point two ajason children or a  node had less size than its corresponding sso , we collapse them O(B)+O(iteration(B))+O(1+)=O(B+)=O(1+).

### node destroy definition :
  we  ( either unsharing or dealocating) the unused children O(1+).

### substring:

* case of no length: destroy the whole rope , set to sso.
* case of a leaf : we know that sharing substings is O(1) , an SSO  memmove is O(B)=O(1) , a lazy substring is a copy of generator and offset and length change O(1)+O(gen-cpy)=O(1) because generators should be cheap to copy.
* case of a node:
  we search for the new begin and end pair , and we find the range of children that the substring needs. O(logB)=O(1).
  * if there was only one child left , we correct our offset, we make that child the new root and  destroy the old root and do a substring  on the new root O(h-1 +).
  * if there was more :
    we  ownerize the current node O(1)( either a reference check or an unshare-destroy copy).
    we destroy each unused node O(1+).
    we reposition the nodes and their keys ( the indexes may need to change) O(B)=O(1).
    we ownerize the first and last child O(1).( if these children's length is not changed,  we do not ownerize).
    we make sure that the first and last child have more than A  children ( not A itself) (  merge or steal , or collapse to root) O(1).
    we  do a a substring on the first and last  children with the right index adjustment( if the length is unchanged we do nothing ) O(h-1).

    

### concatenation of two nodes O(new\_h):
rhs=right hand side. 
lhs=left hand side.
ohs= opposite hand side.
chs=current hand side.

 * case of two roots with same hight:
  merge these (hight may increase). O(1).
  * else:
  choose the taller rope as chs.
   make sure chs has less nodes than B  ( maybe by split and increasing the hight ) and ownerise it O(B)=O(1).
  lets call the ohs child of chs  , C.
   * if ohs has same hight as C :
     we insert ohs at the position of C child of chs O(B)=O(1).
    * else ( we know it has less hight):
     make sure C  has less nodes than B  ( by split  ) and ownerise it O(B)=O(1).
     we concatenate  C( C is going  to change ) with ohs in the correct order. O(new\_h-1).

## Results of Said Properties:

String operations can be performed using substringing, concatenation, and creation.  This rope implementation achieves the property that its big O complexity is independent of the string length for operations other than iterating.

*   `h` is the height of the tree.  This represents the number of levels from the root to a leaf. Since it's a balanced (a,b)-tree, h is O(log\_a(m)).
*   `m` is the number of slices in the rope.  This is *not* the same as the number of characters. The design prioritizes keeping 'm' much smaller than 'n'.
*   `n` is the number of characters in the rope.
*   `k` is the iteration length/number of chunks iterated over.

Given the string constraints, the height `h` is typically bounded by a small constant (e.g., 50), and in practice, `h` is often less than 4.  Therefore, O(h) ≈ O(1).

The (+) indicates the amortized cost of deallocating unused tree segments, with a worst-case cost of O(m). However, this cost is paid during allocation.

Approximate Time Complexity for various operations:

*   Construction:
    *   From empty: O(1) (Create a root node with an empty SSO string.)
    *   From copy: O(1) (Shallow copy of the tree structure, thanks to COW.)
    *   From generator: O(1) (Wrap the generator in a root node. The generation process is lazy.)
    *   From `mjz::string`: O(1) (Wrap the string in a root node; sharing is O(1).)

*   Destruction: O(1+) (Amortized constant time). Most of the time, destruction is just decrementing reference counts. But, occasionally, we need to reclaim memory.

*   Concatenation: O(h) ≈ O(1) (Rebalancing the tree after joining two ropes. Because h is almost constant)

*   Substring: O(h+) ≈ O(1+) (Mostly constant time due to the balanced tree and amortized rebalancing. We are basically finding the start and end slices and decrementing unused ones )

*   Insertion/Deletion of a Rope Segment: O(h1 + h2 +) ≈ O(1+) (Similar to concatenation, mostly constant time to find the insertion point)

*   Indexing (Accessing a Character by Index): O(h) ≈ O(1) (Walking down the tree to find the correct slice). This is a logarithmic operation but h is a relatively small constant.

*   Iteration: (Important note: ALL iterators are `const` due to COW limitations in my string. But I have `for_range` and other functions to allow mutations, similar to how my `mjz::string` works.)
    *   Simple Iterators (index and object): O(h×k) ≈ O(k) These iterators are lightweight but less efficient because they need to traverse the tree for each character. But in reality, h is constant.
    *   Specialized Iterators (for `std::ranges`): O(h+k) These iterators cache the path down the tree, requiring O(h) memory. They also cache the nearby area [i-G, i+G] (currently lacking the best G value) (if in bounds, if not, the value is simply 0). The previous cached index `i` is stored to compare and see if the current index is in the cache.  If not, the cache is updated. If the iteration is smooth, then we save many lazy calls and cache misses.

*   `for_each_slice` (Function called for each slice - doesn't change the rope): O(k+h) .

*   `for_range` (Mutable Reference - CAN change the rope): O(h+k) time, Memory O(k). This is the important one for in-place modifications. It create a continuous `mjz` string and apply the function, then it inserts the new string to the list of strings.

    More explanation: This makes each of the characters go first to a continuous `mjz` string buffer with reserved size of k, then calls the function, then inserts that buffer into the appropriate position, potentially reducing fragmentation for free. This also allows the API to provide a continuous mutable string to the function, improving its performance and user experience. Use with caution: if you have a one-gigabyte file and use this on all of it, you need 2 gigabytes of memory in the middle of the function (buffer + rope), but the end result would be a more continuous rope, reducing fragmentation. It's a trade-off. However, this isn't usually a problem because users would typically only modify small sections (= small k).  Also, a continuous mutable string is easier to work with, and the user can read and modify batches together into a nice continuous chuck, while being easy to use and arguably faster over the long run. This also reduces fragmentation after the operation has completed, therefore, it's both a user-friendly and cache-friendly thing.

## Benefits and Trade-offs:

### Benefits:

*   **Reduced Fragmentation:** Node combining and copy-on-write strategies help minimize memory fragmentation, especially with frequent modifications to large strings. In most cases, the hot sections are SSO buffers, which simplify mutations.

*   **Memory Efficiency:** Copy-on-write semantics and small string optimization reduce memory consumption by sharing data and avoiding unnecessary copies.

*   **Efficient Substring Operations:** Substring operations are efficient because they create a new rope that shares the underlying data with the original rope. The main string shares substrings, which enables more efficient substringing for the rope too.

*   **Lazy Evaluation:** Lazy generators defer the cost of string generation, improving performance when the string data is not immediately needed.

*   **`constexpr`-Friendly Design:** The implementation aims to be `constexpr`-friendly, enabling compile-time string manipulation where possible. Custom allocators support this goal.

*   **Future Unicode Support:** Support will follow when the main string implementation supports Unicode.

### Trade-offs:

*   **Constant-Time Overhead:** There is constant-time overhead associated with accessing and manipulating the rope due to tree traversal and COW management.

*   **Mutable Iteration Limitations:** The COW implementation makes mutable iteration complex, but `for_range` addresses this.

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
In fact, it cannot be a tree for those.  As per the invariants, a rope with a size less than B×64 (e.g., 960) must have at most one sso leaf,
 guaranteeing a single sso node allocation for such small ropes .
Also, for all strings below the 48-byte threshold, the rope collapses to the inline SSO.
so , B is more than just a fanout, the bigger the B , the more continuous and eager the rope.
this means , that for example,  a one gigabyte file,  may have some very hot , 960 (B×64) byte chunks that it works with to edit text ( note that when we own the node , theres no thread contention on it , because we are the only thread that owns it) , and the other parts would probably be some multi megabyte lazy fragments. 

## Conclusion

This paper has presented a custom rope implementation designed to address the challenges of manipulating large strings efficiently. By combining a balanced (a,b)-tree, copy-on-write semantics, small string optimization, and lazy evaluation, this implementation aims to minimize memory fragmentation, reduce memory consumption, and provide efficient substring operations.

## Note

While the implementation is still under development and not yet open source, I would appreciate your feedback. Also, sorry if the markdown is hard to read. This paper is located at:

[https://github.com/Mjz86/String/blob/main/rope\_paper.md](https://github.com/Mjz86/String/blob/main/rope_paper.md)

You may give feedback in:

[https://github.com/Mjz86/String/issues](https://github.com/Mjz86/String/issues)

## References and Inspirations:

My continuous string:

[https://github.com/Mjz86/String/blob/main/README.md](https://github.com/Mjz86/String/blob/main/README.md)

Learn about (a,b) trees and b trees :

[https://youtube.com/watch?v=lifFgyB77zc&si=IPNlqVVdr0nU-n\_G](https://youtube.com/watch?v=lifFgyB77zc&si=IPNlqVVdr0nU-n_G)

[https://www.youtube.com/watch?v=K1a2Bk8NrYQ](https://www.youtube.com/watch?v=K1a2Bk8NrYQ)

Learn about the cache:

[https://www.youtube.com/watch?v=dFIqNZ8VbRY](https://www.youtube.com/watch?v=dFIqNZ8VbRY)

Lazy evaluation |Ruby Conference 2007 Ropes： An Alternative to Ruby's Strings by Eric Ivancich :

[https://www.youtube.com/watch?v=5Xt6qN269Uo](https://www.youtube.com/watch?v=5Xt6qN269Uo)

Amortized analysis:

[https://www.youtube.com/watch?v=3MpzavN3Mco](https://www.youtube.com/watch?v=3MpzavN3Mco)

Rope data structure |strings:

[https://www.youtube.com/watch?v=NinWEPPrkDQ](https://www.youtube.com/watch?v=NinWEPPrkDQ)

[https://en.m.wikipedia.org/wiki/Rope\_(data\_structure)](https://en.m.wikipedia.org/wiki/Rope_(data_structure))

[https://www.cs.tufts.edu/comp/150FP/archive/hans-boehm/ropes.pdf](https://www.cs.tufts.edu/comp/150FP/archive/hans-boehm/ropes.pdf)

Standard any:

[https://en.cppreference.com/w/cpp/utility/any](https://en.cppreference.com/w/cpp/utility/any)

String view usage:

[https://www.youtube.com/watch?v=PEvkBmuMIr8](https://www.youtube.com/watch?v=PEvkBmuMIr8)

std::basic\_string:

[https://en.cppreference.com/w/cpp/string/basic\_string](https://en.cppreference.com/w/cpp/string/basic_string)

std::basic\_string\_view:

[https://en.cppreference.com/w/cpp/string/basic\_string\_view](https://en.cppreference.com/w/cpp/string/basic_string_view)

CppCon 2017： Barbara Geller & Ansel Sermersheim “Unicode Strings： Why the Implementation Matters” :

[https://www.youtube.com/watch?v=ysh2B6ZgNXk](https://www.youtube.com/watch?v=ysh2B6ZgNXk)

CppCon 2016： Nicholas Ormrod “The strange details of std：：string at Facebook＂ :

[https://www.youtube.com/watch?v=kPR8h4-qZdk](https://www.youtube.com/watch?v=kPR8h4-qZdk)

Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik, Morgan McGuire CppCon 22 :

[https://www.youtube.com/watch?v=fglXeSWGVDc](https://www.youtube.com/watch?v=fglXeSWGVDc)

CppCon 2018： Victor Ciura “Enough string\_view to Hang Ourselves” :

[https://www.youtube.com/watch?v=xwP4YCP\_0q0](https://www.youtube.com/watch?v=xwP4YCP_0q0)

Postmodern immutable data structures :

[https://www.youtube.com/watch?v=sPhpelUfu8Q](https://www.youtube.com/watch?v=sPhpelUfu8Q)

strings 1 - Why COW is ungood for std string\strings 1 - Why COW is ungood for std string :

[https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755](https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755)
