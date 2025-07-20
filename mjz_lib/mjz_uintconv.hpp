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

#ifndef MJZ_STD_HAS_SIMD_LIB_
#if defined(__cpp_lib_experimental_parallel_simd) || \
    defined(_LIBCPP_ENABLE_EXPERIMENTAL)
#define MJZ_STD_HAS_SIMD_LIB_ true
#else

#define MJZ_STD_HAS_SIMD_LIB_ false
#endif
#endif  // ! MJZ_STD_HAS_SIMD_LIB_

#include <stdint.h>

#include <array>
#include <bit>
#include <concepts>
#include <cstring>
#include <string>
namespace mjz {
inline namespace uint_to_ascci_ns0 {
namespace details_ns {

static constexpr inline std::array<uint64_t, 9> inv_p10_b57 = []() noexcept {
  std::array<uint64_t, 9> ret{};
  ret[0] = (uint64_t(1) << 57);
  for (size_t i{1}; i < 9; i++) {
    ret[i] = (ret[i - 1] / 10) + !!(ret[i - 1] % 10);
  }
  return ret;
}();
constexpr static inline std::array<uint64_t, 20> floor10_table = []() noexcept {
  std::array<uint64_t, 20> ret{};
  ret[0] = 1;
  for (size_t i{1}; i < 20; i++) {
    ret[i] = ret[i - 1] * 10;
  }
  return ret;
}();

constexpr std::span<const uint64_t, 10> pow_ten_table =
    std::span(floor10_table).subspan<1, 10>();

static constexpr inline std::array<uint16_t, 101> radix_ascii_p2_v_ =
    std::bit_cast<std::array<uint16_t, 101>>(
        std::array<char, 202>{"0001020304050607080910111213141516171819"
                              "2021222324252627282930313233343536373839"
                              "4041424344454647484950515253545556575859"
                              "6061626364656667686970717273747576777879"
                              "8081828384858687888990919293949596979899\0"});
constexpr const inline static size_t lookup_dbl_pow5_table_len_ = 325;
template <int = 0>
constexpr auto const inline static lookup_dbl_pow5_table_ = []() noexcept {
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
static inline constexpr const double* const lookup_dbl_pow5_table_ptr_ =
    lookup_dbl_pow5_table_<i>.data() + lookup_dbl_pow5_table_len_;

constexpr static auto simd_8digit_conv_u64(auto n) noexcept {
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
constexpr static const std::span<const uint16_t, 100> modolo_raidex_table =
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
static constexpr inline const uint16_t* radix_ascii_p2_ =
    radix_ascii_p2_v_.data();

constexpr static uint64_t lookup_iota_8digits_ascii(const uint64_t n) noexcept {
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

[[maybe_unused]] constexpr static uint64_t lookup_iota_8digits(
    const uint64_t n) noexcept {
  return lookup_iota_8digits_ascii(n) & ~ascii_offset;
}

[[maybe_unused]] constexpr static uint64_t lookup_iota_8digits_ascii_noif(
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
[[maybe_unused]] constexpr static std::array<uint16_t, 5>
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

[[maybe_unused]] constexpr static uint64_t
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

[[maybe_unused]] constexpr static auto hybrid_iota_8digits_noif_noload_u64(
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
[[maybe_unused]] constexpr static uint64_t
hybrid_iota_8digits_noif_ascii_noload(uint64_t n) noexcept {
  return hybrid_iota_8digits_noif_noload_u64(n) | ascii_offset;
}

constexpr static uint32_t iota_3digits(uint32_t n) noexcept {
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

constexpr static auto iota_4_u32digits(auto n) noexcept {
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
constexpr static auto iota_2_u16digits(auto n) noexcept {
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

constexpr static uint64_t iota_5_u32_tou64digits(uint64_t n) noexcept {
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
constexpr static std::array<char, 4> iota_forward_3digits(uint32_t n) noexcept {
  uint32_t awnser = iota_3digits(n);
  int num0ch{};
  if constexpr (std::endian::big == std::endian::native) {
    num0ch = std::countl_zero(awnser) >> 3;
    num0ch = std::max(num0ch, 3);
    awnser <<= num0ch << 3;
    awnser |= 4 - num0ch;
  } else {
    num0ch = std::countr_zero(awnser) >> 3;
    num0ch = std::max(num0ch, 3);
    awnser >>= num0ch << 3;
    awnser |= (4 - num0ch) << 24;
  }
  return std::bit_cast<std::array<char, 4>>(awnser |
                                            std::bit_cast<uint32_t>("000"));
}
[[maybe_unused]] constexpr static uint64_t iota_3digits_u64(
    uint32_t n) noexcept {
  return std::bit_cast<uint64_t>(std::array{uint32_t(0), iota_3digits(n)});
}
template <bool branch_less_v = true>
inline std::tuple<std::array<uint64_t, 3>, size_t, size_t>
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
  uint64_t u64ch = word_register[iteration_count_backwards];

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

inline std::tuple<std::array<uint64_t, 2>, size_t, size_t>
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

[[noreturn]] inline static void mjz_unreachable_impl_() noexcept {
#if 1 < _MSC_VER
#define MJZ_forceinline_ __forceinline
  __assume(false);
#elif defined(__GNUC__)
#define MJZ_forceinline_ __attribute__((always_inline))
  __builtin_unreachable();
#else
  static_assert(false, "compiler is not msvc,clang,gcc :(");
#endif
}
constexpr MJZ_forceinline_ static void mjz_assume_impl_(const bool b) noexcept {
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
constexpr static MJZ_forceinline_ int dec_width(
    const std::integral auto x_pos_) noexcept {
  const uint64_t x = uint64_t(x_pos_);
  const int log2_ceil = std::bit_width(x) - std::has_single_bit(x);
  int log10_2_bx = 5;
  int bx = 4;
  using T = std::remove_cvref_t<decltype(x_pos_)>;
  if constexpr (std::same_as<T, uint64_t>) {
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
  const int correct_or_1_plus_correct = ((log2_ceil * log10_2_bx) >> bx);
  const bool is_correct =
      details_ns::floor10_table[size_t(correct_or_1_plus_correct)] <= x;
  return correct_or_1_plus_correct + is_correct;
}

template <std::integral T>
constexpr static MJZ_forceinline_ int signed_dec_width(const T x) noexcept {
  const bool is_neg = x < 0;
  using ut = std::make_unsigned_t<T>;
  return dec_width(is_neg ? ut(~ut(x) + 1) : ut(x)) + is_neg;
}
template <std::integral T>
constexpr static inline size_t int_to_dec_unchekced_size_v =
    size_t(std::max(signed_dec_width(std::numeric_limits<T>::max()),
                    signed_dec_width(std::numeric_limits<T>::min())));

namespace details_ns {

template <size_t size_v, std::unsigned_integral T>
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_semi_par_(
    char* buffer, const size_t dec_width_0_, T num_) noexcept {
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

constexpr static MJZ_forceinline_ void cpy_bitcast_impl_(char* ptr,
                                                         auto v) noexcept {
  if (!std::is_constant_evaluated()) {
    std::memmove(ptr, &v, sizeof(v));
  }
  auto chs = std::bit_cast<std::array<char, sizeof(v)>>(v);
  for (char c : chs) {
    *ptr++ = c;
  }
}
template <typename T>
constexpr static MJZ_forceinline_ T
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
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_seq_(
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

constexpr static MJZ_forceinline_ uint64_t
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
[[maybe_unused]] constexpr static MJZ_forceinline_ uint64_t
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
constexpr static MJZ_forceinline_ uint64_t
inv10p8_b57_num_to_iota_impl_(uint64_t& n) noexcept {
  std::array<uint16_t, 4> indexies{};
  for (uint16_t& index : indexies) {
    n *= 100;
    index = uint8_t(n >> 57);
    n &= uint64_t(-1) >> 7;
  }
  return u16x4_num_to_iota_impl_(std::bit_cast<uint64_t>(indexies));
}

#ifdef __SIZEOF_INT128__
#define MJZ_uint128_type_ unsigned __int128
#elif 1 < _MSC_VER
#define MJZ_uint128_type_ std::_Unsigned128
#else
#endif

[[maybe_unused]] constexpr static MJZ_forceinline_ uint64_t
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

constexpr static MJZ_forceinline_ uint64_t
inv10p8_b57_num_to_iota_impl_ascii_simd_(uint64_t& n) noexcept {
#ifdef MJZ_uint128_type_
  MJZ_uint128_type_ n128 = n;
  MJZ_uint128_type_ mask = uint64_t(-1) >> 7;
  uint8_t indexies[4]{};
  n128 <<= 64;
  n128 |= (n * 10000) & mask;
  mask |= mask << 64;
  n128 *= 100;
  MJZ_uint128_type_ temp0 = (n128 & ~mask);
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
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_seq_lessmul_(
    char* buffer, const size_t dec_width_0_, uint64_t num_) noexcept {
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
    [[maybe_unused]] constexpr uint64_t inv_10p8_b90 =
        uint64_t(12379400392853802749ull);
#ifdef MJZ_uint128_type_
    MJZ_uint128_type_ u128 = num_;
    u128 *= inv_10p8_b90;
    u128 >>= 33;
    n = (uint64_t(u128) & (uint64_t(-1) >> 7));
    num_ = decltype(num_)(uint64_t(u128 >> 57));
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
constexpr static MJZ_forceinline_ size_t
uint_to_dec_pre_calc_impl_seq_lessmul_branching_(char* buffer,
                                                 const size_t dec_width_0_,
                                                 uint32_t num_) noexcept {
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  char*const end_buf = buffer;
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
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_more_mul_(
    char* buffer, const size_t dec_width_0_, uint32_t num_) noexcept {
  mjz_assume_impl_(dec_width_0_ < 11);
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  alignas(32) std::array<char, 32> buf32{};
  constexpr uint64_t mask = uint64_t(-1) >> 7;
  std::array<uint8_t, 5> buf_num{};
#ifdef MJZ_uint128_type_
  constexpr uint32_t inv10p8_b57 = uint32_t(inv_p10_b57[8]);
  static_assert(inv_p10_b57[8] == inv10p8_b57);
  using u128_t_ = MJZ_uint128_type_;
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
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_seq_lessmul_(
    char* buffer, const size_t dec_width_0_, uint32_t num_) noexcept {
  return uint_to_dec_pre_calc_impl_more_mul_<size_v>(
      buffer, dec_width_0_,
                                                     num_);
}
template <size_t size_v>
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_seq_lessmul_(
    char* buffer, const size_t dec_width_0_, uint16_t num_) noexcept {
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
    for (size_t i{}; i < dec_width_0_; i++) {
      buffer[i] = chs[i];
    }
  } else {
    cpy_bitcast_impl_(buffer, temp);
  }
  return dec_width_;
}

template <size_t size_v>
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_seq_lessmul_(
    char* buffer, const size_t dec_width_0_, uint8_t num_) noexcept {
  mjz_assume_impl_(dec_width_0_ < 4);
  const size_t floor_log10 = dec_width_0_ - (0 != dec_width_0_);
  const size_t dec_width_ = floor_log10 + 1;
  constexpr uint32_t inv10_10b = 103;
  constexpr uint32_t mask = uint32_t(uint32_t(-1) >> 22);
  uint32_t temp = inv10_10b * num_;
  alignas(4) std::array<char, 4> chs{};
  const uint16_t* ps1 = radix_ascii_p2_ + ((temp >> 10));
  {
    auto [ch0_, ch1_] = std::bit_cast<std::array<char, 2>>(*ps1);
    chs[0] = ch0_;
    chs[1] = ch1_;
  }

  temp &= mask;
  temp *= 10;
  chs[2] = char((temp >> 10) | '0');
  mjz_assume_impl_((temp >> 10) < 10);
  temp = std::bit_cast<uint32_t>(chs);
  if constexpr (std::endian::big == std::endian::native) {
    temp <<= (3 - dec_width_) << 3;
  } else {
    temp >>= (3 - dec_width_) << 3;
  }
  chs = std::bit_cast<std::array<char, 4>>(temp);

  if constexpr (size_v < sizeof(temp)) {
    for (size_t i{}; i < dec_width_0_; i++) {
      buffer[i] = chs[i];
    }
  } else {
    cpy_bitcast_impl_(buffer, temp);
  }
  return dec_width_;
}

template <size_t size_v, std::unsigned_integral T>
constexpr static MJZ_forceinline_ size_t
uint_to_dec_pre_calc_impl_seq_less_mul_(char* buffer, const size_t dec_width_0_,
                                        T num_) noexcept {
  return uint_to_dec_pre_calc_impl_seq_lessmul_<size_v>(buffer, dec_width_0_,
                                                        num_);
}

template <size_t size_v, std::unsigned_integral T>
constexpr static MJZ_forceinline_ size_t uint_to_dec_pre_calc_impl_(
    char* buffer, const size_t dec_width_0_, T num_) noexcept {
  return uint_to_dec_pre_calc_impl_seq_less_mul_<size_v, T>(buffer,
                                                            dec_width_0_, num_);
}

template <std::integral T>
constexpr static inline size_t integral_to_dec_impl_(char* buffer,
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
constexpr static inline size_t integral_to_dec_impl_unchecked_(
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

  [[maybe_unused]] constexpr static inline std::weak_ordering
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

  constexpr double_64_t_impl_(double val) noexcept
      : double_64_t_impl_(pos_real_dbl_to_bf_impl_(val)) {};

  MJZ_CX_FN double_64_t_impl_ static pos_real_dbl_to_bf_impl_(
      double val) noexcept {
    // In the IEEE 754 standard binary64
    const uint64_t u64_val = std::bit_cast<uint64_t>(val);
    constexpr uint64_t exp_mask = ((uint64_t(1) << 11) - 1) << 52;
    constexpr uint64_t mantisa_mask = ((uint64_t(1) << 52) - 1);
    asserts(asserts.assume_rn, !(u64_val >> 63));
    // no nanny or infs
    asserts(asserts.assume_rn, (u64_val & exp_mask) != exp_mask);
    const bool is_subnormal = !(u64_val & exp_mask);
    const int64_t exponent = int64_t(u64_val >> 52) - 1023 - 52 + is_subnormal;
    const uint64_t coeffient =
        uint64_t((mantisa_mask & u64_val) | (uint64_t(!is_subnormal) << 52));
    double_64_t_impl_ ret{};
    ret.m_coeffient = coeffient;
    ret.m_exponent = exponent & -(coeffient != 0);
    return ret;
  }
};

// 1+floor(log_10(x)) , x=0 -> 0
template <int I_0_ = 0>
constexpr static MJZ_forceinline_ int dec_width_dbl_(
    const double_64_t_impl_ x_pos_real_) noexcept {
  uint64_t x = uint64_t(x_pos_real_.m_coeffient);
  const int log2_exp = int(x_pos_real_.m_exponent);
  const bool had_1bit = std::has_single_bit(x);
  const int log2_ceil = std::bit_width(x) - had_1bit + log2_exp;
  const int log10_2_bx = 19729;
  const int bx = 16;
  bool is_neg_log = log2_ceil < 0;
  const int correct_or_1_plus_correct_abs =
      int((uint32_t(is_neg_log ? +(!had_1bit) - log2_ceil : log2_ceil) *
           log10_2_bx) >>
          bx);
  const int correct_or_1_plus_correct = is_neg_log
                                            ? -correct_or_1_plus_correct_abs
                                            : correct_or_1_plus_correct_abs;
  double_64_t_impl_ dbl = *(details_ns::lookup_dbl_pow5_table_ptr_<I_0_> +
                            correct_or_1_plus_correct);
  dbl.m_exponent += correct_or_1_plus_correct;

  const bool is_correct = dbl <= x_pos_real_;
  return correct_or_1_plus_correct + is_correct;
}

}  // namespace details_ns

constexpr static inline size_t int_to_dec(char* buffer, const size_t cap,
                                          std::integral auto num_) noexcept {
  return details_ns::integral_to_dec_impl_(buffer, cap, num_);
}
template <size_t size_v, std::integral T>
  requires(int_to_dec_unchekced_size_v<T> <= size_v)
constexpr static inline size_t int_to_dec_unchecked(char* buffer,
                                                    T num_) noexcept {
  return details_ns::integral_to_dec_impl_unchecked_<size_v, T>(buffer, num_);
}

}  // namespace uint_to_ascci_ns0
};  // namespace mjz

#endif  // MJZ_UINTCONV_LIB_HPP_FILE_