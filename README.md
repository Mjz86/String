#reconsidering COW , a modern c++20 string implementation :
</br>
</br>
</br>
</br>#abstract:
</br> this paper presents a custom string class , designed for memory efficiency.
</br>the paper is mainly focused on the 64bit little endian implementation but  the other platforms also have equivalent functionality.
</br>the string class provides a way to store various encodings in different string types , its  allocator aware and uses the SSO and COW features while adding others with minimal overhead.
</br>the class is as constexpr friendly as the standard string, mostly comparable to the gcc implementation.
</br>
</br>
</br>#introduction:
</br> many c++ programmers have worked with strings , several of them have been forced to use either immutable value semantics with views or reference semantics and allocations for mutable strings , 
</br> this string class is a bridge between these , providing a owning tagged string view as its storage.
</br> 
</br> 
</br>#implementation details:
</br> 
</br> the basic layout can be thought of as the following (not valid c++,  but its equivalent)(total of 32 bytes in my own implementation):
</br>
 ```
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
 ```
</br> the control byte can be thought of as:
```
 struct {
 uint8_t  is_sso:1; //a bit redundant,  but i used it for more safety 
 uint8_t  is_sharable:1; 
 uint8_t unused_for_now_:1;
 uint8_t has_null:1;
 uint8_t  is_threaded:1; 
uint8_t  encoding:3; 
};
```
</br>
</br>
</br>the encoding  flags are for knowing the encoding of the stored data.
</br>
</br>the heap block can be thought of as :

