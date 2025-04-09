
# Reconsidering COW, a Modern C++20 String Implementation

**tl;dr:**

Less lifetime management (dangling views) while maintaining view (and value)
semantics and performance characteristics (no copying or allocation), also , the wrappers allow for tuning the sso size to any desired size , and all of them are compatible with each other. And
planned Unicode support.

# Abstract

This paper presents a custom string class designed for memory efficiency.
The paper is mainly focused on the 64-bit little-endian implementation, but other platforms also have equivalent functionality. The string class provides a way to store various encodings in different string types. It's allocator-aware and uses SSO and COW features while adding others with minimal overhead. The class is as `constexpr` friendly as the standard string, mostly comparable to the GCC implementation.

# Introduction

Many C++ programmers have worked with strings. Several have been forced to use either immutable value semantics with views or reference semantics and allocations for mutable strings. This string class bridges these, providing an owning tagged string view as its storage.

# Implementation Details

note that in the 32bit platform, there are two options,
one is big address space ( 56 bit) and one is small address space ( 24bit) , in the small case , the object size is 16bytes and in the large case 32bytes.

The basic layout can be thought of as the following (not valid C++, but its equivalent) (total of 32 bytes in my own implementation):

```c++
struct {
  (allocator reference as empty base);
  const char* begin_ptr;
  size_t  length;
  union{
    char  buffer[15];
    struct referencal_t{// the string is a reference to something
      char* data_block;
      size_t capacity:56;// in actuality only 7 bytes
    };
  };
  uint8_t  control_byte;
};
```

The control byte can be thought of as:

```c++
struct {
uint8_t  /*the negation of this is actually stored*/is_threaded:1;

  uint8_t  is_sharable:1;// this  indicates  that we are in a heap or litteral view , vs , sso or stack buffer or copying view.


 uint8_t is_ownerized:1;   /*this flag is not propagated by copy or share, only by move,in the non-move case,  if any side (dest or src) has this flag set to true, a memcpy and a potential allocation occurs (if sso or stack buffer is not large enough) */
 // to always disable cow and viewer for a specific string ,
 // to remove the reference_count checks,
 //controled  with  always_ownerize(bool flag_state),
 //  (can_share()&&!is_ownerized) determines sharability
 // both of these flags are essential and they do not correlate.

  uint8_t has_null:1;// needed to share substrings

  uint8_t:1;//unused_for_now_
  uint8_t  encoding:3; // we are not a bag of bytes
};
```

The encoding flags are for knowing the encoding of the stored data.

The heap block can be thought of as:

```c++
struct {// this is just a 8 or 16 byte aligned char array with this layout
  size_t  reference_count;
  char heap_buffer[capacity];
};
```

# Invariants

1.  **At the string view state:**

    - `begin != buffer`.
    - `active union member == referencal_t`.
    - `begin != nullptr || 0 == length`.
    - `data_block == nullptr`.
    - `capacity == 0`.
    - `is_owner() == false`.
    - `is_ownerized == false`.
      the view optimization is essentially a cow string with static lifetime, therfore its not ownerized.

2.  **At the SSO string state:**

    - `begin == buffer`.
    - `active union member == buffer`.
    - `capacity == 15`.
    - `is_owner() == true`.
    - `is_sharable == false`.
    - `has_null == (length != 15)`.

3.  **Heap string state:**

    - `begin != buffer`.
    - `active union member == referencal_t`.
    - `begin != nullptr`.
    - `data_block == &heap_buffer`.
    - `capacity != 0`.
    - (`capacity` is almost always bigger than 15, but no guarantees are made)
    - `is_owner() ==(is_ownerized ||  (reference_count < 2))`.
    - ` !is_ownerized || reference_count < 2`.
    - `is_sharable == true`.

4.  **Stack buffer string state:**

    - `begin != buffer`.
    - `active union member == referencal_t`.
    - `begin != nullptr`.
    - `data_block != nullptr`.
    - `capacity != 0`.
    - `is_owner() == true`.
    - `is_sharable == false`.

Also, `[begin, end)` is a continuous sub-range of
`[data_block, data_block + capacity)` if and only if `data_block` is not
null and alive.

