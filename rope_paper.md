
</br># considering fragmentation, a custom rope implementation:
</br>
</br>#abstract:
</br>this paper presents my rope datastructure based on my semi-immutable string and a lazy generator. 
</br>it reduces fragmentation while also maintaining a sufficient amount of
</br>memory efficiency. the rope would be fully usable in a
</br>constexpr context, but not as a constexpr variable in general.
</br>(  again,  for simplicity i only talk about 64bit little endian, but others are similar)
</br>#introduction :
</br>while most developers dont use ropes , there are applications
</br>where they are essential. while i haven't reached a usable and
</br>efficient implementation, i do think its worth making a
</br>reference for.
</br>
</br>
</br>#implementation details:
</br>heres some conceptual code, not actual code , but less hard to understand:
</br>
</br>
```
//firstly,  we care about  not trashing the cache line , therfore,  each node has this padded base as the reference count holder.
//because its going to be padded anyway , i wanted to put sone redundant members for convenience in the destruction.
struct  alignas(std::hardware_destructive_interference_size)
node_shared_cache{
size_t  reference_count;
std::span<elem> elems;
size_t  length;
allocator  my_alloc;
bool is_threaded;
bool is_sso;
};

struct node:node_shared_cache{
union {
elem children[B]; 
// we can say that its B*64 ,
// so , for a typical B=15 ( B is a template pram ),
// the node  sso segment would be 960 bytes , 
// efficiently,  the rope slice progression is from root(48) to slice(56) to node(960) to more .
// effectively  , the maximum m would be n/28 , and the minimum would be 1.
// we know that m is only dependent on the number of operations that the user did , and not n so we can say that in almost all cases fragmentation is low.
char sso_buffer[sizeof(children)];
};
};

struct node_ref{
node*object;
//size_t  offset; i could have done this for easier substring,  but i think it'll result in more fragmentation in many cases,  so no thanks.
size_t  length;
size_t elem_count;
size_t  tree_hight;// helps in the concatenation algorithm,  to identify  the tree depth that the concatenation needs to take place. 
allocator  node_alloc;
bool is_threaded;
bool is_sso; 
};
// this is not padded , because sso is biggest member.
struct alignas(64) elem{
size_t  index_of_end:62;// we can calculate the sso length using this and its previous index.
size_t  type:2;
union {
char sso_buffer[56];
mjz::string  string;
Lazy lazy;
node_ref node_ref;
};
};


// this is somewhat similar to implementation of standard any , but the v table has an extra subrange iteration function that calls a slice view functor , and an allocator reference   along side the "void*" .
// Basically like this but not with  a standard function, but a no allocation stack version of it):
// void iterate ( const Lazy&obj, std::function<void( string_view slice)> callback);
// i used a trick to get the same functionality of void* , basically an empty base class called void_struct_t is used as a workaround.
struct Lazy{
size_t  length;
size_t  offset;
lazy_Vtable* vtable;
union lazy_storage;// with  size of max(64-32,16)
};





struct  root{
allocator alloc;
size_t encoding:3;
size_t is_threaded:1;
size_t type:2;
size_t length:58;
union {
char sso_buffer[48];
mjz::string  leaf;
Lazy lazy;
node_ref node_ref;
};
};
```


