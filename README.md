

# Reconsidering COW, a Modern C++20 String Implementation

**tl;dr:**

Less lifetime management (dangling views) while maintaining view (and value)
semantics and performance characteristics (no copying or allocation).  And
planned Unicode support.

# Abstract

This paper presents a custom string class designed for memory efficiency.
The paper is mainly focused on the 64-bit little-endian implementation, but other platforms also have equivalent functionality. The string class provides a way to store various encodings in different string types. It's allocator-aware and uses SSO and COW features while adding others with minimal overhead. The class is as `constexpr` friendly as the standard string, mostly comparable to the GCC implementation.

# Introduction

Many C++ programmers have worked with strings. Several have been forced to use either immutable value semantics with views or reference semantics and allocations for mutable strings. This string class bridges these, providing an owning tagged string view as its storage.

# Implementation Details

The basic layout can be thought of as the following (not valid C++, but its equivalent) (total of 32 bytes in my own implementation):

```c++
struct {
  (allocator reference as empty base);
  const char* begin_ptr;
  size_t  length;
  union{
    char  buffer[15];
    struct referencal_t{
      char* data_block;
      size_t capacity:56;
    };
  };
  uint8_t  control_byte;
};
```

The control byte can be thought of as:

```c++
struct {
  uint8_t  is_sso:1; // a bit redundant, but I used it for more safety
  uint8_t  is_sharable:1;
  uint8_t unused_for_now_:1;
  uint8_t has_null:1;
  uint8_t  is_threaded:1;
  uint8_t  encoding:3;
};
```

The encoding flags are for knowing the encoding of the stored data.

The heap block can be thought of as:

```c++
struct {
  size_t  reference_count;
  char heap_buffer[capacity];
};
```

# Invariants

1.  **At the string view state:**

    *   `begin != buffer`.
    *   `active union member == referencal_t`.
    *   `begin != nullptr || 0 == length`.
    *   `data_block == nullptr`.
    *   `capacity == 0`.
    *   `is_owner() == false`.
    *   `is_sso == false`.
    *   (Note that `is_sharable` is true for string literals or strings that the
        user "promised" to outlive the object and its accessors)

2.  **At the SSO string state:**

    *   `begin == buffer`.
    *   `active union member == buffer`.
    *   `capacity == 15`.
    *   `is_owner() == true`.
    *   `is_sso == true`.
    *   `is_sharable == false`.
    *   `has_null == (length != 15)`.

3.  **Heap string state:**

    *   `begin != buffer`.
    *   `active union member == referencal_t`.
    *   `begin != nullptr`.
    *   `data_block == &heap_buffer`.
    *   `capacity != 0`.
    *   (`capacity` is almost always bigger than 15, but no guarantees are made)
    *   `is_owner() == (reference_count < 2)`.
    *   `is_sso == false`.
    *   `is_sharable == true`.

4.  **Stack buffer string state:**

    *   `begin != buffer`.
    *   `active union member == referencal_t`.
    *   `begin != nullptr`.
    *   `data_block != nullptr`.
    *   `capacity != 0`.
    *   `is_owner() == true`.
    *   `is_sso == false`.
    *   `is_sharable == false`.

Also, `[begin, end)` is a continuous sub-range of
`[heap_buffer, heap_buffer + capacity)` if and only if `heap_buffer` is not
null and alive.

# Addressing COW and the Drawbacks

1.  All of the iterators, the methods, and functions only give constant
    references to the data.

2.  For mutable iteration over a string, one can call a specialized function
    to involve a monad that gives the data as a continuous temporary range (I mean
    the lifetime of the range is only guaranteed in the function call).
    Like this (we need to know that `*this` is not captured in any way):

    ```c++
    temp = move(*this);
    temp.ownerize();
    change(temp);
    *this = move(temp);
    ```

