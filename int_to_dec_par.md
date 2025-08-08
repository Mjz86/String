# converting integral types to strings:


# final length? 
 lets imagine the destination to be base 2 for a second , the number of characters would be `std::bit_width(abs(n))+ (n<=0)` ( for the sign , and 0->0 )

 so for that in base 10, we need a dec_width to replace bit width, defined as :
 
 `floor(log10(x))+1`
 
 or if x=0, 0

so its `floor(log10(x))+1=floor(log2(x)/log2(10))+1`  and for devition we use a trick of aproximation to ceil(128*log10(2))=39 , and this is enough,

we already have log2 by `bit_width` so , we just do some math,

thereis a problem with this, 
the awnser is somtimes off by 1, so we do a final check by a lookup and we have our result.

# calculation?

note :
by devide i mean a multiply (with the trick from https://github.com/Mjz86/String/blob/main/uint_conv_old.md),

first, thw how:

we can prove  that  for every fixed  point number ( the digits after the point have negetive powers of 2) F=AAAAAAAAAAAA.XYBBBBBBBBBBBB
if we do :
F2=(ceil(128F)&127)
we get :
F2= 128*(0.XYCCCCCCC)
and  we can extract XY  by a lookup table of 128 enteries.



so , with the above  in mind  we do :

look up :`MI=2^K*128/pow(10,-2*i)` .( with a large enough K so that the deviation by multiply would be valid) 

FI=`127&((n*MI+2^k-1)>>k)` ( same as the ceil function )
and we lookup FI. 
( this operates on pairs of digits , but for odd cases , one of the pairs is discarded)

and if you notice, FI has no dependency on F(I-1) , so it can be parralelized.


the generation of the lookup and multiply needed my big and handy uintN_t type , and its similar to lib devide , but its written by me.




# refrences :
https://github.com/ridiculousfish/libdivide

https://github.com/Mjz86/String/blob/main/uint_conv_old.md

// also the math header , because i needed the uint192_t from there for the devition lib
https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp

https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/charconv.h 