# Addressing COW and the Drawbacks

- note , if a string is showing bottlenecks on cow, try `ownerize()` or `always_ownerize(true)` or the `ownerized_string`.
- on the main string:
  All of the iterators, the methods, and functions only give constant
  references to the data.
- for mutatable iteration, consider the following:
  the `ownerized_string` wrapper is an equivalent of `std::string` ( null termination requirement is given via template flag to provide `c_str`).
  users can convert between these easily without any lifetime issues,
  and because its a wrapper, it wouldn't have code bloat.
  i think rust had a cow and non cow string as well.

```c++
struct ownerized_string /* in actuality,  this is a using statement of the real template wrapper*/{
private:
  //....
  mjz::string value;// is_ownerized being true is an invariant.
  // the standard string interface can be used( other than c_str )
};
```

i dont think the null terminator is important enough for a wrapper,
it shouldn't be dependent upon at all , its purely legacy.

but , its easy enough to add another template flag for it , so why not.

# How to do This Without My Help

The string design presented may be complex for readers to implement on their
own, but they can make it easily if they don't care about the object size or
indirections and allocation strategy (I needed the object size to be small,
but if not):

```c++
struct simpler_version{
  std::variant <std::shared_ptr<std::string> // cow
                ,std::array<char,sizeof(std::string)>         // sso
                ,std::span<char>             // stack buffer
                > data;
  std::string_view view;
  encodings_t encoding;
  bool has_null;
  bool is_ownerized;
  // functionality....
};
```

# Thread-Safety (Opt-Out)

The string ensures to use atomic operations if `is_threaded` is true. For a
brief summary, the thread-safety grantees are similar to a `shared_ptr` of a
`std::string` if the flag is true.

if thread-safety is turned off manually( advanced users) , 
then :
the string object,  its (non-safe) copies and its moves and views , must all begin and end their lifetime in a single thread with the exact same id.
the Allocators must support at least 8 byte alignment ( selected via flag) 

if thread-safety is on ( default):
the Allocator must support thread-safe allocation mode ( selected via the  custom Allocator api flags) ,
the Allocators must support at least 64 byte alignment ( selected via flag).







# Constexpr Compatibility

The `reference_count` variable is stored as 8 bytes and is bitcasted (no
thread-safety in `constexpr` mode because of obvious reasons). Only the static
string view can be stored as a static constant expression value, but the
string is fully functional in a `constexpr` context. No `reinterpret_cast` is
used (the only time we do use it is for the thread-safe atomic operations
which are not `constexpr` anyways). A `constexpr` friendly allocator is provided
by the implemented library.

# C Interoperability
use the `c_string` wrapper for null terminated strings.

```c++
struct c_string {
//.... another using statement of the real template wrapper,
// null termination is an invariant
private:
 //....
 mjz::string value;
};
```

this has the `c_str` function.

# Features and Implementation

The built-in string viewer and shared substrings: the string is accessed via
the begin and length pair, they provide the minimal functionality of a string
viewer. A substring function may share the underlying data if and only if `!is_ownerized&&can_share()`.

# Mutable Strings

- the main string:
  The string manages its resources and can be modified using the other part of
  the object. The functions ensure correct COW semantics, and they allocate when
  necessary. Almost all equipment functionality of `std::string` can be
  supported, except the following: the value of `operator[](i)`, `at(i)`,
  `*data()`, `front()`, `back()` cannot be mutated (see the historical COW
  reference for why). `c_str()` does not exist (`as_c_str()` does, but it can
  mutate and it only gives a pointer to `const`). I cannot prove a `const`
  alternative; I do not want pointers to temporaries nor relying on `has_null`
  being true. The "as" means that it modified it to be "as" requested (that's
  what the prefix in the name means). We can also use another name, but the
  function name isn't important in the design (`add_null_c_str`).
  note that the `data()` function will have output a null terminated `const char*` if and only if `has_null()`.
  also , if you called `as_c_str()` without failing, you know that `has_null()` is true.
  
 -for null terminated strings , we recommend the `c_string` wrapper.
  
- for mutatable strings we recommend `ownerized_string` wrapper .

# COW Overhead

