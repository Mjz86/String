#reconsidering COW , a modern c++20 string implementation :



*abstract:
 this paper presents a custom string class , designed for memory efficiency.
the paper is mainly focused on the 64bit little endian implementation but  the other platforms also have equivalent functionality.
the string class provides a way to store various encodings in different string types , its  allocator aware and uses the SSO and COW features while adding others with minimal overhead.
the class is as constexpr friendly as the standard string, mostly comparable to the gcc implementation.


*introduction:
 many c++ programmers have worked with strings , several of them have been forced to use either immutable value semantics with views or reference semantics and allocations for mutable strings , 
 this string class is a bridge between these , providing a owning tagged string view as its storage.
 
 
 *implementation details:
 
 the basic layout can be thought of as the following (not valid c++,  but its equivalent)(total of 32 bytes):
 
 struct {
 ( allocator reference as empty base);
 const char* begin_ptr;
 size_t  length;
 union{
 char  buffer[15];
 struct referencal_t{
char*data_block;
size_t capacity:56;
};
 };
 uint8_t  control_byte; 
 };
 
 the control byte can be thought of as:
 struct {
 uint8_t  is_sso:1; //a bit redundant,  but i used it for more safety 
 uint8_t  is_sharable:1; 
 uint8_t unused_for_now_:1;
 uint8_t has_null:1;
 uint8_t  is_threaded:1; 
uint8_t  encoding:3; 
};


the encoding  flags are for knowing the encoding of the stored data.

the heap block is :
struct {
size_t  reference_count;
char heap_buffer[capacity];
}




*invariants:

1.at the string view state:
begin!=buffer.
active union member== referencal_t.
begin || not length.
heap_block==nullptr.
capacity==0.
is_owner()==false.
is_sso==false.
(note that is_sharable is true for string litterals or strings that the user "promised" to outlive the object and its accessors)

2.at the sso string  state:
begin==buffer.
active union member== buffer.
capacity==15.
is_owner()==true.
is_sso==true.
is_sharable==false.
has_null==(length!=15).

3.heap string state:
begin!=buffer.
active union member== referencal_t.
!!begin .
data_block==&heap_buffer.
capacity!=0.
( capacity  is almost always bigger than 15 but no grantees are made)
is_owner()==(reference_count<2).
is_sso==false.
is_sharable==true.




4.stack buffer  string state:
begin!=buffer.
active union member== referencal_t.
!!begin .
data_block!=nullptr.
capacity!=0.
is_owner()==true.
is_sso==false.
is_sharable==false.




also ,
[begin,end) is a continuous sub range of [heap_buffer,heap_buffer+capacity) if and only if heap_buffer is not null and alive.




*addressing cow and the drawbacks:
1.all of the iterators,  the methods and functions only give a constant references to the data.
2. for mutable iteration over a string , one can call a specilized function to involve a monad,  that gives the data as a continuous temporary( i mean the lifetime of the range is only granteed in the function call)  range .
 like this ( we need to know  that *this is not captued in any way ):
temp=move(*this);
temp.ownerize();
change(temp);
*this=move(temp);


* thread-safety(opt-out):
the string ensures to use atomic operations if is_threaded is true 


*constexpr compatibility:
the reference_count  variable  is stored as 8 bytes and is bitcasted, ( no thread-safety in constexpr mode because of obvious reasons).
only the static string view can be stored as a static  constant expression value,  but the string is fully functional in a constexpr context.
no reinterpret_cast  is used ( the only time we do use it is for the thread-safe atomic operations which are not constexpr anyways).
a constexpr friendly allocator is provided by the implemented library. 


*c interoperability:
the has null flag is used to see if theres a null terminator,  if not , we may add it in the  as_c_str function (non const function , use .data() , .length() and .has_null() and memcpy() if you want a constant alternative ).


*features and implementation:

the built-in string viewer and shared substrings :
the string is accessed via the begin and length pair , they provide the minimal functionality of a string viewer. 
a substring function  may share the underlying data if and only if its sharable. 