</br>
```
struct {
size_t  reference_count;
char heap_buffer[capacity];
};
```
</br>
</br>
</br>
</br>
</br>#invariants:
</br>
</br>1.at the string view state:
</br>begin!=buffer.
</br>active union member== referencal_t.
</br>begin || not length.
</br>heap_block==nullptr.
</br>capacity==0.
</br>is_owner()==false.
</br>is_sso==false.
</br>(note that is_sharable is true for string litterals or strings that the user "promised" to outlive the object and its accessors)
</br>
</br>2.at the sso string  state:
</br>begin==buffer.
</br>active union member== buffer.
</br>capacity==15.
</br>is_owner()==true.
</br>is_sso==true.
</br>is_sharable==false.
</br>has_null==(length!=15).
</br>
</br>3.heap string state:
</br>begin!=buffer.
</br>active union member== referencal_t.
</br>!!begin .
</br>data_block==&heap_buffer.
</br>capacity!=0.
</br>( capacity  is almost always bigger than 15 but no grantees are made)
</br>is_owner()==(reference_count<2).
</br>is_sso==false.
</br>is_sharable==true.
</br>
</br>
</br>
</br>
</br>4.stack buffer  string state:
</br>begin!=buffer.
</br>active union member== referencal_t.
</br>!!begin .
</br>data_block!=nullptr.
</br>capacity!=0.
</br>is_owner()==true.
</br>is_sso==false.
</br>is_sharable==false.
</br>
</br>
</br>
</br>
</br>also ,
</br>[begin,end) is a continuous sub range of [heap_buffer,heap_buffer+capacity) if and only if heap_buffer is not null and alive.
</br>
</br>
</br>
</br>
</br>#addressing cow and the drawbacks:
</br>1.all of the iterators,  the methods and functions only give a constant references to the data.
</br>2. for mutable iteration over a string , one can call a specilized function to involve a monad,  that gives the data as a continuous temporary( i mean the lifetime of the range is only granteed in the function call)  range .
</br> like this ( we need to know  that *this is not captued in any way ):
</br>temp=move(*this);
</br>temp.ownerize();
</br>change(temp);
</br>*this=move(temp);
</br>
</br>
</br>possible plan for easier mutable iterators would be( its good , but idk if its  intuitive):
```
struct iterator {
size_t i;
mjz::string*str;
// stuff ....
auto& operator=(char ch){
//.... stuff
str->replace_ch(i,ch);
return*this;
}
};
```
</br>
</br>
</br># thread-safety(opt-out):
</br>the string ensures to use atomic operations if is_threaded is true 
</br> for a berif summery , the thread-safety grantees are similar to a shared_ptr of a std string if the flag is true.
</br>
</br>#constexpr compatibility:
</br>the reference_count  variable  is stored as 8 bytes and is bitcasted, ( no thread-safety in constexpr mode because of obvious reasons).
</br>only the static string view can be stored as a static  constant expression value,  but the string is fully functional in a constexpr context.
</br>no reinterpret_cast  is used ( the only time we do use it is for the thread-safe atomic operations which are not constexpr anyways).
</br>a constexpr friendly allocator is provided by the implemented library. 
</br>
</br>
</br>#c interoperability:
</br>the has null flag is used to see if theres a null terminator,  if not , we may add it in the  as_c_str function (non const function , use .data() , .length() and .has_null() and memcpy() if you want a constant alternative ).
</br>
</br>
</br>#features and implementation:
</br>
</br>the built-in string viewer and shared substrings :
</br>the string is accessed via the begin and length pair , they provide the minimal functionality of a string viewer. 
</br>a substring function  may share the underlying data if and only if its sharable. 
</br>
</br>
</br>#mutable strings:
</br>the string manages its resources and can be modified using the other part of the object, 
</br>the functions ensure correct COW semantics and  they allocate when necessary.
</br> almost all equipment functionality of std string can ve supported , except  the following:
</br>  the value of  operator[](i),at(i), *data() , front(),back()  cannot  be mutated.
</br> c_str() does not exist ( as_c_str()  does,  but it can mutate, and it only gives a pointer to const ).
</br>
</br>
</br>#built-in stack buffer optimization( advanced users only):
</br>by using a stack buffer, you ensure that no allocation occurs as long as the buffer is big enough,  if not , allocation may occur.
</br> the users must ensure that the buffer outlives the string object  and the objects that it moved to or a view that it was converted to , but unlike the Allocators,  they dont need to grantee outliving a copy of it.
</br>
</br>
</br>#the optimizations of remove prefix/suffix ,  push back , pop back , push front and pop front:
</br>
</br>because  the begin pointer is not limited to the beginning of the buffer , we can use the view semantics to remove the extra character without memmove.
</br>
</br>we also have 3 mods of the first position alignment of the range:
</br>
</br>centeral buffer  ( begin=buffer+(cap-len)/2)
</br>
</br>front buffer   ( begin=buffer)
</br>
</br>back buffer  ( begin=buffer+cap-len)
</br>
</br>after that , the position may change , but we could  append and prepend  to the range without memmove in many cases if we want.
</br>
</br>
</br>
</br>#small string optimization:
</br>the 15 bytes of sso capacity  allows us to not allocate anything for small strings.
</br>
</br>#copy on write optimization:
</br>allows us to share most of the data , even sharing the substrings , reducing fragmentation , allocations and improving performance.
</br>
</br>#built-in string view optimization:
</br>when initializating a string from a litteral, no allocation is preformed. 
</br>for example in the following case  , we do not allocate but std does:
</br>
</br>void fn(std::string str);
</br>void mjz_fn(mjz::string str);
</br>
</br>fn("im too long too fit in sso ............"s);
</br>mjz_fn("im too long too fit in sso ............"_str);
</br>
</br>
</br>#unicode support:
</br>while i haven't made that part in the library,  we can easily support unicode or any other encoding just by using one of the 8 states of encoding flags ( if they were too small,  we could use 2 bits ( is_sso and the reserved bit) to add support for 32 separate encodings , but i dont see any reason  for supporting mire than 8 encodings at the same  time ).
</br>strings with different encodings may not interact,  if they do , thats an error and will throw if you allow it.
</br>
</br>
</br>#exception safely :
</br>in my library , almost everything is noexcept, i mainly wanted everything to be testable in constexpr mode,  therfore,  i added a custom error encoding  for making the string an error value , but , if anyone wants exceptions , thats easy to do with a wrapper ( or a different class , but i currently really like noexcept,  so i wont do that for now).
</br>
</br>
</br>#allocators( advanced users only) : 
</br> while  a generic implementation could allow any allocators , because of the amount of templates in it , 
</br>i  made my library with a optional constexpr friendly pmr-like allocator  , the string would be 8 bytes more with it , but its beneficial for some contexts.
</br> everything is noexcept in its api and a failure is a simple nullptr return. 
</br> the Allocator object ( memory resource like) needs to outline the string object, its copies and its views.
</br> 
</br># value semantics:
</br>the string is a value type , in my library,  all of the move and copy functions are implemented and are noexcept,  theres also a third one for const r values, but i won't touch on that implementation detail, because  this is more of a nice to have thing.
</br> but as a summary,  
</br> move moves everything.
</br> share (const r value ) shares ( no alloc) sharables and copies(alloc) the non sharables.
</br> copy  dose a memcpy ( no alloc) if  an allocation doesn't occur, if not calls share.
</br>#the rope counterpart:
</br>im currently designing a semi-immutable post modorn COW and SSO optimized rope class based on an (a,b)-tree of  slices of this string and its lazy counterpart , but i havent still implemented it im the library.
</br>hopefully i will present it soon.
</br>
</br>
</br>
</br>#conclusion:
</br>with the growing use of string views , there has become an opportunity to get the best of both worlds ,
</br>we can use our strings like a string view , get value semantics,  still not copy or allocate,  and use a unified type for our strings , making using the string as a mutable reference easier and reducing the overhead for functions who need to change the string in certain areas but not the others.
</br>while there's some inherent complexity in this method,  this was the best implemented out of 5 that i made ,
</br>but this provides a good way to minimize UB of use-after-free with using a reference counted string view , while still benefiting from most of its upsides.
</br>
</br>
</br>
</br>
</br>
</br>
</br>
</br>note:
</br>although I can share my library,  I'm a bit scared to do so , because of how open source products are treated,  
</br> I think I will eventually make it open source,  but not for rn.
</br> 
</br> also , sorry if markdown is not professional,  i don't have much experience with it
</br>
</br>references and inspirations:
</br>
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
</br>
