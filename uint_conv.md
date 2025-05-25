# converting any 8 digit binary number to ascii with very few instructions ( min 1mul , max 4mul ):

based on a very insightful explanation of integer multiplication from https://github.com/jeaiii ,
i found a very fast way to convert large integers ,


note the standard spesifies that, uint64_t multiplication is mod 2^64.


we know that if we have a base 10 number, its :
XXXXXXXX.YYAAAAAAA

0<= 0.YY*100 < 100

so we know that 

0<= 0.YY*100 < 128

so 


0<= 0.YY*100/128 < 1


therefore , *all* of the information of YY is stored in 7 bits past the least significant bit .

but we dont have that , right???


well , we can cheat ,

if we multiply a 64bit number by ceil(2^64/10^p) , its as if we did a full multiply in 128 digits , the cut off the 64 bit upper half,
therfore , all of the bits in the result is past the least significant digit in the imaginary upper half,


in the lower half we have kinda the (value%(10^p))*2^64

and we know that the most significant 7bits store the information  of YY,

we shift by 57 to get the 7bits in place ,
then , we do our final trick,

128 is not a big number , we can have 256bytes to store the lookup .

so we make a lookup for the last ascii value, 
take care of the endian ness,and were done.


because I dont want to burden you with my lib , i made it only depend on std


 here is the code:

 https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp

```c++
constexpr uint64_t inv10p8_64b = 184467440738;
constexpr uint64_t inv10p6_64b = 18446744073710;
constexpr uint64_t inv10p4_64b = 1844674407370956;
constexpr uint64_t inv10p2_64b = 184467440737095517;



    constexpr static uint64_t lookup_iota_8digits_ascii(const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 4> ret{
      std::bit_cast<std::array<uint16_t, 4>>(ascii_offset)};
  if (1000'000 < n) ret[0] = modolo_raidex_table[(n * inv10p8_64b) >> 57];
  if (10000 < n) ret[1] = modolo_raidex_table[(n * inv10p6_64b) >> 57];
  if (100 < n) ret[2] = modolo_raidex_table[(n * inv10p4_64b) >> 57];
  ret[3] = modolo_raidex_table[(n * inv10p2_64b) >> 57];
  return std::bit_cast<uint64_t>(ret);
}
```




