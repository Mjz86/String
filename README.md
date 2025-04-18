
# A Modern C++20 String Implementation

**tl;dr:**

Less lifetime management (dangling views) while maintaining view (and value)
semantics and performance characteristics (no copying or allocation), also , the template properties allow for tuning the sso size to any desired size , and all of them are compatible with each other. And
planned Unicode support.

# Abstract

This paper presents a custom string class designed for memory efficiency.
The paper is mainly focused on the 64-bit little-endian implementation, but other platforms also have equivalent functionality. The string class provides a way to store various encodings in different string types. It's allocator-aware and uses SSO and COW features while adding others with minimal overhead. The class is as `constexpr` friendly as the standard string, mostly comparable to the clang implementation.

# Introduction

Many C++ programmers have worked with strings. Several have been forced to use either immutable value semantics with views or reference semantics and allocations for mutable strings. This string class bridges these, providing an owning tagged string view as its storage.

# Implementation Details

note that in the 32bit platform, there are two options,
one is big address space ( 56 bit) and one is small address space ( 24bit) , in the small case , the object size is 16bytes and in the large case 32bytes.

The basic layout can be thought of as the following (not valid C++, but its equivalent) (total of 32 bytes in my own implementation)( this is not padded , its a diagram ! ):

```c++
struct properties_t {
  uintlen_t sso_min_cap{};// sso_cap is more than or equal to sso_min_cap
  bool has_alloc{};// the pmr allocator is a private base if and only if has_alloc. 
  // else , we use new and aligned new and their delete counterpart, note that the user can overload new and delete,  so the user must make the overload thread-safe if they work in multithreaded environments.
  bool has_null{};//  c_str exists if and only if has_null
  bool is_ownerized{};// mutable iteration and mutable index access ( like operator[]) exists if and only if is_ownerized 
  //the string doesn't NOT use cow if its ownerized 
  
  // note that the thread-safey guarantees are like fbstring when this is true
  std::optional<bool> is_threaded{};
enum class align_direction_e : char {
  relaxed = 0,
  center = 1,
  front = 2,
  back = 3
}align;
};
// template pram 
<properties>// determine if dome grantees can be made to improve performance because of edge case reduction,  and to add some api components like `c_str` or mutatable iteration. 
struct {
  (allocator reference as empty base);
   uint8_t  control_byte;
  union{
    char  buffer[sso_cap+sizeof(sso_len_t)];// the last bytes hold the length information,  they use the fbstring trick of storing  capacity minus length,  so the string will always be null terminated. 
    struct referencal_t{// the string is a reference to something
      char /*size_t*/ capacity[7]/*:56*/;// in actuality only 7 bytes
      char* data_block;
      const char* begin_ptr;
      size_t is_sharable : 1 ;
      ( always true if properties is null terminated) 
      size_t has_null : 1 ;// needed to share substrings
      size_t length : sizeof(size_t) * 8 - 2 ;// this  indicates  that we are in a heap or litteral view , vs , sso or stack buffer or copying view.

    };
  };
};
```

The control byte can be thought of as:

```c++
struct {
uint8_t is_sso:1;
  ( const and always enabled/disabled if properties are strictly that way)
uint8_t  /*the negation of this is actually stored*/is_threaded:1;
 
 ( const and always true if properties is ownerized)
 uint8_t is_ownerized:1;   /*this flag is not propagated by copy or share, only by move,in the non-move case,  if any side (dest or src) has this flag set to true, a memcpy and a potential allocation occurs (if sso or stack buffer is not large enough) */
 // to always disable cow and viewer for a specific string ,
 // to remove the reference_count checks,
 //controled  with  always_ownerize(bool flag_state),
 //  (can_share()&&!is_ownerized) determines sharability
 // both of these flags are essential and they do not correlate.

  uint8_t  encoding:5; // we are not a bag of bytes
};
```

The encoding flags are for knowing the encoding of the stored data.

The heap block can be thought of as:

