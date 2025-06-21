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
constexpr static auto simd_8digit_conv_u64(auto n) noexcept {
  constexpr uint64_t inv10p4_b40 = 109951163;
  constexpr uint64_t inv10p2_b19 = 5243;
  constexpr uint64_t inv10p1_b10 = 103;
  constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
  constexpr uint64_t modolo10p4_40b_mask = 0x0000'00ff'ffff'ffff;
  constexpr uint64_t mask_upper_16b = 0xfff8'0000'fff8'0000;
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
  auto upper = holder & mask_upper_16b;

  // Upper 2-digits in lower 16-bits.
  result_high = upper;

  // Lower 2-digits in upper 16-bits.
  result_low = ((holder & ~mask_upper_16b) * 100) & mask_upper_16b;

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
constexpr static const std::array<uint16_t, 100> modolo_raidex_table = []() {
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
}();

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

constexpr static void iota_5_u32_tou64digits(auto n, auto& ret) noexcept {
  constexpr uint32_t inv10000_28b = 26844;
  constexpr uint32_t mask = uint32_t(-1) >> 4;
  n *= inv10000_28b;
  using u64_t_ = std::remove_cvref_t<decltype(ret)>;
  if constexpr (std::endian::little == std::endian::native) {
    ret |= u64_t_(n >> 28);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << 8;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << 16;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << 24;
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << 32;
  } else {
    ret |= u64_t_(n >> 28) << (32 + 32);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << (32 + 24);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << (32 + 16);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << (32 + 8);
    n &= mask;
    n = (n << 1) + (n << 3);
    ret |= u64_t_(n >> 28) << (32 + 0);
  }
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
  uint64_t ret{};
  iota_5_u32_tou64digits(uint32_t(number_), ret);

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

constexpr std::array<uint64_t, 10> pow_ten_table = []() noexcept {
  std::array<uint64_t, 10> ret{};
  ret[0] = 10;
  for (uint64_t i{1}; i < 10; i++) {
    ret[i] = ret[i - 1] * 10;
  }
  return ret;
}();
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
  log2_floor += pow_ten_table[log_floor_ofset] <= number_0_;
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
constexpr size_t uint_to_dec_aligned_unchekced_size_v{
    sizeof(std::get<0>(details_ns::dec_from_uint_impl_semi_parallel_impl_ncx_(T())))};

template <std::unsigned_integral T,size_t min_align_v=1>
[[maybe_unused]] inline size_t uint_to_dec_aligned_unchekced_branchless(
    char* buffer, size_t cap, T number_0_) noexcept {
  auto [str_int_buf, num_ch, offset] =
      details_ns ::dec_from_uint_impl_semi_parallel_impl_ncx_(number_0_);
  if (cap < sizeof(str_int_buf) || !buffer) {
#if 1 < _MSC_VER
    __assume(false);
#elif defined(__GNUC__)
    __builtin_unreachable();
#else
    static_assert(false, "compiler is not msvc,clang,gcc :(");
#endif
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
  } else{
    shift_little = 32 - shift_little;
    std::array<uint64_t, num_words> words_highlow{};
    for (size_t i{}; i < num_words; i++) {
      words_highlow[i] = uint64_t(words[i]) << shift_little;
    }
    words[0] = uint32_t(words_highlow[0]>>32);
    for (size_t i{1}; i < num_words; i++) {
      words[i] =
          uint32_t(words_highlow[i - 1]) | uint32_t(words_highlow[i] >> 32);
    }
  }
alignas(8)  std::array<uint32_t, num_words * 2> shifted_words{};
  for (size_t i{}; i < num_words; i++) {
    shifted_words[num_words + i - shift_big] = words[i];
  }

  std::memcpy(ptr, &shifted_words[num_words], sizeof(words));
  return num_ch;
}

template <std::unsigned_integral T, size_t min_align_v = 1>
[[maybe_unused]] inline size_t uint_to_dec_aligned_unchekced_branching(
    char* buffer, size_t cap, T number_0_) noexcept {
  if(uint8_t(number_0_) == number_0_){
    return uint_to_dec_aligned_unchekced_branchless <uint8_t , min_align_v>(buffer, cap, uint8_t(number_0_));
  }
  if (uint16_t(number_0_) == number_0_) {
    return uint_to_dec_aligned_unchekced_branchless<uint16_t, min_align_v>(
        buffer, cap, uint16_t(number_0_));
  }
  if (uint32_t(number_0_) == number_0_) {
    return uint_to_dec_aligned_unchekced_branchless<uint32_t, min_align_v>(
        buffer, cap, uint32_t(number_0_));
  }
  return uint_to_dec_aligned_unchekced_branchless<uint64_t, min_align_v>(buffer, cap, uint64_t(number_0_));
}

template <std::unsigned_integral T, size_t min_align_v = 1>
[[maybe_unused]] inline size_t
uint_to_dec_aligned_unchekced(char* buffer, size_t cap,
                                                   T number_0_) noexcept {
  return uint_to_dec_aligned_unchekced_branching(buffer, cap,
                                                            number_0_);
}


}  // namespace uint_to_ascci_ns0
};  // namespace mjz

#endif  // MJZ_UINTCONV_LIB_HPP_FILE_