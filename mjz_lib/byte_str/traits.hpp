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

#include "../maths.hpp"
#include "base.hpp"
#ifndef MJZ_USE_cpp_lib_to_chars_int
#define MJZ_USE_cpp_lib_to_chars_int false
#endif
#ifndef MJZ_USE_cpp_lib_to_chars_float
#define MJZ_USE_cpp_lib_to_chars_float false
#endif
#if MJZ_USE_cpp_lib_to_chars_float || MJZ_USE_cpp_lib_to_chars_int
#ifdef __cpp_lib_to_chars
#include <charconv>
#endif
#endif
#ifndef MJZ_BYTE_STRING_traits_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_traits_LIB_HPP_FILE_
namespace mjz::bstr_ns {

enum class floating_format_e : uint8_t {
  scientific = 0b001,
  fixed = 0b010,
  hex = 0b100,
  general = fixed | scientific,
};
template <version_t version_v>
struct byte_traits_t : parse_math_helper_t_<version_v> {
  using parse_math_helper_t_<version_v>::divide_modulo;
  using parse_math_helper_t_<version_v>::signed_divide_modulo;
  using enum floating_format_e;
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(auto)
  npos{std::min((uintlen_t(-1) >> 8) + 1, std_view_max_size)};

  MJZ_CX_ND_FN static intlen_t pv_compare(const char *rhs, const char *lhs,
                                          uintlen_t len) noexcept {
    if (lhs == rhs) return 0;
    MJZ_IFN_CONSTEVAL { return std::memcmp(rhs, lhs, len); }
    for (; 0 < len; --len, ++rhs, ++lhs) {
      if (*rhs != *lhs) {
        return *rhs < *lhs ? -1 : +1;
      }
    }
    return 0;
  }
  MJZ_CX_ND_FN static uintlen_t pv_strlen(const char *begin) noexcept {
    if (!begin) return 0;
    MJZ_IFN_CONSTEVAL { return std::strlen(begin); }
    uintlen_t len = 0;
    while (*begin != '\0') {
      ++len;
      ++begin;
    }

    return len;
  }

  MJZ_CX_ND_FN static const char *pv_find(const char *begin, uintlen_t len,
                                          char c) noexcept {
    MJZ_IFN_CONSTEVAL {
      return reinterpret_cast<const char *>(std::memchr(begin, c, len));
    }
    for (; 0 < len; --len, ++begin) {
      if (*begin == c) {
        return begin;
      }
    }

    return nullptr;
  }
  MJZ_CX_ND_FN static uintlen_t u_diff(const char *rhs,
                                       const char *lhs) noexcept {
    return static_cast<uintlen_t>(rhs - lhs);
  }

  MJZ_CX_ND_FN static bool equal(const char *lhs, uintlen_t lhs_len,
                                 const char *rhs, uintlen_t rhs_len) noexcept {
    return lhs_len == rhs_len && pv_compare(lhs, rhs, lhs_len) == 0;
  }

  MJZ_CX_ND_FN static intlen_t compare(const char *lhs, uintlen_t lhs_len,
                                       const char *rhs,
                                       uintlen_t rhs_len) noexcept {
    intlen_t ret = pv_compare(lhs, rhs, std::min(lhs_len, rhs_len));

    if (ret != 0) {
      return ret;
    }
    if (lhs_len < rhs_len) {
      return -1;
    }
    if (lhs_len > rhs_len) {
      return 1;
    }
    return 0;
  }

  MJZ_CX_ND_FN static uintlen_t find(const char *hay_stack, uintlen_t hay_len,
                                     uintlen_t offset, const char *needle,
                                     uintlen_t needle_len) noexcept {
    if (needle_len > hay_len || offset > hay_len - needle_len) {
      return npos;
    }

    if (needle_len == 0) {
      return offset;
    }

    const auto search_end = hay_stack + (hay_len - needle_len) + 1;
    for (auto canidate = hay_stack + offset;; ++canidate) {
      canidate = pv_find(canidate, u_diff(search_end, canidate), *needle);
      if (!canidate) {
        return npos;
      }
      if (pv_compare(canidate, needle, needle_len) == 0) {
        return u_diff(canidate, hay_stack);
      }
    }
  }

  MJZ_CX_ND_FN static uintlen_t find_ch(const char *hay_stack,
                                        uintlen_t hay_len, uintlen_t offset,
                                        char c) noexcept {
    if (offset < hay_len) {
      const auto where = pv_find(hay_stack + offset, hay_len - offset, c);
      if (where) {
        return u_diff(where, hay_stack);
      }
    }

    return npos;
  }

  MJZ_CX_ND_FN static uintlen_t rfind(const char *hay_stack, uintlen_t hay_len,
                                      uintlen_t offset, const char *needle,
                                      uintlen_t needle_len) noexcept {
    if (needle_len == 0) {
      return std::min(offset, hay_len);
    }

    if (hay_len < needle_len) {
      return npos;
    }
    const char *canidate = hay_stack + std::min(offset, hay_len - needle_len);
    auto mached = [&]() noexcept {
      return *canidate == *needle &&
             pv_compare(canidate, needle, needle_len) == 0;
    };
    for (; canidate != hay_stack; --canidate) {
      if (!mached()) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }
    return mached() ? 0 : npos;
  }

  MJZ_CX_ND_FN static uintlen_t rfind_ch(const char *hay_stack,
                                         uintlen_t hay_len, uintlen_t offset,
                                         const char Ch) noexcept {
    if (hay_len == 0) {
      return npos;
    }
    auto canidate = hay_stack + std::min(offset, hay_len - 1);
    for (; canidate != hay_stack; --canidate) {
      if (*canidate != Ch) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }

    return *canidate == Ch ? 0 : npos;  // no match
  }

  MJZ_CX_ND_FN static uintlen_t find_first_of(const char *hay_stack,
                                              uintlen_t hay_len,
                                              uintlen_t offset,
                                              const char *needle,
                                              uintlen_t needle_len) noexcept {
    if (needle_len == 0 || offset >= hay_len) {
      return npos;
    }
    const auto hey_end = hay_stack + hay_len;
    for (auto canidate = hay_stack + offset; canidate < hey_end; ++canidate) {
      if (!pv_find(needle, needle_len, *canidate)) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }

    return npos;
  }

  MJZ_CX_ND_FN static uintlen_t find_last_of(const char *hay_stack,
                                             uintlen_t hay_len,
                                             uintlen_t offset,
                                             const char *needle,
                                             uintlen_t needle_len) noexcept {
    if (needle_len == 0 || hay_len == 0) {
      return npos;
    }
    auto canidate = hay_stack + std::min(offset, hay_len - 1);
    for (; canidate != hay_stack; --canidate) {
      if (!pv_find(needle, needle_len, *canidate)) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }
    return pv_find(needle, needle_len, *canidate) ? 0 : npos;
  }

  MJZ_CX_ND_FN static uintlen_t find_first_not_of(
      const char *hay_stack, uintlen_t hay_len, uintlen_t offset,
      const char *needle, uintlen_t needle_len) noexcept {
    if (offset >= hay_len) {
      return npos;
    }
    const auto hey_end = hay_stack + hay_len;
    for (auto canidate = hay_stack + offset; canidate < hey_end; ++canidate) {
      if (pv_find(needle, needle_len, *canidate)) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }
    return npos;
  }

  MJZ_CX_ND_FN static uintlen_t find_not_ch(const char *hay_stack,
                                            uintlen_t hay_len, uintlen_t offset,
                                            const char Ch) noexcept {
    if (offset >= hay_len) {
      return npos;
    }
    const auto hey_end = hay_stack + hay_len;
    for (auto canidate = hay_stack + offset; canidate < hey_end; ++canidate) {
      if (*canidate == Ch) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }

    return npos;
  }

  MJZ_CX_ND_FN static uintlen_t find_last_not_of(
      const char *hay_stack, uintlen_t hay_len, uintlen_t offset,
      const char *needle, uintlen_t needle_len) noexcept {
    if (hay_len == 0) {
      return npos;
    }
    auto canidate = hay_stack + std::min(offset, hay_len - 1);
    for (; canidate != hay_stack; --canidate) {
      if (pv_find(needle, needle_len, *canidate)) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }

    return pv_find(needle, needle_len, *canidate) ? npos : 0;  // no match
  }

  MJZ_CX_ND_FN static uintlen_t rfind_not_ch(const char *hay_stack,
                                             uintlen_t hay_len,
                                             uintlen_t offset,
                                             char c) noexcept {
    if (hay_len == 0) {
      return npos;
    }
    auto canidate = hay_stack + std::min(offset, hay_len - 1);
    for (; canidate != hay_stack; --canidate) {
      if (*canidate == c) {
        continue;
      }
      return u_diff(canidate, hay_stack);
    }

    return *canidate == c ? npos : 0;
  }
  MJZ_CX_ND_FN static bool starts_with(const char *lhs, uintlen_t lhs_len,
                                       const char *rhs,
                                       uintlen_t rhs_len) noexcept {
    if (lhs_len < rhs_len) {
      return false;
    }
    return pv_compare(lhs, rhs, rhs_len) == 0;
  }
  MJZ_CX_ND_FN static bool starts_with(const char *lhs, uintlen_t lhs_len,
                                       char rhs) noexcept {
    if (lhs_len < 1) {
      return false;
    }
    return *lhs == rhs;
  }

  MJZ_CX_ND_FN static bool ends_with(const char *lhs, uintlen_t lhs_len,
                                     const char *rhs,
                                     uintlen_t rhs_len) noexcept {
    if (lhs_len < rhs_len) {
      return false;
    }
    return pv_compare(lhs + (lhs_len - rhs_len), rhs, rhs_len) == 0;
  }
  MJZ_CX_ND_FN static bool ends_with(const char *lhs, uintlen_t lhs_len,
                                     char rhs) noexcept {
    if (lhs_len < 1) {
      return false;
    }
    return lhs[lhs_len - 1] == rhs;
  }
  MJZ_CX_ND_FN static bool contains(const char *lhs, uintlen_t lhs_len,
                                    const char *rhs,
                                    uintlen_t rhs_len) noexcept {
    return find(lhs, lhs_len, 0, rhs, rhs_len) != npos;
  }
  MJZ_CX_ND_FN static bool contains(const char *lhs, uintlen_t lhs_len,
                                    char rhs) noexcept {
    return find_ch(lhs, lhs_len, 0, rhs) != npos;
  }

  MJZ_CX_ND_FN static std::optional<uint8_t> ascii_to_num(char c) noexcept {
    if (c >= '0' && c <= '9') {
      return (uint8_t)(c - '0');
    }
    if (c >= 'A' && c <= 'Z') {
      return (uint8_t)(c - 'A' + 10);
    }
    if (c >= 'a' && c <= 'z') {
      return (uint8_t)(c - 'a' + 10);
    }
    return std::nullopt;
  }

 private:
  MJZ_CONSTANT(auto)
  alphabett_table_lower = "0123456789abcdefghijklmnopqrstuvwxyz";
  MJZ_CONSTANT(auto)
  alphabett_table_upper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  template <std::integral T>
  MJZ_CX_FN static uint8_t max_len_of_integral(uint8_t raidex) noexcept {
    std::numeric_limits<T> limits{};
    auto v = limits.max();
    uint8_t len{};
    while (v) {
      v /= T(raidex);
      len++;
    }
    return len;
  }