*mutable strings:
the string manages its resources and can be modified using the other part of the object, 
the functions ensure correct COW semantics and  they allocate when necessary.



*built-in stack buffer( advanced users only):
by using a stack buffer, you ensure that no allocation occurs as long as the buffer is big enough,  if not , allocation may occur.



*the optimizations of remove prefix/suffix ,  push back , pop back , push front and pop front:

because  the begin pointer is not limited to the beginning of the buffer , we can use the view semantics to remove the extra character without memmove.

we also have 3 mods of the first position alignment of the range:

centeral buffer  ( begin=buffer+(cap-len)/2)

front buffer   ( begin=buffer)

back buffer  ( begin=buffer+cap-len)

after that , the position may change , but we could  append and prepend  to the range without memmove in many cases if we want.



*small string optimization:
the 15 bytes of sso capacity  allows us to not allocate anything for small strings.

*copy on write:
allows us to share most of the data , even sharing the substrings , reducing fragmentation , allocations and improving performance.

*built-in string view:
when initializating a string from a litteral, no allocation is preformed. 
for example in the following case  , we do not allocate but std does:

void fn(std::string str);
void mjz_fn(mjz::string str);

fn("im too long too fit in sso ............"s);
mjz_fn("im too long too fit in sso ............"_str);


*unicode support:
while i haven't made that part in the library,  we can easily support unicode or any other encoding just by using one of the 8 states of encoding flags ( if they were too small,  we could use 2 bits ( is_sso and the reserved bit) to add support for 32 separate encodings , but i dont see any reason  for supporting mire than 8 encodings at the same  time ).
strings with different encodings may not interact,  if they do , thats an error and will throw if you allow it.


*exception safely :
in my library , almost everything is noexcept, i mainly wanted everything to be testable in constexpr mode,  therfore,  i added a custom error encoding  for making the string an error value , but , if anyone wants exceptions , thats easy to do with a wrapper ( or a different class , but i currently really like noexcept,  so i wont do that for now).



*the rope counterpart:
im currently designing a semi-immutable post modorn COW and SSO optimized rope class based on an (a,b)-tree of  slices of this string and its lazy counterpart , but i havent still implemented it im the library.
hopefully i will present it soon.



*conclusion:
with the growing use of string views , there has become an opportunity to get the best of both worlds ,
we can use our strings like a string view , get value semantics,  still not copy or allocate,  and use a unified type for our strings , making using the string as a mutable reference easier and reducing the overhead for functions who need to change the string in certain areas but not the others.
while there's some inherent complexity in this method,  this was the best implemented out of 5 that i made ,
but this provides a good way to minimize UB of use-after-free with using a reference counted string view , while still benefiting from most of its upsides.







note:
although I can share my library,  I'm a bit scared to do so , because of how open source products are treated,  
 I think I will eventually make it open source,  but not for rn.

references and inspirations:

string view usage:
https://www.youtube.com/watch?v=PEvkBmuMIr8
std::basic_string:
https://en.cppreference.com/w/cpp/string/basic_string
std::basic_string_view:
https://en.cppreference.com/w/cpp/string/basic_string_view
CppCon 2017： Barbara Geller & Ansel Sermersheim “Unicode Strings： Why the Implementation Matters” :
https://www.youtube.com/watch?v=ysh2B6ZgNXk
CppCon 2016： Nicholas Ormrod “The strange details of std：：string at Facebook＂ :
https://www.youtube.com/watch?v=kPR8h4-qZdk
Optimizing A String Class for Computer Graphics in Cpp - Zander Majercik,  Morgan McGuire  CppCon 22 :
https://www.youtube.com/watch?v=fglXeSWGVDc
CppCon 2018： Victor Ciura “Enough string_view to Hang Ourselves” :
https://www.youtube.com/watch?v=xwP4YCP_0q0
Postmodern immutable data structures :
https://www.youtube.com/watch?v=sPhpelUfu8Q
