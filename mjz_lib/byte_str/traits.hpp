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

  MJZ_CX_AL_FN static uintlen_t dec_from_uint64_impl_(
      char *out_buf, uintlen_t out_len, uint64_t number_,
      char (&modolo10)[8 * 3]) noexcept {
    int16_t v1000modolos[8]{};
    int8_t i_1000modolo{7};
    constexpr uint32_t max_i32_div = 1000 * 1000 * 1000;

    auto div_fn = [&](uint32_t number) noexcept {
      for (; 0 <= i_1000modolo;) {
        if (number < 1000) {
          v1000modolos[i_1000modolo] = int16_t(number);
          i_1000modolo--;
          break;
        }

        const uint32_t number_div = number / 1000;
        v1000modolos[i_1000modolo] = int16_t(number % 1000);
        number = number_div;
        i_1000modolo--;
      }
    };
    // no bigger is needed
    constexpr uint64_t max_2_i32_div = uint64_t(max_i32_div) * max_i32_div;
    if (max_2_i32_div < number_) {
      const uint64_t big_div = number_ / max_2_i32_div;
      div_fn(uint32_t(big_div));
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

    int num_ch{24};
    for (int8_t i{7}; i_1000modolo < i; i--) {
      const int offset = i * 3;
      const int16_t div_res = divition10_table[size_t(v1000modolos[i])];
      modolo10[offset] = char(div_res & 15) + '0';
      modolo10[offset + 1] = char((div_res >> 4) & 15) + '0';
      modolo10[offset + 2] = char((div_res >> 8) & 15) + '0';
      num_ch = (div_res >> 12) ? offset + 3 - (div_res >> 12) : num_ch;
    }
    num_ch = std::max(1, 24 - num_ch);
    if (out_len < uintlen_t(num_ch)) {
      return 0;
    }
    mjz::memcpy(out_buf, modolo10 + 24 - num_ch, uintlen_t(num_ch));
    return uintlen_t(num_ch);
  }
  MJZ_CX_FN static uintlen_t dec_from_uint64(char *out_buf, uintlen_t out_len,
                                             uint64_t number_) noexcept {
    MJZ_IF_CONSTEVAL {
      alignas(8) char modolo10[8 * 3]{};
      return dec_from_uint64_impl_(out_buf, out_len, number_, modolo10);
    }
    else {
      alignas(8) char modolo10[8 * 3];
      return dec_from_uint64_impl_(out_buf, out_len, number_, modolo10);
    }
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN static std::optional<uintlen_t> from_integral_fill(
      char *buf, uintlen_t len, T val_rg_, bool upper_case,
      const uint8_t raidex) noexcept {
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
    if (raidex == 10) {
      if constexpr (std::signed_integral<T>) {
        if (val_rg_ < 0) {
          *buf++ = '-';
          len--;
          uintlen_t ret = dec_from_uint64(buf, len, uint64_t(-val_rg_));
          return ret ? std::optional<uintlen_t>(ret + 1) : std::nullopt;
        }
      }
      uintlen_t ret = dec_from_uint64(buf, len, uint64_t(val_rg_));
      return ret ? std::optional<uintlen_t>(ret) : std::nullopt;
    }
    return [=]() mutable noexcept -> std::optional<uintlen_t> {
      if (!buf || !len || 36 < raidex || !raidex) return std::nullopt;
      using UT = std::make_unsigned_t<T>;
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
                                    max_pow_pow10_double> &powers_of_ten_table =
      make_static_data([]() noexcept {
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
      });
  MJZ_CX_FN int64_t static exponent_log10_and_component_(
      big_float_t<version_v> &val) noexcept {
    big_float_t<version_v> f_val = val;
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
    asserts(asserts.assume_rn,
            !(f_val < big_float_t<version_v>::float_from_i(1) ||
              big_float_t<version_v>::float_from_i(10) <= f_val));
    val = f_val;
    return exponent_log10;
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
    number = fn_number_extract(number);
    *ptr++ = '.';
    for (uintlen_t i{}; i < f_accuracacy; i++) {
      *ptr++ = char(number.nth_word(1) + '0');
      number = fn_number_extract(number);
    }
    if (number.nth_bit(63)) {
      ptr--;
      while (*ptr == '9') {
        ptr--;
      }
      if (*ptr == '.') {
        (*--ptr)++;
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
    asserts(asserts.assume_rn,
            uint64_t(uint32_t(exponent_log10)) == uint64_t(exponent_log10));
    ptr += dec_from_uint64(ptr, 24, uint64_t(exponent_log10));

    if (f_len < uintlen_t(ptr - buffer_)) return {};
    memcpy(f_buf, buffer_, uintlen_t(ptr - buffer_));
    return uintlen_t(ptr - buffer_);
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
    big_float_t<version_v> f_val{*big_float_t<version_v>::float_from(val)};
    uintlen_t ret_{};
    if (bval & sign_mask) {
      *f_buf++ = '-';
      f_len--;
      ret_++;
      f_val.m_coeffient = -f_val.m_coeffient;
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
    if (f_val.m_coeffient == 0) {
      *f_buf = '0';
      return ret_ + 1;
    }

    if (floating_format == floating_format_e::hex) {
      uintlen_t ret =
          from_float_fill_hex(f_buf, f_len, val, upper_case, '.', add_prefix);
      return ret ? ret_ + ret : 0;
    }
    int64_t exponent_log10 = exponent_log10_and_component_(f_val);
    if (floating_format_e::general == floating_format) {
      floating_format =
          std::max(exponent_log10, -exponent_log10) <= int64_t(f_accuracacy)
              ? floating_format_e::fixed
              : floating_format_e::scientific;
    }
    if (floating_format_e::scientific == floating_format) {
      uintlen_t ret = from_dec_positive_float_fill_sientific(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy, upper_case);

      return ret ? ret_ + ret : 0;
    } else {
      uintlen_t ret = from_dec_positive_float_fill_general(
          f_buf, f_len, f_val, exponent_log10, f_accuracacy);
      return ret ? ret_ + ret : 0;
    }
  }
};

}  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_traits_LIB_HPP_FILE_