 public:
  MJZ_CX_ND_FN static std::optional<char> num_to_ascii(uint8_t i,
                                                       bool is_upper) noexcept {
    if (i < 37) {
      return (is_upper ? alphabett_table_upper : alphabett_table_lower)[i];
    }
    return std::nullopt;
  }

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static std::optional<T> to_integral_pv(const char *ptr,
                                                      uintlen_t len,
                                                      uint8_t raidex) noexcept {
    if (36 < raidex || !ptr || !len || !raidex) return std::nullopt;
    using UT = std::make_unsigned_t<T>;
    constexpr UT max_v = UT(-1);
    constexpr UT sign_bit = std::same_as<T, UT> ? UT(0) : UT(~(UT(-1) >> 1));
    const UT pre_max_v = UT(divide_modulo(max_v, UT(raidex)).first);

    bool is_neg{};

    if (*ptr == '-' || *ptr == '+') {
      ptr++;
      len--;
      is_neg = *ptr == '-';
      if (!len) return std::nullopt;
    }
    while (len && *ptr == '0') {
      ptr++;
      len--;
    }
    if (!len) return T();
    if (is_neg && !std::is_signed_v<T>) return std::nullopt;
    if (max_len_of_integral<UT>(raidex) < len) return std::nullopt;
    UT var{};
    for (uintlen_t i{}; i < len; i++) {
      auto digit = ascii_to_num(ptr[i]);
      if (!digit || raidex <= *digit) return std::nullopt;
      if (pre_max_v < var) {
        return std::nullopt;
      }
      var *= UT(raidex);
      var += UT(*digit);
    }
    if (!is_neg || !var) {
      if (sign_bit & var) {
        return std::nullopt;
      }
      return T(var);
    }
    if (sign_bit & (var - UT(1))) {
      return std::nullopt;
    }
    return T((~var) + UT(1));
  }

  MJZ_CONSTANT(auto)
  defualt_is_point_fn = [](char ch,
                           MJZ_UNUSED uint8_t raidex) noexcept -> bool {
    return ch == '.' || ch == ',';
  };

  MJZ_CONSTANT(auto)
  defualt_power_fn = [](char ch, uint8_t raidex) noexcept
      -> std::optional<
          std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>> {
    if (raidex < *ascii_to_num('E') && (ch == 'e' || ch == 'E')) {
      return std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>{
          uintlen_t(10), uint8_t(10)};
    }
    if (raidex < *ascii_to_num('P') && (ch == 'p' || ch == 'P')) {
      return std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>{
          uintlen_t(2), uint8_t(10)};
    }
    if (ch == '^') {
      return std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>{
          uintlen_t(raidex), raidex};
    }
    return std::nullopt;
  };
  template <std::floating_point T,
            callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
                decltype(defualt_is_point_fn),
            callable_c<std::optional<
                std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>>(
                const char &, const uint8_t &) noexcept>
                power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN static std::optional<T> to_real_floating_pv(
      const char *ptr, uintlen_t len, const uint8_t raidex_,
      is_point_fn_t &&is_point_fn = is_point_fn_t{},
      power_fn_t &&power_fn = power_fn_t{}) noexcept {
    if (36 < raidex_ || !ptr || !len || !raidex_ || is_point_fn('+', raidex_) ||
        is_point_fn('-', raidex_) || power_fn('+', raidex_) ||
        power_fn('-', raidex_))
      return std::nullopt;
    bool is_neg{};
    if (*ptr == '-' || *ptr == '+') {
      ptr++;
      len--;
      is_neg = *ptr == '-';
      if (!len) return std::nullopt;
    }
    bool had_some_numbers_first{};
    while (len && *ptr == '0') {
      ptr++;
      len--;
      had_some_numbers_first = true;
    }
    if (!len) return T(0);
    struct range_t {
      uintlen_t i{npos};
      uintlen_t len{};
    };
    range_t dummy{};
    range_t sientific_coeffient_section1{0, 0};
    range_t sientific_coeffient_section2{};
    range_t sientific_exponent{};
    bool neg_sientific_exponent{};
    uintlen_t exponent_base{raidex_};
    uint8_t exponent_raidex{raidex_};
    range_t *previous_section{&sientific_coeffient_section1};
    for (uintlen_t i{}; i < len; i++) {
      uint8_t raidex =
          &sientific_exponent == previous_section ? exponent_raidex : raidex_;
      if (auto ch = ascii_to_num(ptr[i]); ch && *ch < raidex) {
        if (power_fn(ptr[i], raidex) || is_point_fn(ptr[i], raidex))
          return std::nullopt;  // ambiguous!
        previous_section->len++;
        continue;
      }
      MJZ_RELEASE { previous_section->i = 1 + i; };
      if (is_point_fn(ptr[i], raidex)) {
        if (&sientific_coeffient_section1 != previous_section ||
            power_fn(ptr[i], raidex))
          return std::nullopt;
        if (i + 1 == len) {
          previous_section = &dummy;
          continue;
        }
        previous_section = &sientific_coeffient_section2;
        continue;
      }
      if (auto pow = power_fn(ptr[i], raidex)) {
        if (&sientific_exponent == previous_section || i + 2 > len) {
          return std::nullopt;
        }
        previous_section = &sientific_exponent;
        exponent_base = std::get<0>(*pow);
        exponent_raidex = std::get<1>(*pow);
        continue;
      }
      if (ptr[i] == '+' || ptr[i] == '-') {
        if (i + 2 > len || previous_section != &sientific_exponent ||
            !power_fn(ptr[i - 1], raidex))
          return std::nullopt;
        neg_sientific_exponent = ptr[i] == '-';
        continue;
      }
      return std::nullopt;
    }
    using mjz_float_t = big_float_t<version_v>;

    auto get_sec1_val = [=](uint8_t raidex, const char *begin,
                            uintlen_t length) noexcept -> mjz_float_t {
      mjz_float_t var{};
      for (uintlen_t i{}; i < length; i++) {
        auto digit = ascii_to_num(begin[i]);
        var = var * mjz_float_t::float_from_i(raidex);
        var = var + mjz_float_t::float_from_i(*digit);
      }

      return var;
    };
    auto get_sec2_val = [=](uint8_t raidex, const char *begin,
                            uintlen_t length) noexcept -> mjz_float_t {
      mjz_float_t var{};
      mjz_float_t r = mjz_float_t::float_from_i(1);
      mjz_float_t inverse = r / mjz_float_t::float_from_i(raidex);
      for (uintlen_t i{}; i < length; i++) {
        auto digit = ascii_to_num(begin[i]);
        r = r * inverse;
        var = var + r * mjz_float_t::float_from_i(*digit);
      }

      return var;
    };
    mjz_float_t exponent{};
    if (&sientific_exponent == previous_section) {
      auto v = get_sec1_val(exponent_raidex, ptr + sientific_exponent.i,
                            sientific_exponent.len);
      exponent = neg_sientific_exponent ? -v : v;
    }

    mjz_float_t coeffient{};
    if (sientific_coeffient_section1.len) {
      coeffient = coeffient + get_sec1_val(raidex_,
                                           ptr + sientific_coeffient_section1.i,
                                           sientific_coeffient_section1.len);
    }

    if (sientific_coeffient_section2.len) {
      coeffient = coeffient + get_sec2_val(raidex_,
                                           ptr + sientific_coeffient_section2.i,
                                           sientific_coeffient_section2.len);
    }
    if (!sientific_coeffient_section1.len &&
        !sientific_coeffient_section2.len && !had_some_numbers_first) {
      return std::nullopt;
    }
    std::optional<std::pair<int64_t, mjz_float_t>> opt =
        exponent.to_integral_and_fraction();
    if (!opt) {
      return std::nullopt;
    }
    auto &&[power, idk] = *opt;
    if (mjz_float_t::float_from(.5) < idk) {
      if (power >= int64_t(uint64_t(-1) >> 2)) return std::nullopt;
      power++;
    }
    mjz_float_t value_exp = mjz_float_t::float_from_i(exponent_base);
    if (power < 0) {
      power = -power;
      value_exp = mjz_float_t::float_from_i(1) / value_exp;
    }
    uint64_t power_v{uint64_t(power)};
    for (; power_v;) {
      if (power_v & 1) {
        coeffient = coeffient * value_exp;
      }
      value_exp = value_exp * value_exp;
      power_v >>= 1;
    }
    if (is_neg) {
      coeffient = -coeffient;
    }
    return coeffient.template to_float<T>(false);
  }

