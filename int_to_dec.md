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


let   `W=dec_width(abs(n))+ (n<=0)`

at first :

for 8<W:
we devide by `10^8` and let the rest of the fraction for the next round with W=W-8 , this is makes the next rounds easier to parallelize for the cpu while keeping the multiply count small ( most W-es are less than 8 anyway) .

for W=1,W=2:
shortcut by lookup or add to end.

for odd W:

devide by  `l0^(W-1)` (lookup) ,

then we get the  value after the decimal point , and turn it into ascii by an add , then we only let the rest remain


for even W :


devide by  `l0^(W-2)` (lookup),

then we get the  value after the decimal point , and turn it into ascii by a lookup , then we only let the rest remain


we continue  by :

  we know that we have an even number of digits left , so we multipky by 100 at each round , extracting 2 digitds by lookup , then we only let the rest of the fraction to remain,





  this way , the numer of multiplis is less , 5W/8 + 1 approximately





# refrences :
https://github.com/Mjz86/String/blob/main/uint_conv_old.md

https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp

https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/charconv.h 