```c++
union heap_alloc_t /* not a type , just a layout mapped to the allocated character block, this is a diagram */{
struct can_share_true_t{
size_t  reference_count;
char padding [is_threaded?std::hardware_destructive_interference_size-8:0];
 alignas(is_threaded?std::hardware_destructive_interference_size :8 )  char heap_buffer[capacity];
} cow_heap;
struct can_share_false_t{
//assertions of is_threaded for picking this layout. 
alignas(std::hardware_destructive_interference_size )  char heap_buffer[capacity];
}owned_heap;
};
```

# Invariants

1.  **At the string view state:**

    - `!is_sso `.
    - `active union member == referencal_t`.
    - `begin != nullptr || 0 == length`.
    - `data_block == nullptr`.
    - `capacity == 0`.
    - `is_owner() == false`.
    - `is_ownerized == false`.
      the view optimization is essentially a cow string with static lifetime, therfore its not ownerized.

2.  **At the SSO string state:**

    - `is_sso `.
    - `active union member == buffer`.
    - `capacity == sso_cap`.
    - `is_owner() == true`.
    - `is_sharable == false`.
    - `has_null == true`.
     the following uses bitcast in constexpr mode, and `reinterpret_cast` in runtime ( for performance, its safe because all the subobjects are `char` types):
    - `length ==sso_cap-*(sso_len_t*)(&buffer[sso_cap])`
    
3.  **Heap string state:**

    - `!is_sso `.
    - `active union member == referencal_t`.
    - `begin != nullptr`.
    - `data_block == &heap_buffer`. ( the flags show if the padding andor reference count exist or not).
    - `capacity != 0`.
    - (`capacity` is almost always bigger than `sso_cap`, but no guarantees are made ( because its not necessary) )
    - `is_owner() ==(is_ownerized ||  !can_share() ||(reference_count < 2))`.
    - ` !is_ownerized || (!can_share() || reference_count < 2)`.
    - `is_sharable == true`.
    - if `is_threaded ` then `sizeof(heap_alloc_t)==can_share()?capacity+std::hardware_destructive_interference_size:capacity ` and `alignof(heap_alloc_t)=std::hardware_destructive_interference_size`.
    else , `sizeof(heap_alloc_t)==capacity+8`, and `alignof(heap_alloc_t)=8`.
    
   - `can_share() == !is_threaded ||  256bytes( or the configured threashold) < capacity `
   

4.  **Stack buffer string state:**

    - `!is_sso `.
    - `active union member == referencal_t`.
    - `begin != nullptr`.
    - `data_block != nullptr`.
    - `capacity != 0`.
    - `is_owner() == true`.
    - `is_sharable == false`.

Also, `[begin, end)` is a continuous sub-range of
`[data_block, data_block + capacity)` if and only if `data_block` is not
null and alive.




# How to do This Without My Help

The string design presented may be complex for readers to implement on their
own, but they can make it easily if they don't care about the object size or
indirections and allocation strategy.
i will not make the design simpler,  except if it helps with performance,  
as far as i saw , the performance of the simpler design was bad .
 (I needed the object size to be small,
but if not):

