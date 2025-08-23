/*MIT License

Copyright (c) 2025 Mjz86

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/* has no depenance on my headers for portability */
#ifndef MJZ_UINTCONV_LIB_HPP_FILE_
#define MJZ_UINTCONV_LIB_HPP_FILE_

#ifndef MJZ_string_lib_macros_
#include <stdint.h>

#include <array>
#include <bit>
#include <concepts>
#include <cstring>
#include <span>
#include <string>
#include <utility>

#undef MJZ_JUST_FORCED_INLINE_
#undef MJZ_JUST_UNREACHABLE_
#if 1 < _MSC_VER
#define MJZ_JUST_FORCED_INLINE_ __forceinline
#define MJZ_JUST_UNREACHABLE_() __assume(false)
#elif defined(__GNUC__)
#define MJZ_JUST_FORCED_INLINE_ __attribute__((always_inline))
#define MJZ_JUST_UNREACHABLE_() __builtin_unreachable()
#else
#define MJZ_JUST_FORCED_INLINE_ inline
#define MJZ_JUST_UNREACHABLE_()
#endif
#else
#endif
namespace mjz {
inline namespace uint_to_ascci_ns0 {
namespace details_ns {

 constexpr inline std::array<uint64_t, 9> inv_p10_b57 = []() noexcept {
  std::array<uint64_t, 9> ret{};
  uint64_t p10_{1};
  for (size_t i{}; i < 9; i++) {
    ret[i] = ((uint64_t(1) << 57) / p10_) + !!((uint64_t(1) << 57) % p10_);
    p10_ *= 10;
  }
  return ret;
}(); /*
 constexpr   inline std::array<uint64_t, 21> floor10_table = []() noexcept
 { std::array<uint64_t, 21> ret{}; ret[0] = 1; for (size_t i{1}; i < 20; i++) {
     ret[i] = ret[i - 1] * 10;
   }// coudnt show max pow 10 , but based on the   (64*39/128)==19 we know that
 this is correct ret[20] = uint64_t(-1); return ret;
 }(); */
  constexpr inline const auto floor10_table =
    std::array<uint64_t, 21>{1ull,
                             10ull,
                             100ull,
                             1000ull,
                             10000ull,
                             100000ull,
                             1000000ull,
                             10000000ull,
                             100000000ull,
                             1000000000ull,
                             10000000000ull,
                             100000000000ull,
                             1000000000000ull,
                             10000000000000ull,
                             100000000000000ull,
                             1000000000000000ull,
                             10000000000000000ull,
                             100000000000000000ull,
                             1000000000000000000ull,
                             10000000000000000000ull,
                             uint64_t(-1)};

constexpr std::span<const uint64_t, 10> pow_ten_table =
    std::span(floor10_table).subspan<1, 10>();

  constexpr inline std::array<uint16_t, 101> radix_ascii_p2_v_ =
    std::bit_cast<std::array<uint16_t, 101>>(
        std::array<char, 202>{"0001020304050607080910111213141516171819"
                              "2021222324252627282930313233343536373839"
                              "4041424344454647484950515253545556575859"
                              "6061626364656667686970717273747576777879"
                              "8081828384858687888990919293949596979899\0"});

struct alignas(16) fallback_u128_type_t_ {
  uint64_t low{}, high{};
  [[maybe_unused]] constexpr inline fallback_u128_type_t_() noexcept {}
  [[maybe_unused]] constexpr inline fallback_u128_type_t_(auto i) noexcept
      : low{uint64_t(i)}, high{(~uint64_t(i < 0)) + 1} {}

  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator*(
      const fallback_u128_type_t_ lhs,
      const fallback_u128_type_t_ rhs) noexcept {
    // https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/uint128_t.h
    const uint64_t x = lhs.low;
    const uint64_t y = rhs.low;
    const uint64_t ah = rhs.high;
    const uint64_t th = lhs.high;
    const uint64_t xl = x & 0xffffffff;
    const uint64_t xh = x >> 32;
    const uint64_t yl = y & 0xffffffff;
    const uint64_t yh = y >> 32;
    const uint64_t ll = xl * yl;
    const uint64_t lh = xl * yh;
    const uint64_t hl = xh * yl;
    const uint64_t hh = xh * yh;
    const uint64_t m = (ll >> 32) + lh + (hl & 0xffffffff);
    const uint64_t l = (ll & 0xffffffff) | (m << 32);
    const uint64_t h = (m >> 32) + (hl >> 32) + hh;
    const uint64_t mh = h + x * ah + th * y;
    const uint64_t ml = l;
    fallback_u128_type_t_ ret{};
    ret.low = ml;
    ret.high = mh;
    return ret;
  }
  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator>>(
      fallback_u128_type_t_ lhs, const int rhs) noexcept {
    const uint8_t tiny_shift = uint8_t(rhs & 63);
    lhs.low >>= tiny_shift;
    lhs.low |= tiny_shift ? (lhs.high << (64 - tiny_shift)) : uint64_t();
    lhs.high >>= tiny_shift;
    lhs.low = rhs & 64 ? lhs.high : lhs.low;
    lhs.high = rhs & 64 ? 0 : lhs.high;
    return lhs;
  }
  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator<<(
      fallback_u128_type_t_ lhs, const int rhs) noexcept {
    const uint8_t tiny_shift = uint8_t(rhs & 63);
    lhs.high <<= tiny_shift;
    lhs.high |= tiny_shift ? (lhs.low >> (64 - tiny_shift)) : uint64_t();
    lhs.low <<= tiny_shift;
    lhs.high = rhs & 64 ? lhs.low : lhs.high;
    lhs.low = rhs & 64 ? 0 : lhs.low;
    return lhs;
  }

  [[maybe_unused]] constexpr inline operator uint64_t() const noexcept {
    return low;
  }

  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator&(
      fallback_u128_type_t_ lhs, const fallback_u128_type_t_ rhs) noexcept {
    lhs.high &= rhs.high;
    lhs.low &= rhs.low;
    return lhs;
  }
  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator|(
      fallback_u128_type_t_ lhs, const fallback_u128_type_t_ rhs) noexcept {
    lhs.high |= rhs.high;
    lhs.low |= rhs.low;
    return lhs;
  }
  [[maybe_unused]] constexpr inline friend fallback_u128_type_t_ operator+(
      fallback_u128_type_t_ lhs, const fallback_u128_type_t_ rhs) noexcept {
    fallback_u128_type_t_ ret{};
    ret.low = lhs.low + rhs.low;
    ret.high = lhs.high + rhs.high;
    ret.high += lhs.low > uint64_t(-1) - rhs.low;
    return ret;
  }
};
template <class>
struct inv10p_i_table_t_ {
  /*

  generated by

  for (uintlen_t j{4}; j < 8;j++) {
  uintlen_t n = 1;
n <<= j;
  uintN_t<version_v, 256> t(10);
  for (uintlen_t i{1}; i < 21; i++) {
    uintN_t<version_v, 256> val{1};
    val <<= n ;
    auto temp = val.to_modulo_ret_devide(t);
    val = temp + uintN_t<version_v, 256>(!!val);
    temp = val * t;
    temp -= (uintN_t<version_v, 256>(1) << (n));
    // k-D =M

    println("uintN_t<version_v, 128>({},{})  {} {} \n\n    accuracy {} bits
modulo100 and {} bits div \n\n\n\n"_fmt, val.nth_word(0), val.nth_word(1),j,i,
intlen_t(n  - temp.bit_width()), intlen_t(n  - t.bit_width() +
t.countr_zero())); t *= uintN_t<version_v, 256>(10);
  }
}
println("========================"_fmt);
}












  uintN_t<version_v, 128>(6554,0)  4 1

  accuracy 13 bits modulo100 and 13 bits div




uintN_t<version_v, 128>(656,0)  4 2

  accuracy 9 bits modulo100 and 11 bits div




uintN_t<version_v, 128>(66,0)  4 3

  accuracy 7 bits modulo100 and 9 bits div




uintN_t<version_v, 128>(7,0)  4 4

  accuracy 3 bits modulo100 and 6 bits div




uintN_t<version_v, 128>(1,0)  4 5

  accuracy 0 bits modulo100 and 4 bits div




uintN_t<version_v, 128>(1,0)  4 6

  accuracy -4 bits modulo100 and 2 bits div




uintN_t<version_v, 128>(1,0)  4 7

  accuracy -8 bits modulo100 and -1 bits div




uintN_t<version_v, 128>(1,0)  4 8

  accuracy -11 bits modulo100 and -3 bits div




uintN_t<version_v, 128>(1,0)  4 9

  accuracy -14 bits modulo100 and -5 bits div




uintN_t<version_v, 128>(1,0)  4 10

  accuracy -18 bits modulo100 and -8 bits div




uintN_t<version_v, 128>(1,0)  4 11

  accuracy -21 bits modulo100 and -10 bits div




uintN_t<version_v, 128>(1,0)  4 12

  accuracy -24 bits modulo100 and -12 bits div




uintN_t<version_v, 128>(1,0)  4 13

  accuracy -28 bits modulo100 and -15 bits div




uintN_t<version_v, 128>(1,0)  4 14

  accuracy -31 bits modulo100 and -17 bits div




uintN_t<version_v, 128>(1,0)  4 15

  accuracy -34 bits modulo100 and -19 bits div




uintN_t<version_v, 128>(1,0)  4 16

  accuracy -38 bits modulo100 and -22 bits div




uintN_t<version_v, 128>(1,0)  4 17

  accuracy -41 bits modulo100 and -24 bits div




uintN_t<version_v, 128>(1,0)  4 18

  accuracy -44 bits modulo100 and -26 bits div




uintN_t<version_v, 128>(1,0)  4 19

  accuracy -48 bits modulo100 and -29 bits div




uintN_t<version_v, 128>(1,0)  4 20

  accuracy -51 bits modulo100 and -31 bits div




uintN_t<version_v, 128>(429496730,0)  5 1

  accuracy 29 bits modulo100 and 29 bits div




uintN_t<version_v, 128>(42949673,0)  5 2

  accuracy 29 bits modulo100 and 27 bits div




uintN_t<version_v, 128>(4294968,0)  5 3

  accuracy 22 bits modulo100 and 25 bits div




uintN_t<version_v, 128>(429497,0)  5 4

  accuracy 20 bits modulo100 and 22 bits div




uintN_t<version_v, 128>(42950,0)  5 5

  accuracy 17 bits modulo100 and 20 bits div




uintN_t<version_v, 128>(4295,0)  5 6

  accuracy 17 bits modulo100 and 18 bits div




uintN_t<version_v, 128>(430,0)  5 7

  accuracy 9 bits modulo100 and 15 bits div




uintN_t<version_v, 128>(43,0)  5 8

  accuracy 9 bits modulo100 and 13 bits div




uintN_t<version_v, 128>(5,0)  5 9

  accuracy 2 bits modulo100 and 11 bits div




uintN_t<version_v, 128>(1,0)  5 10

  accuracy -1 bits modulo100 and 8 bits div




uintN_t<version_v, 128>(1,0)  5 11

  accuracy -5 bits modulo100 and 6 bits div




uintN_t<version_v, 128>(1,0)  5 12

  accuracy -8 bits modulo100 and 4 bits div




uintN_t<version_v, 128>(1,0)  5 13

  accuracy -12 bits modulo100 and 1 bits div




uintN_t<version_v, 128>(1,0)  5 14

  accuracy -15 bits modulo100 and -1 bits div




uintN_t<version_v, 128>(1,0)  5 15

  accuracy -18 bits modulo100 and -3 bits div




uintN_t<version_v, 128>(1,0)  5 16

  accuracy -22 bits modulo100 and -6 bits div




uintN_t<version_v, 128>(1,0)  5 17

  accuracy -25 bits modulo100 and -8 bits div




uintN_t<version_v, 128>(1,0)  5 18

  accuracy -28 bits modulo100 and -10 bits div




uintN_t<version_v, 128>(1,0)  5 19

  accuracy -32 bits modulo100 and -13 bits div




uintN_t<version_v, 128>(1,0)  5 20

  accuracy -35 bits modulo100 and -15 bits div




uintN_t<version_v, 128>(1844674407370955162,0)  6 1

  accuracy 61 bits modulo100 and 61 bits div




uintN_t<version_v, 128>(184467440737095517,0)  6 2

  accuracy 57 bits modulo100 and 59 bits div




uintN_t<version_v, 128>(18446744073709552,0)  6 3

  accuracy 55 bits modulo100 and 57 bits div




uintN_t<version_v, 128>(1844674407370956,0)  6 4

  accuracy 50 bits modulo100 and 54 bits div




uintN_t<version_v, 128>(184467440737096,0)  6 5

  accuracy 48 bits modulo100 and 52 bits div




uintN_t<version_v, 128>(18446744073710,0)  6 6

  accuracy 45 bits modulo100 and 50 bits div




uintN_t<version_v, 128>(1844674407371,0)  6 7

  accuracy 45 bits modulo100 and 47 bits div




uintN_t<version_v, 128>(184467440738,0)  6 8

  accuracy 37 bits modulo100 and 45 bits div




uintN_t<version_v, 128>(18446744074,0)  6 9

  accuracy 35 bits modulo100 and 43 bits div




uintN_t<version_v, 128>(1844674408,0)  6 10

  accuracy 31 bits modulo100 and 40 bits div




uintN_t<version_v, 128>(184467441,0)  6 11

  accuracy 29 bits modulo100 and 38 bits div




uintN_t<version_v, 128>(18446745,0)  6 12

  accuracy 24 bits modulo100 and 36 bits div




uintN_t<version_v, 128>(1844675,0)  6 13

  accuracy 21 bits modulo100 and 33 bits div




uintN_t<version_v, 128>(184468,0)  6 14

  accuracy 18 bits modulo100 and 31 bits div




uintN_t<version_v, 128>(18447,0)  6 15

  accuracy 16 bits modulo100 and 29 bits div




uintN_t<version_v, 128>(1845,0)  6 16

  accuracy 12 bits modulo100 and 26 bits div




uintN_t<version_v, 128>(185,0)  6 17

  accuracy 8 bits modulo100 and 24 bits div




uintN_t<version_v, 128>(19,0)  6 18

  accuracy 5 bits modulo100 and 22 bits div




uintN_t<version_v, 128>(2,0)  6 19

  accuracy 3 bits modulo100 and 19 bits div




uintN_t<version_v, 128>(1,0)  6 20

  accuracy -3 bits modulo100 and 17 bits div




uintN_t<version_v, 128>(11068046444225730970,1844674407370955161)  7 1

  accuracy 125 bits modulo100 and 125 bits div




uintN_t<version_v, 128>(2951479051793528259,184467440737095516)  7 2

  accuracy 122 bits modulo100 and 123 bits div




uintN_t<version_v, 128>(11363194349405083796,18446744073709551)  7 3

  accuracy 118 bits modulo100 and 121 bits div




uintN_t<version_v, 128>(2980993842311463542,1844674407370955)  7 4

  accuracy 114 bits modulo100 and 118 bits div




uintN_t<version_v, 128>(9521471421085922163,184467440737095)  7 5

  accuracy 111 bits modulo100 and 116 bits div




uintN_t<version_v, 128>(10175519178963368025,18446744073709)  7 6

  accuracy 108 bits modulo100 and 114 bits div




uintN_t<version_v, 128>(17619621584234933257,1844674407370)  7 7

  accuracy 107 bits modulo100 and 111 bits div




uintN_t<version_v, 128>(1761962158423493326,184467440737)  7 8

  accuracy 103 bits modulo100 and 109 bits div




uintN_t<version_v, 128>(13088917067439035464,18446744073)  7 9

  accuracy 100 bits modulo100 and 107 bits div




uintN_t<version_v, 128>(6842914928856769032,1844674407)  7 10

  accuracy 95 bits modulo100 and 104 bits div




uintN_t<version_v, 128>(13597012344482363035,184467440)  7 11

  accuracy 92 bits modulo100 and 102 bits div




uintN_t<version_v, 128>(1359701234448236304,18446744)  7 12

  accuracy 88 bits modulo100 and 100 bits div




uintN_t<version_v, 128>(7514667752928644277,1844674)  7 13

  accuracy 86 bits modulo100 and 97 bits div




uintN_t<version_v, 128>(8130164404776685075,184467)  7 14

  accuracy 81 bits modulo100 and 95 bits div




uintN_t<version_v, 128>(13725737292074354639,18446)  7 15

  accuracy 79 bits modulo100 and 93 bits div




uintN_t<version_v, 128>(12440620173433166434,1844)  7 16

  accuracy 75 bits modulo100 and 90 bits div




uintN_t<version_v, 128>(8622759646827137290,184)  7 17

  accuracy 73 bits modulo100 and 88 bits div




uintN_t<version_v, 128>(8240973594166534376,18)  7 18

  accuracy 68 bits modulo100 and 86 bits div




uintN_t<version_v, 128>(15581492618384294731,1)  7 19

  accuracy 65 bits modulo100 and 83 bits div




uintN_t<version_v, 128>(3402823669209384635,0)  7 20

  accuracy 63 bits modulo100 and 81 bits div








  these are ceil(2^(2N)/pow10i) , ignore the ones with negative accuracy or
below N



100*((mul_result +(((N-8)<<1)-1))>>(N-8));

for the last digits(accuracy is not enuogh):

1 digit:
num/pow(10,dec_width(max)-1)
or
2 digit:
num/pow(10,dec_width(max)-2)



by :

constexpr auto ivk =
    get_devide_inverse_and_shift<>(uint64_t(-1), 1000000000000000000, {});


ikv= {{{{{2820903858849102351Ui64, 1Ui64, 0Ui64}}}, {124Ui64}, {129Ui64},
{0U}, {{{15999999999999999999Ui64, 0Ui64, 0Ui64}}}}}

so:

 x from 64 bit as 128 bit

mul_res=  x*2820903858849102351 + 1*(x<<64)

integer_part = uint64_t(mul_res>>124)+
uint64_t(x>15999999999999999999)<<(129-124)

fraction_part= mul_res<<(128-124)
 with 64 bits of mod accuracy



 (21267647932558653967*1000000000000000000>>124)-1== 2.5347750593952523710125260321821e-20;

(2.5347750593952523710125260321821e-20 << 64)==
0.46758346805086148378904908895493; so its
always less than an integer and so is correct
( generally  the output shifts by get_devision_by_mul_rs_shift_and_bit_count ans
so get_devide_inverse_and_shift is correct)


 ///////////////////////



constexpr auto ivk =
    get_devide_inverse_and_shift<>(uint32_t(-1), 100000000, {});

  {{{{{5764607524Ui64, 0Ui64, 0Ui64}}}, {59Ui64}, {65Ui64}, {0U},
{{{����3199999999Ui64, 0Ui64, 0Ui64}}}}}


thankfully 5764607524 is 1441151881 *4  so all the inv_p10_b57 is usable beacuse
59-2=57

((1441151881 >>57 )*100000000 -1 )==1.6753354259435582207515835762024e-10

and (1.6753354259435582207515835762024e-10<<32 )==0.71955108642578125 so its
always less than an integer and so is correct



////////////////////////



constexpr auto ivk =
    get_devide_inverse_and_shift<>(uint16_t(-1), 1000, {});


    (67109*1000>>26)-1==0.00000202655792236328125

   ( 0.00000202655792236328125<<16)==0.1328125 so its always less than an
integer and so is correct

   we dont really care if we do a 32 bit mul vs  a 64 bit mul so thats ok


   /////////////////////



constexpr auto ivk =
    get_devide_inverse_and_shift<>(uint8_t(-1), 10, true);

    {{{{{205Ui64, 0Ui64, 0Ui64}}}, {11Ui64}, {16Ui64}, {0U}, {{{159Ui64,
0U����i64, 0Ui64}}}}}


    (10*205>>11)-1 ==0.0009765625

    0.25==0.0009765625 *256  so its always less than an integer and so is
correct

    also its even just an 16 bit div ( max integer part is 25)




========================



{
6554,
656,
}
{
429496730,
42949673,
4294968,
429497,
42950,
4295,
}
{
1844674407370955162,
184467440737095517,
18446744073709552,
1844674407370956,
184467440737096,
18446744073710,
1844674407371,
184467440738,
18446744074,
}
{
T(11068046444225730970)+(T(1844674407370955161)<<64),
T(2951479051793528259)+(T(184467440737095516)<<64),
T(11363194349405083796)+(T(18446744073709551)<<64),
T(2980993842311463542)+(T(1844674407370955)<<64),
T(9521471421085922163)+(T(184467440737095)<<64),
T(10175519178963368025)+(T(18446744073709)<<64),
T(17619621584234933257)+(T(1844674407370)<<64),
T(1761962158423493326)+(T(184467440737)<<64),
T(13088917067439035464)+(T(18446744073)<<64),
T(6842914928856769032)+(T(1844674407)<<64),
T(13597012344482363035)+(T(184467440)<<64),
T(1359701234448236304)+(T(18446744)<<64),
T(7514667752928644277)+(T(1844674)<<64),
T(8130164404776685075)+(T(184467)<<64),
T(13725737292074354639)+(T(18446)<<64),
T(12440620173433166434)+(T(1844)<<64),
T(8622759646827137290)+(T(184)<<64),
T(8240973594166534376)+(T(18)<<64),
T(15581492618384294731)+(T(1)<<64),
}







*/
};
template <>
struct inv10p_i_table_t_<uint8_t> {
  constexpr static  inline std::array<uint16_t, 2> inv10p_16b_iplus1_table{
      6554,
      656,
  };
  using mask_type_t = uint16_t;
};
template <>
struct inv10p_i_table_t_<uint16_t> {
  constexpr static  inline std::array<uint32_t, 6> inv10p_32b_iplus1_table{
      429496730, 42949673, 4294968, 429497, 42950, 4295,
  };

  using mask_type_t = uint32_t;
};
template <>
struct inv10p_i_table_t_<uint32_t> {
  constexpr  static inline std::span<const uint64_t, 8> inv10p_57b_iplus1_table{
      std::span(inv_p10_b57).subspan<1>()};
  using mask_type_t = uint64_t;
};
template <>
struct inv10p_i_table_t_<uint64_t> {
#ifndef MJZ_uint128_t_impl_t_
  using mask_type_t = fallback_u128_type_t_;
#else
  // using mask_type_t = MJZ_uint128_t_impl_t_;
  using mask_type_t = fallback_u128_type_t_;
#endif  // ! MJZ_uint128_t_impl_t_
  using T = mask_type_t;
  constexpr  static  inline std::array<T, 19> inv10p_128b_iplus1_table{
      T(11068046444225730970ull) + (T(1844674407370955161ull) << 64),
      T(2951479051793528259ull) + (T(184467440737095516ull) << 64),
      T(11363194349405083796ull) + (T(18446744073709551ull) << 64),
      T(2980993842311463542ull) + (T(1844674407370955ull) << 64),
      T(9521471421085922163ull) + (T(184467440737095ull) << 64),
      T(10175519178963368025ull) + (T(18446744073709ull) << 64),
      T(17619621584234933257ull) + (T(1844674407370ull) << 64),
      T(1761962158423493326ull) + (T(184467440737ull) << 64),
      T(13088917067439035464ull) + (T(18446744073ull) << 64),
      T(6842914928856769032ull) + (T(1844674407ull) << 64),
      T(13597012344482363035ull) + (T(184467440ull) << 64),
      T(1359701234448236304ull) + (T(18446744ull) << 64),
      T(7514667752928644277ull) + (T(1844674ull) << 64),
      T(8130164404776685075ull) + (T(184467ull) << 64),
      T(13725737292074354639ull) + (T(18446ull) << 64),
      T(12440620173433166434ull) + (T(1844ull) << 64),
      T(8622759646827137290ull) + (T(184ull) << 64),
      T(8240973594166534376ull) + (T(18ull) << 64),
      T(15581492618384294731ull) + (T(1ull) << 64),
  };
};

constexpr const inline  size_t lookup_dbl_pow5_table_len_ = 325;
template <int = 0>
constexpr auto const inline  lookup_dbl_pow5_table_ = []() noexcept {
  constexpr auto len_ = lookup_dbl_pow5_table_len_;
  std::array<double, len_ * 2> ret{};
  ret[len_] = 1;
  ret[len_ - 1] = 0.2;
  for (size_t i{1}; i < len_; i++) {
    ret[i + len_] = ret[i + len_ - 1] * 5;
    ret[len_ - i - 1] = ret[len_ - i] * 0.2;
  }
  return ret;
}();

template <int i = 0>
 inline constexpr const double* const lookup_dbl_pow5_table_ptr_ =
    lookup_dbl_pow5_table_<i>.data() + lookup_dbl_pow5_table_len_;

constexpr  auto simd_8digit_conv_u64(auto n) noexcept {
  constexpr uint64_t inv10p4_b40 = 109951163;
  constexpr uint64_t inv10p2_b19 = 5243;
  constexpr uint64_t inv10p1_b10 = 103;
  constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
  constexpr uint64_t modolo10p4_40b_mask = 0x0000'00ff'ffff'ffff;
  constexpr uint64_t mask_upper_19b = 0xfff8'0000'fff8'0000;
  // ceil(2^40/10000)
  auto holder = n * inv10p4_b40;

  // Upper 4-digits in lower 32-bits.
  auto result_high = holder;

  // Lower 4-digits in upper 32-bits.
  auto result_low =
      // muliply the modolous by 10000 and shift by 40 simplified,
      // then move to upper 32 bit
      ((((holder & modolo10p4_40b_mask) * 625) >> 36));

  decltype(n) result{};
  if constexpr (std::endian::big == std::endian::native) {
    result = (result_high >> 8) | result_low;
  } else {
    result = (result_high >> 40) | (result_low << 32);
  }

  holder = result * inv10p2_b19;
  auto upper = holder & mask_upper_19b;

  // Upper 2-digits in lower 16-bits.
  result_high = upper;

  // Lower 2-digits in upper 16-bits.
  result_low = ((holder & ~mask_upper_19b) * 100) & mask_upper_19b;

  if constexpr (std::endian::big == std::endian::native) {
    result = (result_low >> 19) | (result_high >> 3);
  } else {
    result = (result_high >> 19) | (result_low >> 3);
  }

  holder = result * inv10p1_b10;
  upper = holder & mask_upper_6b;

  // Upper digit in lower 8-bits.
  result_high = upper;

  // Lower digit in upper 8-bits.
  result_low = (((holder & ~mask_upper_6b) * 10) & mask_upper_6b);

  if constexpr (std::endian::big == std::endian::native) {
    result = (result_high >> 2) | (result_low >> 10);
  } else {
    result = (result_high >> 10) | (result_low >> 2);
  }

  return result;
}

constexpr uint64_t ascii_offset =
    std::bit_cast<uint64_t>(std::array{'0', '0', '0', '0', '0', '0', '0', '0'});
constexpr  const std::span<const uint16_t, 100> modolo_raidex_table =
    std::span(radix_ascii_p2_v_).subspan<0, 100>(); /* []() {
   std::array<uint16_t, 100> ret{};
   for (uint32_t i{}; i < 100; i++) {
     const uint32_t var = i;
     uint64_t result = simd_8digit_conv_u64(uint64_t(var));

     result |= ascii_offset;

     struct char_array_t {
       char array[6];
       uint16_t table_entry;
     } char_array = std::bit_cast<char_array_t>(result);
     ret[i] = char_array.table_entry;
   }
   return ret;
 }();*/
 constexpr inline const uint16_t* radix_ascii_p2_ =
    radix_ascii_p2_v_.data();

constexpr  uint64_t lookup_iota_8digits_ascii(const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 4> ret{
      std::bit_cast<std::array<uint16_t, 4>>(ascii_offset)};
  constexpr uint64_t inv10p2xi_b57[4]{1ull << 57, 1441151880758559,
                                      14411518807586, 144115188076};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  uint64_t val{n};
  if (n < 100) {
    ret[3] = modolo_raidex_table[val];
    return std::bit_cast<uint64_t>(ret);
  }
  const uint64_t temp = n < 1000000 ? inv10p2xi_b57[2] : inv10p2xi_b57[3];
  val *= n < 10000 ? inv10p2xi_b57[1] : temp;

  if (1000000 <= n) {
    ret[0] = modolo_raidex_table[(val) >> 57];
    val &= mask;
    val *= 100;
  }
  if (10000 <= n) {
    ret[1] = modolo_raidex_table[(val) >> 57];
    val &= mask;
    val *= 100;
  }
  ret[2] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[3] = modolo_raidex_table[(val) >> 57];
  return std::bit_cast<uint64_t>(ret);
}

[[maybe_unused]] constexpr  uint64_t lookup_iota_8digits(
    const uint64_t n) noexcept {
  return lookup_iota_8digits_ascii(n) & ~ascii_offset;
}

[[maybe_unused]] constexpr  uint64_t lookup_iota_8digits_ascii_noif(
    const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 4> ret{
      std::bit_cast<std::array<uint16_t, 4>>(ascii_offset)};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  constexpr uint64_t inv10p6_57b = 144115188076;
  uint64_t val{n};
  val *= inv10p6_57b;
  ret[0] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[1] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[2] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[3] = modolo_raidex_table[(val) >> 57];
  return std::bit_cast<uint64_t>(ret);
}
[[maybe_unused]] constexpr  std::array<uint16_t, 5>
lookup_iota_10digits_ascii_noif(const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 5> ret{std::bit_cast<std::array<uint16_t, 5>>(
      std::array{'0', '0', '0', '0', '0', '0', '0', '0', '0', '0'})};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  constexpr uint64_t inv10p8_57b = 14411518801;
  uint64_t val{n};
  val *= inv10p8_57b;
  ret[0] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[1] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[2] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[3] = modolo_raidex_table[(val) >> 57];
  val &= mask;
  val *= 100;
  ret[4] = modolo_raidex_table[(val) >> 57];
  return std::bit_cast<std::array<uint16_t, 5>>(ret);
}

[[maybe_unused]] constexpr  uint64_t
hybrid_iota_8digits_ascii_noif_noload(const uint64_t n) noexcept {
  alignas(8) std::array<uint16_t, 4> ret{
      std::bit_cast<std::array<uint16_t, 4>>(ascii_offset)};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  constexpr uint64_t inv10p6_57b = 144115188076;
  constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
  constexpr uint64_t inv10p1_b10 = 103;
  uint64_t val{n};
  val *= inv10p6_57b;
  ret[0] = val >> 57;
  val &= mask;
  val *= 100;
  ret[1] = val >> 57;
  val &= mask;
  val *= 100;
  ret[2] = val >> 57;
  val &= mask;
  val *= 100;
  ret[3] = val >> 57;
  uint64_t paralell4_old = std::bit_cast<uint64_t>(ret);
  uint64_t both = paralell4_old * inv10p1_b10;
  uint64_t high = both & mask_upper_6b;
  uint64_t low = (((both & ~mask_upper_6b) * 10) & mask_upper_6b);
  if constexpr (std::endian::big == std::endian::native) {
    return (high >> 2) | (low >> 10) | ascii_offset;
  } else {
    return (high >> 10) | (low >> 2) | ascii_offset;
  }
}

[[maybe_unused]] constexpr  auto hybrid_iota_8digits_noif_noload_u64(
    auto n) noexcept {
  const uint64_t inv10p7_60b = 115292150461;
  n *= inv10p7_60b;
  decltype(n) ret{};
  for (size_t i{}; i < 8; i++) {
    if constexpr (std::endian::little == std::endian::native) {
      ret |= (n >> 60) << (i << 3);
    } else {
      ret |= (n >> 60) << (56 - (i << 3));
    }
    n &= uint64_t(-1) >> 4;
    n = (n << 1) + (n << 3);
  }
  return ret;
}
[[maybe_unused]] constexpr  uint64_t
hybrid_iota_8digits_noif_ascii_noload(uint64_t n) noexcept {
  return hybrid_iota_8digits_noif_noload_u64(n) | ascii_offset;
}

constexpr  uint32_t iota_3digits(uint32_t n) noexcept {
  constexpr uint32_t inv100_28b = 2684355;
  constexpr uint32_t mask = uint32_t(-1) >> 4;
  n *= inv100_28b;
  std::array<char, 4> ret{};
  ret[1] = char(n >> 28);
  n &= mask;
  n *= 10;
  ret[2] = char(n >> 28);
  n &= mask;
  n *= 10;
  ret[3] = char(n >> 28);
  return std::bit_cast<uint32_t>(ret);
}

constexpr  auto iota_4_u32digits(auto n) noexcept {
  constexpr uint32_t inv1000_28b = 268436;
  constexpr uint32_t mask = uint32_t(-1) >> 4;
  n *= inv1000_28b;
  decltype(n) ret{};
  if constexpr (std::endian::little == std::endian::native) {
    ret |= (n >> 28);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28) << 8;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28) << 16;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28) << 24;
  } else {
    ret |= (n >> 28) << 24;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28) << 16;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28) << 8;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 28);
  }
  return ret;
}
constexpr  auto iota_2_u16digits(auto n) noexcept {
  constexpr uint16_t inv10_10b = 103;
  constexpr uint16_t mask = uint16_t(uint16_t(-1) >> 9);
  n *= inv10_10b;
  using T = decltype(n);
  T ret{};
  if constexpr (std::endian::big == std::endian::native) {
    ret |= T((n >> 10) << 8);
    n &= mask;
    n = T((n << 1) + (n << 3));
    ret |= T(n >> 10);
  } else {
    ret |= T(n >> 10);
    n &= mask;
    n = T((n << 1) + (n << 3));
    ret |= T((n >> 10) << 8);
  }
  return ret;
}