Possible plan for easier mutable iterators would be (it's good, but I don't know if it's
intuitive; I didn't do it; it seems too hacky):

```c++
struct iterator {
  size_t i;
  mjz::string* str;
  // stuff ....
  auto& operator=(char ch) {
    //.... stuff
    str->replace_ch(i, ch);
    return *this;
  }
};
```

Another possible solution that I think will be the best bet is a proxy string, basically this (haven't tried yet):

```c++
// as a friend class to the string
struct proxy {
private:
  //....
  mjz::string value;
  //copy and move are deleted.
  //  the only way to get the value back out is by moving it out with a
  //  function.
  // and the value is always the owner of the mutable string.
  // we can get the usual mutable string interface.
};
```

# How to do This Without My Help

The string design presented may be complex for readers to implement on their
own, but they can make it easily if they don't care about the object size or
indirections and allocation strategy (I needed the object size to be small,
but if not):

```c++
struct simpler_version{
  std::variant <std::shared_ptr<std::string> // cow
                ,std::array<char,16>         // sso
                ,std::span<char>             // stack buffer
                > data;
  std::string_view view;
  encodings_t encoding;
  bool has_null;
  // functionality....
};
```

# Thread-Safety (Opt-Out)

The string ensures to use atomic operations if `is_threaded` is true. For a
brief summary, the thread-safety grantees are similar to a `shared_ptr` of a
`std::string` if the flag is true.

# Constexpr Compatibility

The `reference_count` variable is stored as 8 bytes and is bitcasted (no
thread-safety in `constexpr` mode because of obvious reasons). Only the static
string view can be stored as a static constant expression value, but the
string is fully functional in a `constexpr` context. No `reinterpret_cast` is
used (the only time we do use it is for the thread-safe atomic operations
which are not `constexpr` anyways). A `constexpr` friendly allocator is provided
by the implemented library.

# C Interoperability

The `has_null` flag is used to see if there's a null terminator. If not, we may
add it in the `as_c_str` function (non-const function, use `.data()`,
`.length()`, `.has_null()`, and `memcpy()` if you want a constant
alternative). Also, calling the copy (or share) constructor and using
`as_c_str` on the l-value you made is effectively a const alternative with the
same overhead (if we assume that stack buffer was not used on the first part).

# Features and Implementation

The built-in string viewer and shared substrings: the string is accessed via
the begin and length pair, they provide the minimal functionality of a string
viewer. A substring function may share the underlying data if and only if it's
sharable.

# Mutable Strings

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
note that the `data()` function  will have output a null terminated `const char*` if and only if `has_null()`.
also , if you called `as_c_str()` without failing,  you know that `has_null()` is true.
when we have `const` `data()`, i see no need for `const` `c_str()`

# COW Overhead

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

# Built-In Stack Buffer Optimization (Advanced Users Only)

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

* we may provide a safe wrapper class that has a bigger sso buffer , while also reusing all the code of the string , think of it like an implace vector , this wouldn't need lifetime knowledge,  so it would be for intermediate users .

# The Optimizations of Remove Prefix/Suffix, Push Back, Pop Back, Push Front and Pop Front

Because the begin pointer is not limited to the beginning of the buffer, we
can use the view semantics to remove the extra character without memmove.

We also have 3 mods of the first position alignment of the range:

*   Central buffer  (`begin = buffer + (cap - len) / 2`)
*   Front buffer   (`begin = buffer`)
*   Back buffer  (`begin = buffer + cap - len`)

After that, the position may change, but we could append and prepend to the
range without memmove in many cases if we want to.

# Small String Optimization

The 15 bytes of SSO capacity allows us to not allocate anything for small
strings.

# Copy on Write Optimization

Allows us to share most of the data, even sharing the substrings, reducing
fragmentation, allocations, and improving performance.

# Built-In String View Optimization

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
they were too small, we could use 2 bits ( `is_sso` and the reserved bit) to add
support for 32 separate encodings, but I don't see any reason for supporting
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
like) needs to outlive the string object, its copies, and its views.
i have both constexpr friendly memory resources , and a standard pmr adaptor if anyone is interested. 

# Value Semantics

The string is a value type. In my library, all of the move and copy functions
are implemented and are `noexcept`. There's also a third one for const r-values,
but I won't touch on that implementation detail because this is more of a
nice-to-have thing. But as a summary, move moves everything. Share (const r-value)
shares (no alloc) sharables and copies (alloc) the non-sharables. Copy
does a memcpy (no alloc) if an allocation doesn't occur; if not, calls share.

# The Rope Counterpart

I'm currently designing a semi-immutable post-modern COW and SSO-optimized rope
class based on an (a,b)-tree of slices of this string and its lazy
counterpart, but I haven't still implemented it in the library. It's the ajason
paper in the repository for anyone interested.

# Usability

All of the `string_view` functionality is supported (an equivalent of it in my
library) because we focused on being like views, we lost the ease of these two
functionalities: mutable iteration + null terminated. We do almost the same
thing for views (`std::string` is like the proxy object I talked about) (the
string view has no `c_str` method), but other than the above, we have
equivalent functionality for `std::string` (by equivalent, I mean if you don't
consider mine being encoding aware).

Any algorithm for a continuous string is usable and implemented (with regard
to its encoding; ASCII is like the standard C implementation).

As I said, we know that this is an implementation to be in-between of view and
string, so this is an acceptable tradeoff. If you want to complain about
mutable iteration, I don't think you needed a viewing type in the first place,
and do you think the operation that you want to do can use the proxy? It's
just 2 move operations and a reference count check if you owned it before, and
if not, you would have copied anyways. After than that, you get a `char*` as
your iterator type with all of the normal string functionality (potentially
null terminated if requested). If you want to complain about null terminators,
I think you either need to be comfortable with C APIs (explicit work with
`memcpy`) or rethink your design. When the standard string is allowed to have
intermediate null terminators, I think it would be a bug to require null
termination (see the talk on Folly's string implementation), and again, we did
put `as_c_str`, so I think there's no valid complaints.

# Extensions:
in my library,  i have a fmt like format library,  to generate these strings , 
so , mutable iteration is not a problem for me at all, in my opinion. 
my formatting library should also support ropes in a efficient way when they are implemented  
( every section would be like rope sections, and if it get it right,  the formatting library would have a copyless output to the rope ( generators and cow sharing ) in cases where its doable )
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

**Note:**

Although I can share my library, I'm a bit scared to do so because of how
open-source products are treated. I think I will eventually make it open
source, but not for rn.

Also, sorry if the markdown is not professional; I don't have much experience
with it.

This paper is located at:
[https://github.com/Mjz86/String_description/blob/main/README.md](https://github.com/Mjz86/String_description/blob/main/README.md)

You may give feedback in:
[https://github.com/Mjz86/String_description/issues](https://github.com/Mjz86/String_description/issues)

**References and Inspirations:**

*   Learn about the cache:
    [https://www.youtube.com/watch?v=dFIqNZ8VbRY](https://www.youtube.com/watch?v=dFIqNZ8VbRY)
*   String view usage:
    [https://www.youtube.com/watch?v=PEvkBmuMIr8](https://www.youtube.com/watch?v=PEvkBmuMIr8)
*   `std::basic_string`:
    [https://en.cppreference.com/w/cpp/string/basic_string](https://en.cppreference.com/w/cpp/string/basic_string)
*   `std::basic_string_view`:
    [https://en.cppreference.com/w/cpp/string/basic_string_view](https://en.cppreference.com/w/cpp/string/basic_string_view)
*   CppCon 2017: Barbara Geller & Ansel Sermersheim “Unicode Strings: Why the
    Implementation Matters”:
    [https://www.youtube.com/watch?v=ysh2B6ZgNXk](https://www.youtube.com/watch?v=ysh2B6ZgNXk)
*   CppCon 2016: Nicholas Ormrod “The strange details of std::string at
    Facebook”:
    [https://www.youtube.com/watch?v=kPR8h4-qZdk](https://www.youtube.com/watch?v=kPR8h4-qZdk)
*   Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik,
    Morgan McGuire CppCon 22:
    [https://www.youtube.com/watch?v=fglXeSWGVDc](https://www.youtube.com/watch?v=fglXeSWGVDc)
*   CppCon 2018: Victor Ciura “Enough string_view to Hang Ourselves”:
    [https://www.youtube.com/watch?v=xwP4YCP_0q0](https://www.youtube.com/watch?v=xwP4YCP_0q0)
*   Postmodern immutable data structures:
    [https://www.youtube.com/watch?v=sPhpelUfu8Q](https://www.youtube.com/watch?v=sPhpelUfu8Q)
*   strings 1 - Why COW is ungood for std string\strings 1 - Why COW is
    ungood for std string:
    [https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755](https://gist.github.com/alf-p-steinbach/c53794c3711eb74e7558bb514204e755)
* lib format :
   [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)