- can be turned off for a specific string, or by using the (`ownerized_string`) .

Other than the destruction and construction, which may need a branch to the
non-view path if the optimizer doesn't realize triviality, the string const
overhead is similar to a view, which is minimal. We can easily make a view
out of this, no branching required. Actually, I had my previous string layout
with an SSO size of 31, but the downside was a branch in the const view path. I
discarded that and made all of the view information accessible without
branching. The only time where the overhead is felt is in modification of the
string content. I tried my best to get the modification function as efficient
as possible, but in these worse cases, I can't do much else: the string was
cold, the reference count was cold or has false sharing (contention when
modification trashes the cache line) (heap strings), the data was cold, the
data needed deallocating or reallocation.

But at last, any function that is marked const doesn't even think about the
storage location or strategy nor lifetime; it's as if it was a view all along.

And the ones that are not marked const are the ones who need to know about
other stuff. Also, there are some functions that are not const
(`remove_suffix` or `prefix`, `as_substring`) that only address the SSO part
and treat the other parts as views; these don't even need to know about COW nor
ownership.

# why not 30byte default sso in my main 32byte object?
* my earlier design,  it has all of the benefits of the msin string,  they can be converted to each other very easily , and i may add it.
* the `packed_string`  is more similar to fbstring and the standard clang string,  because the sso is very big in both of them and both check for it with more branches. 
* my main string however is more like the standard gcc implementation in its layout. 
* i may add another type called `packed_string`  , because why not have both if they can be in different headers?  ( if i go down this path,  i will be certain that 8 is the most encodings that a string may have).
* the  `packed_string` has half the object size as the  `implace_string<30>` , but with the cost of one extra branch in all const view paths.
* the `packed_string`  does allow for stack buffer optimization ( = tunable sso ) and all the other optimizations , its just a bit trickier, mostly more code to write.
* move convertions and pure-sharing( by ref count ) from the `packed_string`  type to `implace_string<31>` should  never allocate because the heap layout of them are the same ( and the sso buffers match)  , and the heap buffer can also be shared between packed snd non packed types.
* the integration of the `packed_string`  type would be easy if necessary,  and this would  probably be just a way to store a string without a big object,  but the main one and its wrappers would be for passing strings around.
* addition of the `packed_string` would not introduce any overhead in the main string,  they will be in different headers and the `packed_string` would only need to conform to the ABI of the heap string to integrate seamlessly with the main string, i personally like adding it , because why should my string have less default sso than clang's 24byte object,  it doesn't seem fair to not have a more aggressive sso ( the problem with the `implace_string<N>`  is that it has about 32bytes more than N as its size) .
* the only questions to ask now is , is it worth integrating and writing the `packed_string` ? is the loss of potential for more encodings acceptable ?
- the earlier design (= `packed_string` )was like this :

```

struct alignas(8) {
  (allocator reference as empty base);
  ( pragma pack start)
  struct{
    union{
   struct sso_t{
 char  buffer[30];
 char valueof_30_minus_len; // has_null is always true in the sso case.
 // we could have did a trick with has_null to increase the sso to 31 , but that would be an extra branch just for a single byte , which is not good at all. 
 };
    struct referencal_t{
      const char* begin_ptr;
       size_t  length;
      char* data_block;
  alias_t<char[7]>  /*size_t*/ capacity/*:56*/; 
    };
  };
  uint8_t  control_byte;// the same stuff , but the unused bit is called is_sso  
  };
  ( pragma pack ends)
};
```

the  `packed_string`  invariants were mostly similar , except the begin and sso buffer relationship being managed with a flag,  but the difference is ,
like in fbstring,  the  `packed_string`  needed at least an extra branch in each call of the string, 
and this is our problem.



* like how clang and fbstring use sso sizes that almost match the object size , such as 22 or 23byte sso for 24byte clang string,  and 22 or 23 bytes for fbstring. 
* gcc and msvc are different,  and both have smaller sso compared to object size, for example 15 or 16bytes in gcc with 32bytes per string object. 