  template <std::floating_point T,
            callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
                decltype(defualt_is_point_fn),
            callable_c<std::optional<
                std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>>(
                const char &, const uint8_t &) noexcept>
                power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN static std::optional<T> to_floating_pv(
      const char *ptr, uintlen_t len,
      is_point_fn_t &&is_point_fn = is_point_fn_t{},
      power_fn_t &&power_fn = power_fn_t{}) noexcept {
    uint8_t raidex = 10;
    if (ascii_to_num(std::min(std::min('N', 'A'), std::min('I', 'F'))) <
        raidex) {
      return std::nullopt;  // ambigous , NAN or INF could be a valid number!
    }
    if (std::optional<T> v =
            to_real_floating<T>(ptr, len, is_point_fn, power_fn)) {
      return v;
    }
    if (36 < raidex || !ptr || !len || !raidex || is_point_fn('+', raidex) ||
        is_point_fn('-', raidex) || power_fn('+', raidex) ||
        power_fn('-', raidex))
      return std::nullopt;
    bool is_neg{};
    bool had_sign{};
    if (*ptr == '-' || *ptr == '+') {
      ptr++;
      len--;
      had_sign = true;
      is_neg = *ptr == '-';
      if (!len) return std::nullopt;
    }
    auto is_eq = [len, ptr]<uintlen_t N>(const char(&str)[N]) noexcept {
      uintlen_t i{};
      while (i < len && i < N - 1 &&
             ascii_to_num(str[i]) == ascii_to_num(ptr[i])) {
        i++;
      }
      return i == N - 1 && len == i;
    };
    if (is_eq("inf")) {
      MJZ_FCONSTANT(auto) v = std::numeric_limits<T>().infinity();
      return is_neg ? -v : v;
    }
    if (is_eq("nan")) {
      if (!had_sign) return std::numeric_limits<T>().signaling_NaN();
      MJZ_FCONSTANT(auto) v = std::numeric_limits<T>().quiet_NaN();
      return is_neg ? -v : v;
    }
    return std::nullopt;
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static uintlen_t from_integral_len(
      T val, const uint8_t raidex) noexcept {
    using UT = std::make_unsigned_t<T>;
    UT v = UT(val);
    if (!val) val++;
    uintlen_t len{uintlen_t(val < 0 ? 1 : 0)};
    while (v) {
      v /= UT(raidex);
      len++;
    };
    return std::max(uintlen_t(1), len);
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static uintlen_t from_integral_max_len(
      const uint8_t raidex) noexcept {
    using UT = std::make_unsigned_t<T>;
    return from_integral_len<T>(
        T(std::same_as<T, UT> ? UT(-1) : (UT(-1) >> 1) + 1), raidex);
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static std::optional<uintlen_t> from_integral_fill_backwards(
      char *buf, uintlen_t len, T val_rg_, bool upper_case,
      const uint8_t raidex) noexcept {
    if (!buf || !len || 36 < raidex || !raidex) return std::nullopt;
    using UT = std::make_unsigned_t<T>;
    constexpr UT sign_bit = std::same_as<T, UT> ? 0 : ~(UT(-1) >> 1);
    bool is_neg{};
    UT val{UT(val_rg_)};
    if (sign_bit & val) {
      is_neg = true;
      val = UT((~val) + UT(1));
      len--;
      buf++;
      if (!len) return std::nullopt;
    }
    if (!val) {
      *buf = '0';
      return 1;
    }
    uintlen_t n{};
    uintlen_t i{len - 1};
    for (; n < len && val; i--) {
      auto [mul, r] = divide_modulo(val, UT(raidex));
      val = UT(mul);
      auto v = num_to_ascii(uint8_t(r), upper_case);
      if (!v) {
        return std::nullopt;
      }
      buf[i] = *v;
      n++;
    }
    if (val) return std::nullopt;
    if (is_neg) {
      buf[len - n - 1] = '-';
      n++;
    }
    return n;
  }
  static constexpr const std::array<int16_t, 1000> &divition10_table =
      make_static_data([]() noexcept {
        std::array<int16_t, 1000> ret{};
        for (size_t i{}; i < 1000; i++) {
          const size_t i2{i / 100}, i1{(i / 10) % 10}, i0{i % 10},
              ies{i2 | (i1 << 4) | (i0 << 8) |
                  (size_t(i2 ? 3 : (i1 ? 2 : (i0 ? 1 : 0))) << 12)};
          ret[i] = int16_t(ies);
        }
        return ret;
      });

  static constexpr bool use_parralel_integer_conv_v = true;
  template <std::unsigned_integral T>
  MJZ_CX_AL_FN static uintlen_t dec_from_uint_impl_lookup_sequential(
      char *out_buf, uintlen_t out_len, T number_0_, char *modolo10) noexcept {
    uint64_t number_{number_0_};
    int8_t i_1000modolo{};
    int8_t max_i_1000modolo{};
    int num_ch{};
    auto div_fn = [&]<std::unsigned_integral T_01_>(T_01_ number) noexcept {
      asserts(asserts.assume_rn, 0 <= i_1000modolo);
      for (;;) {
        int16_t modolos{int16_t(number)};
        bool small_ = number < 1000;
        if (!small_) {
          const T_01_ number_div = T_01_(number / 1000);
          modolos = int16_t(number % 1000);
          number = number_div;
        }
        const int offset = (i_1000modolo--) * 3;
        asserts(asserts.assume_rn, 0 <= offset);
        const int16_t div_res = divition10_table[size_t(modolos)];
        modolo10[offset] = char(div_res & 15) + '0';
        modolo10[offset + 1] = char((div_res >> 4) & 15) + '0';
        modolo10[offset + 2] = char((div_res >> 8) & 15) + '0';
        num_ch += small_ ? div_res >> 12 : 3;
        if ((int(i_1000modolo < 0) | int(small_)) == 1) break;
      }
    };
    if constexpr (partial_same_as<uint64_t, T>) {
      constexpr uint32_t max_i32_div = 1000 * 1000 * 1000;
      // no bigger is needed
      constexpr uint64_t max_2_i32_div = uint64_t(max_i32_div) * max_i32_div;
      i_1000modolo = max_i_1000modolo = 6;
      if (max_2_i32_div < number_) {
        const uint64_t big_div = number_ / max_2_i32_div;
        asserts(asserts.assume_rn, big_div < 1000);
        div_fn(uint32_t(big_div));  // we know that
        number_ = number_ % max_2_i32_div;
      }
      if (max_i32_div < number_) {
        const uint64_t big_div = number_ / max_i32_div;
        const uint64_t big_mod = number_ % max_i32_div;
        div_fn(uint32_t(big_div));
        div_fn(uint32_t(big_mod));
      } else {
        div_fn(uint32_t(number_));
      }
    } else if constexpr (partial_same_as<uint32_t, T>) {
      i_1000modolo = max_i_1000modolo = 3;
      div_fn(uint32_t(number_));
    } else if constexpr (partial_same_as<uint16_t, T>) {
      i_1000modolo = max_i_1000modolo = 1;
      div_fn(uint16_t(number_));
    } else if constexpr (partial_same_as<uint8_t, T>) {
      i_1000modolo = max_i_1000modolo = 0;
      div_fn(uint8_t(number_));
    } else {
      asserts.unreachable();
    }
    const int max_ch = (max_i_1000modolo + 1) * 3;
    num_ch = std::max(1, num_ch);
    if (out_len < uintlen_t(num_ch)) {
      return 0;
    }
    mjz::memcpy(out_buf, modolo10 + max_ch - num_ch, uintlen_t(num_ch));
    return uintlen_t(num_ch);
  }

  template <std::unsigned_integral T>
  MJZ_CX_AL_FN static uintlen_t dec_from_uint_impl_(
      char *out_buf, uintlen_t out_len, T number_0_,
      MJZ_MAYBE_UNUSED char *modolo10) noexcept {
    if constexpr (use_parralel_integer_conv_v) {
      return dec_from_uint_impl_semi_parallel(out_buf, out_len, number_0_);
    } else {
      return dec_from_uint_impl_lookup_sequential(out_buf, out_len, number_0_,
                                                  modolo10);
    }
  }
  MJZ_NCX_FN static uintlen_t ncx_dec_from_uint_(
      char *out_buf, uintlen_t out_len,
      std::unsigned_integral auto number_) noexcept {
    MJZ_DISABLE_ALL_WANINGS_START_;
    struct buffer_0_t_ {
      alignas(8) char raw[8 * 3];
      MJZ_NCX_FN buffer_0_t_() noexcept {}
    } modolo10;
    MJZ_DISABLE_ALL_WANINGS_END_;
    return dec_from_uint_impl_(out_buf, out_len, number_, modolo10.raw);
  }
  MJZ_CX_AL_FN static uintlen_t dec_from_uint(
      char *out_buf, uintlen_t out_len,
      std::unsigned_integral auto number_) noexcept {
    MJZ_IF_CONSTEVAL {
      alignas(8) char modolo10[8 * 3]{};
      return dec_from_uint_impl_(out_buf, out_len, number_, modolo10);
    }
    else {
      return ncx_dec_from_uint_(out_buf, out_len, number_);
    }
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static std::optional<uintlen_t> from_integral_fill(
      char *buf, uintlen_t len, T val_rg_, bool upper_case,
      const uint8_t raidex) noexcept {
    using UT = std::make_unsigned_t<T>;
#if MJZ_USE_cpp_lib_to_chars_int
#ifdef __cpp_lib_to_chars
#ifdef __cpp_lib_constexpr_charconv
    if constexpr (true)
#else
    MJZ_IFN_CONSTEVAL
#endif
    {
      std::to_chars_result res = std::to_chars(buf, buf + len, val_rg_, raidex);
      if (res.ec == std::errc{}) {
        return uintlen_t(res.ptr - buf);
      }
      return {};
    }
#endif
#endif
    if (!len || !buf) return {};
    if (raidex == 10) {
      if constexpr (std::signed_integral<T>) {
        const bool is_neg{val_rg_ < 0};
        *buf = '-';
        buf += is_neg;
        len -= is_neg;
        uintlen_t ret =
            dec_from_uint(buf, len, UT(is_neg ? -val_rg_ : val_rg_));
        return ret ? std::optional<uintlen_t>(ret + is_neg) : std::nullopt;
      } else {
        uintlen_t ret = dec_from_uint(buf, len, val_rg_);
        return ret ? std::optional<uintlen_t>(ret) : std::nullopt;
      }
    }
    return [=]() mutable noexcept -> std::optional<uintlen_t> {
      if (!buf || !len || 36 < raidex || !raidex) return std::nullopt;
      constexpr UT sign_bit = std::same_as<T, UT> ? UT(0) : UT(~(UT(-1) >> 1));
      bool is_neg{};
      UT val{UT(val_rg_)};
      if (sign_bit & val) {
        is_neg = true;
        val = UT((~val) + UT(1));
        len--;
        buf++;
        if (!len) return std::nullopt;
      }
      if (!val) {
        *buf = '0';
        return 1;
      }
      uintlen_t n{};
      uintlen_t i{len - 1};
      for (; n < len && val; i--) {
        auto [mul, r] = divide_modulo(UT(val), UT(raidex));
        val = UT(mul);
        auto v = num_to_ascii(uint8_t(r), upper_case);
        if (!v) {
          return std::nullopt;
        }
        buf[i] = *v;
        n++;
      }
      if (val) return std::nullopt;
      memomve_overlap(buf, &buf[len - n], n);
      if (is_neg) {
        *(--buf) = '-';
        n++;
      }
      return n;
    }();
  }

  template <std::integral T>
  using big_buff_t = std::pair<std::array<char, from_integral_max_len<T>(2)>,
                               std::optional<uintlen_t>>;
  template <std::floating_point T>
  using big_buff2_t =
      std::pair<std::array<char, sizeof(T) * 8>, std::optional<uintlen_t>>;
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static big_buff_t<T> from_integral(
      T val_rg_, bool upper_case, const uint8_t raidex) noexcept {
    big_buff_t<T> ret{};
    ret.second = from_integral_fill(&ret.first[0], ret.first.size(), val_rg_,
                                    upper_case, raidex);
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_ND_FN static big_buff2_t<T> from_float(
      T val, MJZ_MAYBE_UNUSED const uint8_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case,
      floating_format_e floating_format = floating_format_e::general,
      bool add_prefix = true) noexcept {
    big_buff2_t<T> ret{};
    ret.second = from_float_format_fill(&ret.first[0], ret.first.size(), val,
                                        f_accuracacy, upper_case,
                                        floating_format, add_prefix);
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_FN static std::optional<uintlen_t> from_float_fill_sientific(
      char *const f_buf, const uintlen_t f_len, T f_val,
      MJZ_MAYBE_UNUSED const uint8_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case, const uint8_t raidex,
      MJZ_MAYBE_UNUSED uint8_t exp_base, MJZ_MAYBE_UNUSED char point_ch = '.',
      MJZ_MAYBE_UNUSED char power_ch = '^') noexcept {
    if (36 < raidex || !f_buf || !f_len || exp_base < 2 || raidex < 2 ||
        point_ch == '+' || point_ch == '-')
      return std::nullopt;
    char *buf{f_buf};
    MJZ_MAYBE_UNUSED uintlen_t accuracacy{f_accuracacy};
    uintlen_t len{f_len};

    using mjz_float_t = big_float_t<version_v>;
    auto opt_ = mjz_float_t::template float_from<T>(f_val, false);
    if (!opt_) {
      opt_ = mjz_float_t::template float_from<T>(f_val, true);

      if (!opt_ || len < 4) return std::nullopt;
      auto inf = mjz_float_t::template float_from<T>(
          std::numeric_limits<T>().infinity(), true);
      if ((*opt_).m_coeffient < 0) {
        *buf++ = '-';

        len -= 1;
      }
      if (inf && (*opt_ == *inf || *opt_ == -*inf)) {
        *buf++ = 'I';
        *buf++ = 'N';
        *buf++ = 'F';
        len -= 3;
      } else {
        *buf++ = 'N';
        *buf++ = 'A';
        *buf++ = 'N';
        len -= 3;
      }
      return f_len - len;
    }
    if (f_val < 0) {
      if (len < 1) return std::nullopt;
      f_val = -f_val;
      *opt_ = -*opt_;
      *buf = '-';
      len--;
      buf++;
    }
    if (f_val == T()) {
      if (len < 1) return std::nullopt;
      *buf = '0';
      len--;
      return f_len - len;
    }
    mjz_float_t value{*opt_};
    {
      mjz_float_t half_{};
      half_.m_exponent--;
      half_.m_coeffient++;
      mjz_float_t one = mjz_float_t::float_from_i(1);

      mjz_float_t expo = mjz_float_t::float_from_i(exp_base);
      mjz_float_t coeffient = one;
      mjz_float_t r = expo;
      uintlen_t pow{accuracacy};
      for (; pow;) {
        if (pow & 1) {
          coeffient = coeffient * r;
        }
        r = r * r;
        pow >>= 1;
      }

      mjz_float_t rounder = value * coeffient;
      auto [big, fraction] = rounder.to_big_and_fraction();
      mjz_float_t one_over_cof{one / coeffient};
      if (half_ <= fraction) {
        value = value + one_over_cof;
      }
    }
    auto sb1_ = value.to_log_and_coeffient(exp_base);
    // https://stackoverflow.com/questions/46114214/lambda-implicit-capture-fails-with-variable-declared-from-structured-binding
    auto &&[ceil_log_0_, fractionic_val_0_] = sb1_;
    std::optional<std::pair<int64_t, mjz_float_t>> int_and_float =
        fractionic_val_0_.to_integral_and_fraction();
    auto &&[Int_, Float_] = *int_and_float;
    auto len_diff = from_integral_fill(buf, len, Int_, upper_case, raidex);
    if (!len_diff) return std::nullopt;
    len -= *len_diff;
    buf += *len_diff;
    if (!len) return std::nullopt;
    auto limit_num = [&]() noexcept {
      auto &&[ceil_log, fractionic_val] = sb1_;
      int64_t max_log2 =
          log2_ceil_of_val_create(uint64_t(fractionic_val.m_coeffient)) +
          fractionic_val.m_exponent;
      int64_t max_log{
          signed_divide_modulo(max_log2, log2_ceil_of_val_create(raidex))
              .first};
      if (accuracacy < uint64_t(-max_log)) {
        fractionic_val.m_coeffient = 0;
      }
    };
    auto &&[ceil_log, fractionic_val] = sb1_;
    fractionic_val = Float_;
    limit_num();
    if (accuracacy && fractionic_val.m_coeffient) {
      *buf = point_ch;
      len -= 1;
      buf += 1;
    }
    mjz_float_t r = mjz_float_t::float_from_i(raidex);
    for (fractionic_val = fractionic_val * r;
         fractionic_val.m_coeffient && accuracacy;
         accuracacy--, limit_num(), fractionic_val = fractionic_val * r) {
      int_and_float = fractionic_val.to_integral_and_fraction();
      if (!int_and_float) return nullopt;
      auto &&[front_decimal, left_decimal] = *int_and_float;
      fractionic_val = left_decimal;
      auto v = num_to_ascii(uint8_t(front_decimal), upper_case);
      if (!v) {
        return std::nullopt;
      }
      if (!len) return std::nullopt;
      *buf = *v;
      len -= 1;
      buf += 1;
    }
    if (!ceil_log) return f_len - len;
    if (!len) return std::nullopt;
    *buf++ = power_ch;
    len -= 1;
    len_diff = from_integral_fill(buf, len, ceil_log, upper_case, exp_base);
    if (!len_diff) return std::nullopt;
    len -= *len_diff;
    buf += *len_diff;
    return f_len - len;
  }

  MJZ_CX_FN static bool is_space(char c) noexcept {
    return ' ' == c || '\0' == c || c == '\r' || c == '\n';
  }

  template <std::floating_point T>
  MJZ_CX_FN static std::optional<uintlen_t> from_float_fill_fixed(
      char *const f_buf, const uintlen_t f_len, T f_val,
      MJZ_MAYBE_UNUSED const uintlen_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case, const uint8_t raidex, uint8_t exp_base,
      char point_ch = '.') noexcept {
    if (36 < raidex || !f_buf || !f_len || exp_base < 2 || raidex < 2 ||
        point_ch == '+' || point_ch == '-')
      return std::nullopt;
    char *buf{f_buf};
    MJZ_MAYBE_UNUSED uintlen_t accuracacy{f_accuracacy};
    uintlen_t len{f_len};

    using mjz_float_t = big_float_t<version_v>;
    auto opt_ = mjz_float_t::template float_from<T>(f_val, false);
    if (!opt_) {
      opt_ = mjz_float_t::template float_from<T>(f_val, true);

      if (!opt_ || len < 4) return std::nullopt;
      auto inf = mjz_float_t::template float_from<T>(
          std::numeric_limits<T>().infinity(), true);
      if ((*opt_).m_coeffient < 0) {
        *buf++ = '-';

        len -= 1;
      }
      if (inf && (*opt_ == *inf || *opt_ == -*inf)) {
        *buf++ = 'I';
        *buf++ = 'N';
        *buf++ = 'F';
        len -= 3;
      } else {
        *buf++ = 'N';
        *buf++ = 'A';
        *buf++ = 'N';
        len -= 3;
      }
      return f_len - len;
    }
    if (f_val < 0) {
      if (len < 1) return std::nullopt;
      f_val = -f_val;
      *opt_ = -*opt_;
      *buf = '-';
      len--;
      buf++;
    }
    if (f_val == T()) {
      if (len < 1) return std::nullopt;
      *buf = '0';
      len--;
      return f_len - len;
    }
    mjz_float_t value{*opt_};
    {
      mjz_float_t half_{};
      half_.m_exponent--;
      half_.m_coeffient++;
      mjz_float_t one = mjz_float_t::float_from_i(1);

      mjz_float_t expo = mjz_float_t::float_from_i(exp_base);
      mjz_float_t coeffient = one;
      mjz_float_t r = expo;
      uintlen_t pow{accuracacy};
      for (; pow;) {
        if (pow & 1) {
          coeffient = coeffient * r;
        }
        r = r * r;
        pow >>= 1;
      }

      mjz_float_t rounder = value * coeffient;
      auto [big, fraction] = rounder.to_big_and_fraction();
      mjz_float_t one_over_cof{one / coeffient};
      if (half_ <= fraction) {
        value = value + one_over_cof;
      }
    }

    std::optional<std::pair<int64_t, mjz_float_t>> int_and_float =
        value.to_integral_and_fraction();

    int64_t Int_{};
    mjz_float_t Float_{};
    if (!int_and_float) {
      auto [ceil_log, fractionic_val] = value.to_log_and_coeffient(exp_base);

      std::optional<std::pair<int64_t, mjz_float_t>> int_and_float_{
          std::pair<int64_t, mjz_float_t>{}};
      mjz_float_t expo = mjz_float_t::float_from_i(exp_base);
      for (int_and_float_ = fractionic_val.to_integral_and_fraction(); ceil_log;
           ceil_log--, fractionic_val = fractionic_val * expo,
          int_and_float_ = fractionic_val.to_integral_and_fraction()) {
        if (!int_and_float_) return nullopt;
        auto [front_decimal, left_decimal] =
            std::pair<int64_t, mjz_float_t>(*int_and_float_);
        fractionic_val = left_decimal;
        auto v = num_to_ascii(uint8_t(front_decimal), upper_case);
        if (!v) {
          return std::nullopt;
        }
        if (!len) return std::nullopt;
        *buf = *v;
        len -= 1;
        buf += 1;
      }
      auto &&[Int_val, Float_val] = *int_and_float_;
      Float_ = Float_val;
      Int_ = Int_val;
    } else {
      auto &&[Int_val, Float_val] = *int_and_float;
      Float_ = Float_val;
      Int_ = Int_val;
    }
    auto len_diff = from_integral_fill(buf, len, Int_, upper_case, raidex);
    if (!len_diff) return std::nullopt;
    len -= *len_diff;
    buf += *len_diff;
    if (!len) return std::nullopt;
    auto limit_num = [&]() noexcept {
      int64_t max_log2 = log2_ceil_of_val_create(uint64_t(value.m_coeffient)) +
                         value.m_exponent;
      bool neg_ = max_log2 < 0;
      max_log2 = neg_ ? -max_log2 : max_log2;
      int64_t max_log{int64_t(
          divide_modulo(uint64_t(max_log2), log2_ceil_of_val_create(raidex))
              .first)};
      max_log = neg_ ? -max_log : max_log;
      if (accuracacy < uint64_t(-max_log)) {
        value.m_coeffient = 0;
      }
    };
    value = Float_;
    limit_num();
    if (!accuracacy && value.m_coeffient) return f_len - len;
    *buf = point_ch;
    len -= 1;
    buf += 1;
    mjz_float_t r = mjz_float_t::float_from_i(raidex);
    for (value = value * r; value.m_coeffient && accuracacy;
         accuracacy--, limit_num(), value = value * r) {
      int_and_float = value.to_integral_and_fraction();
      if (!int_and_float) return nullopt;
      auto &&[front_decimal, left_decimal] = *int_and_float;
      value = left_decimal;
      auto v = num_to_ascii(uint8_t(front_decimal), upper_case);
      if (!v) {
        return std::nullopt;
      }
      if (!len) {
        return std::nullopt;
      }
      *buf = *v;
      len -= 1;
      buf += 1;
    }

    return f_len - len;
  }

  template <std::floating_point T>
  MJZ_CX_FN static uintlen_t from_float_fill_hex(char *const f_buf,
                                                 const uintlen_t f_len, T f_val,
                                                 bool upper_case,
                                                 char point_ch = '.',
                                                 bool add_0x = true) noexcept {
    if (!f_buf || !f_len || point_ch == '+' || point_ch == '-') return 0;
    char *buf{f_buf};
    uintlen_t len{f_len};
    using mjz_float_t = big_float_t<version_v>;
    auto opt_ = mjz_float_t::template float_from<T>(f_val, false);
    if (len < 4) return 0;
    mjz_float_t value{*opt_};
    if (add_0x) {
      *buf++ = '0';
      *buf++ = upper_case ? 'X' : 'x';
      len -= 2;
    }
    len -= 1;
    buf += 1;
    auto len_diff =
        from_integral_fill(buf, len, value.m_coeffient, upper_case, 16);
    if (!len_diff) return 0;
    buf[-1] = std::exchange(buf[0], point_ch);
    len -= *len_diff;
    buf += *len_diff;
    value.m_exponent += uint64_t(*len_diff - 1) << 2;
    if (len < 2) return 0;
    *buf = upper_case ? 'P' : 'p';
    len -= 1;
    buf += 1;
    len_diff = from_integral_fill(buf, len, value.m_exponent, upper_case, 10);
    if (!len_diff) return 0;
    len -= *len_diff;
    buf += *len_diff;
    return f_len - len;
  }

  template <std::floating_point T>
  MJZ_CX_FN static uintlen_t from_float_format_fill(
      char *f_buf, uintlen_t f_len, T f_val, const uint8_t f_accuracacy = 6,
      bool upper_case = true,
      floating_format_e floating_format = floating_format_e::general,
      bool add_prefix = true) noexcept {
#if MJZ_USE_cpp_lib_to_chars_float
#ifdef __cpp_lib_to_chars
    MJZ_IFN_CONSTEVAL {
      char *buf_c_ = f_buf;
      bool add_prefix_ = add_prefix;
      add_prefix = false;
      std::to_chars_result res{

      };
      switch (floating_format) {
        case floating_format_e::fixed:
          res = std::to_chars(f_buf, f_buf + f_len, f_val,
                              std::chars_format::fixed, int(f_accuracacy));
          break;
        case floating_format_e::hex:
          add_prefix = add_prefix_;
          if (add_prefix) {
            if (f_len < 2) return {};
            f_buf += 2;
            f_len -= 2;
          }
          res = std::to_chars(f_buf, f_buf + f_len, f_val,
                              std::chars_format::hex, int(f_accuracacy));
          break;
        case floating_format_e::general:
          res = std::to_chars(f_buf, f_buf + f_len, f_val,
                              std::chars_format::general, int(f_accuracacy));
          break;
        case floating_format_e::scientific:
          res = std::to_chars(f_buf, f_buf + f_len, f_val,
                              std::chars_format::scientific, int(f_accuracacy));
          break;
        default:
          return 0;
          break;
      }
      if (res.ec != std::errc{}) {
        return {};
      }
      if (add_prefix) {
        if (*f_buf == '+' || *f_buf == ' ' || *f_buf == '-') {
          *(f_buf - 2) = *f_buf;
          f_buf++;
        }
        f_buf -= 2;
        f_buf[0] = '0';
        f_buf[1] = upper_case ? 'X' : 'x';
        f_len += 2;
        if (upper_case) {
          uintlen_t v = find_ch(f_buf, f_len, 0, 'p');
          if (v < f_len) f_buf[v] = 'P';
        };
      }
      return uintlen_t(res.ptr - buf_c_);
    }
#endif
#endif
    return from_dec_float_fill(f_buf, f_len, double(f_val), f_accuracacy,
                               upper_case, floating_format, add_prefix);
  }

  template <std::floating_point T,
            callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
                decltype(defualt_is_point_fn),
            callable_c<std::optional<
                std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>>(
                const char &, const uint8_t &) noexcept>
                power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN static std::optional<T> to_real_floating(
      const char *ptr, uintlen_t len,
      is_point_fn_t &&is_point_fn = is_point_fn_t{},
      power_fn_t &&power_fn = power_fn_t{}) noexcept {
    uint8_t raidex_{10};

    bool is_neg{};
    while (len && *ptr == ' ') {
      ptr++;
      len--;
    }
    if (len && (*ptr == '-' || *ptr == '+')) {
      is_neg = *ptr == '-';
      ptr++;
      len--;
    }
    if (len && *ptr != '0') {
      return to_real_floating_pv<T>(ptr, len, 10, is_point_fn, power_fn);
    }
    if (len) {
      ptr++;
      len--;
    }
    raidex_ = 10;
    if (len && (*ptr == 'X' || *ptr == 'x')) {
      raidex_ = 16;
      ptr++;
      len--;
    }

    std::optional<T> ret =
        to_real_floating_pv<T>(ptr, len, raidex_, is_point_fn, power_fn);
    if (ret) *ret = is_neg ? -*ret : *ret;
    return ret;
  }

  template <std::floating_point T,
            callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
                std::remove_cvref_t<decltype(defualt_is_point_fn)>,
            callable_c<std::optional<
                std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>>(
                const char &, const uint8_t &) noexcept>
                power_fn_t = std::remove_cvref_t<decltype(defualt_power_fn)>>
  MJZ_CX_ND_FN static std::optional<T> to_floating(
      const char *ptr, uintlen_t len,
      is_point_fn_t &&is_point_fn = is_point_fn_t{},
      power_fn_t &&power_fn = power_fn_t{}) noexcept {
    uint8_t raidex_{10};
    bool is_neg{};
    while (len && *ptr == ' ') {
      ptr++;
      len--;
    }
    if (len && (*ptr == '-' || *ptr == '+')) {
      is_neg = *ptr == '-';
      ptr++;
      len--;
    }
    if (len && *ptr != '0') {
      return to_floating_pv<T>(ptr, len, is_point_fn, power_fn);
    }
    if (len) {
      ptr++;
      len--;
    }
    raidex_ = 10;
    if (len && (*ptr == 'X' || *ptr == 'x')) {
      raidex_ = 16;
      ptr++;
      len--;
    }

    std::optional<T> ret =
        to_real_floating_pv<T>(ptr, len, raidex_, is_point_fn, power_fn);
    if (ret) *ret = is_neg ? -*ret : *ret;
    return ret;
  }

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static std::optional<T> to_integral(
      const char *ptr, uintlen_t len, uint8_t raidex_ = 0) noexcept {
    bool is_neg{};
    auto do_ret = [&](std::optional<T> ret) noexcept -> std::optional<T> {
      if constexpr (std::signed_integral<T>) {
        if (ret) *ret = is_neg ? -*ret : *ret;
      } else {
        if (is_neg) {
          return nullopt;
        }
      }
      return ret;
    };
    if (raidex_) {
      return do_ret(to_integral_pv<T>(ptr, len, raidex_));
    }
    while (len && *ptr == ' ') {
      ptr++;
      len--;
    }
    if (len && (*ptr == '-' || *ptr == '+')) {
      is_neg = *ptr == '-';
      ptr++;
      len--;
    }
    if (len && *ptr != '0') {
      return do_ret(to_integral_pv<T>(ptr, len, 10));
    }
    if (len) {
      ptr++;
      len--;
      if (!len) return do_ret(T(0));
    }
    raidex_ = 8;
    if (len && (*ptr == 'X' || *ptr == 'x')) {
      raidex_ = 16;
      ptr++;
      len--;
    }
    return do_ret(to_integral_pv<T>(ptr, len, raidex_));
  }
  static constexpr uintlen_t max_pow_pow10_double = 9;

  static constexpr const std::array<std::array<big_float_t<version_v>, 2>,
                                    max_pow_pow10_double>
      powers_of_ten_table = []() noexcept {
        std::array<std::array<big_float_t<version_v>, 2>, max_pow_pow10_double>
            powers_of_ten{};
        powers_of_ten[0][0] = big_float_t<version_v>::float_from_i(10);
        powers_of_ten[0][1] = *big_float_t<version_v>::float_from(0.1);
        for (uintlen_t i{1}; i < max_pow_pow10_double; i++) {
          for (uintlen_t j{}; j < 2; j++) {
            powers_of_ten[i][j] =
                powers_of_ten[i - 1][j] * powers_of_ten[i - 1][j];
          }
        }
        return powers_of_ten;
      }();
  static constexpr const std::array<std::array<double, max_pow_pow10_double>, 2>
      powers_of_5_table = []() noexcept {
        std::array<std::array<double, max_pow_pow10_double>, 2> powers_of_5{};
        powers_of_5[0][0] = 5.0;
        powers_of_5[1][0] = 0.2;
        for (uintlen_t i{1}; i < max_pow_pow10_double; i++) {
          for (uintlen_t j{}; j < 2; j++) {
            powers_of_5[j][i] = powers_of_5[j][i - 1] * powers_of_5[j][i - 1];
          }
        }
        return powers_of_5;
      }();
  MJZ_CX_FN bool static double_is_IEEE754_binary64_assert() noexcept {
    constexpr std::numeric_limits<double> dbnl{};
    constexpr const double IEE754_array_[6]{(1.7976931348623157E308),
                                            (2.2250738585072014E-308),
                                            (4.9E-324),
                                            (2.220446049250313E-16),
                                            (0.5),
                                            (-1.7976931348623157E308)};
    constexpr const uint64_t IEE754_array_bits[6]{9218868437227405311ull,
                                                  4503599627370496ull,
                                                  1ull,
                                                  4372995238176751616ull,
                                                  4602678819172646912ull,
                                                  18442240474082181119ull};

    constexpr const double std_array_bads_[]{dbnl.infinity(), dbnl.quiet_NaN(),
                                             dbnl.signaling_NaN()};
    constexpr const double std_array_[6]{dbnl.max(),         dbnl.min(),
                                         dbnl.denorm_min(),  dbnl.epsilon(),
                                         dbnl.round_error(), dbnl.lowest()};
    constexpr const uint64_t std_array_bits[6]{
        std::bit_cast<uint64_t>(std_array_[0]),
        std::bit_cast<uint64_t>(std_array_[1]),
        std::bit_cast<uint64_t>(std_array_[2]),
        std::bit_cast<uint64_t>(std_array_[3]),
        std::bit_cast<uint64_t>(std_array_[4]),
        std::bit_cast<uint64_t>(std_array_[5]),
    };
    constexpr const uint64_t std_array_bits_bad[2]{
        std::bit_cast<uint64_t>(std_array_bads_[1]),
        std::bit_cast<uint64_t>(std_array_bads_[2]),
    };
    bool good{true};
    for (uintlen_t i{}; i < 6; i++) {
      good &= IEE754_array_[i] == std_array_[i];
      good &= std_array_bits[i] == IEE754_array_bits[i];
    }
    good &=
        9218868437227405312ull == std::bit_cast<uint64_t>(std_array_bads_[0]);

    const uint64_t exp_mask = ((uint64_t(1) << 11) - 1) << 52;
    const uint64_t sleepy_nan_mask = (uint64_t(1) << 51);

    good &= ((sleepy_nan_mask | exp_mask) & std_array_bits_bad[0]) ==
            (sleepy_nan_mask | exp_mask);
    good &=
        ((sleepy_nan_mask | exp_mask) & std_array_bits_bad[1]) == (exp_mask);
    return good;
  }
  static_assert(double_is_IEEE754_binary64_assert());
  MJZ_CX_FN big_float_t<version_v> static pos_real_dbl_to_bf_impl_(
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
    const int64_t coeffient =
        int64_t((mantisa_mask & u64_val) | (uint64_t(!is_subnormal) << 52));
    big_float_t<version_v> ret{};
    ret.m_coeffient = coeffient;
    ret.m_exponent = exponent;
    return ret;
  }
  MJZ_CX_FN int64_t static exponent_log10_and_component_(
      big_float_t<version_v> &val) noexcept {
    big_float_t<version_v> f_val = val;
    if (f_val.m_coeffient == 0) {
      return 0;
    }
    asserts(asserts.assume_rn, 0 < f_val.m_coeffient);
    int64_t exponent_log10{};
    if (big_float_t<version_v>::float_from_i(10) <= f_val) {
      for (intlen_t i{intlen_t(max_pow_pow10_double - 1)}; 0 <= i; i--) {
        if (powers_of_ten_table[size_t(i)][0] <= f_val) {
          exponent_log10 += int64_t(1) << i;
          f_val = f_val * powers_of_ten_table[size_t(i)][1];
        }
      }
    } else if (f_val < big_float_t<version_v>::float_from_i(1)) {
      for (intlen_t i{intlen_t(max_pow_pow10_double - 1)}; 0 <= i; i--) {
        if (f_val <= powers_of_ten_table[size_t(i)][1]) {
          exponent_log10 -= int64_t(1) << i;
          f_val = f_val * powers_of_ten_table[size_t(i)][0];
        }
      }
      if (f_val < big_float_t<version_v>::float_from_i(1)) {
        exponent_log10 -= 1;
        f_val = f_val * powers_of_ten_table[0][0];
      }
    }
    val = f_val;
    asserts(asserts.assume_rn,
            !(f_val < big_float_t<version_v>::float_from_i(1) ||
              big_float_t<version_v>::float_from_i(10) <= f_val));
    return exponent_log10;
  }
  MJZ_CX_FN int64_t static exponent_log10_and_component_aprox_(
      big_float_t<version_v> &val) noexcept {
    big_float_t<version_v> f_val = val;
    if (f_val.m_coeffient == 0) {
      return 0;
    }  // based on desmos , log10_floor_prox is either floor(log10(x)) or
       // floor(log10(x))+1 or floor(log10(x))-1
    /*
    g\left(x\right)=x-1,
    f\left(x\right)=\operatorname{floor}\left(\frac{\log\left(x\right)}{\log\left(2\right)}\right),
    k\left(x\right)=x-\operatorname{floor}\left(x\right),
    m\left(x\right)=\frac{\operatorname{floor}\left(2^{32+k\left(\frac{\log\left(x\right)}{\log2}\right)}\right)}{2^{32}},
    h\left(x\right)=f\left(x\right)+g\left(m\left(x\right)\right),

    graph:

    \operatorname{floor}\left(\log
    x\right)-\operatorname{floor}\left(h\left(x\right)\cdot\log2\right)

    */
    asserts(asserts.assume_rn, 0 < f_val.m_coeffient);
    const int cnt = std::countl_zero(uint64_t(f_val.m_coeffient)) - 1;
    f_val.m_coeffient <<= cnt;
    f_val.m_exponent -= cnt;
    int64_t log2_floor = f_val.m_exponent + 62;
    asserts(asserts.assume_rn, int32_t(log2_floor) == log2_floor);
    int64_t mantisa_ = int64_t(uint32_t(uint64_t(f_val.m_coeffient) >>
                                        30) /* the msb goes away*/) +
                       (log2_floor << 32);
    big_float_t<version_v> log2_val{};
    log2_val.m_coeffient = mantisa_;
    log2_val.m_exponent = -32;
    constexpr const big_float_t<version_v> log10_2 =
        *big_float_t<version_v>::float_from(0.30102999566398119521373889472449);
    constexpr const big_float_t<version_v> inv_ten =
        *big_float_t<version_v>::float_from(0.1);
    constexpr const big_float_t<version_v> ten =
        big_float_t<version_v>::float_from_i(10);
    constexpr const big_float_t<version_v> one =
        big_float_t<version_v>::float_from_i(1);
    const big_float_t<version_v> log10_val = log2_val * log10_2;

    asserts(asserts.assume_rn, log10_val.m_exponent < 64);
    const int64_t log10_floor_prox =
        log10_val.m_exponent <= -64
            ? 0
            : (log10_val.m_exponent < 0
                   ? log10_val.m_coeffient >> (-log10_val.m_exponent)
                   : log10_val.m_coeffient << log10_val.m_exponent);

    const int64_t abs_log10_floor_prox{
        std::max(log10_floor_prox, -log10_floor_prox)};
    double inv_pow5_of_log10_floor_prox_d{1.0};
    for (uintlen_t i{}; i < max_pow_pow10_double; i++) {
      inv_pow5_of_log10_floor_prox_d = branchless_teranary(
          !!((int64_t(1) << i) & abs_log10_floor_prox),
          inv_pow5_of_log10_floor_prox_d *
              powers_of_5_table[abs_log10_floor_prox == log10_floor_prox][i],
          inv_pow5_of_log10_floor_prox_d);
    }
    big_float_t<version_v> inv_pow10_of_log10_floor_prox =
        pos_real_dbl_to_bf_impl_(inv_pow5_of_log10_floor_prox_d);
    inv_pow10_of_log10_floor_prox.m_exponent -= log10_floor_prox;
    f_val = f_val * inv_pow10_of_log10_floor_prox;
    int64_t ret{};
    if (ten <= f_val) {
      f_val = f_val * inv_ten;
      ret = log10_floor_prox + 1;
    } else if (f_val < one) {
      f_val = f_val * ten;
      ret = log10_floor_prox - 1;
    } else {
      ret = log10_floor_prox;
    }
    val = f_val;
    asserts(asserts.assume_rn, int32_t(ret) == ret);
    return ret;
  }
  constexpr static auto const fn_number_extract =
      [](uintN_t<version_v, 128> number) noexcept {
        number.nth_word(1) = 0;
        number <<= 1;
        number += number << 2;
        return number;
      };

  MJZ_CX_AL_FN static uintlen_t from_dec_positive_float_fill_sientific_impl_(
      char *const f_buf, const uintlen_t f_len,
      big_float_t<version_v> /* normalized */ f_val, int64_t exponent_log10,
      uint8_t f_accuracacy, bool upper_case,
      char (&buffer_1_)[64 /*do not reduce*/]) noexcept {
    char *const buffer_ = buffer_1_;
    char *ptr = buffer_;
    if (0 < f_val.m_exponent) {
      f_val.m_coeffient <<= f_val.m_exponent;
      f_val.m_exponent = 0;
    }
    f_accuracacy = std::min<uint8_t>(f_accuracacy, 32);
    uintN_t<version_v, 128> number{0 /*float error*/,
                                   uint64_t(f_val.m_coeffient)};
    number >>= uintlen_t(-f_val.m_exponent);
    *ptr++ = char(number.nth_word(1) + '0');
    *ptr++ = '.';
    constexpr uintN_t<version_v, 128> ten_p3(1000);
    for (uintlen_t i{}; i < f_accuracacy;) {
      number.nth_word(1) = 0;
      number *= ten_p3;
      constexpr uint64_t zero_8parallel_ascii =  cpy_bitcast<uint64_t>("00000000");

      uint64_t u64val =
          dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_<
              version_v.is_BE()>(uint32_t(number.nth_word(1))) |
          zero_8parallel_ascii;
      if constexpr(version_v.is_LE()){
        u64val >>= 40;
      } else{
        u64val <<= 40;
      }
      const auto uch64 = make_bitcast(u64val);
      for (uintlen_t j{}; j < 3; j++) {
        bool good = i < f_accuracacy;
        *ptr = branchless_teranary(good, uch64[j], *ptr);
        ptr += good;
        i += good;
      }
    }
    if (number.nth_bit(63)) {
      ptr--;
      while (*ptr == '9') {
        ptr--;
      }
      if (*ptr == '.') {
        char *dot_ptr{ptr--};
        while (*ptr == '9') {
          if (ptr != buffer_) {
            *ptr-- = '0';
            continue;
          }
          *ptr = '0';
          exponent_log10++;
          break;
        }
        (*ptr)++;
        ptr = --dot_ptr;
      } else {
        (*ptr)++;
      }
      ptr++;
    } else {
      ptr--;
      while (*ptr == '0') {
        ptr--;
      }
      if (*ptr == '.') {
        ptr--;
      }
      ptr++;
    }
    *ptr++ = upper_case ? 'E' : 'e';
    if (exponent_log10 < 0) {
      *ptr++ = '-';
      exponent_log10 = -exponent_log10;
    }
    asserts(asserts.assume_rn, exponent_log10 < 512);
    ptr += dec_from_uint(ptr, 24, uint16_t(exponent_log10));

    if (f_len < uintlen_t(ptr - buffer_)) return {};
    memcpy(f_buf, buffer_, uintlen_t(ptr - buffer_));
    return uintlen_t(ptr - buffer_);
  }
  MJZ_NCX_FN static uintlen_t ncx_from_dec_positive_float_fill_sientific_impl_(
      char *const f_buf, const uintlen_t f_len,
      big_float_t<version_v> /* normalized */ f_val, int64_t exponent_log10,
      uint8_t f_accuracacy, bool upper_case) noexcept {
    MJZ_DISABLE_ALL_WANINGS_START_;
    struct buffer_0_t_ {
      MJZ_NCX_FN buffer_0_t_() noexcept {}  // ctor does not initialize 'i'
      alignas(16) char buffer_0_[64 /*do not reduce*/];
    } a;
    MJZ_DISABLE_ALL_WANINGS_END_;
    return from_dec_positive_float_fill_sientific_impl_(
        f_buf, f_len, f_val, exponent_log10, f_accuracacy, upper_case,
        a.buffer_0_);
  }
  MJZ_CX_FN static uintlen_t from_dec_positive_float_fill_sientific(
      char *const f_buf, const uintlen_t f_len,
      big_float_t<version_v> /* normalized */ f_val, int64_t exponent_log10,
      uint8_t f_accuracacy, bool upper_case) noexcept {
    MJZ_IF_CONSTEVAL {
      alignas(16) char buffer_0_[64 /*do not reduce*/]{};
      return from_dec_positive_float_fill_sientific_impl_(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy, upper_case,
          buffer_0_);
    }
    else {
      return ncx_from_dec_positive_float_fill_sientific_impl_(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy, upper_case);
    }
  }
  MJZ_CX_FN static uintlen_t from_dec_positive_float_fill_general(
      char *const f_buf, const uintlen_t f_len,
      big_float_t<version_v> /* normalized */ f_val, int64_t exponent_log10,
      uint8_t f_accuracacy) noexcept {
    char *ptr = f_buf;
    const char *const ptr_end = f_buf + f_len;
    big_float_t<version_v> frac_ = f_val;

    uintN_t<version_v, 128> number{0 /*float error*/,
                                   uint64_t(frac_.m_coeffient)};
    number >>= uintlen_t(-frac_.m_exponent);
    int64_t number_of_chars_rem =
        std::max(exponent_log10, -exponent_log10) + 1 + f_accuracacy;
    if (ptr_end - ptr < number_of_chars_rem + 2) {
      return {};
    }
    if (0 <= exponent_log10) {
      for (; 0 <= exponent_log10; exponent_log10--) {
        *ptr++ = char(number.nth_word(1) + '0');
        number = fn_number_extract(number);
        if (!number.nth_word(0)) {
          if (ptr_end - ptr < exponent_log10) {
            return {};
          }
          memset(ptr, uintlen_t(exponent_log10), '0');
          ptr += exponent_log10;
          return uintlen_t(ptr - f_buf);
        }
      }
      number_of_chars_rem -= exponent_log10;
    } else {
      *ptr++ = '0';
      number_of_chars_rem -= 1;
    }
    if (!number_of_chars_rem) return uintlen_t(ptr - f_buf);
    *ptr++ = '.';
    auto remove_stupid_zeros = [&ptr]() noexcept {
      ptr--;
      while (*ptr == '0') {
        ptr--;
      }
      if (*ptr == '.') {
        ptr--;
      }
      ptr++;
    };
    if (exponent_log10 < 0) {
      memset(ptr, uintlen_t(-exponent_log10), '0');
      number_of_chars_rem += exponent_log10;
      ptr -= exponent_log10;
      exponent_log10 = 0;
      ptr--;
    }

    for (; number_of_chars_rem; number_of_chars_rem--) {
      *ptr++ = char(number.nth_word(1) + '0');
      number = fn_number_extract(number);
      if (!number.nth_word(0)) {
        remove_stupid_zeros();
        return uintlen_t(ptr - f_buf);
      }
    }

    if (!number.nth_bit(63)) {
      remove_stupid_zeros();
      return uintlen_t(ptr - f_buf);
    }
    ptr--;
    while (*ptr == '9') {
      ptr--;
    }
    if (*ptr != '.') {
      (*ptr++)++;
      return uintlen_t(ptr - f_buf);
    }
    char *dot_ptr{ptr--};
    while (*ptr == '9') {
      if (ptr != f_buf) {
        *ptr-- = '0';
        continue;
      }
      memomve_overlap(ptr + 1, ptr, uintlen_t(dot_ptr - ptr));
      dot_ptr++;
      break;
    }
    (*ptr++)++;
    return uintlen_t(/*dont include dot*/ dot_ptr - f_buf);
  }

  MJZ_CX_FN static uintlen_t from_dec_float_fill(
      char *f_buf, uintlen_t f_len, double val, const uint8_t f_accuracacy = 6,
      bool upper_case = true,
      floating_format_e floating_format = floating_format_e::general,
      bool add_prefix = true) noexcept {
    if (!f_len || !f_buf) {
      return {};
    }
    // IEEE 754 double-precision binary floating-point format: binary64
    const uint64_t bval = std::bit_cast<uint64_t>(val);

    const uint64_t sign_mask = uint64_t(1) << 63;
    uintlen_t ret_{};
    if (bval & sign_mask) {
      *f_buf++ = '-';
      f_len--;
      ret_++;
      val = -val;
    }
    const uint64_t exp_mask = ((uint64_t(1) << 11) - 1) << 52;
    const uint64_t mantisa_mask = ((uint64_t(1) << 52) - 1);
    const uint64_t sleepy_nan_mask = (uint64_t(1) << 51);
    if ((exp_mask & bval) == exp_mask) {
      // from: charconv standard header Copyright (c) Microsoft Corporation.
      // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
      const char *str_ptr_{};
      size_t str_len_{};
      const uint64_t mantisa = bval & mantisa_mask;
      if (mantisa == 0) {
        str_ptr_ = "inf";
        str_len_ = 3;
      } else if (bool(bval & sign_mask) && mantisa == sleepy_nan_mask) {
        str_ptr_ = "nan(ind)";
        str_len_ = 8;
      } else if (bool(mantisa & sleepy_nan_mask)) {
        str_ptr_ = "nan";
        str_len_ = 3;
      } else {
        str_ptr_ = "nan(snan)";
        str_len_ = 9;
      }
      if (f_len < str_len_) {
        return 0;
      }
      memcpy(f_buf, str_ptr_, str_len_);
      return ret_ + str_len_;
    }
    big_float_t<version_v> f_val{pos_real_dbl_to_bf_impl_(val)};
    if (f_val.m_coeffient == 0) {
      *f_buf = '0';
      return ret_ + 1;
    }

    if (floating_format == floating_format_e::hex) {
      uintlen_t ret =
          from_float_fill_hex(f_buf, f_len, val, upper_case, '.', add_prefix);
      return ret ? ret_ + ret : 0;
    }
    int64_t exponent_log10 = exponent_log10_and_component_aprox_(f_val);
    if (floating_format_e::general == floating_format) {
      auto abs_exp = std::max(exponent_log10, -exponent_log10);
      floating_format = !(intlen_t(f_len) < abs_exp + 1 + f_accuracacy) &&
                                abs_exp <= int64_t(f_accuracacy)
                            ? floating_format_e::fixed
                            : floating_format_e::scientific;
    }
    if (floating_format_e::scientific == floating_format) {
      uintlen_t ret = from_dec_positive_float_fill_sientific(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy, upper_case);

      return ret ? ret_ + ret : 0;
    } else {
      uintlen_t ret = from_dec_positive_float_fill_general_parallel(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy);
      return ret ? ret_ + ret : 0;
    }
  }
  /*
  2 u64 muls (*10000,*(1/10000)).
  2 u64 muls (*100,*(1/100)).
  2 u64 muls (*10,*(1/10)).

  */
  template <bool native_endian = true>
  MJZ_CX_AL_FN static uint64_t
  dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
      const uint32_t lower_half, const uint32_t upper_half) noexcept {
    asserts(asserts.assume_rn, upper_half < 10000 && lower_half < 10000);
    constexpr uint64_t inv25_16b = 2622;

    constexpr uint64_t inv5_8b = 52;

    constexpr uint64_t mask_16b =
        uint64_t(uint16_t(-1)) | (uint64_t(uint16_t(-1)) << 32);
    constexpr uint64_t mask_8b =
        uint64_t(uint8_t(-1)) | (uint64_t(uint8_t(-1)) << 16) |
        (uint64_t(uint8_t(-1)) << 32) | (uint64_t(uint8_t(-1)) << 48);

    /*
    D\left(x\right)=\operatorname{floor}\left(\frac{\operatorname{ceil}\left(\frac{2^{16}}{25}\right)\operatorname{floor}\left(x\right)}{2^{16}}\right)

    f\left(x\right)=D\left(\operatorname{floor}\left(\frac{x}{4}\right)\right)-\operatorname{floor}\left(\frac{x}{100}\right)

    \int_{0}^{100^{2}}f\left(x\right)dx=0

    */
    /*
    D\left(x\right)=\operatorname{floor}\left(\frac{\operatorname{ceil}\left(\frac{2^{8}}{5}\right)\operatorname{floor}\left(x\right)}{2^{8}}\right)

    f\left(x\right)=D\left(\operatorname{floor}\left(\frac{x}{2}\right)\right)-\operatorname{floor}\left(\frac{x}{10}\right)

    \int_{0}^{10^{2}}f\left(x\right)dx=0
    */
    if constexpr (native_endian) {
      const uint64_t div_2parellel_old =
          ((uint64_t(upper_half) << 32) | uint64_t(lower_half));

      const uint64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const uint64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const uint64_t div_4parellel_old =
          modulo_2parallel | (div_2parallel << 16);

      const uint64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const uint64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return modulo_4parallel | (div_4parellel << 8);
    } else {
      const uint64_t div_2parellel_old =
          (uint64_t(upper_half) | (uint64_t(lower_half) << 32));

      const uint64_t div_2parallel =
          ((((div_2parellel_old >> 2) & mask_16b) * inv25_16b) >> 16) &
          mask_16b;

      const uint64_t modulo_2parallel = div_2parellel_old - 100 * div_2parallel;

      const uint64_t div_4parellel_old =
          div_2parallel | (modulo_2parallel << 16);

      const uint64_t div_4parellel =
          ((((div_4parellel_old >> 1) & mask_8b) * inv5_8b) >> 8) & mask_8b;

      const uint64_t modulo_4parallel = div_4parellel_old - 10 * div_4parellel;
      return div_4parellel | (modulo_4parallel << 8);
    }
  }
  MJZ_CX_FN static uintlen_t
  dec_from_uint_backwards_parallel_less_than_pow10_8_impl_(
      char *end_ptr /*at leat 8 bytes in length in back*/,
      const uint32_t number_less_than_pow10_8) noexcept {
    asserts(asserts.assume_rn, number_less_than_pow10_8 < 100000000);

    constexpr uint64_t zero_8parallel_ascii = cpy_bitcast<uint64_t>("00000000");
    const uint32_t upper_half{number_less_than_pow10_8 / 10000};
    const uint32_t lower_half{number_less_than_pow10_8 % 10000};
    const uint64_t awnser_8parallel =
        dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
            lower_half, upper_half);
    const uint8_t num_0ch =
        uint8_t(std::min(std::countl_zero(awnser_8parallel) >> 3, 7));
    const uint64_t awnser_8parallel_ascii =
        awnser_8parallel | zero_8parallel_ascii;
    alignas(8) char awnser_8ch[8]{};
    cpy_bitcast(awnser_8ch, awnser_8parallel_ascii);
    if constexpr (version_v.is_LE()) {
      mem_byteswap(awnser_8ch, 8);
    }
    const uintlen_t count_len = 8 - uintlen_t(num_0ch);
    memcpy(end_ptr - 8, awnser_8ch, 8);
    return count_len;
  }

  MJZ_CX_FN static uintN_t<version_v, 128>
  dec_from_uint_backwards_parallel_less_than_pow10_16_impl_both_(
      const uint32_t upper_half, const uint32_t lower_half) noexcept {
    asserts(asserts.assume_rn,
            upper_half < 100000000 && lower_half < 100000000);
    constexpr uintN_t<version_v, 128> inv5p4_32b = 6871948;
    constexpr uintN_t<version_v, 128> val10p4_32b = 100000;
    constexpr uintN_t<version_v, 128> mask_32b{uint32_t(-1), uint32_t(-1)};

    /*
    D\left(x\right)=\operatorname{floor}\left(\frac{\operatorname{ceil}\left(\frac{2^{32}}{5^{4}}\right)\operatorname{floor}\left(x\right)}{2^{32}}\right)


    f\left(x\right)=D\left(\operatorname{floor}\left(\frac{x}{2^{4}}\right)\right)-\operatorname{floor}\left(\frac{x}{10^{4}}\right)

    \int_{0}^{10^{8}}f\left(x\right)dx=0
    */

    const uintN_t<version_v, 128> parrellel2_old{lower_half, upper_half};

    const uintN_t<version_v, 128> parrellel2_div =
        ((((parrellel2_old >> 4) & mask_32b) * inv5p4_32b) >> 32) & mask_32b;

    const uintN_t<version_v, 128> parrellel2_modolo =
        parrellel2_old - parrellel2_div * val10p4_32b;

    const uintN_t<version_v, 128> parrellel4 =
        (parrellel2_div << 32) | parrellel2_modolo;

    return uintN_t<version_v, 128>{
        dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
            uint32_t(parrellel4.nth_word(0) >> 32),
            uint32_t(parrellel4.nth_word(0) & uint32_t(-1))),
        dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_(
            uint32_t(parrellel4.nth_word(1) >> 32),
            uint32_t(parrellel4.nth_word(1) & uint32_t(-1)))};
  }

  MJZ_CX_FN static uintN_t<version_v, 256>
  dec_from_uint_backwards_parallel_less_than_pow10_32_impl_both_(
      const uint64_t upper_half, const uint64_t lower_half) noexcept {
    asserts(asserts.assume_rn,
            upper_half < 10000000000000000 && lower_half < 10000000000000000);
    constexpr uintN_t<version_v, 256> inv5p8_64b = 47223664828697;
    constexpr uintN_t<version_v, 256> val10p8_64b = 100000000;
    constexpr uintN_t<version_v, 256> mask_64b{uint64_t(-1), 0, uint64_t(-1),
                                               0};

    /*
    D\left(x\right)=\operatorname{floor}\left(\frac{\operatorname{ceil}\left(\frac{2^{64}}{5^{8}}\right)\operatorname{floor}\left(x\right)}{2^{64}}\right)

    f\left(x\right)=D\left(\operatorname{floor}\left(\frac{x}{2^{8}}\right)\right)-\operatorname{floor}\left(\frac{x}{10^{8}}\right)

    \int_{0}^{10^{16}}f\left(x\right)dx=0
    */

    const uintN_t<version_v, 256> parrellel2_old{lower_half, 0, upper_half, 0};

    const uintN_t<version_v, 256> parrellel2_div =
        ((((parrellel2_old >> 8) & mask_64b) * inv5p8_64b) >> 64) & mask_64b;

    const uintN_t<version_v, 256> parrellel2_modolo =
        parrellel2_old - parrellel2_div * val10p8_64b;

    const uintN_t<version_v, 256> parrellel4 =
        (parrellel2_div << 64) | parrellel2_modolo;

    return uintN_t<version_v, 256>{
        dec_from_uint_backwards_parallel_less_than_pow10_16_impl_both_(
            uint32_t(parrellel4.nth_word(1)), uint32_t(parrellel4.nth_word(0))),
        dec_from_uint_backwards_parallel_less_than_pow10_16_impl_both_(
            uint32_t(parrellel4.nth_word(3)),
            uint32_t(parrellel4.nth_word(2)))};
  }

  MJZ_CX_FN static uintlen_t dec_from_uint_impl_parallel_impl_(
      char *out_buf, uintlen_t out_len, uint64_t number_0_,
      char *modolo10) noexcept {
    constexpr const uint64_t least_parrellel = 100000000;
    uintlen_t ret{};
    do {
      uint32_t less_least_parrellel{uint32_t(number_0_)};
      if (least_parrellel <= number_0_) {
        less_least_parrellel = uint32_t(number_0_ % least_parrellel);
        number_0_ = number_0_ / least_parrellel;
      } else {
        number_0_ = 0;
      }
      ret += dec_from_uint_backwards_parallel_less_than_pow10_8_impl_(
          modolo10 + 24 - ret, less_least_parrellel);
    } while (number_0_);
    if (out_len < ret) return 0;
    memcpy(out_buf, modolo10 + 24 - ret, ret);
    return ret;
  }

  template <bool native_endian = true>
  MJZ_CX_AL_FN static uint64_t
  dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_(
      const uint32_t number_0_) noexcept {
    const int16_t div_res = divition10_table[number_0_];
    const uint64_t most_significant_digit = uint8_t(div_res & 15);
    const uint64_t middle_digit = uint8_t((div_res >> 4) & 15);
    const uint64_t least_significant_digit = uint8_t((div_res >> 8) & 15);
    if constexpr (!native_endian) {
      return ((least_significant_digit << 16) | (middle_digit << 8) |
              (most_significant_digit))
             << 40;
    }
    return (least_significant_digit) | (middle_digit << 8) |
           (most_significant_digit << 16);
  }
  template <bool native_endian = true>
  MJZ_CX_AL_FN static uint64_t
  dec_from_uint_backwards_parallel_less_than_pow10_6_pair_impl_(
      const uint32_t lower_half, const uint32_t upper_half) {
    const uint64_t most_significant_digits =
        dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_<
            native_endian>(upper_half);
    const uint64_t least_significant_digits =
        dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_<
            native_endian>(lower_half);
    if constexpr (!native_endian) {
      return (least_significant_digits) | (most_significant_digits >> 24);
    }
    return (least_significant_digits) | (most_significant_digits << 24);
  }

  template <std::unsigned_integral T>
  constexpr static const uintlen_t
      dec_from_uint_impl_semi_parallel_impl_count_max =
          sizeof(T) == 1 ? 1 : (sizeof(T) == 2 ? 1 : (sizeof(T) == 4 ? 2 : 3));
  template <std::unsigned_integral T>
  MJZ_CX_AL_FN static std::tuple<
      uintN_t<version_v,
              dec_from_uint_impl_semi_parallel_impl_count_max<T> * 64>,
      uintlen_t, uintlen_t, uintlen_t>
  dec_from_uint_impl_semi_parallel_impl_(const T number_) noexcept {
    constexpr uint64_t zero_8parallel_ascii = cpy_bitcast<uint64_t>("00000000");
    constexpr uint64_t lookup_full = 1000;
    constexpr uint64_t parallel_half = 10000;
    constexpr uint64_t parallel_full = parallel_half * parallel_half;
    constexpr uint64_t count_max =
        dec_from_uint_impl_semi_parallel_impl_count_max<T>;
    uintlen_t iteration_count_backwards{count_max};
    uintN_t<version_v, count_max * 64> str_int_buf{};
    uint64_t number_0_ = number_;
    do {
      asserts(asserts.assume_rn, !!iteration_count_backwards);
      iteration_count_backwards--;
      uint32_t upper_half{};
      uint32_t lower_half{};

      uint64_t u64ch_{};
      if (number_0_ < lookup_full) {
        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_<
            version_v.is_BE()>(uint32_t(number_0_));
        number_0_ = 0;
      } else if (number_0_ < lookup_full * lookup_full) {
        if constexpr (sizeof(T) < 2) {
          asserts.unreachable();
        }
        lower_half = uint32_t(number_0_ % lookup_full);
        upper_half = uint32_t(number_0_ / lookup_full);
        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_6_pair_impl_<
            version_v.is_BE()>(lower_half, upper_half);
        number_0_ = 0;
      } else {
        if constexpr (sizeof(T) < 4) {
          asserts.unreachable();
        }
        uint32_t number_less_than_pow10_8{};
        if (number_0_ < parallel_full) {
          number_less_than_pow10_8 = uint32_t(number_0_);
          number_0_ = 0;
        } else {
          number_less_than_pow10_8 = uint32_t(number_0_ % parallel_full);
          number_0_ = number_0_ / parallel_full;
        }
        lower_half = uint32_t(number_less_than_pow10_8 % parallel_half);
        upper_half = uint32_t(number_less_than_pow10_8 / parallel_half);

        u64ch_ = dec_from_uint_backwards_parallel_less_than_pow10_8_pair_impl_<
            version_v.is_BE()>(lower_half, upper_half);
      }
      const uint64_t u64ch = u64ch_;
      uint64_t u64ch_ascii = u64ch | zero_8parallel_ascii;
      str_int_buf.words[count_max - 1 - iteration_count_backwards] =
          u64ch_ascii;
      if (number_0_) continue;
      const uint64_t num_high_0ch =
          uintlen_t((version_v.is_BE() ? std::countl_zero(u64ch)
                                       : std::countr_zero(u64ch)) >>
                    3);
      const uintlen_t num_0ch{num_high_0ch +
                              uintlen_t(iteration_count_backwards << 3)};
      const uintlen_t num_ch =
          std::max<uintlen_t>(sizeof(str_int_buf) - num_0ch, 1);
      return {str_int_buf, num_ch, iteration_count_backwards,
              std::min<uintlen_t>(7, num_high_0ch)};
    } while (true);

    return {};
  }

  template <std::unsigned_integral T>
  MJZ_CX_AL_FN static uintlen_t dec_from_uint_impl_semi_parallel(
      char *outbuf, uintlen_t outlen, T number_0_) noexcept {
    const auto [str_int_buf, num_ch, iteration_count_backwards, offset] =
        dec_from_uint_impl_semi_parallel_impl_(number_0_);
    if (outlen < num_ch) return 0;
    MJZ_IF_CONSTEVAL {
      auto temp = make_bitcast(str_int_buf);
      memcpy(outbuf, temp.data() + offset, num_ch);
      return num_ch;
    }
    memcpy(outbuf, reinterpret_cast<const char *>(&str_int_buf) + offset,
           num_ch);
    return num_ch;
  }

  MJZ_CX_FN static uintlen_t from_dec_positive_float_fill_general_parallel(
      char *const f_buf, const uintlen_t f_len,
      big_float_t<version_v> /* normalized */ f_val, int64_t exponent_log10,
      uint8_t f_accuracacy) noexcept {
    char *ptr = f_buf;
    const char *const ptr_end = f_buf + f_len;
    big_float_t<version_v> frac_ = f_val;

    constexpr uintN_t<version_v, 128> ten_p3(1000);
    constexpr uintN_t<version_v, 128> ten_p2(100);
    uintN_t<version_v, 128> number{0 /*float error*/,
                                   uint64_t(frac_.m_coeffient)};
    number >>= uintlen_t(-frac_.m_exponent);
    number *= ten_p2;  // prossesing is 3 digits at a time
    int64_t number_of_chars_rem =
        std::max(exponent_log10, -exponent_log10) + 1 + f_accuracacy;
    if (ptr_end - ptr < number_of_chars_rem + 2) {
      return {};
    }
    bool is_past_dot{};
    if (exponent_log10 < 0) {
      *ptr++ = '0';
    }
    uint64_t ch64u8{};
    uintlen_t ch64u8_count{};

    for (intlen_t i{}; i < (number_of_chars_rem); i++) {
      const bool need_dot = !is_past_dot && exponent_log10 < 0;
      if (need_dot) {
        *ptr++ = '.';
      }
      is_past_dot |= need_dot;

      uintlen_t num_parallel = uintlen_t(number_of_chars_rem - i);
      num_parallel = branchless_teranary(is_past_dot, num_parallel,
                                         uintlen_t(exponent_log10+1));
      num_parallel = std::min<uintlen_t>(num_parallel, 6);
      if (!num_parallel) continue;
      if (ch64u8_count < num_parallel) {
        const uintlen_t zero_bytes = 6 - ch64u8_count;
        constexpr uint64_t ascii3_par = cpy_bitcast<uint64_t>("000""\0\0\0\0\0");

        uint64_t ch64u8_val{};
        for (uintlen_t j{ch64u8_count}; j < zero_bytes; j += 3) {
          asserts(asserts.assume_rn, number.nth_word(1) < 1000);
          uint64_t u64val =
              dec_from_uint_backwards_parallel_less_than_pow10_3_pair_impl_<
                  version_v.is_BE()>(uint32_t(number.nth_word(1)));
          if constexpr(version_v.is_LE()){
            u64val >>= 40;
          } else{
            u64val <<= 40;
          }
         const uint64_t ch3par = u64val |
              ascii3_par;
          if constexpr (version_v.is_BE()) {
           ch64u8_val |= ch3par >> (j * 8);
          } else {
           ch64u8_val |= ch3par << (j * 8);
          }
          number.nth_word(1) = 0;
          if (number.nth_word(0)) {
            number *= ten_p3;
          }
          
        }
        ch64u8 |= ch64u8_val;
      }
      uint64_t ch64u8_val{ch64u8};
      if constexpr (version_v.is_BE()) {
        ch64u8 <<= num_parallel * 8;
      } else {
        ch64u8 >>= num_parallel * 8;
      }
      ch64u8_count -= num_parallel;
      exponent_log10 -= intlen_t(num_parallel);
      i += intlen_t(num_parallel);
      for (uintlen_t j{}; j < 48; j += 8) {
        uint8_t ch_{};
        if constexpr (version_v.is_BE()) {
          ch_ = uint8_t(ch64u8_val >> (56 - j));
        } else {
          ch_ = uint8_t(ch64u8_val >> j);
        }
        const char ch = char(ch_);
        const bool good = j < num_parallel*8;
        *ptr = branchless_teranary(good, ch, *ptr);
        ptr = branchless_teranary(good, ptr + 1, ptr);
      }
    }
    bool needs_rounding{number.nth_bit(63)};
    {
      char ch{};
      if constexpr (version_v.is_BE()) {
        ch = char(uint8_t(ch64u8 >> 56));
      } else {
        ch = char(uint8_t(ch64u8));
      }
      needs_rounding =
          branchless_teranary<bool>(!!ch, '5' <= ch, needs_rounding);
    }
    if (!needs_rounding) {
      ptr--;
      while (*ptr == '0') {
        ptr--;
      }
      if (*ptr == '.') {
        ptr--;
      }
      ptr++;
      return uintlen_t(ptr - f_buf);
    }
    ptr--;
    while (*ptr == '9') {
      ptr--;
    }
    if (*ptr != '.') {
      (*ptr++)++;
      return uintlen_t(ptr - f_buf);
    }
    char *dot_ptr{ptr--};
    while (*ptr == '9') {
      if (ptr != f_buf) {
        *ptr-- = '0';
        continue;
      }
      memomve_overlap(ptr + 1, ptr, uintlen_t(dot_ptr - ptr));
      dot_ptr++;
      break;
    }
    (*ptr++)++;
    return uintlen_t(/*dont include dot*/ dot_ptr - f_buf);
  }
};

}  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_traits_LIB_HPP_FILE_