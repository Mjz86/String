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
#include <array>
#include <bit>
#include <cstring>
#include <string>

namespace mjz {
inline namespace uint_to_ascci_ns0 {
namespace details_ns {

// credit to https://github.com/jeaiii for their help
constexpr static uint64_t swar_itoa_8digits(std::uint64_t n) noexcept {
  constexpr uint64_t inv10p4_b40 = 109951163;
  constexpr uint64_t inv10p2_b19 = 5243;
  constexpr uint64_t inv10p1_b10 = 103;
  constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
  constexpr uint64_t modolo10p4_40b_mask = 0x0000'00ff'ffff'ffff;
  constexpr uint64_t mask_upper_16b = 0xfff8'0000'fff8'0000;
  // ceil(2^40/10000)
  uint64_t holder = n * inv10p4_b40;

  // Upper 4-digits in lower 32-bits.
  uint64_t result_high = holder;

  // Lower 4-digits in upper 32-bits.
  uint64_t result_low =
      // muliply the modolous by 10000 and shift by 40 simplified,
      // then move to upper 32 bit
      ((((holder & modolo10p4_40b_mask) * 625) >> 36));

  uint64_t result{};
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
  /*
  auto const number_of_leading_zeros = std::countr_zero(result) / 8;
  auto length = 8 - number_of_leading_zeros;

  // Cook up a 64-bit unsigned integer consisting of 8 copies of '0'.
  constexpr std::uint64_t offset_vector = [] {
    auto offset = std::uint64_t('0');
    offset |= (offset << 8);
    offset |= (offset << 16);
    offset |= (offset << 32);
    return offset;
  }();

  result |= offset_vector;

  struct char_array_t {
    char array[8];
  } char_array = std::bit_cast<char_array_t>(result);

 // return std::string(char_array.array + number_of_leading_zeros, length);*/
}

constexpr uint64_t ascii_offset =
    std::bit_cast<uint64_t>(std::array{'0', '0', '0', '0', '0', '0', '0', '0'});
constexpr static const std::array<uint16_t, 100> modolo_raidex_table = []() {
  std::array<uint16_t, 100> ret{};
  for (uint32_t i{}; i < 100; i++) {
    const uint32_t var = i;
    uint64_t result = swar_itoa_8digits(var);

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

inline std::tuple<std::array<uint64_t, 3>, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint64_t number_) noexcept {
  constexpr uint64_t zero_8parallel_ascii = 0x3030303030303030;
  constexpr uint64_t parallel_half = 10000;
  constexpr uint64_t parallel_full = parallel_half * parallel_half;
  constexpr uint64_t count_max = 3;
// i dont like my warning as error on this on my ide
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

    u64ch_ = lookup_iota_8digits(number_less_than_pow10_8);

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
}

};  // namespace details_ns

[[maybe_unused]] inline size_t uint_to_dec_less1e9(
    char* buffer, size_t cap, uint32_t number_0_) noexcept {
  constexpr uint64_t zero_8parallel_ascii = 0x3030303030303030;
  const uint64_t u64ch = details_ns ::lookup_iota_8digits(number_0_);

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

[[maybe_unused]] inline size_t uint_to_dec(char* buffer, size_t cap,
                                           uint64_t number_0_) noexcept {
  const auto [str_int_buf, num_ch, offset] =
      details_ns ::dec_from_uint_impl_semi_parallel_impl_ncx_(number_0_);
  if (cap < num_ch) return 0;
  std::memcpy(buffer, reinterpret_cast<const char*>(&str_int_buf) + offset,
              num_ch);
  return num_ch;
}
}  // namespace uint_to_ascci_ns0
};  // namespace mjz

#endif  // MJZ_UINTCONV_LIB_HPP_FILE_