first of all , that was my 3rd out of 5 old designs.
for these reasons:
excessive branching for the const view paths.
when the length , begin ,and encoding are not easily accessible in a branchless way , 
many functions suddenly become worse , 
the inlining of code becomes harder , 
and the code becomes really hard to write without geter seter function calls everywhere.
the constant checking of  sso vs the non-sso case becomes exhausting. 
while the object size can be the same 32 bytes,  the code size may increase a lot , and the performance hit may be more noticeable. 

while 64 byte object for the tunable sso size of  30 is not ideal, 
its a compromise between  being very packed,
or easier to write and execute. 


although,  this is likely not a big problem because most of the strings would be lass than 30 bytes , 
and the clang implementation has this branch cost , with sso of 22 bytes,  so  this is not a bad tradeoff, hence why i am questioning if the  `packed_string`  is a welcome addition to the library or not.




# Built-In Stack Buffer Optimization (Advanced Users Only)

- use a safe wrapper of this feature for a better quality of life (`implace_string`) .

By using a stack buffer, you ensure that no allocation occurs as long as the
buffer is big enough. If not, allocation may occur. The users must ensure
that the buffer outlives the string object and the objects that it moved to or
a view that it was converted to, but unlike the Allocators, they don't need to
guarantee outliving a copy of it. Notice that copies are allowed to outlive
the buffer; this is because COW doesn't apply to stack buffers because of
obvious reasons. Also, this is not checked; it's raw performance of a span of
chars, and most users won't ever need such performance (lifetimes are hard;
this is discouraged), but some places (in the internals of my rope
implementation) may need it, so it's there.

# what is the type of the owner? 
### ( standard and custom string compatibility outside of the mjz library)( next experimental release)( another wrapper):
 * this feature is  currently not implemented, but after the implementation,  this should be a safe to use feature. 
 
   a wrapper  of our string type and another string type as a compatibility layer can be made.
   this is how it  works:
   the external string type acts as a storage mechanism, 
   the main string is stored alongside the external one .
   the data buffer of the external string is used with the "stack" buffer feature, 
   the external string only manages the buffer lifetime.
   the main string provides compatible api with the library, while in actuality, 
   the data was stored in another string type all along .
   when the external string is needed, 
   the string is shifted and the length is reduced( if necessary,  because of how we use a subrange of the data , we need to  make the subrange subrange the actual range again),
   then the external string is provided. 
   ( actually,  the `implace_string` is kinda just this , but with a standard array) 



# tunable sso , no code bloat, no big types:

- this will be provided with the name (`implace_string`) .
  ( a safe wrapper of the stack buffer )
  we may provide a safe wrapper ( the string would be a private member ,the buffer would be a private member) class that has a bigger sso buffer ,
  while also reusing all the code of the string , think of it like an implace vector ,
  this wouldn't need lifetime knowledge, so it would be for intermediate users .
  even if they never use the unsafe stack buffer directly.
  this is like the game industry's sso strategy, but with minimal code bloat,
  as a bonus, you can seamlessly pass this around without lifetime issues ( the !is_sharable flag disables cow for the private sso buffer, so no dangling references)
  it can be converted to other wrappers of different sizes and to the string itself .
  also , if you remember, from the copy construction section,
  we would de-share automatically for these strings , essentially,
  the template size argument and type incompatibility dissappears (but does its job).

# The Optimizations of Remove Prefix/Suffix, Push Back, Pop Back, Push Front and Pop Front

- note that the null terminator retirement makes this harder for the `c_string`, mainly , sharing substings will break cow more with null terminator requirements.

Because the begin pointer is not limited to the beginning of the buffer, we
can use the view semantics to remove the extra character without memmove.

We also have 3 mods of the first position alignment of the range:

- Central buffer (`begin = buffer + (cap - len) / 2`)
- Front buffer (`begin = buffer`)
- Back buffer (`begin = buffer + cap - len`)

After that, the position may change, but we could append and prepend to the
range without memmove in many cases if we want to.