constexpr  uint64_t iota_5_u32_tou64digits(uint64_t n) noexcept {
  constexpr uint64_t inv10000_32b = 429497;
  constexpr uint64_t mask = uint32_t(-1);
  n *= inv10000_32b;
  uint64_t ret{};
  if constexpr (std::endian::little == std::endian::native) {
    ret |= (n >> 32);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << 8;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << 16;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << 24;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << 32;
  } else {
    ret |= (n >> 32) << (32 + 24);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << (32 + 16);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << (32 + 8);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << (32 + 0);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= (n >> 32) << (32 - 8);
  }
  return ret;
}

[[maybe_unused]]
constexpr  std::array<char, 4> iota_forward_3digits(uint32_t n) noexcept {
  uint32_t awnser = iota_3digits(n);
  int num0ch{};
  if constexpr (std::endian::big == std::endian::native) {
    num0ch = std::countl_zero(awnser) >> 3;
    num0ch = std::max(num0ch, 3);
    awnser <<= num0ch << 3;
    awnser |= uint32_t(4 - num0ch);
  } else {
    num0ch = std::countr_zero(awnser) >> 3;
    num0ch = std::max(num0ch, 3);
    awnser >>= num0ch << 3;
    awnser |= uint32_t(4 - num0ch) << 24;
  }
  return std::bit_cast<std::array<char, 4>>(awnser |
                                            std::bit_cast<uint32_t>("000"));
}
[[maybe_unused]] constexpr  uint64_t iota_3digits_u64(
    uint32_t n) noexcept {
  return std::bit_cast<uint64_t>(std::array{uint32_t(0), iota_3digits(n)});
}
template <bool branch_less_v = true>

