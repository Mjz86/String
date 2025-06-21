
# converting a base2 integer to a base10 string


# introduction:
 i made a way to convert 8digit ingeters with only 6  64bit-integer multiplications , without a lookup.


# details:
 if we look at most compiler outputs (in ghidra) for devitions , we see a patter of :
```
const idk_t factor =ceil((1<<N)/k);

 X/k  as (X*factor) >>N
```

this gave me an idea , 

what if we use the smallest posible N to make it faster?

 i got to desmos and found some pretty good values :


 such that 
`floor((fllor(X)*ceil((2^N)/k)) /2^N) == floor(X/k)`

i wanted  the devition for powers of 10 ( or at leat 5) , 
and surprisingly i found very good ones:


`52(x/2)/256` for numbers less than 100 to divide by 10.


`2622*(x/4)/65536` for numbers less than 10000 to divide by 100.



and i got an idea , what if we devide all of them in a single multiply instruction,

this may sound crazy , but its possible ,

if we want to do a multiply of 8 bit integers , 16 bits is enough  for getting the overflow,

so we can put 4 16bit integers side by side in a 64bit one , and multiply that , 
we get it as if we did each of them separately  ,

this is valid for 2 32 muls as well


in ascii '0'|n is 'n' .

so we can now write the code 
:
(by making them signed (  it wont overflow because upper bit of ascii is 0, so its always positove ) , the compiler can pull more triks)

(Note that 

10x10<128
and
100x100<2^16

Ascii char is positive 

So
No overflow and safe)

the implementation is here:
https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp

if std::experimental::simd is provided, a full branchless version using simd is available