# false sharing consideration :

  when the `is_threaded` flag is true , we make the   buffer padded with `(std::hardware_destructive_interference_size-sizeof(size_t))=56`bytes at the beginning. ( the address of the  `heap_buffer` member is aligned to 64bits )
  
  the alignment requirement of the allocated data char array also goes up to 64bytes.

  this does increase overhead to about 64bytes per heap string with threading enabled, but the benefit would be that the string data itself is not in the same cache line as the reference count ,
  
 by doing this we ensure that the constant shared data stored in the string , is never affected by the reference count modifications that take place on that string.

 this is especially important when each string is a leaf of the rope , because the rope is often read concurrently and the substring sharing  is very important in the ropes design. 

 sadly because of layout compatability requirements,  the ownerized_string is also going to have the 64byte overhead in the thread-safe case , even tho its not technically necessary, the need for very efficient convention from owerized to main mandates this.
 
 the thread-safety option would be the default,  but considering that almost all the strings are at least max(16,N) (N of tunable sso) bytes when in heap ,
 and that the heap size exponentially grows , in normal standard strings and vectors ,
 i dont see why 64bytes is bad , especially considering the amout of subtle false sharing it reduces in the rope ( which is a crucial factor in the library,  because the rope performance is very important).
 
 ###  fbstring-like cow size threshold :
  from the multithread benchmarks for medium heap strings ( not large ones ) ,  the Allocators performed faster than incrementing the reference count.
  
 in my test  the increment copy varies from  10ns ( in no threads) to  1500ns ( in 100 threads) per operation, ( you can measure yourself,  or see the Facebook's fbstring's reasons, which  is similar in this regard).
 
 but the copy was consistent from 50ns to 200ns in the standard string.
 
 so for strings smaller than `4*std::hardware_destructive_interference_size=64*4=256 `  , 
the reference count block is dropped,  and the block is copied  ( note that `is_sharable` is true  , the`can_share()=is_sharable&&(!is_threaded||256 < cap)` function is used to determine sharability).