[[maybe_unused]] inline std::tuple<std::array<uint64_t, 3>, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint64_t number_) noexcept {
  constexpr uint64_t zero_8parallel_ascii = 0x3030303030303030;
  constexpr uint64_t parallel_half = 10000;
  constexpr uint64_t parallel_full = parallel_half * parallel_half;
  constexpr uint64_t count_max = 3;
// i dont like my warning as error on this on my ide
#if MJZ_STD_HAS_SIMD_LIB_

  namespace stdx = std::experimental;
  int iteration_count_backwards{2};
  stdx::fixed_size_simd<uint64_t, 4> str_int_buf{};
  stdx::fixed_size_simd<uint64_t, 4> word_register{};

  if constexpr (!branch_less_v) {
    if (number_ < parallel_full) {
      word_register[2] = simd_8digit_conv_u64(number_);
      iteration_count_backwards = 2;
    } else {
      uint64_t number = number_;
      if (number_ < parallel_full * parallel_full) {
        iteration_count_backwards = 1;
      } else {
        iteration_count_backwards = 0;
        uint32_t temp = uint32_t(number / (parallel_full * parallel_full));
        number %= (parallel_full * parallel_full);
        temp = iota_4_u32digits(temp);

        word_register[0] = temp;
        if constexpr (std::endian::big != std::endian::native) {
          word_register[0] <<= 32;
        }
      }
      stdx::fixed_size_simd<uint64_t, 2> u128_{};
      u128_[1] = number_ % parallel_full;
      u128_[0] = number_ / parallel_full;
      u128_ = simd_8digit_conv_u64(u128_);
      word_register[1] = u128_[0];
      word_register[2] = u128_[1];
    }
  } else {
    iteration_count_backwards =
        (number_ < parallel_full) + (number_ < parallel_full * parallel_full);
    uint64_t number = number_;
    uint32_t temp = uint32_t(number / (parallel_full * parallel_full));
    number %= (parallel_full * parallel_full);
    temp = iota_4_u32digits(temp);
    word_register[0] = temp;
    if constexpr (std::endian::big != std::endian::native) {
      word_register[0] <<= 32;
    }
    stdx::fixed_size_simd<uint64_t, 2> u128_{};
    u128_[1] = number_ % parallel_full;
    u128_[0] = number_ / parallel_full;
    u128_ = simd_8digit_conv_u64(u128_);
    word_register[1] = u128_[0];
    word_register[2] = u128_[1];
  }
  str_int_buf = word_register | zero_8parallel_ascii;
  uint64_t u64ch = word_register[size_t(iteration_count_backwards)];

  const size_t num_high_0ch =
      size_t((std::endian::big == std::endian::native
                  ? std::countl_zero(uint64_t(u64ch))
                  : std::countr_zero(uint64_t(u64ch))) >>
             3);
  const size_t num_0ch{num_high_0ch + size_t(iteration_count_backwards << 3)};
  const size_t num_ch = std::max<size_t>(24 - num_0ch, 1);
  return {{str_int_buf[0], str_int_buf[1], str_int_buf[2]},
          num_ch,
          std::min<size_t>(size_t(count_max * 8 - 1), num_0ch)};
#else

#if 0 < _MSC_VER 
  std::array<uint64_t, 3> str_int_buf{};
#else
  std::array<uint64_t, 3> str_int_buf;
#endif  // DEBUG

  uint64_t number_0_ = number_;
  for (size_t i{}, iteration_count_backwards{count_max}; i < count_max; i++) {
    iteration_count_backwards--;

    uint64_t u64ch_{};

    uint32_t number_less_than_pow10_8;
    if (number_0_ < parallel_full) {
      number_less_than_pow10_8 = uint32_t(number_0_);
      number_0_ = 0;
    } else {
      number_less_than_pow10_8 = uint32_t(number_0_ % parallel_full);
      number_0_ = number_0_ / parallel_full;
    }

    u64ch_ = simd_8digit_conv_u64(uint64_t(number_less_than_pow10_8));

    const uint64_t u64ch = u64ch_;
    uint64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
    str_int_buf[iteration_count_backwards] = u64ch_ascii;
    if (number_0_) continue;
    const size_t num_high_0ch =
        size_t((std::endian::big == std::endian::native
                    ? std::countl_zero(uint64_t(u64ch))
                    : std::countr_zero(uint64_t(u64ch))) >>
               3);
    const size_t num_0ch{num_high_0ch + size_t(iteration_count_backwards << 3)};
    const size_t num_ch = std::max<size_t>(sizeof(str_int_buf) - num_0ch, 1);
    return {str_int_buf, num_ch,
            std::min<size_t>(size_t(count_max * 8 - 1), num_0ch)};
  }

  return {};

#endif  // MJZ_STD_HAS_SIMD_LIB_
}

[[maybe_unused]] inline std::tuple<std::array<uint64_t, 2>, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint32_t number_) noexcept {
  std::array<uint64_t, 2> ret{};
  uint32_t low_number = number_;
  uint32_t high_number = number_ / 10000'0000;
  low_number = number_ % 10000'0000;
  ret[0] = iota_2_u16digits(uint16_t(high_number));
  if constexpr (std::endian::big != std::endian::native) {
    ret[0] <<= 48;
  }
  size_t high_i = number_ < 10000'0000;
  ret[1] = simd_8digit_conv_u64(uint64_t(low_number));
  uint64_t high = ret[high_i];

  const size_t num_high_0ch =
      std::min<size_t>(7, size_t((std::endian::big == std::endian::native
                                      ? std::countl_zero(high)
                                      : std::countr_zero(high)) >>
                                 3)) +
      (high_i << 3);
  return {{ret[0] | ascii_offset, ret[1] | ascii_offset},
          16 - num_high_0ch,
          num_high_0ch};
}
[[maybe_unused]]
inline std::tuple<uint64_t, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint16_t number_) noexcept {
  uint64_t ret = iota_5_u32_tou64digits(number_);

  const size_t num_high_0ch =
      std::min<size_t>(7, size_t((std::endian::big == std::endian::native
                                      ? std::countl_zero(ret)
                                      : std::countr_zero(ret)) >>
                                 3));
  return {ret | ascii_offset, 8 - num_high_0ch, num_high_0ch};
}
[[maybe_unused]]
inline std::tuple<uint32_t, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint8_t number_) noexcept {
  uint32_t ret = iota_3digits(number_);
  const size_t num_high_0ch =
      std::min<size_t>(3, size_t((std::endian::big == std::endian::native
                                      ? std::countl_zero(ret)
                                      : std::countr_zero(ret)) >>
                                 3));
  return {uint32_t(ret | ascii_offset), 4 - num_high_0ch, num_high_0ch};
}