```c++
struct simpler_version{
  std::variant <std::shared_ptr<std::string>,// cow
  std::string ,// the less than cow threashold 
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


# C Interoperability
use the properties  for null terminated strings.
this will add the `c_str` function.
but be careful,  because  some optimizations may be disabled ( substring without memmove may not work).

# tunable sso , runtime and compile time:

## the tunable sso parameter with minimal overhead:

the string object size is about `sso_cap+2` ,
the minimum object size is 32bytes with sso capacity of 30bytes.
in my opinion,  this is enough for most use cases, 
but you may tune it if you want.
just  don't make the string object too large.


## stack buffer optimization ( runtime sso tuning) ( advanced users):
- use the  tunable sso  feature for a better quality of life , it has more safety.
- if code bloat of the main tunable sso is a concern , then use the `implace_string`.
- if the stack buffer is not big enough,  unlike in C , we do not overflow,  but we allocate on the heap , this ensures safety that C did not have in its stack buffers ( char arrays in C are kinda just stack buffers) 
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

### what is the type of the owner?  ( standard and custom string compatibility outside of the mjz library)( next experimental release)( another wrapper):
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
- standard allocator?  why not :
similar  to the previous section , a string without an allocator can be wrapped with a normal allocator, 
and the resulting string wouldn't do cow ( because cow is only supported on my allocator api) , but it would have all functionality of the string with that allocator.
this way , almost all continuous  characters containers can be supported as mediums of storage ,
this type would represent all of them with ease .

### tunable sso but without code bloat(`implace_string<props,N>`):
like the above sections,  we may use a standard array and wrap it with a wrapper, 
and use the stack buffer optimization for it .
this will make the stack buffer the "sso" storage ,
and no allocation would be made.
this is safe raii wrapper that ensures that the stack buffer is managed without user mental overhead. 

# easy conversions :
you may convert from any mjz string with any properties to any other properties, 
usually the conversions are cheap , but if some requirements were placed,  some of the conversions may allocate .
this allows for use of any desired  string in any place.

but i personally recommend to only make some 2 or 3 using statements , to notake too many template instantiations.

# COW Overhead

- can be turned off for a specific string, or by using the ownerized properties. 

Other than the destruction and construction, which may need a branch to the
non-view path if the optimizer doesn't realize triviality, the string const
overhead is similar to a view, which is fairly minimal. We can easily make a view
out of this, no branching required. The only time where the overhead is felt is in modification of the
string content. I tried my best to get the modification function as efficient
as possible, but in these worse cases, I can't do much else: the string was
cold, the reference count was cold or has sharing (contention when
modification trashes the cache line) (heap strings), the data was cold, the
data needed deallocating or reallocation.

But at last, any function that is marked const doesn't even think about the
storage location or strategy nor lifetime; it's as if it was a view all along.


the following  is for the times were null termination is not a property:
And the ones that are not marked const are the ones who need to know about
other stuff. Also, there are some functions that are not const
(`remove_suffix` or `prefix`, `as_substring`) that only address the SSO part
and treat the other parts as views; these don't even need to know about COW nor
ownership.

# branchless access:
 with some branchless programing,  the string code got much more predictable to execute. 
  a string to view conversion is done without any branding, 
  most functions are written in a way to use more `if constexpr` for redundant branches, 
  especially those who access outside the object. 
  
  i have drastically reduce the branches, 
  its bare minimum.
  and yes , i do check the generated binary with ghidra.
  
  
  

# Mutable Strings
 
 - if the string is not ownerized and is not null terminated:
 
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
  
 -for null terminated strings , we recommend the null termination property
  
- for mutatable strings we recommend  ownerized property. 


# The Optimizations of Remove Prefix/Suffix, Push Back, Pop Back, Push Front and Pop Front

- note that the null terminator requirement makes this harder for the  null terminated case, mainly , sharing substings will break cow more with null terminator requirements.

Because the begin pointer is not limited to the beginning of the buffer, we
can use the view semantics to remove the extra character without memmove.

We also have 3 mods of the first position alignment of the range:

- Central buffer (`begin = buffer + (cap - len) / 2`) ( Amortized logarithmic push back or front)
- Front buffer (`begin = buffer`) ( push front is O(n) , push back is O(1) Amortized)
- Back buffer (`begin = buffer +max( 1, cap - len )-1`)(push back is O(n) , push front is O(1) Amortized)

After that, the position may change, but we could append and prepend to the
range without memmove in many cases if we want to.



# Addressing COW and thread-safety and false sharing:

* if the properties are ownerized,  cow is disabled ( but the heap layout is unchanged to allow for heap abi compatibility between  template specilizations ).

- in the non thread-safe  :

case  , the heap always has a reference reference count to the back of it .

- in the thread-safe case :


like fbstring,  we have a cow threashold of 256bytes( or the configured threashold).

 if the buffer size is less than 256bytes( or the configured threashold) , no reference count is added ,
 the heap block is aligned to 64bytes and is eagerly copied if needed.
 
 in the heap block is more than the threshold,  a 64byte ( a cache line )with the reference count is added. 
 
 this ensures that we are not doing false sharing , but be careful,  because  aligned allocation may be more costly.
  the thread-safety grantees apply to the non thread-safe ownerized string as well , so , that may be more suitable if you see that thread-safety mode is causing bottlenecks in performance  .

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




# sharing substings , and easy substing operations :

if null termination is not required,  these optimizations become relatively easy and O(1).

if cow is enabled,  we can share substrings, meaning  that the  substring function is as fast as pointer arithmetic. 

if a string has been called with `remove_suffix`, `remove_prefix` the removal is as easy as a pointer arithmetic. 


# Built-In String viewer:

if the string is not ownerized,  then we will just put any string litteral with any length into the view state, 
saving the allocations from litterals that didn't go into the sso.

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
or any other encoding just by using one of the 32 states of encoding flags ( I don't see any reason for supporting
more than 32 encodings at the same time). Strings with different encodings
may not interact; if they do, that's an error and will throw if you allow it.

```c++

