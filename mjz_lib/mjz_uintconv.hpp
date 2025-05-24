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
namespace details_ns {

inline int64_t dec_from_uint_parallel_less_than_pow10_8_pair_impl_ncx_(
    const int32_t lower_half, const int32_t upper_half) noexcept {
  constexpr int64_t inv25_16b = 2622;

  constexpr int64_t inv5_8b = 52;

  constexpr int64_t mask_16b =
      int64_t(uint16_t(-1)) | (int64_t(uint16_t(-1)) << 32);
  constexpr int64_t mask_8b =
      int64_t(uint8_t(-1)) | (int64_t(uint8_t(-1)) << 16) |
      (int64_t(uint8_t(-1)) << 32) | (int64_t(uint8_t(-1)) << 48);
  if constexpr (std::endian::big == std::endian::native) {
    const int64_t div_2parellel_old =
        ((int64_t(upper_half) << 32) | int64_t(lower_half));

    const int64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) & mask_16b;

    const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const int64_t div_4parellel_old = modulo_2parallel | (div_2parallel << 16);

    const int64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return modulo_4parallel | (div_4parellel << 8);
  } else {
    const int64_t div_2parellel_old =
        (int64_t(upper_half) | (int64_t(lower_half) << 32));

    const int64_t div_2parallel =
        ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) & mask_16b;

    const int64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

    const int64_t div_4parellel_old = div_2parallel | (modulo_2parallel << 16);

    const int64_t div_4parellel =
        ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

    const int64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
    return div_4parellel | (modulo_4parallel << 8);
  }
}

inline std::tuple<std::array<int64_t, 3>, size_t, size_t>
dec_from_uint_impl_semi_parallel_impl_ncx_(const uint64_t number_) noexcept {
  constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
  constexpr uint64_t parallel_half = 10000;
  constexpr int64_t parallel_full = parallel_half * parallel_half;
  constexpr int64_t count_max = 3;
  struct unitilied_t_{
    std::array<int64_t, 3> str_int_buf_;
    inline unitilied_t_() noexcept {}
  } unitilied;
  std::array<int64_t, 3>& str_int_buf{unitilied.str_int_buf_};
  uint64_t number_0_ = number_;
  for (size_t i{}, iteration_count_backwards{count_max}; i < count_max; i++) {
    iteration_count_backwards--;
    int32_t upper_half{};
    int32_t lower_half{};

    int64_t u64ch_{};

    int32_t number_less_than_pow10_8;
    if (number_0_ < parallel_full) {
      number_less_than_pow10_8 = int32_t(number_0_);
      number_0_ = 0;
    } else {
      number_less_than_pow10_8 = int32_t(number_0_ % parallel_full);
      number_0_ = number_0_ / parallel_full;
    }
    lower_half = number_less_than_pow10_8 % 10000;
    upper_half = number_less_than_pow10_8 / 10000;

    u64ch_ = dec_from_uint_parallel_less_than_pow10_8_pair_impl_ncx_(
        lower_half, upper_half);

    const int64_t u64ch = u64ch_;
    int64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
    str_int_buf[count_max - 1 - iteration_count_backwards] = u64ch_ascii;
    if (number_0_) continue;
    const size_t num_high_0ch =
        size_t((std::endian::big == std::endian::native
                    ? std::countl_zero(uint64_t(u64ch))
                    : std::countr_zero(uint64_t(u64ch))) >>
               3);
    const size_t num_0ch{num_high_0ch + size_t(iteration_count_backwards << 3)};
    const size_t num_ch = std::max<size_t>(sizeof(str_int_buf) - num_0ch, 1);
    return {str_int_buf, num_ch, std::min<size_t>(7, num_high_0ch)};
  }

  return {};
}
};



[[maybe_unused]] inline size_t uint_to_dec_less1e9(
    char* buffer, size_t cap,
                                  uint32_t number_0_) noexcept {
  const int32_t lower_half = int32_t(number_0_ % 10000);
  const int32_t upper_half = int32_t(number_0_ / 10000);
  constexpr int64_t zero_8parallel_ascii = 0x3030303030303030;
  const int64_t u64ch =
      details_ns :: dec_from_uint_parallel_less_than_pow10_8_pair_impl_ncx_(
          lower_half,
                                                                    upper_half);

  int64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
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

};  // namespace mjz

#endif  // MJZ_UINTCONV_LIB_HPP_FILE_