/* has bug ?*/
[[maybe_unused]] inline void uint_to_dec_forward_less1e11(char* buffer,
                                                          uint64_t count,
                                                          uint64_t n) noexcept {
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  constexpr uint64_t inv10p8_57b = 14411518801;
  uint64_t val{n};
  val *= inv10p8_57b;
  uint64_t i{};

  while (count >> 1) {
    auto temp =
        std::bit_cast<std::array<char, 2>>(modolo_raidex_table[(val) >> 57]);
    buffer[i] = temp[0];
    buffer[i + 1] = temp[0];
    val &= mask;
    count -= 2;
    i += 2;
    val *= 100;
  };
  if (!count) return;
  buffer[i] =
      std::bit_cast<std::array<char, 2>>(modolo_raidex_table[(val) >> 57])[0];
}

/* has bug ?*/
[[maybe_unused]] inline size_t uint_to_dec_forward(
    char* buffer, size_t cap, uint64_t number_0_) noexcept {
  constexpr uint64_t log10_2_32b = 1292913986;
  constexpr uint64_t pow10_10 = 10'000'000'000;
  uint64_t both[2]{};
  uint64_t& high{both[1]};
  uint64_t& low{both[0]};
  uint64_t log2_floor{};
  if (pow10_10 < number_0_) {
    high = number_0_ / pow10_10;
    low = number_0_ % pow10_10;
    log2_floor += 10;
    number_0_ = high;
  } else {
    low = number_0_;
  }
  uint64_t log_floor_ofset =
      (uint64_t(63 - std::max(63, std::countl_zero(number_0_))) *
       log10_2_32b) >>
      32;
  log2_floor += log_floor_ofset;
  log2_floor += details_ns::pow_ten_table[log_floor_ofset] <= number_0_;
  const uint64_t count = log2_floor + 1;
  if (cap < count) return false;
  uint_to_dec_forward_less1e11(buffer, std::max<uint64_t>(10, count), low);
  if (count < 10) {
    return count;
  }
  uint_to_dec_forward_less1e11(buffer, count - 10, low);
  return count;
}
};  // namespace details_ns

[[maybe_unused]] inline size_t uint_to_dec_less1e9(
    char* buffer, size_t cap, uint32_t number_0_) noexcept {
  constexpr uint64_t zero_8parallel_ascii = 0x3030303030303030;
  const uint64_t u64ch = details_ns ::simd_8digit_conv_u64(uint64_t(number_0_));

  uint64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
  const size_t num_0ch = size_t((std::endian::big == std::endian::native
                                     ? std::countl_zero(uint64_t(u64ch))
                                     : std::countr_zero(uint64_t(u64ch))) >>
                                3);
  const size_t num_ch = std::max<size_t>(sizeof(u64ch_ascii) - num_0ch, 1);
  if (cap < num_ch) return 0;
  std::memcpy(buffer, reinterpret_cast<const char*>(&u64ch_ascii) + 8 - num_ch,
              num_ch);
  return num_ch;
}
template <std::unsigned_integral T>
[[maybe_unused]] inline size_t uint_to_dec(char* buffer, size_t cap,
                                           T number_0_) noexcept {
  auto [str_int_buf, num_ch, offset] =
      details_ns ::dec_from_uint_impl_semi_parallel_impl_ncx_(number_0_);
  num_ch = cap < num_ch ? 0 : num_ch;
  std::memcpy(buffer, reinterpret_cast<const char*>(&str_int_buf) + offset,
              num_ch);
  return num_ch;
}

template <std::unsigned_integral T>
constexpr size_t uint_to_dec_aligned_unchekced_size_v{sizeof(
    std::get<0>(details_ns::dec_from_uint_impl_semi_parallel_impl_ncx_(T())))};

  [[noreturn]] inline void mjz_unreachable_impl_() noexcept {
  MJZ_JUST_UNREACHABLE_();
}
constexpr MJZ_JUST_FORCED_INLINE_  void mjz_assume_impl_(
    const bool b) noexcept {
  if (b) return;
  mjz_unreachable_impl_();
}

template <std::unsigned_integral T, size_t min_align_v = 1>
[[maybe_unused]] inline size_t uint_to_dec_aligned_unchekced_branchless(
    char* buffer, size_t cap, T number_0_) noexcept {
  auto [str_int_buf, num_ch, offset] =
      details_ns ::dec_from_uint_impl_semi_parallel_impl_ncx_(number_0_);
  if (cap < sizeof(str_int_buf) || !buffer) {
    mjz_unreachable_impl_();
  }
  char* ptr = std::assume_aligned<min_align_v>(buffer);
  constexpr size_t num_words = sizeof(str_int_buf) / 4;

  std::array<uint32_t, num_words> words =
      std::bit_cast<std::array<uint32_t, num_words>>(str_int_buf);
  size_t shift_little = (offset & 3) << 3;
  size_t shift_big = (offset >> 2);

  if constexpr (std::endian::big == std::endian::native) {
    std::array<uint64_t, num_words> words_highlow{};
    for (size_t i{}; i < num_words; i++) {
      words_highlow[i] = uint64_t(words[i]) << shift_little;
    }
    words[0] = uint32_t(words_highlow[0]);
    for (size_t i{1}; i < num_words; i++) {
      words[i] =
          uint32_t(words_highlow[i]) | uint32_t(words_highlow[i - 1] >> 32);
    }
  } else {
    shift_little = 32 - shift_little;
    std::array<uint64_t, num_words> words_highlow{};
    for (size_t i{}; i < num_words; i++) {
      words_highlow[i] = uint64_t(words[i]) << shift_little;
    }
    words[0] = uint32_t(words_highlow[0] >> 32);
    for (size_t i{1}; i < num_words; i++) {
      words[i] =
          uint32_t(words_highlow[i - 1]) | uint32_t(words_highlow[i] >> 32);
    }
  }
  alignas(8) std::array<uint32_t, num_words * 2> shifted_words{};
  for (size_t i{}; i < num_words; i++) {
    shifted_words[num_words + i - shift_big] = words[i];
  }

  std::memcpy(ptr, &shifted_words[num_words], sizeof(words));
  return num_ch;
}

template <std::unsigned_integral T, size_t min_align_v = 1>
[[maybe_unused]] inline size_t uint_to_dec_aligned_unchekced_branching(
    char* buffer, size_t cap, T number_0_) noexcept {
  if (uint8_t(number_0_) == number_0_) {
    return uint_to_dec_aligned_unchekced_branchless<uint8_t, min_align_v>(
        buffer, cap, uint8_t(number_0_));
  }
  if (uint16_t(number_0_) == number_0_) {
    return uint_to_dec_aligned_unchekced_branchless<uint16_t, min_align_v>(
        buffer, cap, uint16_t(number_0_));
  }
  if (uint32_t(number_0_) == number_0_) {
    return uint_to_dec_aligned_unchekced_branchless<uint32_t, min_align_v>(
        buffer, cap, uint32_t(number_0_));
  }
  return uint_to_dec_aligned_unchekced_branchless<uint64_t, min_align_v>(
      buffer, cap, uint64_t(number_0_));
}

template <std::unsigned_integral T, size_t min_align_v = 1>
[[maybe_unused]] inline size_t uint_to_dec_aligned_unchekced(
    char* buffer, size_t cap, T number_0_) noexcept {
  return uint_to_dec_aligned_unchekced_branching(buffer, cap, number_0_);
}

// 1+floor(log_10(x)) , x=0 -> 0
constexpr  MJZ_JUST_FORCED_INLINE_ int dec_width(
    const std::unsigned_integral auto x_pos_) noexcept {
  constexpr bool give_hints_v = true;
  const uint64_t x = uint64_t(x_pos_) | 1;
  const uint32_t log2_ceil =
      uint32_t(std::bit_width(x) /*- std::has_single_bit(x) unneeded*/);
  uint32_t log10_2_bx = 5;
  uint32_t bx = 4;
  using T = std::remove_cvref_t<decltype(x_pos_)>;
  if constexpr (!std::signed_integral<T> && sizeof(T) == 8) {
    // desmos ( x being log(input), y
    // being floor(log(input))  :
    //  \operatorname{floor}\left(\frac{\operatorname{ceil}\left(\frac{x}{\log2}\right)5}{16}\right)
    //  is accurate up to 2^63
    // the next one is ceil(128* log2)=39 that has 2^64 as well
    log10_2_bx = 39;
    bx = 7;
  } else if constexpr (std::signed_integral<T>) {
    mjz_assume_impl_(0 <= x_pos_);
  }
  const uint32_t correct_or_1_plus_correct = ((log2_ceil * log10_2_bx) >> bx);
  const bool is_correct =
      details_ns::floor10_table[size_t(correct_or_1_plus_correct)] <= x;
  auto res = correct_or_1_plus_correct + is_correct - (0 == x_pos_);
  const auto final_dec_width = uint8_t(res);
  const auto num_ = x_pos_;
  const auto ret_val{int(res)};
  mjz_assume_impl_(0 <= ret_val && ret_val < 21);
  if constexpr (!give_hints_v) {
    return ret_val;
  }
  mjz_assume_impl_((final_dec_width == 0) == (0 <= num_ && num_ < 1));
  mjz_assume_impl_((final_dec_width == 1) == (1 <= num_ && num_ < 10));
  mjz_assume_impl_((final_dec_width == 2) == (10 <= num_ && num_ < 100));
  mjz_assume_impl_((final_dec_width == 3) == (100 <= num_ && num_ < 1000));
  mjz_assume_impl_((final_dec_width == 4) == (1000 <= num_ && num_ < 10000));
  mjz_assume_impl_((final_dec_width == 5) == (10000 <= num_ && num_ < 100000));
  mjz_assume_impl_((final_dec_width == 6) ==
                   (100000 <= num_ && num_ < 1000000));
  mjz_assume_impl_((final_dec_width == 7) ==
                   (1000000 <= num_ && num_ < 10000000));
  mjz_assume_impl_((final_dec_width == 8) ==
                   (10000000 <= num_ && num_ < 100000000));
  mjz_assume_impl_((final_dec_width == 9) ==
                   (100000000 <= num_ && num_ < 1000000000));
  mjz_assume_impl_((final_dec_width == 10) ==
                   (1000000000 <= num_ && num_ < 10000000000));
  mjz_assume_impl_((final_dec_width == 11) ==
                   (10000000000 <= num_ && num_ < 100000000000));
  mjz_assume_impl_((final_dec_width == 12) ==
                   (100000000000 <= num_ && num_ < 1000000000000));
  mjz_assume_impl_((final_dec_width == 13) ==
                   (1000000000000 <= num_ && num_ < 10000000000000));
  mjz_assume_impl_((final_dec_width == 14) ==
                   (10000000000000 <= num_ && num_ < 100000000000000));
  mjz_assume_impl_((final_dec_width == 15) ==
                   (100000000000000 <= num_ && num_ < 1000000000000000));
  mjz_assume_impl_((final_dec_width == 16) ==
                   (1000000000000000 <= num_ && num_ < 10000000000000000));
  mjz_assume_impl_((final_dec_width == 17) ==
                   (10000000000000000 <= num_ && num_ < 100000000000000000));
  mjz_assume_impl_((final_dec_width == 18) ==
                   (100000000000000000 <= num_ && num_ < 1000000000000000000));
  mjz_assume_impl_((final_dec_width == 19) == (1000000000000000000 <= num_ &&
                                               num_ < 10000000000000000000ull));
  mjz_assume_impl_((final_dec_width == 20) ==
                   (10000000000000000000ull <= num_));
  return ret_val;
}

template <std::integral T>
constexpr  MJZ_JUST_FORCED_INLINE_ int signed_dec_width(
    const T x) noexcept {
  const bool is_neg = x < 0;
  using ut = std::make_unsigned_t<T>;
  return dec_width<ut>(is_neg ? ut(~ut(x) + 1) : ut(x)) + is_neg;
}
template <std::integral T>
constexpr  inline size_t int_to_dec_unchekced_size_v =
    size_t(std::max(signed_dec_width(std::numeric_limits<T>::max()),
                    signed_dec_width(std::numeric_limits<T>::min())));