// IMPORTANT : the byte-string API works with byte-indexies, 
// for using it , you must use correct offsets , 
// i recomend making wrapper strings/iterators/ranges for it , to make it easier

enum class encodings_e : uint8_t {
  bytes,
  latin1 = bytes,
  ascii = bytes,
  utf8,// UTF-8 
  utf16_le,// little endian UTF-16 
  utf16_be,// big endian UTF-16
  utf32_le,// little  endian UTF-32
  utf32_be,// big endian UTF-32
  ////////////25 user-specified encodings ////////////////
  usr_0,
  usr_1,
  usr_2,
  usr_3,
  usr_4,
  usr_5,
  usr_6,
  usr_7,
  usr_8,
  usr_9,
  usr_10,
  usr_11,
  usr_12,
  usr_13,
  usr_14,
  usr_15,
  usr_16,
  usr_17,
  usr_18,
  usr_19,
  usr_20,
  usr_21,
  usr_22,
  usr_23,
  usr_24,  
  //--the string error type--//
  err_bytes,
  err_latin1 = err_bytes,
  err_ascii = err_bytes,  // we have only 5 bits for the encoding
};
```

- you may use another  unicode library that knows how to work with ranges, till i see how i could do this

- my curant issue is that the speed of the algorithms is important and there are many diffrent variations of them,  and they usually involve big tables and need to be up-to-date with the latest Unicode standard,  but also , they need a constexpr path (  without simd) for constexpr support,  and another runtime path ( potentially simd  ) for performance,
i will put some links to relevant talks and refrences.

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

- reasons for my polymorphic allocator design,  that gives almost all control of heap management to the implementation of the allocator( the criticism of the standard allocator) :
1. no constexpr support for pmr .
2. cannot tell the Allocator to allocate more than needed, ( the exact size is usually rounded,  but the delta is wasted).
3. cannot grantee thread-safety. 
4. has no notion of owning a block ( chained allocators are very hard).
5. has no idea of the property of the allocated block ( we cant fail fast , we cant say that this is a common size of our nodes , we cant say that we can afford more time for less fragmentation,  ect...).
7. no alignment requirement for `char`.
8. is a template pram,  the types are always different, so much workaround for nonexistent problems ( a memory resource handle throwing exception on move or copy ) .
9. the Allocator type ( template pram) has unpredictable size.
10. compiler cannot Optimize  try and catch away, because they rely on the stack unwinder , but nullptr checks are easy. 
11. my library cant throw exceptions in some platforms that i use it in ( esp32 , ect) , ( platform doesn't matter for the code tho, only endian-ness ). 


**important note**:
 the cow threashold can be configured in the Allocator virtual table at runtime , but custom Allocators must be used for configuration.
 the  normal threashold  i am talking about can be configured with a global constexpr variable at compile time , but i use 256 as the default,  and talk in that way ,
 just know that this is also configurable,  because its the  Allocators ( on numa , on areana ...ect) that know what is best threashold  for their implementation, 
 but a note to consider is that an allocator object cant change that after its creation ar runtime ( the vtable construction will determine that , and the vtable is a constant  member as the base class). 
 ( i font use the virtual keyword,  i made  the virtual table to be an object , and its just as good as the standard pmr way , with the upside of more configuration at vtable creation).
 
 
*  all of the above are not present in my allocator.

 - downsides:
 1. less inlining. 
 2. virtual function call cost
 3. Allocators  that have reference counting enabled have more contention in their thread safe mode.
 4. Allocators without reference counting  need to manage lifetimes


 * is virtual function call that bad?
 i refer you to  John Lakos “Local ('Arena') Memory Allocators":
 " compated to the global allocator,  the virtual call overhead is just noise".
 

# Value Semantics

The string is a value type. In my library, all of the move and copy functions
are implemented and are `noexcept`. There's also a third one for const r-values,
but I won't touch on that implementation detail because this is more of a
nice-to-have thing. But as a summary, move moves everything. Share (const r-value)
shares (no alloc) sharables and copies (alloc) the non-sharables based on `(can_share()&&!is_ownerized)`. Copy
does a memcpy (no alloc) if an allocation doesn't occur; if not, calls share.




# Constexpr Compatibility

The `reference_count` variable is stored as 8 bytes and is bitcasted (no
thread-safety in `constexpr` mode because of obvious reasons). Only the static
string view and sso strings can be stored as a static constant expression value, but the
string is fully functional in a `constexpr` context. A `constexpr` friendly allocator is provided



 
 
# Small String Optimization 
The minimum 30 bytes of SSO capacity allows us to not allocate anything for small
strings.
our minimum object size is also a common size (32) among strings.

this is very efficient as most strings bellow the 30 byte limit will only introduce fragmentation to the Allocator.


- is bigger better?
 * when the minimum is good enough:
we  say bigger is not a need , but could be better.
 32 is a nice size for our object. 
 * when string probability distribution of length (pdf ) is showing otherwise:
 we can always increase to 38bytes (40byte object size),
 or to 46 bytes (48byte size) 
 or to 54 bytes (56byte size) 
 or to 62 bytes (64byte size)
  or.....
  or to 1021 bytes (1024byte)
  but in that case ( bigger than 126), 
  i would consider if the word "small" in the name has lost meaning or not.
   but if you need it ( constexpr string needed to fit it to be a static constant expression ) , then use it with care.
  
# Copy on Write Optimization

- not available in ownerized string. 

Allows us to share most of the data, even sharing the substrings, reducing
fragmentation, allocations, and improving performance.
most of its downsides has been mitigated , and is similar to fbstring in many cases.
i also chose my threashold to be equal to that of the fbstring,  
because fbstring is a proven implementation and a valuable inspiration fro this implementation. 

if the user suspected that a peice of code had false sharing ( thread contention on reference count) , we recommend the `ownerize()` method,
it should make the string the owner of the data , note that this does mostly nothing if we are the owner .
note that sharing is not applied in the copy constructor or assignment if the destination buffer ia large enough to hold the data , and is an owner , this is because we dont want to deallocate a hot buffer for no reason.
then we could make the strings with the most contention, force ownerized,using , `always_ownerize(true)` so , no one would ever change their reference count to false share,
this effectively kills cow for things that would suffer from it.

- as a summary, of benefits of cow+viewer vs ownerized:

    *   Reduced Memory Usage:  
    COW can significantly reduce memory usage when multiple strings share the same underlying buffer, the string viewer can also be very cheap to pass around. 
    
   * Reduced Fragmentation: 
   even in the shared thread-safe case, because the string data( text) is never modified,  it would be very fast to access,  and the l3 cache would almost always contain it  if it was accessed by another thread, 
   and because its const , the cache would remain valid.
   
   *   Faster Copy and substring  Operations:  
     Copying/substringing  a COW string involves only copying a pointer and incrementing a reference count, which is much faster than copying the entire string buffer.
 
    * not that bad on thread-safe mode:
     with the mitigations in place,  most of the time , the thread-safe version would avoid the overhead of reference counting. 
     and when the overhead becomes comparable to allocation,  ( threashold  ) then it uses cow.
     
     * no false sharing , minimal  true sharing:
      in non thread-safe mode , there is no worry, 
      in thread-safe mode , either we never shared , or we only shared only the reference count. 

     * usually we dont need thread-safety: 
      how many programs have you seen that needed more than a handful of shared strings among threads? 
      and if you did, could you call a function ( `always_ownerize`) on that specific string construction?  
      im sure you can. 
    
     * you can always turn it off:
     you may use `always_ownerize` if you think you need it ,
     but in many cases you dont.
     
    * not in the "cow is dead" mindset:
    have you taken a look at the awful use of `shared_ptr` just because they can?
    have you thought about why   `shared_ptr` is praised  but cow is hated?
    are you sure that you really care about cow performance beyond 256bytes( or the configured threashold), when copyin them would take longer? 
    im pretty sure `shared_ptr` is worse than cow , because its both shared and mutable , and is not padded when needed. 
    before you talk about the overhead of cow , and say how good the ability to turn it off is , just see,
    is there some specific positions of complain,
    or just saying "cow bad" ,"cow slow" without thinking again?
    thats why its optional , because you must *measure*.

     * rope integration:
     the planned rope datastructure is very efficient, 
     its  constant time amortized for many operations,
     and the key to its success is sharing the substrings. 

    * const is what i need:
       most of the time , a non mutable api is what we need.
       
    * you can always pick the best threashold ( even at runtime when you create the allocator):
       by using the configuration options, 
       there are many ways to pick a threashold other than 256.
       this way , you may even use cow in numa systems, 
       but with big threasholds like 4kb or something else, 
       its entirely up to you.
       the only thing is that after an allocator object got created, you cant configure more .
       but this is not much of a limitation,  it is seen in the standard that memory resource objects only configure at creation. 
       
    * a carefully chosen threashold will improve performance in many cases:
       cow is not used when its not optimal, 
       synchronization and reference counting is not even presented below the threashold, 
       cost of these optimizations tends to be large , but when they are not preformed,  they eont cost anything, 
       especially because most branches are predictable,  and in the case that  cow is used  ,
       the length of the string would already have been too large to begin with , and the memcpy alone would cost more than some cache misses. 
       the key here is to choose the best threashold for your specific allocator. 


  
    
- why you might choose ownerized:
 
     * mutable api :
      
       we have a string,  therfore we own and can modify it.
    
  * we wont ever trash any chaches:
  
      we might need a bigger and fragmented memory, 
      but we never trashed the cache of the other core.
 
   * more inlining in trivial cases:
     
      no code for reference counting makes inlining easier. 
 
   * i work on numa(multi-die, non uniform memory architecture)  systems :
    
       if this is your point , i would consider watching the "Unlocking Modern CPU Power - Next-Gen C++ Optimization Techniques - Fedor G Pikus - C++Now 2024 ", 
       and also , see if you can use only a single die , usually a single die is good enough,  and also , if not , you can always use `always_ownerize ` on runtime. 
       also see if the threashold can be adjusted to  your personal needs.

# future versions:

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
size_t  length:64-7;
size_t  encoding:5;
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

the mutatable iteration and null terminator requirements are for the other properties ,
usually its better to  use the wrappers when needed on the fly , and use the main string ( or `implace_string` if beneficial) for storage or passing around. 


* how i think of this :
  the main string is more of a constant string type like a name of an object , or a key into a map,
  but when someone wants to change a string, they need a mutable type ,
  and for changing a string , someone can use a ownerized string with a relevant buffer size ( for example 126 bytes , make a 128 byte string ) ,
  and when the modifications are all performed, the string would be turned back into the main string for storage .
  also , no one says this is how to use it ,
  someone may make a vector of these 128byte strings because they expect many different strings with sizes mostly less than 126.
  for example , the mutatable big sso string could be used as a buffer for user input, then different parts may be stored in different strings and ect .



## the benefits of this design and the shortcomings:

 - designed for resources constrained environments .
 - very low fragmentation and high memory efficiency when configured correctly. 
 - having cow when a large portion of string operations are read-only.
 - disabling cow when its not beneficial. 
 - not being a bag of bytes. 
 - all of the functions and implementations are noexcept , the allocator are not allowed to throw exceptions.
 - having the ability to make the most optimal decisions on the fly.
 - being extensible. 
 - the library is header only and works on 3 major compilers.
 - the Allocators are generally the best it can get before going into heavy template land.
 - my design is for modorn hardware, but its also easy to use on embedded platforms ( my esp32 project).
 - very big sso ( 30 is a big size for sso).
 - can use stack buffers if the user can manage it.
 - (for those in cpp : not written in rust)
 - will eventually get unicode support .
 - will eventually get rope counterpart. 
 - very few allocations in common single threaded use.
 - tunable sso.
 -  no null termination requirement unless specified.
 - the `implace_string<N>` ( stack buffer wrapper that takes care of  lifetime management of its stack buffer member( RAII)) is a safe alternative to `char[N]` in C, because it will never overflow,  and its lifetime is granteed, also it has a string interface. 
 - no `strlen` is used in the code ,to make a string,  the length amd encoding must be specified,  this is to ensure safety,  the litteral operators `u8""_str` and `""_str` help to make the code easy and safe for most use cases.
 - Branchless access to all string information.
 - constexpr friendly and noexcept, all functionality is completely constexpr friendly and noexcept ( excluding the friend shift operators of standard `cin`, and `cout`)
  - cow threshold can be used at runtime to configure the amout of copying the string should do in multithreaded environments. 
 
 
 
 
 - the code is "hard to read".
 - not a standard component.   
 - currently not popular.
 - if used without cow , many of the template code is not used but parsed by the compiler ( the implementation uses `if constexpr`  to reduce unnecessary code gen).
 - not the simplest string in the world. 
 - (for those in rust : not written in rust)
 - another string library to consider or manage. 


 -  just something to point out :
 i will not reduce complexity unless its provable to be faster ,
 if you want a simple design,  go use the standard string.


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

  * please look at the following video ("Let's Talk Open Source - Prime Reacts ") before doing a pull request,  a issue or any other massage:
  
   [https://www.youtube.com/watch?v=fXqX2S6s3rU](https://www.youtube.com/watch?v=fXqX2S6s3rU)
   
   [https://youtube.com/watch?v=SQ0mBnJmd6I&si=eFCaaEIP0xvqpiWJ](https://youtube.com/watch?v=SQ0mBnJmd6I&si=eFCaaEIP0xvqpiWJ)

   [https://www.youtube.com/watch?v=H9GkSSWbw4c](https://www.youtube.com/watch?v=H9GkSSWbw4c)

## why another string, arent you tired of new strings? 

well, first of all,  you have a valid point ,
if you are comfortable with a certain piece of code ,
then use it.
 but to awnser the question directly, 
 i made this and its pervious versions , mostly because i  was interested ,
 and because the arduino string in the esp32 that i used was not feeling like a cpp string,  it felt like c .
 my other motivation was the fact that i want to have my code run at compile time ,
 but most tools arent designed that way.
 i am currently trying to make a noexcept and constexpr alternative library, 
 this is an important part of it.
 
 ## my fmt-like  formatting library ( similar to standard format ):
  i made a noexcept  constexpr friendly formatting library, to work with these strings, 
  it feels very convenient,  its also more customizable in my opinion. 
  it currently lacks unicode support,  but is encoding aware ( but non ascii is currently an encoding mismatch error )
  
 ## abi stability :
almost all types have a version number in their typeinfo ,
if you want to use a newer version of the library,  update the version id macro , to use the newer version without subtle undetectable mismatches,  
i would try to make most of the types without version information in their type  stable , 
if you see any issues with the type information,  you could notify me,
but know that two incompatible version ids will not link in the linker.

 # my default recommendation( usually good enough) :
   i recommend to use the following  as default strings to go to:

 - mutable(=owerized, without cow)  string with relaxed thread-safety ( nullopt) ,  without null terminator,   with 30byte sso , and no allocator , and relaxed direction alignment .
 
  this is for changing the string like normal.  
 
 - immutable (= with cow ) string with relaxed thread-safety ( nullopt) ,  without null terminator,   with 30byte sso , and no allocator and relaxed direction alignment .
 
 this is like a string slice in rust , or a string view ,
 but the flexibility of storing this without being bound by its source is good.

- and in cases where you know you dont need more : pure string view.

- the thread-safe cow threashold being 256( like fbstring).
 
 
 ### why is this my recommendation?
 because usually this is good enough. 
 but if you suspected some problems with allocation or contention, 
 then i recommend the use of the custom Allocators , or just use ownerized. 
 usually the 30bytes of sso and `implace_string` is sufficient for most allocation bottlenecks.  
 
 
 
# where would you place this:

- mutable owner:
  `std::string` vs `mjz::string`  ( disabled cow)
- in between:
  `mjz::string`,continuous.  vs `fbstring` ( fbstring is still using cow , so even tho its api is mutable , it may hold an immutable storage)?
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
the best implemented out of 6 that I made, but this provides a good way to
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

- Branchless Programming in C++ - Fedor Pikus - CppCon 2021 :

[https://youtube.com/watch?v=g-WPhYREFjk&si=9c_haYLVkI0GOcr_](https://youtube.com/watch?v=g-WPhYREFjk&si=9c_haYLVkI0GOcr_)

-Taking a Byte Out of C++ - Avoiding Punning by Starting Lifetimes - Robert Leahy - CppCon 2022:

[https://www.youtube.com/watch?v=pbkQG09grFw](https://www.youtube.com/watch?v=pbkQG09grFw)

- CppCon 2017： P. McKenney, M. Michael & M. Wong “Is Parallel Programming still hard？ PART 1&2 of 2” :

[https://www.youtube.com/watch?v=YM8Xy6oKVQg](https://www.youtube.com/watch?v=YM8Xy6oKVQg)

[https://www.youtube.com/watch?v=74QjNwYAJ7M](https://www.youtube.com/watch?v=74QjNwYAJ7M)

-Non-Uniform Memory Architecture (NUMA)： A Nearly Unfathomable Morass of Arcana - Fedor Pikus  CppNow :

[https://www.youtube.com/watch?v=f0ZKBusa4CI](https://www.youtube.com/watch?v=f0ZKBusa4CI)

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

- FBString:
  
 [https://github.com/facebook/folly/blob/main/folly/docs/FBString.md](https://github.com/facebook/folly/blob/main/folly/docs/FBString.md)

- lib format :
  
  [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)

- Ghidra Software Reverse Engineering Framework:

[https://github.com/NationalSecurityAgency/ghidra](https://github.com/NationalSecurityAgency/ghidra)

 - [Catch ⬆️： Unicode for C++23 - JeanHeyd Meneide - CppCon 2019](https://www.youtube.com/watch?v=BdUipluIf1E)
 
 - [C++Now 2018： Z. Laine “Boost.Text： Fixing std：：string, and Adding Unicode to Standard C++ (part 1)” ](https://www.youtube.com/watch?v=944GjKxwMBo) 
 -  [C++Now 2018： Z. Laine “Boost.Text： Fixing std：：string, and Adding Unicode to Standard C++ (part 2)” ](https://www.youtube.com/watch?v=GJ2xMAqCZL8)
 
-[Unicode - going down the rabbit hole - Peter Bindels](https://www.youtube.com/watch?v=IXNIqThaSs8)
 
-[A Crash Course in Unicode for C++ Developers - Steve Downey - CppNow 2021](https://www.youtube.com/watch?v=iQWtiYNK3kQ)
 
 - [Applying the Lessons of std：：ranges to Unicode in the C++ Standard Library - Zach Laine  CppNow 2023](https://www.youtube.com/watch?v=AoLl_ZZqyOk)
 
 - [Unicode in C++  CppCon ](https://www.youtube.com/watch?v=MW884pluTw8)

- [Unicode in C++ - James McNellis - Meeting C++ 2016 ](https://www.youtube.com/watch?v=tOHnXt3Ycfo)
 
 - [CppCon 2014： James McNellis ＂Unicode in C++](https://www.youtube.com/watch?v=n0GK-9f4dl8)
 
 - [Unicode： Going Down the Rabbit Hole - Peter Bindels - CppCon 2019](https://www.youtube.com/watch?v=SMSmKg1nApM)
 
 
 
 
 