in the lower than threashold case,buffer has no overhead but is still aligned to 64bytes (for technical reasons).

 this is not a change in the O(1) ness of cow copy , because thenon cow case has a limit. 
 
 this does mean more fragmentation,  so , this is only for the thread-safe version because of obvious reasons.
 the non thread safe version doesn't need to do this , it had no contention to begin with,  it had minimal fragmentation. 
 this encourages strings with less than 256 capacity to not waste  1/4  of the space for nothing but a number who makes things slower. 
 while also making big chucks of data that are expensive to allocate lower.
 
 this makes users who need performance think about if thread-safety safe is necessary for them or not ,
 because as it seems,  thread-safety grantees are not necessary for many strings ,
 but keep in mind that they have different layouts and that their allocation strategy may be different ( my pmr Allocators  take thread safety and many other handy flags as arguments,  these should be yhe same arguments passed to the deallocate,  but the user doesn't need to worry about these).
 for example the allocator may do something completely different if it knows it is only working on the same thread ( thread local arena).
 but thats not the focus of the string implementation, its a costumization api nonetheless. 
    
 
 
# Small String Optimization 
 * technicality,  tunable sso is the stack buffer optimization , but both of them have the same outcome, so they have the same name in this documentation. 

The 15 bytes of SSO capacity allows us to not allocate anything for small
strings.

# Copy on Write Optimization

- not available in (`ownerized_string`) .

Allows us to share most of the data, even sharing the substrings, reducing
fragmentation, allocations, and improving performance.
if the user suspected that a peice of code had false sharing ( thread contention on reference count) , we recommend the `ownerize()` method,
it should make the string the owner of the data , note that this does mostly nothing if we are the owner .
note that sharing is not applied in the copy constructor or assignment if the destination buffer ia large enough to hold the data , and is an owner , this is because we dont want to deallocate a hot buffer for no reason.
then we could make the strings with the most contention, force ownerized,using , `always_ownerize(true)` so , no one would ever change their reference count to false share,
this effectively kills cow for things that would suffer from it.

# Built-In String View Optimization

- not available in (`ownerized_string`) .

When initializing a string from a literal, no allocation is performed. For
example, in the following case, we do not allocate, but `std::string` does:

```c++
void fn(std::string str);
void mjz_fn(mjz::string str);

fn("im too long too fit in sso ............"s);
mjz_fn("im too long too fit in sso ............"_str);
```

# Unicode Support

While I haven't made that part in the library, we can easily support Unicode
or any other encoding just by using one of the 8 states of encoding flags (if
they were too small, we could use 1 bits ( `unused_for_now_` ) to add
support for 16 separate encodings, but I don't see any reason for supporting
more than 8 encodings at the same time). Strings with different encodings
may not interact; if they do, that's an error and will throw if you allow it.

# Exception Safety

In my library, almost everything is `noexcept`. I mainly wanted everything to be
testable in `constexpr` mode; therefore, I added a custom error encoding for
making the string an error value. But, if anyone wants exceptions, that's easy
to do with a wrapper (or a different class, but I currently really like
`noexcept`, so I won't do that for now).

# Allocators (Advanced Users Only)

While a generic implementation could allow any allocators, because of the
amount of templates in it, I made my library with an optional `constexpr`
friendly pmr-like allocator. The string would be 8 bytes more with it, but
it's beneficial for some contexts. Everything is `noexcept` in its API, and a
failure is a simple `nullptr` return. The Allocator object (memory resource
like) needs to outlive the string object, its copies, and its views,
i have a safe way of providing such allocators (  the Allocators are refrence counted, which is opt-out, but the benefit is that if a allocator is destroyed in debug mode,  the assertion ensure that no polymorphic refrences are alive , if not , the debug will crash on assertion,  this is better than a security vulnerability,  but , the reference counting is opt out if the user suspected overhead with doing a reference counted allocator).
i have both constexpr friendly memory resources , and a standard pmr adaptor if anyone is interested.

## why not the standard allocator? 

the string class in integrated  with my highly customizable allocator module that allows very precise content over the heap.

 reasons for my polymorphic allocator design,  that gives almost all control of heap management to the implementation of the allocator( the criticism of the standard allocator) :
1. no constexpr support for pmr .
2. cannot tell the Allocator to allocate more than needed, ( the exact size is usually rounded,  but the delta is wasted).
3. cannot grantee thread-safety. 
4. has no notion of owning a block ( chained allocators are very hard).
5. has no idea of the property of the allocated block ( we cant fail fast , we cant say that this is a common size of our nodes , we cant say that we can afford more time for less fragmentation,  ect...).
7. no alignment requirement for `char`.
8. is a template pram,  the types are always different, so much workaround for nonexistent problems ( a memory resource handle throwing exception on move or copy ) .
9. the Allocator type ( template pram) has unpredictable size.
10. compiler cannot Optimize  try and catch away, because they rely on the stack unwinder , but nullptr checks are easy. 

*  all of the above are not present in my allocator.

# Value Semantics

The string is a value type. In my library, all of the move and copy functions
are implemented and are `noexcept`. There's also a third one for const r-values,
but I won't touch on that implementation detail because this is more of a
nice-to-have thing. But as a summary, move moves everything. Share (const r-value)
shares (no alloc) sharables and copies (alloc) the non-sharables based on `(can_share()&&!is_ownerized)`. Copy
does a memcpy (no alloc) if an allocation doesn't occur; if not, calls share.

# future versions:

- the string wrapper is available as an experimental feature, the wrapper accepts a template pram for the properties to ensure ( `sso_cap,has_null,is_ownerized`), the stack buffer with the appropriate length will be provided ( if cap is more than sso), the string would be null terminated if required, and the string would be easily mutable if required, the 3 different string wrappers can be made by using a specific template pram, or a combination of them if needed ( mutable and big sso for example),
`ownerized_string` is that wrapper with `is_ownerized=true`, `c_string` is that wrapper with `has_null=true` , and `implace_string<N>` is that wrapper with  `sso_cap=N` , also , this way , the traits can be combined,  for example an owned implace string with size of 1024, note that the size of the `implace_string<N>` object is around 32+N ( for 15<N, N mod 8 being zero).

i plan to support unicode with another library component that integrates with the byte-string,
currently, i want to implement the rope,
but this is planned.

# The Rope Counterpart

I'm currently designing a semi-immutable post-modern COW and SSO-optimized rope
class based on an (a,b)-tree of slices of this string and its lazy
counterpart, but I haven't still implemented it in the library. It's the ajason
paper in the repository for anyone interested.

# the pure view counterpart ( advanced users):

- similar to string view , we need lifetime insurance with this type.

ill briefly talk about this , as its an important part.
when we need a trivial view type , this is it ,
but when we need safety and can afford 16 bytes more ,
use the main string.
we have this as the view type in the library:

```c++
struct {
size_t  length:59;
size_t  encoding:3;
size_t has_null:1;
size_t  is_static:1;// equivalent to is_sharable,  but in the view world,  this is just a tag.
const char*begin;
};
```

# Usability

- this is a header only library, i currently support gcc and msvc and clang.

All of the `string_view` functionality is supported (an equivalent of it in my
library) because we focused on being like views, we lost the ease of these two
functionalities: mutable iteration + null terminated. We do almost the same
thing for views (`std::string` is like the proxy object I talked about) (the
string view has no `c_str` method), but other than the above, we have
equivalent functionality for `std::string` (by equivalent, I mean if you don't
consider mine being encoding aware).

Any algorithm for a continuous string is usable and implemented (with regard
to its encoding; ASCII is like the standard C implementation).

the mutatable iteration and null terminator requirements are for the wrappers ,
usually its better to  use the wrappers when needed on the fly , and use the main string ( or `implace_string` if beneficial) for storage or passing around. 


* how i think of this :
  the main string is more of a constant string type like a name of an object , or a key into a map,
  but when someone wants to change a string, they need a mutable type ,
  and for changing a string , someone can use a ownerized string with a relevant buffer size ( for example 96 bytes , make a 128 byte string wrapper) ,
  and when the modifications are all performed, the string would be turned back into the main string for storage .
  also , no one says this is how to use it ,
  someone may make a vector of these 128byte strings because they expect many different strings with sizes mostly less than 96.
  for example , the mutatable big sso string could be used as a buffer for user input, then different parts may be stored in different strings and ect .

# Extensions:

in my library, i have a fmt like format library, to generate these strings ,
so , mutable iteration is not a problem for me at all, in my opinion.
my formatting library should also support ropes in a efficient way when they are implemented  
( every section would be like rope sections, and if it get it right, the formatting library would have a copyless output to the rope ( generators and cow sharing ) in cases where its doable )

# my implementation: 
 the implementation of this paper is found in the papers repository,
 my implementation is a demonstration of this paper , 
 im sure the functions could be more performant ,
 if you think there are ways to improve performance, readability or other things ,
 you may do a pull request or issue on that part,
 or you may put this paper as a reference and implement your own string with this layout or design.
 the license is an open source mit license , so this is more of a reminder. 

# where would you place this:

- mutable owner:
  `std::string` vs `mjz::ownerized_string`  
- in between:
  `mjz::string`,continuous. vs `mjz::packed_string` vs `fbstring` ( fbstring is still using cow , so even tho its api is mutable , it may hold an immutable storage)?
  ( maybe rust cow string)
  `mjz::rope`, discontinues. vs ?
  ( maybe other lazy ropes)
- immutable viewer:
  `std::string_view` vs `mjz::string_view`

# Conclusion

With the growing use of string views, there has become an opportunity to get
the best of both worlds. We can use our strings like a string view, get value
semantics, still not copy or allocate, and use a unified type for our strings,
making using the string as a mutable reference easier and reducing the
overhead for functions who need to change the string in certain areas but not
the others. While there's some inherent complexity in this method, this was
the best implemented out of 5 that I made, but this provides a good way to
minimize UB of use-after-free with using a reference-counted string view,
while still benefiting from most of its upsides.

### Note+context:

- because I use msvc as my ide+compiler , i may miss some things about other compilers , so , i'll appreciate if someone points that out.
my two compilers in how.txt:

Win11:
`clang version 20.1.0`

`g++.exe (Rev3, Built by MSYS2 project) 13.2.0`

`Microsoft (R) C/C++ Optimizing Compiler Version 19.43.34809 for x86`


Linux-subsys:

 `clang++ version 20.1.1 (https://github.com/llvm/llvm-project 424c2d9b7e4de40d0804dd374721e6411c27d1d1)`
 
 `g++ (Debian 12.2.0-14) 12.2.0`
 
Also, sorry if the markdown is not professional; I don't have much experience
with it.



This paper is located at:
[https://github.com/Mjz86/String/blob/main/README.md](https://github.com/Mjz86/String/blob/main/README.md)

You may give feedback in:
[https://github.com/Mjz86/String/issues](https://github.com/Mjz86/String/issues)

**References and Inspirations:**
- CppCon 2015： Andrei Alexandrescu “std：：allocator...” :
 [https://www.youtube.com/watch?v=LIb3L4vKZ7U](https://www.youtube.com/watch?v=LIb3L4vKZ7U)
- CppCon 2017： John Lakos “Local ('Arena') Memory Allocators (part 1&2 of 2)” :
 [https://www.youtube.com/watch?v=nZNd5FjSquk](https://www.youtube.com/watch?v=nZNd5FjSquk)
 [https://www.youtube.com/watch?v=CFzuFNSpycI](https://www.youtube.com/watch?v=CFzuFNSpycI)
- legacy cow vs sso in the standard  ( old video):
  [https://youtu.be/OMbwbXZWtDM?si=eeu8WQdb1CuwpxIF](https://youtu.be/OMbwbXZWtDM?si=eeu8WQdb1CuwpxIF)
- All Rust string types explained :
  [https://www.youtube.com/watch?v=CpvzeyzgQdw](https://www.youtube.com/watch?v=CpvzeyzgQdw)
- Learn about the cache:
  [https://www.youtube.com/watch?v=dFIqNZ8VbRY](https://www.youtube.com/watch?v=dFIqNZ8VbRY)
- CppCon 2016： Timur Doumler “Want fast C++？ Know your hardware!＂:
  [https://www.youtube.com/watch?v=BP6NxVxDQIs](https://www.youtube.com/watch?v=BP6NxVxDQIs)
- what does  atomic do to the cache | CppCon 2017： Fedor Pikus “C++ atomics, from basic to advanced.  What do they really do？” :
  [https://www.youtube.com/watch?v=ZQFzMfHIxng](https://www.youtube.com/watch?v=ZQFzMfHIxng)
-  about branch prediction, multiple die systems ,false sharing | Unlocking Modern CPU Power - Next-Gen C++ Optimization Techniques - Fedor G Pikus - C++Now 2024 :
  [https://www.youtube.com/watch?v=wGSSUSeaLgA](https://www.youtube.com/watch?v=wGSSUSeaLgA)

- String view usage:
  [https://www.youtube.com/watch?v=PEvkBmuMIr8](https://www.youtube.com/watch?v=PEvkBmuMIr8)
- `std::basic_string`:
  [https://en.cppreference.com/w/cpp/string/basic_string](https://en.cppreference.com/w/cpp/string/basic_string)
- `std::basic_string_view`:
  [https://en.cppreference.com/w/cpp/string/basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)
- CppCon 2017: Barbara Geller & Ansel Sermersheim “Unicode Strings: Why the
  Implementation Matters”:
  [https://www.youtube.com/watch?v=ysh2B6ZgNXk](https://www.youtube.com/watch?v=ysh2B6ZgNXk)
- CppCon 2016: Nicholas Ormrod “The strange details of std::string at
  Facebook”:
  [https://www.youtube.com/watch?v=kPR8h4-qZdk](https://www.youtube.com/watch?v=kPR8h4-qZdk)
- Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik,
  Morgan McGuire CppCon 22:
  [https://www.youtube.com/watch?v=fglXeSWGVDc](https://www.youtube.com/watch?v=fglXeSWGVDc)
- CppCon 2018: Victor Ciura “Enough string_view to Hang Ourselves”:
  [https://www.youtube.com/watch?v=xwP4YCP_0q0](https://www.youtube.com/watch?v=xwP4YCP_0q0)
- Postmodern immutable data structures:
  [https://www.youtube.com/watch?v=sPhpelUfu8Q](https://www.youtube.com/watch?v=sPhpelUfu8Q)
- strings 1 - Why COW is ungood for std string\strings 1 - Why COW is
  ungood for std string:
  [https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755](https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755)
- lib format :
  [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)