namespace details_ns {

template <size_t size_v, std::unsigned_integral T>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_semi_par_(char* buffer, const size_t dec_width_0_,
                                    T num_) noexcept {
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  char* end_buf = buffer + dec_width_;
  constexpr uint32_t inv10p4_b40 = 109951163;
  constexpr uint32_t inv10p2_b19 = 5243;
  constexpr uint16_t inv10_10b = 103;
  uint64_t num = num_;
  mjz_assume_impl_(dec_width_0_ <= 20);
  if (floor_log10 & 16) {
    mjz_assume_impl_(10000000000000000 <= num);
    alignas(16) std::array<uint64_t, 2> temp{};
    temp[0] = num % 10000000000000000;
    num /= 10000000000000000;
    end_buf -= 16;
    temp[1] = temp[0] % 100000000;
    temp[0] /= 100000000;
#if MJZ_STD_HAS_SIMD_LIB_
    if (!std::is_constant_evaluated()) {
      namespace stdx = std::experimental;
      stdx::fixed_size_simd<uint64_t, 2> words(&temp[0], stdx::overaligned<16>);
      (details_ns ::simd_8digit_conv_u64(words) | ascii_offset)
          .copy_to(&temp[0], stdx::overaligned<16>);
    } else
#endif
    {
      for (uint64_t& t : temp)
        t = details_ns ::simd_8digit_conv_u64(uint64_t(t)) | ascii_offset;
    }
    const auto chs = std::bit_cast<std::array<char, 16>>(temp);
    for (size_t i{}; i < 16; i++) {
      end_buf[i] = chs[i];
    }
  }
  mjz_assume_impl_(num < 10000000000000000);
  if (floor_log10 & 8) {
    end_buf -= 8;
    mjz_assume_impl_(100000000 <= num);
    uint32_t temp = uint32_t(num % 100000000);
    num /= 100000000;
    const uint64_t u64ch = details_ns ::simd_8digit_conv_u64(uint64_t(temp));
    auto chs = std::bit_cast<std::array<char, 8>>(u64ch | ascii_offset);
    for (size_t i{}; i < 8; i++) {
      (end_buf)[i] = chs[i];
    }
  }
  mjz_assume_impl_(num < 100000000);
  if (floor_log10 & 4) {
    end_buf -= 4;
    mjz_assume_impl_(10000 <= num);
    constexpr uint64_t mask = ~(uint64_t(-1) << 40);
    num *= inv10p4_b40;
    uint64_t temp = num;
    num >>= 40;

    std::array<char, 4> store_par{};
    for (size_t i{}; i < 4; i++) {
      temp &= mask;
      temp *= 10;
      mjz_assume_impl_((temp >> 40) < 10);
      store_par[i] = char(temp >> 40);
    }
    store_par = std::bit_cast<std::array<char, 4>>(
        uint32_t(std::bit_cast<uint32_t>(store_par) | ascii_offset));

    for (size_t i{}; i < 4; i++) {
      end_buf[i] = store_par[i];
    }
  }
  mjz_assume_impl_(num < 10000);
  if (floor_log10 & 2) {
    end_buf -= 2;
    mjz_assume_impl_(100 <= num);
    constexpr uint64_t mask = ~(uint64_t(-1) << 19);
    num *= inv10p2_b19;
    uint64_t temp = num;
    num >>= 19;
    std::array<char, 2> store_par{};
    for (size_t i{}; i < 2; i++) {
      temp &= mask;
      temp *= 10;
      mjz_assume_impl_((temp >> 19) < 10);
      store_par[i] = char(temp >> 19);
    }
    store_par = std::bit_cast<std::array<char, 2>>(
        uint16_t(std::bit_cast<uint16_t>(store_par) | ascii_offset));
    for (size_t i{}; i < 2; i++) {
      end_buf[i] = store_par[i];
    }
  }
  mjz_assume_impl_(num < 100);
  if (floor_log10 & 1) {
    end_buf -= 1;
    mjz_assume_impl_(10 <= num);
    constexpr uint64_t mask = ~(uint64_t(-1) << 10);
    num *= inv10_10b;
    uint64_t temp = num;
    num >>= 10;
    temp &= mask;
    temp *= 10;
    mjz_assume_impl_((temp >> 10) < 10);
    end_buf[0] = char((temp >> 10) | '0');
  }
  mjz_assume_impl_(num < 10);
  *(end_buf - 1) = char(num | '0');
  return dec_width_;
}

constexpr  MJZ_JUST_FORCED_INLINE_ void cpy_bitcast_impl_(
    char* ptr, auto v) noexcept {
  if (!std::is_constant_evaluated()) {
    std::memmove(ptr, &v, sizeof(v));
  }
  auto chs = std::bit_cast<std::array<char, sizeof(v)>>(v);
  for (char c : chs) {
    *ptr++ = c;
  }
}
template <typename T>
constexpr  MJZ_JUST_FORCED_INLINE_ T
cpy_bitcast_impl_(const char* ptr) noexcept {
  if (!std::is_constant_evaluated()) {
    T ret{};
    std::memmove(&ret, ptr, sizeof(T));
    return ret;
  }
  std::array<char, sizeof(T)> t{};
  for (char& c : t) {
    c = *ptr++;
  }
  return std::bit_cast<T>(t);
}

template <size_t size_v, std::unsigned_integral T>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t uint_to_dec_pre_calc_impl_seq_(
    char* buffer, const size_t dec_width_0_, T num_) noexcept {
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  char* end_buf = buffer + dec_width_;
  size_t dec_left = floor_log10;
  while (1 < dec_left) {
    const size_t index = (num_ % 100);
    num_ /= 100;
    end_buf -= 2;
    dec_left -= 2;
    auto [ch0_, ch1_] =
        std::bit_cast<std::array<char, 2>>(radix_ascii_p2_[index]);
    end_buf[0] = ch0_;
    end_buf[1] = ch1_;
  }
  if (dec_left) {
    end_buf -= 2;
    const size_t index = num_;
    auto [ch0_, ch1_] =
        std::bit_cast<std::array<char, 2>>(radix_ascii_p2_[index]);
    end_buf[0] = ch0_;
    end_buf[1] = ch1_;
  } else {
    end_buf -= 1;
    *end_buf = char(num_ | '0');
  }
  return dec_width_;
}

constexpr  MJZ_JUST_FORCED_INLINE_ uint64_t
u16x4_num_to_iota_impl_(uint64_t temp) noexcept {
  constexpr uint64_t inv10p1_b10 = 103;
  constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
  temp *= inv10p1_b10;
  uint64_t high = temp & mask_upper_6b;
  uint64_t low = ((temp & ~mask_upper_6b) * 10) & mask_upper_6b;
  if constexpr (std::endian::big == std::endian::native) {
    temp = (high >> 2) | (low >> 10);
  } else {
    temp = (high >> 10) | (low >> 2);
  }
  return temp;
}
[[maybe_unused]] constexpr  MJZ_JUST_FORCED_INLINE_ uint64_t
inv10p8_b57_mul100_num_to_iota_impl_(uint64_t& n) noexcept {
  std::array<uint16_t, 4> indexies{};
  for (uint16_t& index : indexies) {
    index = uint8_t(n >> 57);
    n &= uint64_t(-1) >> 7;
    n *= 100;
  }
  return u16x4_num_to_iota_impl_(std::bit_cast<uint64_t>(indexies));
}

[[maybe_unused]]
constexpr  MJZ_JUST_FORCED_INLINE_ uint64_t
inv10p8_b57_num_to_iota_impl_(uint64_t& n) noexcept {
  std::array<uint16_t, 4> indexies{};
  for (uint16_t& index : indexies) {
    n *= 100;
    index = uint8_t(n >> 57);
    n &= uint64_t(-1) >> 7;
  }
  return u16x4_num_to_iota_impl_(std::bit_cast<uint64_t>(indexies));
}

[[maybe_unused]] constexpr  MJZ_JUST_FORCED_INLINE_ uint64_t
inv10p8_b57_num_to_iota_impl_ascii_(uint64_t& n) noexcept {
  uint8_t indexies[4]{};
  for (uint8_t& index : indexies) {
    n *= 100;
    index = uint8_t(n >> 57);
    n &= uint64_t(-1) >> 7;
  }
  return std::bit_cast<uint64_t>(
      std::array{radix_ascii_p2_[indexies[0]], radix_ascii_p2_[indexies[1]],
                 radix_ascii_p2_[indexies[2]], radix_ascii_p2_[indexies[3]]});
}

constexpr  MJZ_JUST_FORCED_INLINE_ uint64_t
inv10p8_b57_num_to_iota_impl_ascii_simd_(uint64_t& n) noexcept {
#ifdef MJZ_uint128_t_impl_t_
  MJZ_uint128_t_impl_t_ n128 = n;
  MJZ_uint128_t_impl_t_ mask = uint64_t(-1) >> 7;
  uint8_t indexies[4]{};
  n128 <<= 64;
  n128 |= (n * 10000) & mask;
  mask |= mask << 64;
  n128 *= 100;
  MJZ_uint128_t_impl_t_ temp0 = (n128 & ~mask);
  indexies[0] = uint8_t(temp0 >> (64 + 57));
  indexies[2] = uint8_t(temp0 >> (57));
  n128 &= mask;
  n128 *= 100;
  temp0 = (n128 & ~mask);
  indexies[1] = uint8_t(temp0 >> (64 + 57));
  indexies[3] = uint8_t(temp0 >> (57));
  n = uint64_t(n128);
  return std::bit_cast<uint64_t>(
      std::array{radix_ascii_p2_[indexies[0]], radix_ascii_p2_[indexies[1]],
                 radix_ascii_p2_[indexies[2]], radix_ascii_p2_[indexies[3]]});

#else
  return inv10p8_b57_num_to_iota_impl_ascii_(n);
#endif
}

template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_(char* buffer, const size_t dec_width_0_,
                                       uint64_t num_) noexcept {
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  char* end_buf = buffer + dec_width_;
  size_t char_left = dec_width_;
  mjz_assume_impl_(char_left < 21 && char_left);
  while (8 < char_left) {
    uint64_t n{};
    constexpr uint64_t p10_8 = 10000'0000;
    end_buf -= 8;
    char_left -= 8;
    mjz_assume_impl_(p10_8 <= num_);
    // 1.0000000000000000000007074462189e-8
    // means :
    // 7.0744621888597691893377228389284e-22 error
    // measns:
    // 0.013050079345703125 max offset( less than integer so its good)
    // also:
    // bit_width(184,467,440,737.09551616)=38 , 128-90=38
    // so no overflow
    [[maybe_unused]] constexpr uint64_t inv_10p8_b90 =
        uint64_t(12379400392853802749ull);
#ifdef MJZ_uint128_t_impl_t_
    MJZ_uint128_t_impl_t_ u128 = num_;
    u128 *= inv_10p8_b90;

    n = (uint64_t((u128 + ((uint64_t(1) << 33) - 1)) >> 33) &
         (uint64_t(-1) >> 7));
    num_ = decltype(num_)(uint64_t(u128 >> 90));
    cpy_bitcast_impl_(end_buf, inv10p8_b57_num_to_iota_impl_ascii_simd_(n));
#else
    n = (num_ % p10_8) * inv_p10_b57[6];
    num_ = decltype(num_)(num_ / p10_8);
    cpy_bitcast_impl_(end_buf,
                      inv10p8_b57_mul100_num_to_iota_impl_(n) | ascii_offset);
#endif
  }
  size_t round_left{char_left};
  uint64_t n{};
  size_t i{};
  end_buf = buffer;
  mjz_assume_impl_(num_ < 100000000);
  if (round_left & 1) {
    if (round_left != 1) {
      n = num_ * inv_p10_b57[round_left ^ 1];
      *end_buf = char('0' | (n >> 57));
      i++;
      n &= uint64_t(-1) >> 7;
    } else {
      *end_buf = char('0' | num_);
      return dec_width_;
    }
  } else {
    if (round_left == 2) {
      const size_t index = num_;

      char* const p = end_buf + i;
      cpy_bitcast_impl_(p, radix_ascii_p2_[index]);

      return dec_width_;
    } else {
      n = num_ * inv_p10_b57[round_left - 2];
      const size_t index = size_t(n >> 57);
      char* const p = end_buf + i;
      cpy_bitcast_impl_(p, radix_ascii_p2_[index]);
      i += 2;
      n &= uint64_t(-1) >> 7;
      round_left -= 2;
    }
  }
  round_left >>= 1;
  mjz_assume_impl_(round_left < 4 && round_left);
  while (round_left) {
    n *= 100;
    const size_t index = size_t(n >> 57);
    cpy_bitcast_impl_(end_buf + i, radix_ascii_p2_[index]);
    i += 2;
    n &= uint64_t(-1) >> 7;
    round_left--;
  };
  return dec_width_;
}

template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_branching_(char* buffer,
                                                 const size_t dec_width_0_,
                                                 uint32_t num_) noexcept {
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  char* const end_buf = buffer;
  size_t char_left = dec_width_;
  uint64_t n{};
  size_t i{};
  if (char_left & 1) {
    if (char_left != 1) {
      n = num_ * inv_p10_b57[char_left ^ 1];
      *end_buf = char('0' | (n >> 57));
      i++;
      n &= uint64_t(-1) >> 7;
    } else {
      *end_buf = char('0' | num_);
      return dec_width_;
    }
  } else {
    if (char_left == 2) {
      const size_t index = size_t(num_);
      char* const p = end_buf + i;
      cpy_bitcast_impl_(p, radix_ascii_p2_[index]);
      return dec_width_;
    } else {
      n = num_ * inv_p10_b57[char_left - 2];
      const size_t index = size_t(n >> 57);
      char* const p = end_buf + i;
      cpy_bitcast_impl_(p, radix_ascii_p2_[index]);
      i = 2;
      n &= uint64_t(-1) >> 7;
      char_left -= 2;
    }
  }
  char_left >>= 1;
  while (char_left & 3) {
    n *= 100;
    const size_t index = size_t(n >> 57);
    char* const p = end_buf + i;
    cpy_bitcast_impl_(p, radix_ascii_p2_[index]);
    i += 2;
    n &= uint64_t(-1) >> 7;
    char_left--;
  };
  char_left >>= 2;
  while (char_left) {
    cpy_bitcast_impl_(end_buf + i, inv10p8_b57_num_to_iota_impl_ascii_simd_(n));
    i += 8;
    --char_left;
  }

  return dec_width_;
}

template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_more_mul_(char* buffer, const size_t dec_width_0_,
                                    uint32_t num_) noexcept {
  mjz_assume_impl_(dec_width_0_ < 11);
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  alignas(32) std::array<char, 32> buf32{};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  std::array<uint8_t, 5> buf_num{};
#ifdef MJZ_uint128_t_impl_t_
  constexpr uint32_t inv10p8_b57 = uint32_t(inv_p10_b57[8]);
  static_assert(inv_p10_b57[8] == inv10p8_b57);
  using u128_t_ = MJZ_uint128_t_impl_t_;
  constexpr u128_t_ const mask128 = (u128_t_(mask) << 64) | mask;
  constexpr u128_t_ mul128_offset = (1 + (u128_t_(10000) << 64));
  constexpr u128_t_ mul_val = inv10p8_b57 * mul128_offset;
  u128_t_ both = mul_val * num_;
  buf_num[0] = uint8_t(both >> 57);
  for (size_t i{1}; i < 3; i++) {
    both &= mask128;
    both *= 100;
    buf_num[i + 2] = uint8_t(both >> 121);
    buf_num[i] = uint8_t(both >> 57);
  }
#else
  uint64_t n = num_ * inv_p10_b57[8];
  for (size_t i{}; i < 5; i++) {
    buf_num[i] = size_t(n >> 57);
    n = (n & mask) * 100;
  }
#endif
  for (size_t i{}; i < 5; i++) {
    cpy_bitcast_impl_(buf32.data() + i * 2, radix_ascii_p2_[buf_num[i]]);
  }
  const size_t shift = 10 - dec_width_;
  const size_t count_store = size_v < 10 ? dec_width_ : 10;
  for (size_t i{}; i < count_store; i++) {
    buffer[i] = buf32[i + shift];
  }
  return dec_width_;
}
template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_(char* buffer, const size_t dec_width_0_,
                                       uint32_t num_) noexcept {
  return uint_to_dec_pre_calc_impl_more_mul_<size_v>(buffer, dec_width_0_,
                                                     num_);
}
template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_(char* buffer, const size_t dec_width_0_,
                                       uint16_t num_) noexcept {
  mjz_assume_impl_(dec_width_0_ < 6);
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  constexpr uint64_t inv10000_32b = 429497;
  constexpr uint64_t mask = uint32_t(-1);
  uint64_t temp = num_ * inv10000_32b;
  alignas(8) std::array<char, 8> chs{};
  const uint16_t* ps1 = radix_ascii_p2_ + ((temp >> 32));
  temp &= mask;
  temp *= 100;
  const uint16_t* ps2 = radix_ascii_p2_ + ((temp >> 32));
  temp &= mask;
  temp *= 10;
  mjz_assume_impl_((temp >> 32) < 10);
  {
    auto [ch0_, ch1_] = std::bit_cast<std::array<char, 2>>(*ps1);
    chs[0] = ch0_;
    chs[1] = ch1_;
  }
  {
    auto [ch0_, ch1_] = std::bit_cast<std::array<char, 2>>(*ps2);
    chs[2] = ch0_;
    chs[3] = ch1_;
  }
  chs[4] = char((temp >> 32) | '0');
  temp = std::bit_cast<uint64_t>(chs);
  if constexpr (std::endian::big == std::endian::native) {
    temp <<= (5 - dec_width_) << 3;
  } else {
    temp >>= (5 - dec_width_) << 3;
  }
  chs = std::bit_cast<std::array<char, 8>>(temp);

  if constexpr (size_v < sizeof(temp)) {
    for (size_t i{}; i < dec_width_; i++) {
      buffer[i] = chs[i];
    }
  } else {
    cpy_bitcast_impl_(buffer, temp);
  }
  return dec_width_;
}

template <size_t size_v>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_(char* buffer, const size_t dec_width_0_,
                                       uint8_t num_) noexcept {
  mjz_assume_impl_(dec_width_0_ < 4);
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  constexpr uint32_t inv10_11b = 205;
  constexpr uint32_t mask = uint32_t(uint32_t(-1) >> 21);
  uint32_t temp = inv10_11b * num_;
  alignas(4) std::array<char, 4> chs{};
  const uint16_t* ps1 = radix_ascii_p2_ + ((temp >> 11));
  {
    auto [ch0_, ch1_] = std::bit_cast<std::array<char, 2>>(*ps1);
    chs[0] = ch0_;
    chs[1] = ch1_;
  }

  temp &= mask;
  temp *= 10;
  chs[2] = char((temp >> 11) | '0');
  mjz_assume_impl_((temp >> 11) < 10);
  temp = std::bit_cast<uint32_t>(chs);
  if constexpr (std::endian::big == std::endian::native) {
    temp <<= (3 - dec_width_) << 3;
  } else {
    temp >>= (3 - dec_width_) << 3;
  }
  chs = std::bit_cast<std::array<char, 4>>(temp);

  if constexpr (size_v < sizeof(temp)) {
    for (size_t i{}; i < dec_width_; i++) {
      buffer[i] = chs[i];
    }
  } else {
    cpy_bitcast_impl_(buffer, temp);
  }
  return dec_width_;
}
template <size_t size_of_myt>
using uint_sizeof_t =
    type_at_index_t<size_of_myt, uint8_t, uint8_t, uint16_t, uint32_t, uint32_t,
                    uint64_t, uint64_t, uint64_t, uint64_t>;
template <size_t size_of_myt>
using int_sizeof_t = std::make_signed<uint_sizeof_t<size_of_myt>>;

template <size_t size_v, std::unsigned_integral T>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_pre_calc_impl_seq_less_mul_(char* buffer, const size_t dec_width_0_,
                                        T num_) noexcept {
  return uint_to_dec_pre_calc_impl_seq_lessmul_<size_v>(
      buffer, dec_width_0_, uint_sizeof_t<sizeof(num_)>(num_));
}

template <size_t min_size_v, std::unsigned_integral T_>
constexpr  inline size_t uint_to_dec_par_impl_(
    char* const buffer, const size_t final_dec_width, const T_ num_) noexcept;
template <size_t size_v, std::unsigned_integral T>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t uint_to_dec_pre_calc_impl_(
    char* buffer, const size_t dec_width_0_, T num_) noexcept {
  if constexpr (sizeof(T) != 8) {
    return uint_to_dec_par_impl_<size_v>(buffer, dec_width_0_, num_);
  }
  return uint_to_dec_pre_calc_impl_seq_less_mul_<size_v, T>(buffer,
                                                            dec_width_0_, num_);
}

template <std::integral T>
constexpr  inline size_t integral_to_dec_impl_(char* buffer,
                                                     const size_t cap,
                                                     T num_) noexcept {
  const bool is_neg = num_ < 0;
  using u_t = std::make_unsigned_t<decltype(num_)>;
  const auto abs_n = is_neg ? u_t(u_t(~u_t(num_)) + 1) : u_t(num_);
  const int width_ = dec_width(abs_n);
  if (cap < size_t(width_ + ((num_ == 0) || is_neg))) {
    return 0;
  }
  *buffer = '-';
  buffer += is_neg;
  return size_t(is_neg + uint_to_dec_pre_calc_impl_<1, u_t>(
                             buffer, size_t(width_), u_t(abs_n)));
}
template <size_t size_v, std::integral T>
  requires(int_to_dec_unchekced_size_v<T> <= size_v)
constexpr  inline size_t integral_to_dec_impl_unchecked_(
    char* buffer, T num_) noexcept {
  const bool is_neg = num_ < 0;
  using u_t = std::make_unsigned_t<decltype(num_)>;
  const auto abs_n = is_neg ? u_t(u_t(~u_t(num_)) + 1) : u_t(num_);
  const int width_ = dec_width(abs_n);
  *buffer = '-';
  buffer += is_neg;
  return size_t(is_neg + uint_to_dec_pre_calc_impl_<size_v, u_t>(
                             buffer, size_t(width_), u_t(abs_n)));
}

struct double_64_t_impl_ {
  constexpr inline double_64_t_impl_() noexcept {};