</br>#fragmentation: 
</br>  the amount of fragmentation of the rope is dependent on the randomness of the user access patterns, 
</br> because  , we know  that if the user changes some place called i , if j is close enough ( at least 56 as distance) 
</br> then the block which i and j reside must be the same , 
</br> and we know  that in the first operation  , the rope has a minimal fragmentation (m<2) 
</br> so , even if the string that the tope was initialized with was a gigabyte, 
</br> you would have to modify almost all of that data to get the worst fragmentation of m=n/28
</br> and frankly,  the cpu itself struggles with this task for an array , a continuous array!
</br> and i wouldn't expect the users of a rope to want to modify all of that gigabyte 
</br> and if they really wanted to do that , we have a great way to do so while reducing fragmentation back to 1 ( use for_range for this task)
</br> also , a bonus is , if someone just wants to reduce fragmentation of a particular range before iteration, 
</br> they could  just call for_range on that portion and not even touch said range ,
</br> but get a nice continuous slice for their wish.
</br>
</br>#invariants :
</br>we have the index of the string end as the key to the tree.
</br>its more nuanced,  but its still like a tree.
</br>all the ab tree invariants must hold.
</br>and as a bonus,  these are the following optimization invariants:
</br>if two ajason children have less size than  the sso buffer of choice,  then they must be combined into a single sso leaf.
</br>if we have only one leaf , the tree hight is 0 and no node exists.
</br>we cannot  modify any leaf except the sso leaf or the root inline leaf, and by modify,  i mean changing or appending a character by accessing the leaf's buffer, but we can do a substring operation on it.
</br>
</br>#results of said properties:
</br> we know that any string operations can be done using substringing and concatenation and creation .
</br> so , we did everything and we achieved  the property that the rope big O complexity is independent of the length for everything other than iteration over the data.
</br>
</br>h is the height of the tree. Think of it as the number of levels you need to traverse to get from the root to a leaf. Because it is an (a,b) tree that is always balanced, it is O(log_a(m))
</br>
</br>m is the number of slices in the rope. This is not the same as the number of characters! A key goal of this design is to keep m much smaller than n.
</br>
</br>n is the number of characters in the rope.
</br>
</br>Δh represents the change in height after an operation, e.g., concatenation or insertion. In most cases, this will be a small constant because the (a,b)-tree rebalancing keeps things pretty stable.
</br>
</br>k is the length of the iteration length/ number of chunks that is iterated on in a process.
</br>
</br>from the string constraints we know  that the hight is never ever bigger than a constant number such as 50 , and in almost all practical areas,  h is less than 4, so we can say that O(h)≈O(1)
</br>
</br>also , the (+) is the amortized cost of deallocating the unused tree segment with worst case of O(m) , but the cost is payed when we allocate it.
</br>
</br>Now, here's the approximate time complexity for various operations:
</br>
</br>°  Construction:
</br>
</br>  °  From empty: O(1) (just creating a root node with an empty SSO string)
</br>  °  From copy: O(1) (shallow copy of the tree structure, thanks to COW)
</br>  °  From generator: O(1) (wrap the generator in a root node. The generation process is lazy)
</br>  °  From mjz::string: O(1) (wrap the string in a root node, sharing is O(1) )
</br>
</br>°  Destruction: O(1+) (amortized constant time). Most of the time, destruction is just decrementing reference counts. But, occasionally, we need to reclaim memory.
</br>
</br>°  Concatenation: O(Δh) ≈ O(1) (rebalancing the tree after joining two ropes. Because Δh is almost constant)
</br>
</br>°  Substring: O(h+) ≈ O(1+) (again, mostly constant time because of the balanced tree and amortized rebalancing. We are basically finding the start and end slices and decrementing unused ones )
</br>
</br>°  Insertion/Deletion of a Rope Segment: O(h1 + h2 +) ≈ O(1+) (similar to concatenation, mostly constant time to find the insertion point)
</br>
</br>°  Indexing (Accessing a Character by Index): O(h) ≈ O(1) (walking down the tree to find the correct slice). This is a logarithmic operation but h is a relatively small constant.
</br>
</br>°  Iteration: (important note: ALL iterators are const, because of COW limitations in my string. But I have for_range and other functions to allow mutations, similar to how my mjz::string works.)
</br>
</br>  °  Simple Iterators (index and object): O(h×k) ≈ O(k) These iterators are lightweight but less efficient because they need to traverse the tree for each character. But in reality , h is constant.
</br>  °  Specialized Iterators (for std::ranges): O(h+k) These iterators cache the path down the tree, but require O(h) memory to store that path,
</br>    they also cache the nearby area (i-G,i+G] ( currently lacking the best G value to say ) ( if in bounds, if not , the value is simply 0, also , the i that was the previous chache is stored to compare and see if the current index is in cache, if not , we update the cache ) in a temporary storage  and the current index ( if the iteration is smooth , then we save many lazy calls and chche misses)
</br>°  for_each_slice (Function called for each slice - doesn't change the rope): O(k+h) .
</br>
</br>°  for_range (Mutable Reference - CAN change the rope): O(h+k) time, Memory O(k). This is the important one for in-place modifications. It create a continoues mjz string and apply the function and then it inserts the new string to the list of strings.
</br>more explanation:
</br>this makes each of the characters go first  to a continuous mjz string buffer with reserved size of k  , then calles the function, then insert that buffer into the appropriate position,  potentially reducing fragmentation for free, this also makes it possible for the api to actually always give a continuous mutable string to this  function improving its performance and the users quality of life for free.
</br> use this with caution,  if you have a one gigabyte file , and use this on all of it , you need 2 gigabytes of memory in the middle of the function ( buffer+rope) ,  but the end result of this , would be that the rope is more continuous,  reducing fragmentation, so its a trade off.
</br> i would  say that this isnt a problem however,  because users would usually only modify small sections (=small k) and also , this is honestly more convenient for everyone , because a continuous mutable string is easy to work with , and the user can read and modify batches together into a nice continuous chuck , while being easy to use and arguably faster ovdr the long run.
</br>  this , also reduces fragmentation after the operation has completed,  therfore,  its both a user friendly and cache friendly thing.
</br>
</br>
</br>
</br>
</br># Benefits and Trade-offs:
</br>benefits:
</br>
</br>•Reduced Fragmentation:
</br>The node combining and copy-on-write strategies help to minimize memory fragmentation, especially when dealing with frequent modifications to large strings.
</br> in most cases , the hot sections are sso buffers , and make mutations easier.
</br>•Memory Efficiency:
</br> Copy-on-write semantics and small string optimization reduce memory consumption by sharing data and avoiding unnecessary copies.
</br>• Efficient Substring Operations:
</br> Substring operations can be performed efficiently by simply creating a new rope that shares the underlying data with the original rope.
</br> this is because both the  main string shares substrings and enables us to do the same more efficiently. 
</br>•Lazy Evaluation:
</br> Lazy generators allow for deferring the cost of string generation, improving performance in cases where the string data is not immediately needed.
</br>•constexpr-Friendly Design:
</br>The implementation strives to be constexpr-friendly, allowing for compile-time string manipulation where possible.  The use of custom allocators also enable this.
</br>• future unicode support:
</br>when the main string gets supported , this will too.
</br>
</br>#Trade-offs:
</br>•Constant-Time Overhead:
</br> There is some constant-time overhead associated with accessing and manipulating the rope data structure due to the tree traversal and COW management.
</br>•Mutable Iteration Limitations:
</br>Due to the COW implementation, mutable iteration is complex, but the For Range solves this problem .
</br>
</br>
</br>#  usability:
</br> a great way to do undo, redo is to make a vector of ropes , each modification would be stored at the last position, because  of cow , this is easily manageable.
</br>  for synchronization in file operations,  we can pass a rope cow copy to be written to a file , 
</br>  or get a lazy evaluated immutable view into a constant file .
</br>  we can also generate data on the go , 
</br>for example if we have a hard to do computation for knowing the value ( such as decoding a massage encrypted with AES), we dont need to do it right away ,
</br> we can make a generator with a mutext and a mutable sub rope initilized with an internal generator as the storage,  the subrope is materialized using the for_crange( basically for range but we  do not give a mutable reference  , but a constant  one , this is more efficient  in some cases where mutations have to copy a section but this doesn't ,while also reducing  fragmentation) every time a subrange is required 
</br> this way , the generator only runs the algorithm once per block , and saves us many decodings .
</br> the rope api is as close to the main string while also not assuming a continuous representations,  and at last .
</br> the rope doesn't need to be a tree for smal strings , it actually cannot be , if you remember from the invariants,  the rope with a size less than B*64 ( lets say 960 for example) must be at most one leaf , therfore a singular allocation is granteed for such small ropes ( if we assume that the generator object is not going to allocate).
</br> also , for all strings bellow the 48 threshold,  the rope  collapses to the inline sso , and the generator is eagerly executed,  because in such cases, its brutal to not be continuous and inline. 
</br>#Conclusion
</br>
</br>This paper has presented a custom rope implementation designed to address the challenges of manipulating large strings efficiently.
</br> By combining a balanced (a,b)-tree, copy-on-write semantics, small string optimization, and lazy evaluation, 
</br>this implementation aims to minimize memory fragmentation, reduce memory consumption, and provide efficient substring operations. 
</br>
</br>
</br>#note
</br>While the implementation is still under development, and noy yet open source,  i would appreciate your feedback. 
</br>also , sorry if the markdown is hard to read. 
</br>this paper is located at :
</br>https://github.com/Mjz86/String_description/blob/main/rope_paper.md
</br>you may give feedback in:
</br>https://github.com/Mjz86/String_description/issues
</br>
</br>
</br>
</br>
</br># refrences and inspirations:
</br>
</br>my continuous string:
</br>https://github.com/Mjz86/String\_description/blob/main/README.md
</br>learn about (a,b) trees and b trees :
</br>https://youtube.com/watch?v=lifFgyB77zc&si=IPNlqVVdr0nU-n\_G
</br>https://www.youtube.com/watch?v=K1a2Bk8NrYQ
</br>
</br>lazy evaluation |Ruby Conference 2007 Ropes： An Alternative to
</br>Ruby's Strings by Eric Ivancich :
</br>https://www.youtube.com/watch?v=5Xt6qN269Uo
</br>
</br>amortized analysis:
</br>https://www.youtube.com/watch?v=3MpzavN3Mco 
</br>rope data structure |strings:
</br> https://www.youtube.com/watch?v=NinWEPPrkDQ
</br>https://en.m.wikipedia.org/wiki/Rope_(data_structure)
</br>https://www.cs.tufts.edu/comp/150FP/archive/hans-boehm/ropes.pdf
</br>standard any:
</br>https://en.cppreference.com/w/cpp/utility/any
</br>string view usage:
</br>https://www.youtube.com/watch?v=PEvkBmuMIr8
</br>std::basic_string:
</br>https://en.cppreference.com/w/cpp/string/basic_string
</br>std::basic_string_view:
</br>https://en.cppreference.com/w/cpp/string/basic_string_view
</br>CppCon 2017： Barbara Geller & Ansel Sermersheim “Unicode Strings： Why the Implementation Matters” :
</br>https://www.youtube.com/watch?v=ysh2B6ZgNXk
</br>CppCon 2016： Nicholas Ormrod “The strange details of std：：string at Facebook＂ :
</br>https://www.youtube.com/watch?v=kPR8h4-qZdk
</br>Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik,  Morgan McGuire  CppCon 22 :
</br>https://www.youtube.com/watch?v=fglXeSWGVDc
</br>CppCon 2018： Victor Ciura “Enough string_view to Hang Ourselves” :
</br>https://www.youtube.com/watch?v=xwP4YCP_0q0
</br>Postmodern immutable data structures :
</br>https://www.youtube.com/watch?v=sPhpelUfu8Q
</br> strings 1 - Why COW is ungood for std string\strings 1 - Why COW is ungood for std string :
</br>https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755
