# converting any 8 digit binary number to ascii with very few instructions (  max 4mul , min of none  ):

first , we multiply by 2^56/10^p ,(p being 2*floor(logi(x)/2))
this makes sure that if the number was in decimal , it would only be bwtwwn 0 and 99



so we have a decimal between 100
XX.YYYYYYYYYYYYYYYYYYYYYYYYYYY

and we know that XX es information is only stored in the most significant 7bits (56+7=64) and all the YYes are in the low 56 bits



we shift by 57 to get the 7bits in place ,
and get the first 2 digits in a lookup,
 we set the numbers 

 and for the next round we zero out the 7 highs and then  multiply by 100 to get to a similar situation as before 
 ( this can be generalized to more than 4 rounds, but the first multiply is hard to generalize because of big integers)
 
because I dont want to burden you with my lib , i made it only depend on std


 here is the code:

 https://github.com/Mjz86/String/blob/main/mjz_lib/mjz_uintconv.hpp



 ```c++
constexpr static uint64_t lookup_iota_8digits_ascii(const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 4> ret{
      std::bit_cast<std::array<uint16_t, 4>>(ascii_offset)};
  constexpr uint64_t inv10p2xi_b57[4]{1ull << 57, 1441151880758559,
                                      14411518807586, 144115188076};
  const int mul_n = int(1000000 < n) + int(10000 < n) + int(100 < n);
  constexpr uint64_t mask = uint64_t(-1)>>7;
  uint64_t val{n};
  if (!mul_n) {
    ret[3] = modolo_raidex_table[val];
    return std::bit_cast<uint64_t>(ret);
  }
  val *= inv10p2xi_b57[mul_n];

  if (mul_n == 3) {
    ret[0] = modolo_raidex_table[(val) >> 57];
    val &= mask;
    val *= 100;
  }

  if (2 <= mul_n) {
    ret[1] = modolo_raidex_table[(val) >> 57];
    val &= mask;
    val *= 100;
  }
  if (mul_n) {
    ret[2] = modolo_raidex_table[(val) >> 57];
    val &= mask;
    val *= 100;
  }
  ret[3] = modolo_raidex_table[(val) >> 57];
  return std::bit_cast<uint64_t>(ret);
}


```