  uint64_t m_coeffient{};
  int64_t m_exponent{};
  constexpr inline std::strong_ordering operator<=>(
      const double_64_t_impl_& rhs) const noexcept {
    int const lhs_w = std::countl_zero(m_coeffient);
    int const rhs_w = std::countl_zero(rhs.m_coeffient);
    std::strong_ordering ret = m_exponent + rhs_w <=> rhs.m_exponent + lhs_w;
    std::strong_ordering ret2 =
        (m_coeffient << (63 & (lhs_w))) <=> (rhs.m_coeffient << (63 & (rhs_w)));
    return ret != ret.equal ? ret : ret2;
  }

  [[maybe_unused]] constexpr  inline std::weak_ordering
  operator_spaceship_idk_has_bug(
      double_64_t_impl_ lhs, double_64_t_impl_ rhs,
      uint64_t relative_epsilon = uint64_t(1 << 16)) noexcept {
    int const lhs_w = 63 & std::countl_zero(lhs.m_coeffient);
    int const rhs_w = 63 & std::countl_zero(rhs.m_coeffient);
    lhs.m_exponent -= rhs_w;
    rhs.m_exponent -= lhs_w;
    uint64_t& lhs_i = lhs.m_coeffient <<= lhs_w;
    uint64_t& rhs_i = rhs.m_coeffient <<= rhs_w;
    bool swapped{lhs.m_exponent < rhs.m_exponent};
    std::swap(lhs, swapped ? rhs : lhs);
    const uint32_t delta_exp = uint32_t(lhs.m_exponent - rhs.m_exponent);
    std::weak_ordering ret = delta_exp <=> 1;
    std::weak_ordering ret2 = lhs_i <=> rhs_i;
    int64_t delta =
        int64_t((lhs_i >> 1) - (rhs_i >> (1 + (ret == ret.equivalent))));
    delta = std::max(delta, -delta);
    ret2 = (uint64_t(delta) < relative_epsilon) ? std::weak_ordering::equivalent
                                                : ret2;
    ret = ret == ret.greater ? ret : ret2;
    ret2 = 0 <=> ret;
    ret = swapped ? ret2 : ret;
    return ret;
  }

  constexpr double_64_t_impl_(double val) noexcept : double_64_t_impl_() {
    // In the IEEE 754 standard binary64
    const uint64_t u64_val = std::bit_cast<uint64_t>(val);
    constexpr uint64_t exp_mask = ((uint64_t(1) << 11) - 1) << 52;
    constexpr uint64_t mantisa_mask = ((uint64_t(1) << 52) - 1);
    mjz_assume_impl_(!(u64_val >> 63));
    // no nanny or infs
    mjz_assume_impl_((u64_val & exp_mask) != exp_mask);
    const bool is_subnormal = !(u64_val & exp_mask);
    const int64_t exponent = int64_t(u64_val >> 52) - 1023 - 52 + is_subnormal;
    const uint64_t coeffient =
        uint64_t((mantisa_mask & u64_val) | (uint64_t(!is_subnormal) << 52));
    double_64_t_impl_ ret{};
    ret.m_coeffient = coeffient;
    ret.m_exponent = exponent & -(coeffient != 0);
    *this = ret;
  };
};

// 1+floor(log_10(x)) , x=0 -> 0
template <int I_0_ = 0>
constexpr  MJZ_JUST_FORCED_INLINE_ int dec_width_dbl_(
    const double_64_t_impl_ x_pos_real_) noexcept {
  uint64_t x = uint64_t(x_pos_real_.m_coeffient);
  const int32_t log2_exp = int32_t(x_pos_real_.m_exponent);
  const bool had_1bit = std::has_single_bit(x);
  const int32_t log2_ceil = int32_t(std::bit_width(x)) - had_1bit + log2_exp;
  const int32_t log10_2_bx = 19729;
  const int32_t bx = 16;
  bool is_neg_log = log2_ceil < 0;
  const int32_t correct_or_1_plus_correct_abs =
      int32_t((uint32_t(is_neg_log ? +(!had_1bit) - log2_ceil : log2_ceil) *
               log10_2_bx) >>
              bx);
  const int32_t correct_or_1_plus_correct = is_neg_log
                                                ? -correct_or_1_plus_correct_abs
                                                : correct_or_1_plus_correct_abs;
  double_64_t_impl_ dbl = *(details_ns::lookup_dbl_pow5_table_ptr_<I_0_> +
                            correct_or_1_plus_correct);
  dbl.m_exponent += correct_or_1_plus_correct;

  const bool is_correct = dbl <= x_pos_real_;
  return correct_or_1_plus_correct + is_correct;
}
template <size_t final_dec_width, std::unsigned_integral T_,
          bool prefer_less_seq_muls = false>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t
uint_to_dec_par_impl_exact_(char* const buffer, const T_ num_) noexcept {
  if constexpr (final_dec_width == 0) {
    mjz_assume_impl_(num_ < 1);
    *buffer = '0';
    return 1;
  }
  if constexpr (final_dec_width == 1) {  // 0 seq , 0 mul

    mjz_assume_impl_(num_ < 10);
    *buffer = char(char(num_) | '0');
    return final_dec_width;
  }

  if constexpr (final_dec_width == 2) {  // 0 seq , 0 mul

    mjz_assume_impl_(num_ < 100);
    cpy_bitcast_impl_(buffer, details_ns::modolo_raidex_table[num_]);
    return final_dec_width;
  }

  if constexpr (final_dec_width == 3) {
    mjz_assume_impl_(num_ < 1000);
    //  constexpr auto ivk = get_devide_inverse_and_shift<>(999, 10, true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{1639Ui64, 0Ui64,
    // 0Ui64}}}, {14Ui64}, {21Ui64}, {0U}, {{{639Ui64, 0U����i64, 0Ui64}}}}}
    //  also for 8 bit we have 255 as max
    // constexpr auto ivk = get_devide_inverse_and_shift<>(255, 10, true);
    constexpr uint32_t shift = sizeof(num_) == 1 ? 11 : 14;
    constexpr uint32_t mul_inv10 = sizeof(num_) == 1 ? 205 : 1639;
    constexpr uint32_t mask = (uint32_t(1) << shift) - 1;
    const uint32_t n = mul_inv10 * uint16_t(num_);
    cpy_bitcast_impl_(buffer, details_ns::modolo_raidex_table[(n >> shift)]);
    buffer[2] = char(char((n & mask) * 5 >> (shift - 1)) | '0');
    // 2 seq , 2 muls ~~
    return final_dec_width;
  }
  if constexpr (sizeof(num_) == 1) {
    mjz_unreachable_impl_();
    return final_dec_width;
  }

  if constexpr (final_dec_width == 4) {
    mjz_assume_impl_(num_ < 10000);
    // constexpr auto ivk = get_devide_inverse_and_shift<>(9999, 100, true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{5243Ui64, 0Ui64,
    // 0Ui64}}}, {19Ui64}, {27Ui64}, {0U}, {{{12799Ui64, ����0Ui64, 0Ui64}}}}}
    constexpr uint32_t shift = 19;
    constexpr uint32_t mul_inv10 = 5243;
    constexpr uint32_t mask = (uint32_t(1) << shift) - 1;
    const uint32_t n = mul_inv10 * uint32_t(num_);
    cpy_bitcast_impl_(buffer, details_ns::modolo_raidex_table[(n >> shift)]);
    cpy_bitcast_impl_(buffer + 2,
                      details_ns::modolo_raidex_table[uint32_t(
                          uint32_t((n & mask) * 25) >> (shift - 2))]);
    // 2 seq , 2 muls
    return final_dec_width;
  }

  if constexpr (final_dec_width == 5) {
    mjz_assume_impl_(num_ < 100000);
    if constexpr (prefer_less_seq_muls) {
      // first 2 digts
      // constexpr auto ivk = get_devide_inverse_and_shift<>(99999, 1000, true);
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{67109Ui64,
      // 0Ui64, 0Ui64}}}, {26Ui64}, {34Ui64}, {0U}, {{{127999Ui64,����0Ui64,
      // 0Ui64}}}}}
      //
      //
      //--------------------------
      // constexpr auto ivk = get_devide_inverse_and_shift<>(99999, 100, true);
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{167773Ui64,
      // 0Ui64, 0Ui64}}}, {24Ui64}, {35Ui64}, {0U}, {{{102399Ui64����, 0Ui64,
      // 0Ui64}}}}}
      // last 2 digits
      //-----------
      // 24+34=58 so no overflow
      //
      // also we need 10 spare bits ( 5 bits for each high)
      // (26+24)+10=60<=64
      // so its good
      //     ( overflow high | 24+5 bit high| 34 bit low)
      //
      constexpr uint64_t safe_high_offset = 34;
      constexpr uint64_t low_digit_offset = 26;
      constexpr uint64_t safe_high_shift = safe_high_offset + 24;
      constexpr uint64_t mul_pair =
          (uint64_t(167773) << safe_high_offset) | 67109;
      constexpr uint64_t first_2_digit_mask =
          (uint64_t(1) << safe_high_offset) - (uint64_t(1) << low_digit_offset);
      constexpr uint64_t low_half_mask = (uint64_t(1) << low_digit_offset) - 1;
      constexpr uint64_t high_half_mask = (uint64_t(1) << (safe_high_shift)) -
                                          (uint64_t(1) << safe_high_offset);
      constexpr uint64_t res_pair_mask = low_half_mask | high_half_mask;

      const uint64_t res_pair = mul_pair * num_;
      // XX000
      cpy_bitcast_impl_(
          buffer, details_ns::modolo_raidex_table[(
                      (res_pair & first_2_digit_mask) >> low_digit_offset)]);
      const uint64_t second_mul_pair = res_pair & res_pair_mask;
      const uint64_t second_res_pair = second_mul_pair * 25;
      // 25*4 == 100
      // we do the power of 2 offset
      // they also coindecedtally are byte boundries so its not even a shift/and
      // but a byte register load
      constexpr uint64_t second_safe_high_shift = safe_high_shift - 2;
      constexpr uint64_t second_2_digit_mask = first_2_digit_mask >> 2;
      constexpr uint64_t second_low_digit_offset = low_digit_offset - 2;

      // 000XX
      const uint64_t high_digit = second_res_pair >> second_safe_high_shift;
      // 00XX0
      const uint64_t low_digit =
          (second_res_pair & second_2_digit_mask) >> second_low_digit_offset;

      // these overlap and override the excact saame char and thats ok...
      cpy_bitcast_impl_(buffer + 2, details_ns::modolo_raidex_table[low_digit]);
      cpy_bitcast_impl_(buffer + 3,
                        details_ns::modolo_raidex_table[high_digit]);
      // 2 seq , 2 muls
      return final_dec_width;
    }

    constexpr uint64_t inv1000_val = 67109;
    constexpr uint64_t shift_val = 26;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv1000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);

    const uint32_t second_res = uint32_t(mask_val & res) * 25;  // 34-2=32 bit
    constexpr uint32_t second_shift_val = shift_val - 2;
    constexpr uint32_t second_mask_val = (uint32_t(1) << second_shift_val) - 1;
    cpy_bitcast_impl_(
        buffer + 2,
        details_ns::modolo_raidex_table[(second_res >> second_shift_val)]);

    const uint32_t third_res = (second_mask_val & (second_res)) * 5;
    constexpr uint32_t third_shift_val = second_shift_val - 1;
    buffer[4] = char(third_res >> third_shift_val) | '0';
    return final_dec_width;
  }
  if constexpr (sizeof(num_) == 2) {
    mjz_unreachable_impl_();
    return final_dec_width;
  }

  if constexpr (final_dec_width == 6) {
    mjz_assume_impl_(num_ < 1000000);
    // constexpr auto ivk = get_devide_inverse_and_shift<>(999999, 100, true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{671089Ui64,
    // 0Ui64, 0Ui64}}}, {26Ui64}, {40Ui64}, {0U}, {{{819199Ui64����, 0Ui64,
    // 0Ui64}}}}}
    //------------
    // constexpr auto ivk = get_devide_inverse_and_shift<>(999999, 10000, true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{1717987Ui64,
    // 0Ui64, 0Ui64}}}, {34Ui64}, {41Ui64}, {0U}, {{{639999U����i64, 0Ui64,
    // 0Ui64}}}}}
    //-----------
    // so.... 26+41=67 ... we cant do our neet trick with 64 bit integers
    // we still have 64 bit ints but the compiler often does that by 2x64 , so
    // meh i ran out of cross platform simd tricks , but if i want to do it
    // parralel its 2*2 muls but im not defeated!! hmmm i can turn it into 3
    // sequential muls i geuss :/

    constexpr uint64_t inv10000_val = 1717987;
    constexpr uint64_t shift_val = 34;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv10000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);

    const uint64_t second_res = (mask_val & res) * 25;
    constexpr uint64_t second_shift_val = shift_val - 2;
    constexpr uint64_t second_mask_val = (uint64_t(1) << second_shift_val) - 1;
    cpy_bitcast_impl_(
        buffer + 2,
        details_ns::modolo_raidex_table[(second_res >> second_shift_val)]);

    const uint64_t third_res = (second_mask_val & (second_res)) * 25;
    constexpr uint64_t third_shift_val = second_shift_val - 2;
    cpy_bitcast_impl_(
        buffer + 4,
        details_ns::modolo_raidex_table[(third_res >> third_shift_val)]);
    // 3 seq , 3 muls
    return final_dec_width;
  }

  if constexpr (final_dec_width == 7) {
    mjz_assume_impl_(num_ < 10000000);
    if constexpr (prefer_less_seq_muls) {
      // we can do a neat trick here to
      // achieve 4 muls !!
      // constexpr auto ivk = get_devide_inverse_and_shift<>(9'999'999, 100,
      // true);
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{21474837Ui64,
      // 0Ui64, 0Ui64}}}, {31Ui64}, {49Ui64}, {0U}, {{{����13107199Ui64, 0Ui64,
      // 0Ui64}}}}}
      constexpr uint64_t inv100_val = 21474837;
      constexpr uint64_t shift_val = 31;
      constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
      const uint64_t res = inv100_val * num_;
      uint_to_dec_par_impl_exact_<5>(buffer, uint32_t(res >> shift_val));
      constexpr uint64_t second_shift_val = shift_val - 2;
      const uint64_t second_res = (mask_val & res) * 25;
      cpy_bitcast_impl_(
          buffer + 5,
          details_ns::modolo_raidex_table[(second_res >> second_shift_val)]);
      // 3 seq , 4 mul
      return final_dec_width;
    }
    // constexpr auto ivk =  get_devide_inverse_and_shift<>(9'999'999, 100000,
    // true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{21990233Ui64,
    // 0Ui64, 0Ui64}}}, {41Ui64}, {49Ui64}, {0U}, {{{����12799999Ui64, 0Ui64,
    // 0Ui64}}}}}
    constexpr uint64_t inv100000_val = 21990233;
    constexpr uint64_t shift_val = 41;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv100000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);
    const uint64_t res2 = (res & mask_val) * 25;
    cpy_bitcast_impl_(
        buffer + 2, details_ns::modolo_raidex_table[(res2 >> (shift_val - 2))]);
    const uint64_t res3 = (res2 & (mask_val >> 2)) * 25;
    cpy_bitcast_impl_(
        buffer + 4, details_ns::modolo_raidex_table[(res3 >> (shift_val - 4))]);
    const uint64_t res4 = (res3 & (mask_val >> 4)) * 5;
    buffer[6] = char(char(res4 >> (shift_val - 5)) | '0');
    return final_dec_width;  // 4 seq , 4 muls
  }

  if constexpr (final_dec_width == 8) {
    mjz_assume_impl_(num_ < 100000000);
    if constexpr (prefer_less_seq_muls) {
      // we can do a neat trick here to achieve 4 muls  !! ((N/2) seq vs (N-2)
      // par)
      //  sadly we either have to do 3x2 muls or 4 muls ...
      // -----------
      // but we can do 5 muls( *5 is just add and shift)  with 3 seq!!
      // constexpr auto ivk = get_devide_inverse_and_shift<>(99'999'999, 1000,
      // true); C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk =
      // {{{{{68719477Ui64, 0Ui64, 0Ui64}}}, {36Ui64}, {54Ui64}, {0U},
      // {{{����131071999Ui64, 0Ui64, 0Ui64}}}}}
      constexpr uint64_t inv1000_val = 68719477;
      constexpr uint64_t shift_val = 36;
      constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
      const uint64_t res = inv1000_val * num_;
      uint_to_dec_par_impl_exact_<5>(buffer, uint32_t(res >> shift_val));
      constexpr uint64_t second_shift_val = shift_val - 2;
      const uint64_t second_res = (mask_val & res) * 25;
      constexpr uint64_t second_mask_val =
          (uint64_t(1) << second_shift_val) - 1;
      cpy_bitcast_impl_(
          buffer + 5,
          details_ns::modolo_raidex_table[(second_res >> second_shift_val)]);
      const uint64_t third_res = (second_mask_val & (second_res)) * 5;
      constexpr uint64_t third_shift_val = second_shift_val - 1;
      buffer[7] = char(char(third_res >> third_shift_val) | '0');
      // total: 3 seq max , 5 muls
      return final_dec_width;
    }
    // constexpr auto ivk = get_devide_inverse_and_shift<>(99'999'999, 1000000,
    // true); C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk =
    // {{{{{140737489Ui64, 0Ui64, 0Ui64}}}, {47Ui64}, {55Ui64}, {0U},
    // {{{����127999999Ui64, 0Ui64, 0Ui64}}}}}
    constexpr uint64_t inv1000000_val = 140737489;
    constexpr uint64_t shift_val = 47;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv1000000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);
    const uint64_t res2 = (res & mask_val) * 25;
    cpy_bitcast_impl_(
        buffer + 2, details_ns::modolo_raidex_table[(res2 >> (shift_val - 2))]);
    const uint64_t res3 = (res2 & (mask_val >> 2)) * 25;
    cpy_bitcast_impl_(
        buffer + 4, details_ns::modolo_raidex_table[(res3 >> (shift_val - 4))]);
    const uint64_t res4 = (res3 & (mask_val >> 4)) * 25;
    cpy_bitcast_impl_(
        buffer + 6, details_ns::modolo_raidex_table[(res4 >> (shift_val - 6))]);
    return final_dec_width;  // 4 seq , 4 muls
  }

  if constexpr (final_dec_width == 9) {
    mjz_assume_impl_(num_ < 1000000000);
    if constexpr (prefer_less_seq_muls) {
      //  constexpr auto ivk = get_devide_inverse_and_shift<>(999'999'999,
      //  10000, true);
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{1759218605Ui64,
      // 0Ui64, 0Ui64}}}, {44Ui64}, {61Ui64}, {0U}, {{{����655359999Ui64, 0Ui64,
      // 0Ui64}}}}}
      // so we can get either 5x2 muls or 6muls with this trick! ( or 5 seq muls
      // , but thats too much i think)
      constexpr uint64_t inv10000_val = 1759218605;
      constexpr uint64_t shift_val = 44;
      constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
      const uint64_t res = inv10000_val * num_;
      uint_to_dec_par_impl_exact_<5>(buffer,
                                     uint32_t(res >> shift_val));  // 2 seq muls
      // 3X
      constexpr uint64_t second_shift_val = shift_val - 2;
      constexpr uint64_t second_mask_val =
          (uint64_t(1) << second_shift_val) - 1;

      const uint64_t second_res = ((res & mask_val) * 25);
      cpy_bitcast_impl_(
          buffer + 5,
          details_ns::modolo_raidex_table[(second_res >> second_shift_val)]);

      constexpr uint64_t third_shift_val = second_shift_val - 2;
      const uint64_t third_res = (second_res & second_mask_val) * 25;
      cpy_bitcast_impl_(
          buffer + 7,
          details_ns::modolo_raidex_table[(third_res >> third_shift_val)]);
      // total: 3 seq max , 5 muls
      return final_dec_width;
    }
    // constexpr auto ivk = get_devide_inverse_and_shift<>(99'999'9999,
    // 10000000, true);
    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{1801439851Ui64,
    // 0Ui64, 0Ui64}}}, {54Ui64}, {61Ui64}, {0U}, {{{����639999999Ui64, 0Ui64,
    // 0Ui64}}}}}
    constexpr uint64_t inv10000000_val = 1801439851;
    constexpr uint64_t shift_val = 54;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv10000000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);
    const uint64_t res2 = (res & mask_val) * 25;
    cpy_bitcast_impl_(
        buffer + 2, details_ns::modolo_raidex_table[(res2 >> (shift_val - 2))]);
    const uint64_t res3 = (res2 & (mask_val >> 2)) * 25;
    cpy_bitcast_impl_(
        buffer + 4, details_ns::modolo_raidex_table[(res3 >> (shift_val - 4))]);
    const uint64_t res4 = (res3 & (mask_val >> 4)) * 25;
    cpy_bitcast_impl_(
        buffer + 6, details_ns::modolo_raidex_table[(res4 >> (shift_val - 6))]);
    const uint64_t res5 = (res4 & (mask_val >> 6)) * 5;
    buffer[8] = char(char(res5 >> (shift_val - 7)) | '0');
    return final_dec_width;  // 5 seq , 5 muls
  }

  if constexpr (final_dec_width == 10) {
    if constexpr (prefer_less_seq_muls || sizeof(num_) != 4) {
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk =
      // {{{{{22517998137Ui64, 0Ui64, 0Ui64}}}, {51Ui64}, {69Ui64}, {0U},
      // {{{����13107199999Ui64, 0Ui64, 0Ui64}}}}} i ran out of 64 bits :(
      // also
      //  constexpr auto ivk = get_devide_inverse_and_shift<>(9'999'999'999,
      //  100000000, true);
      // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk =
      // {{{{{23058430093Ui64, 0Ui64, 0Ui64}}}, {61Ui64}, {69Ui64}, {0U},
      // {{{����12799999999Ui64, 0Ui64, 0Ui64}}}}} , :(
      //  (64<69)
      //  i now need to do it with compiler magic
      //  128 bit ints or just let the compiler do its thing.
      //  but if we assume each are mulx'es its 5 mul(x)'es in total
      mjz_assume_impl_(num_ < 10000000000);
      const auto high = num_ / 100000;
      mjz_assume_impl_(high < 100000);
      const auto low = num_ % 100000;
      uint_to_dec_par_impl_exact_<5>(buffer, uint32_t(high));     // 2 seq muls
      uint_to_dec_par_impl_exact_<5>(buffer + 5, uint32_t(low));  // 3 seq muls
      // total: 4 seq max , 6 muls
      return final_dec_width;
    }

    // C++ constexpr mjz::tuple_t<...> used_mjz_ns::ivk = {{{{{1441151881Ui64,
    // 0Ui64, 0Ui64}}}, {57Ui64}, {63Ui64}, {0U}, {{{����3199999999Ui64, 0Ui64,
    // 0Ui64}}}}} constexpr auto ivk
    // =get_devide_inverse_and_shift<>(uint32_t(-1), 100000000, true);
    constexpr uint64_t inv100000000_val = 1441151881;
    constexpr uint64_t shift_val = 57;
    constexpr uint64_t mask_val = (uint64_t(1) << shift_val) - 1;
    const uint64_t res = inv100000000_val * num_;
    cpy_bitcast_impl_(buffer,
                      details_ns::modolo_raidex_table[(res >> shift_val)]);
    const uint64_t res2 = (res & mask_val) * 25;
    cpy_bitcast_impl_(
        buffer + 2, details_ns::modolo_raidex_table[(res2 >> (shift_val - 2))]);
    const uint64_t res3 = (res2 & (mask_val >> 2)) * 25;
    cpy_bitcast_impl_(
        buffer + 4, details_ns::modolo_raidex_table[(res3 >> (shift_val - 4))]);
    const uint64_t res4 = (res3 & (mask_val >> 4)) * 25;
    cpy_bitcast_impl_(
        buffer + 6, details_ns::modolo_raidex_table[(res4 >> (shift_val - 6))]);
    const uint64_t res5 = (res4 & (mask_val >> 6)) * 25;
    cpy_bitcast_impl_(
        buffer + 8, details_ns::modolo_raidex_table[(res5 >> (shift_val - 8))]);
    return final_dec_width;  // 5 seq , 5 muls
  }

  if constexpr (sizeof(num_) == 4) {
    mjz_unreachable_impl_();
    return final_dec_width;
  }

  if constexpr (final_dec_width == 11) {  // 4 seq , 7 muls
    mjz_assume_impl_(num_ < 100000000000);
    uint_to_dec_par_impl_exact_<6>(buffer, uint32_t(num_ / 100000));
    uint_to_dec_par_impl_exact_<5>(buffer + 6, uint32_t(num_ % 100000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 12) {  // 5 seq , 8 muls
    mjz_assume_impl_(num_ < 1000000000000);
    uint_to_dec_par_impl_exact_<6>(buffer, uint32_t(num_ / 1000000));
    uint_to_dec_par_impl_exact_<6>(buffer + 6, uint32_t(num_ % 1000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 13) {  // 5 seq,9 muls
    mjz_assume_impl_(num_ < 10000000000000);
    uint_to_dec_par_impl_exact_<7>(buffer, uint32_t(num_ / 1000000));
    uint_to_dec_par_impl_exact_<6>(buffer + 7, uint32_t(num_ % 1000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 14) {  // 5 seq,10 muls
    mjz_assume_impl_(num_ < 100000000000000);
    uint_to_dec_par_impl_exact_<7>(buffer, uint32_t(num_ / 10000000));
    uint_to_dec_par_impl_exact_<7>(buffer + 7, uint32_t(num_ % 10000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 15) {  // 5 seq,11 muls
    mjz_assume_impl_(num_ < 1000000000000000);
    uint_to_dec_par_impl_exact_<8>(buffer, uint32_t(num_ / 10000000));
    uint_to_dec_par_impl_exact_<7>(buffer + 8, uint32_t(num_ % 10000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 16) {  // 5 seq,12 muls
    mjz_assume_impl_(num_ < 10000000000000000);
    uint_to_dec_par_impl_exact_<8>(buffer, uint32_t(num_ / 100000000));
    uint_to_dec_par_impl_exact_<8>(buffer + 8, uint32_t(num_ % 100000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 17) {  // 5 seq,12 muls
    mjz_assume_impl_(num_ < 100000000000000000);
    uint_to_dec_par_impl_exact_<9>(buffer, uint32_t(num_ / 100000000));
    uint_to_dec_par_impl_exact_<8>(buffer + 9, uint32_t(num_ % 100000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 18) {  // 5 seq,12 muls
    mjz_assume_impl_(num_ < 1000000000000000000);
    uint_to_dec_par_impl_exact_<9>(buffer, uint32_t(num_ / 1000000000));
    uint_to_dec_par_impl_exact_<9>(buffer + 9, uint32_t(num_ % 1000000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 19) {  // 5 seq,13 muls
    mjz_assume_impl_(num_ < 10000000000000000000ull);
    uint_to_dec_par_impl_exact_<10>(buffer, uint64_t(num_ / 1000000000));
    uint_to_dec_par_impl_exact_<9>(buffer + 10, uint32_t(num_ % 1000000000));
    return final_dec_width;
  }

  if constexpr (final_dec_width == 20) {  // 6 seq,14 muls
    uint_to_dec_par_impl_exact_<10>(buffer, uint32_t(num_ / 10000000000));
    uint_to_dec_par_impl_exact_<10>(buffer + 10, uint64_t(num_ % 10000000000));
    return final_dec_width;
  }
  mjz_unreachable_impl_();
  return final_dec_width;
}

template <size_t final_dec_width, std::unsigned_integral T_>
constexpr  size_t uint_to_dec_par_impl_exact_nl_(char* const buffer,
                                                       const T_ num_) noexcept {
  return uint_to_dec_par_impl_exact_<final_dec_width>(buffer, num_);
}

template <std::unsigned_integral T_, size_t... Is>
constexpr  MJZ_JUST_FORCED_INLINE_ size_t uint_to_dec_par_impl_helper_(
    char* const buffer, const size_t final_dec_width, const T_ num_,
    std::index_sequence<Is...>) noexcept {
  bool b{};
  b = ((b = final_dec_width == Is,
        b && uint_to_dec_par_impl_exact_<Is>(buffer, num_), b) ||
       ...);
  mjz_assume_impl_(b);
  return final_dec_width;
}
template <size_t min_size_v, std::unsigned_integral T_>
constexpr  inline size_t uint_to_dec_par_impl_(
    char* const buffer, const size_t final_dec_width, const T_ num_) noexcept {
  return uint_to_dec_par_impl_helper_(buffer, final_dec_width, num_,
                                      std::make_index_sequence<21>());
}
template <size_t min_size_v = 0, std::integral T_>
constexpr  inline size_t int_to_dec_par(char* buffer,
                                              const size_t buf_size,
                                              const T_ num_may_neg_) noexcept {
  const bool is_neg = num_may_neg_ < 0;
  using u_t = std::make_unsigned_t<decltype(num_may_neg_)>;
  const auto num_ =
      is_neg ? u_t(u_t(~u_t(num_may_neg_)) + 1) : u_t(num_may_neg_);
  const size_t final_dec_width = size_t(dec_width(num_) | (num_ == 0));
  if constexpr (min_size_v < size_t(int_to_dec_unchekced_size_v<T_>)) {
    if (buf_size < final_dec_width + is_neg) return 0;
  }
  mjz_assume_impl_(0 < final_dec_width);
  if constexpr (std::is_signed_v<T_>) {
    *buffer = '-';
    buffer += is_neg;
  }
  //  constexpr size_t min_uint_size_v = min_size_v - std::is_signed_v<T_>;

  if (num_ < 10) {
    mjz_assume_impl_(1 == final_dec_width);
    return is_neg +
           uint_to_dec_par_impl_exact_<1, uint8_t>(buffer, uint8_t(num_));
  }
  mjz_assume_impl_(1 < final_dec_width);
  if (num_ < 100) {
    mjz_assume_impl_(2 == final_dec_width);
    return is_neg +
           uint_to_dec_par_impl_exact_<2, uint8_t>(buffer, uint8_t(num_));
  }
  if constexpr (sizeof(T_) == 1) {
    return is_neg +
           uint_to_dec_par_impl_exact_<3, uint8_t>(buffer, uint8_t(num_));
  }
  mjz_assume_impl_(2 < final_dec_width);
  if (uint16_t(num_) == num_) {
    if (num_ < 1000) {
      mjz_assume_impl_(final_dec_width == 3);
    } else if (num_ < 10000) {
      mjz_assume_impl_(final_dec_width == 4);
    } else {
      mjz_assume_impl_(final_dec_width == 5);
    }
    if constexpr (false) {
      std::array<char, 8> buf_{};
      uint_to_dec_par_impl_exact_<5, uint16_t>(buf_.data(), uint16_t(num_));
      if constexpr (std::endian::big == std::endian::native) {
        buf_ = std::bit_cast<std::array<char, 8>>(std::bit_cast<uint64_t>(buf_)
                                                  << (5 - final_dec_width));
      } else {
        buf_ = std::bit_cast<std::array<char, 8>>(
            std::bit_cast<uint64_t>(buf_) >> (5 - final_dec_width));
      }
      buffer[0] = buf_[0];
      buffer[1] = buf_[1];
      buffer[2] = buf_[2];
      buffer[3] = buf_[3];
      buffer[4] = buf_[4];
      return is_neg + final_dec_width;
    }
    return is_neg + uint_to_dec_par_impl_helper_<uint16_t>(
                        buffer, final_dec_width, uint16_t(num_),
                        std::make_index_sequence<21>());
  }
  mjz_assume_impl_(4 < final_dec_width);
  if (num_ < 100000) {
    mjz_assume_impl_(5 == final_dec_width);
    return is_neg +
           uint_to_dec_par_impl_exact_<5, uint32_t>(buffer, uint32_t(num_));
  }
  mjz_assume_impl_(5 < final_dec_width);
  if (uint32_t(num_) == num_) {
    if (num_ < 1000000) {
      mjz_assume_impl_(final_dec_width == 6);
    } else if (num_ < 10000000) {
      mjz_assume_impl_(final_dec_width == 7);
    } else if (num_ < 100000000) {
      mjz_assume_impl_(final_dec_width == 8);
    } else if (num_ < 1000000000) {
      mjz_assume_impl_(final_dec_width == 9);
    } else {
      mjz_assume_impl_(final_dec_width == 10);
    }
    return is_neg + uint_to_dec_par_impl_helper_<uint32_t>(
                        buffer, final_dec_width, uint32_t(num_),
                        std::make_index_sequence<21>());
  }
  mjz_assume_impl_(9 < final_dec_width);
  uint64_t low_half{num_};
  if (10000000000 <= num_) {
    mjz_assume_impl_(10 < final_dec_width);
    if (num_ < 100000000000) {
      mjz_assume_impl_(final_dec_width == 11);
    } else if (num_ < 1000000000000) {
      mjz_assume_impl_(final_dec_width == 12);
    } else if (num_ < 10000000000000) {
      mjz_assume_impl_(final_dec_width == 13);
    } else if (num_ < 100000000000000) {
      mjz_assume_impl_(final_dec_width == 14);
    } else if (num_ < 1000000000000000) {
      mjz_assume_impl_(final_dec_width == 15);
    } else if (num_ < 10000000000000000) {
      mjz_assume_impl_(final_dec_width == 16);
    } else if (num_ < 100000000000000000) {
      mjz_assume_impl_(final_dec_width == 17);
    } else if (num_ < 1000000000000000000) {
      mjz_assume_impl_(final_dec_width == 18);
    } else if (num_ < 10000000000000000000ull) {
      mjz_assume_impl_(final_dec_width == 19);
    } else {
      mjz_assume_impl_(final_dec_width == 20);
    }
    uint_to_dec_par_impl_helper_<uint32_t>(buffer, final_dec_width - 10,
                                           uint32_t(num_ / 10000000000),
                                           std::make_index_sequence<21>());
    buffer += final_dec_width - 10;

    low_half = uint64_t(num_ % 10000000000);
  } else {
    mjz_assume_impl_(10 == final_dec_width);
  }
  uint_to_dec_par_impl_exact_<10, uint64_t>(buffer, low_half);
  return is_neg + final_dec_width;
}
}  // namespace details_ns

constexpr  inline size_t int_to_dec(char* buffer, const size_t cap,
                                          std::integral auto num_) noexcept {
  if constexpr (true) return details_ns::int_to_dec_par(buffer, cap, num_);

  return details_ns::integral_to_dec_impl_(buffer, cap, num_);
}
template <size_t size_v, std::integral T>
  requires(int_to_dec_unchekced_size_v<T> <= size_v)
constexpr  inline size_t int_to_dec_unchecked(char* buffer,
                                                    T num_) noexcept {
  if constexpr (true)
    return details_ns::int_to_dec_par<size_v>(buffer, size_v, num_);
  return details_ns::integral_to_dec_impl_unchecked_<size_v, T>(buffer, num_);
}

}  // namespace uint_to_ascci_ns0
};  // namespace mjz


#ifndef MJZ_string_lib_macros_
#undef MJZ_JUST_FORCED_INLINE_ 
#undef MJZ_JUST_UNREACHABLE_
#endif
#endif  // MJZ_UINTCONV_LIB_HPP_FILE_