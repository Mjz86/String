#include "../maths.hpp"
#include "base.hpp"
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
struct byte_traits_t {
  using enum floating_format_e;
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(auto) npos{(uintlen_t(-1) >> 8) + 1};

  MJZ_CX_ND_FN intlen_t pv_compare(const char* rhs, const char* lhs,
                                   uintlen_t len) const noexcept {
    if (lhs == rhs) return 0;
    MJZ_IFN_CONSTEVAL { return std::memcmp(rhs, lhs, len); }
    for (; 0 < len; --len, ++rhs, ++lhs) {
      if (*rhs != *lhs) {
        return *rhs < *lhs ? -1 : +1;
      }
    }
    return 0;
  }

  MJZ_CX_ND_FN uintlen_t pv_strlen(const char* begin) const noexcept {
    if (!begin) return 0;
    MJZ_IFN_CONSTEVAL { return std::strlen(begin); }
    uintlen_t len = 0;
    while (*begin != '\0') {
      ++len;
      ++begin;
    }

    return len;
  }

  MJZ_CX_ND_FN const char* pv_find(const char* begin, uintlen_t len,
                                   char c) const noexcept {
    for (; 0 < len; --len, ++begin) {
      if (*begin == c) {
        return begin;
      }
    }

    return nullptr;
  }
  MJZ_CX_ND_FN uintlen_t u_diff(const char* rhs,
                                const char* lhs) const noexcept {
    return static_cast<uintlen_t>(rhs - lhs);
  }

  MJZ_CX_ND_FN bool equal(const char* lhs, uintlen_t lhs_len, const char* rhs,
                          uintlen_t rhs_len) const noexcept {
    return lhs_len == rhs_len && pv_compare(lhs, rhs, lhs_len) == 0;
  }

  MJZ_CX_ND_FN intlen_t compare(const char* lhs, uintlen_t lhs_len,
                                const char* rhs,
                                uintlen_t rhs_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find(const char* hay_stack, uintlen_t hay_len,
                              uintlen_t offset, const char* needle,
                              uintlen_t needle_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find_ch(const char* hay_stack, uintlen_t hay_len,
                                 uintlen_t offset, char c) const noexcept {
    if (offset < hay_len) {
      const auto where = pv_find(hay_stack + offset, hay_len - offset, c);
      if (where) {
        return u_diff(where, hay_stack);
      }
    }

    return npos;
  }

  MJZ_CX_ND_FN uintlen_t rfind(const char* hay_stack, uintlen_t hay_len,
                               uintlen_t offset, const char* needle,
                               uintlen_t needle_len) const noexcept {
    if (needle_len == 0) {
      return std::min(offset, hay_len);
    }

    if (hay_len < needle_len) {
      return npos;
    }
    const char* canidate = hay_stack + std::min(offset, hay_len - needle_len);
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

  MJZ_CX_ND_FN uintlen_t rfind_ch(const char* hay_stack, uintlen_t hay_len,
                                  uintlen_t offset,
                                  const char Ch) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find_first_of(const char* hay_stack, uintlen_t hay_len,
                                       uintlen_t offset, const char* needle,
                                       uintlen_t needle_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find_last_of(const char* hay_stack, uintlen_t hay_len,
                                      uintlen_t offset, const char* needle,
                                      uintlen_t needle_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t
  find_first_not_of(const char* hay_stack, uintlen_t hay_len, uintlen_t offset,
                    const char* needle, uintlen_t needle_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find_not_ch(const char* hay_stack, uintlen_t hay_len,
                                     uintlen_t offset,
                                     const char Ch) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t find_last_not_of(const char* hay_stack,
                                          uintlen_t hay_len, uintlen_t offset,
                                          const char* needle,
                                          uintlen_t needle_len) const noexcept {
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

  MJZ_CX_ND_FN uintlen_t rfind_not_ch(const char* hay_stack, uintlen_t hay_len,
                                      uintlen_t offset, char c) const noexcept {
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
  MJZ_CX_ND_FN bool starts_with(const char* lhs, uintlen_t lhs_len,
                                const char* rhs,
                                uintlen_t rhs_len) const noexcept {
    if (lhs_len < rhs_len) {
      return false;
    }
    return pv_compare(lhs, rhs, rhs_len) == 0;
  }
  MJZ_CX_ND_FN bool starts_with(const char* lhs, uintlen_t lhs_len,
                                char rhs) const noexcept {
    if (lhs_len < 1) {
      return false;
    }
    return *lhs == rhs;
  }

  MJZ_CX_ND_FN bool ends_with(const char* lhs, uintlen_t lhs_len,
                              const char* rhs,
                              uintlen_t rhs_len) const noexcept {
    if (lhs_len < rhs_len) {
      return false;
    }
    return pv_compare(lhs + (lhs_len - rhs_len), rhs, rhs_len) == 0;
  }
  MJZ_CX_ND_FN bool ends_with(const char* lhs, uintlen_t lhs_len,
                              char rhs) const noexcept {
    if (lhs_len < 1) {
      return false;
    }
    return lhs[lhs_len - 1] == rhs;
  }
  MJZ_CX_ND_FN bool contains(const char* lhs, uintlen_t lhs_len,
                             const char* rhs,
                             uintlen_t rhs_len) const noexcept {
    return find(lhs, lhs_len, 0, rhs, rhs_len) != npos;
  }
  MJZ_CX_ND_FN bool contains(const char* lhs, uintlen_t lhs_len,
                             char rhs) const noexcept {
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
  MJZ_CX_FN uint8_t max_len_of_integral(uint8_t raidex) const noexcept {
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
  MJZ_CX_ND_FN std::optional<char> num_to_ascii(uint8_t i,
                                                bool is_upper) const noexcept {
    if (i < 37) {
      return (is_upper ? alphabett_table_upper : alphabett_table_lower)[i];
    }
    return std::nullopt;
  }

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral_pv(const char* ptr, uintlen_t len,
                                               uint8_t raidex) const noexcept {
    if (36 < raidex || !ptr || !len || !raidex) return std::nullopt;
    using UT = std::make_unsigned_t<T>;
    constexpr UT max_v = UT(-1);
    constexpr UT sign_bit = std::same_as<T, UT> ? 0 : ~(UT(-1) >> 1);
    const UT pre_max_v = UT(max_v / UT(raidex));

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
                const char&, const uint8_t&) noexcept>
                power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN std::optional<T> to_real_floating_pv(
      const char* ptr, uintlen_t len, const uint8_t raidex_,
      is_point_fn_t&& is_point_fn = is_point_fn_t{},
      power_fn_t&& power_fn = power_fn_t{}) const noexcept {
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
    range_t* previous_section{&sientific_coeffient_section1};
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

    auto get_sec1_val = [=](uint8_t raidex, const char* begin,
                            uintlen_t length) noexcept -> mjz_float_t {
      mjz_float_t var{};
      for (uintlen_t i{}; i < length; i++) {
        auto digit = ascii_to_num(begin[i]);
        var = var * mjz_float_t::float_from_i(raidex);
        var = var + mjz_float_t::float_from_i(*digit);
      }

      return var;
    };
    auto get_sec2_val = [=](uint8_t raidex, const char* begin,
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
    auto&& [power, idk] = *opt;
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

  template <
      std::floating_point T,
      callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
          decltype(defualt_is_point_fn),
      callable_c<std::optional<uintlen_t>(const char&, const uint8_t&) noexcept>
          power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN std::optional<T> to_floating_pv(
      const char* ptr, uintlen_t len, const uint8_t raidex,
      is_point_fn_t&& is_point_fn = is_point_fn_t{},
      power_fn_t&& power_fn = power_fn_t{}) const noexcept {
    if (ascii_to_num(std::min(std::min('N', 'A'), std::min('I', 'F'))) <
        raidex) {
      return std::nullopt;  // ambigous , NAN or INF could be a valid number!
    }
    if (std::optional<T> v = this->template to_real_floating<T>(
            ptr, len, raidex, is_point_fn, power_fn)) {
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
  MJZ_CX_ND_FN std::optional<uintlen_t> from_integral_fill_backwards(
      char* buf, uintlen_t len, T val_rg_, bool upper_case,
      const uint8_t raidex) const noexcept {
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
      auto r = val % UT(raidex);
      auto v = num_to_ascii(uint8_t(r), upper_case);
      if (!v) {
        return std::nullopt;
      }
      buf[i] = *v;
      val -= r;
      val /= UT(raidex);
      n++;
    }
    if (val) return std::nullopt;
    if (is_neg) {
      buf[len - n - 1] = '-';
      n++;
    }
    return n;
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<uintlen_t> from_integral_fill(
      char* buf, uintlen_t len, T val_rg_, bool upper_case,
      const uint8_t raidex) const noexcept {
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
      UT r = UT(UT(val) % UT(raidex));
      auto v = num_to_ascii(uint8_t(r), upper_case);
      if (!v) {
        return std::nullopt;
      }
      buf[i] = *v;
      val -= r;
      val /= UT(raidex);
      n++;
    }
    if (val) return std::nullopt;
    memomve_overlap(buf, &buf[len - n], n);
    if (is_neg) {
      *(--buf) = '-';
      n++;
    }
    return n;
  }

  template <std::integral T>
  using big_buff_t = std::pair<std::array<char, from_integral_max_len<T>(2)>,
                               std::optional<uintlen_t>>;
  template <std::floating_point T>
  using big_buff2_t =
      std::pair<std::array<char, sizeof(T) * 8>, std::optional<uintlen_t>>;
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN big_buff_t<T> from_integral(
      T val_rg_, bool upper_case, const uint8_t raidex) const noexcept {
    big_buff_t<T> ret{};
    ret.second = from_integral_fill(&ret.first[0], ret.first.size(), val_rg_,
                                    upper_case, raidex);
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_ND_FN big_buff2_t<T> from_float(
      T val, MJZ_MAYBE_UNUSED const uint8_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case,
      floating_format_e floating_format = floating_format_e::general,
      char point_ch = '.', bool add_prefix = true) const noexcept {
    big_buff2_t<T> ret{};
    ret.second = from_float_format_fill(&ret.first[0], ret.first.size(), val,
                                        f_accuracacy, upper_case,
                                        floating_format, point_ch, add_prefix);
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_FN std::optional<uintlen_t> from_float_fill_sientific(
      char* const f_buf, const uintlen_t f_len, T f_val,
      MJZ_MAYBE_UNUSED const uint8_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case, const uint8_t raidex,
      MJZ_MAYBE_UNUSED uint8_t exp_base, MJZ_MAYBE_UNUSED char point_ch = '.',
      MJZ_MAYBE_UNUSED char power_ch = '^') const noexcept {
    if (36 < raidex || !f_buf || !f_len || exp_base < 2 || raidex < 2 ||
        point_ch == '+' || point_ch == '-')
      return std::nullopt;
    char* buf{f_buf};
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

    auto [ceil_log, fractionic_val] = value.to_log_and_coeffient(exp_base);
    std::optional<std::pair<int64_t, mjz_float_t>> int_and_float =
        fractionic_val.to_integral_and_fraction();
    auto&& [Int_, Float_] = *int_and_float;
    auto len_diff = from_integral_fill(buf, len, Int_, upper_case, raidex);
    if (!len_diff) return std::nullopt;
    len -= *len_diff;
    buf += *len_diff;
    if (!len) return std::nullopt;
    auto limit_num = [&]() noexcept {
      int64_t max_log2 =
          log2_ceil_of_val_create(uint64_t(fractionic_val.m_coeffient)) +
          fractionic_val.m_exponent;
      int64_t max_log{max_log2 / log2_ceil_of_val_create(raidex)};
      if (accuracacy < uint64_t(-max_log)) {
        fractionic_val.m_coeffient = 0;
      }
    };
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
      auto&& [front_decimal, left_decimal] = *int_and_float;
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

  MJZ_CX_FN bool is_space(char c) const noexcept {
    return ' ' == c || '\0' == c || c == '\r' || c == '\n';
  }

  template <std::floating_point T>
  MJZ_CX_FN std::optional<uintlen_t> from_float_fill_fixed(
      char* const f_buf, const uintlen_t f_len, T f_val,
      MJZ_MAYBE_UNUSED const uintlen_t f_accuracacy,
      MJZ_MAYBE_UNUSED bool upper_case, const uint8_t raidex, uint8_t exp_base,
      char point_ch = '.') const noexcept {
    if (36 < raidex || !f_buf || !f_len || exp_base < 2 || raidex < 2 ||
        point_ch == '+' || point_ch == '-')
      return std::nullopt;
    char* buf{f_buf};
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
      auto&& [Int_val, Float_val] = *int_and_float_;
      Float_ = Float_val;
      Int_ = Int_val;
    } else {
      auto&& [Int_val, Float_val] = *int_and_float;
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
      int64_t max_log{max_log2 / log2_ceil_of_val_create(raidex)};
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
      auto&& [front_decimal, left_decimal] = *int_and_float;
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
  MJZ_CX_FN std::optional<uintlen_t> from_float_fill_hex(
      char* const f_buf, const uintlen_t f_len, T f_val, bool upper_case,
      char point_ch = '.', bool add_0x = true) const noexcept {
    if (!f_buf || !f_len || point_ch == '+' || point_ch == '-')
      return std::nullopt;
    char* buf{f_buf};
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

    if (len < 4) return nullopt;
    mjz_float_t value{*opt_};
    if (value.m_coeffient < 0) {
      buf[0] = '-';
      len -= 1;
      buf += 1;
      value.m_coeffient = -value.m_coeffient;
    }
    if (add_0x) {
      *buf++ = '0';
      *buf++ = upper_case ? 'X' : 'x';
      len -= 2;
    }
    len -= 1;
    buf += 1;
    auto len_diff =
        from_integral_fill(buf, len, value.m_coeffient, upper_case, 16);
    if (!len_diff) return std::nullopt;
    buf[-1] = std::exchange(buf[0], point_ch);
    len -= *len_diff;
    buf += *len_diff;
    value.m_exponent += uint64_t(*len_diff - 1) << 2;
    if (len < 2) return std::nullopt;
    *buf = upper_case ? 'P' : 'p';
    len -= 1;
    buf += 1;
    len_diff = from_integral_fill(buf, len, value.m_exponent, upper_case, 10);
    if (!len_diff) return std::nullopt;
    len -= *len_diff;
    buf += *len_diff;
    return f_len - len;
  }

  template <std::floating_point T>
  MJZ_CX_FN std::optional<uintlen_t> from_float_format_fill(
      char* const f_buf, const uintlen_t f_len, T f_val,
      const uintlen_t f_accuracacy = 6, bool upper_case = true,
      floating_format_e floating_format = floating_format_e::general,
      char point_ch = '.', bool add_prefix = true) const noexcept {
    [&]() noexcept {
      if (floating_format_e::general != floating_format) return;
      using mjz_float_t = big_float_t<version_v>;
      auto opt = mjz_float_t::template float_from<T>(f_val, false);
      if (!opt) return;
      mjz_float_t f = *opt;
      auto [log, _] = f.to_log_and_coeffient(10);
      log = log < 0 ? -log : log;
      if (f_accuracacy < uintlen_t(log)) return;
      floating_format = floating_format_e::fixed;
    }();
    switch (floating_format) {
      case floating_format_e::fixed:
        return from_float_fill_fixed(f_buf, f_len, f_val, f_accuracacy,
                                     upper_case, 10, 10, point_ch);
        break;
      case floating_format_e::hex:
        return from_float_fill_hex(f_buf, f_len, f_val, upper_case, point_ch,
                                   add_prefix);
        break;
      case floating_format_e::general:

        MJZ_FALLTHROUGH;
      case floating_format_e::scientific:
        return from_float_fill_sientific(
            f_buf, f_len, f_val,
            uint8_t(std::min(f_accuracacy,
                             sizeof(T) * 8 / log2_ceil_of_val_create(10u))),
            upper_case, 10, 10, point_ch, upper_case ? 'E' : 'e');
      default:
        return nullopt;
        break;
    }
  }

  template <std::floating_point T,
            callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
                decltype(defualt_is_point_fn),
            callable_c<std::optional<
                std::pair<uintlen_t /*pow-exp*/, uint8_t /*pow-raidex*/>>(
                const char&, const uint8_t&) noexcept>
                power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN std::optional<T> to_real_floating(
      const char* ptr, uintlen_t len,
      is_point_fn_t&& is_point_fn = is_point_fn_t{},
      power_fn_t&& power_fn = power_fn_t{}) const noexcept {
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
      return this->template to_real_floating_pv<T>(ptr, len, 10, is_point_fn,
                                                   power_fn);
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

    std::optional<T> ret = this->template to_real_floating_pv<T>(
        ptr, len, raidex_, is_point_fn, power_fn);
    if (ret) *ret = is_neg ? -*ret : *ret;
    return ret;
  }

  template <
      std::floating_point T,
      callable_c<bool(char, uint8_t) noexcept> is_point_fn_t =
          decltype(defualt_is_point_fn),
      callable_c<std::optional<uintlen_t>(const char&, const uint8_t&) noexcept>
          power_fn_t = decltype(defualt_power_fn)>
  MJZ_CX_ND_FN std::optional<T> to_floating(
      const char* ptr, uintlen_t len,
      is_point_fn_t&& is_point_fn = is_point_fn_t{},
      power_fn_t&& power_fn = power_fn_t{}) const noexcept {
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
      return this->template to_floating_pv<T>(ptr, len, 10, is_point_fn,
                                              power_fn);
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

    std::optional<T> ret = this->template to_real_floating_pv<T>(
        ptr, len, raidex_, is_point_fn, power_fn);
    if (ret) *ret = is_neg ? -*ret : *ret;
    return ret;
  }

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral(
      const char* ptr, uintlen_t len, uint8_t raidex_ = 0) const noexcept {
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
      return do_ret(this->template to_integral_pv<T>(ptr, len, raidex_));
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
      return do_ret(this->template to_integral_pv<T>(ptr, len, 10));
    }
    if (len) {
      ptr++;
      len--;
      if (!len) return do_ret(0);
    }
    raidex_ = 8;
    if (len && (*ptr == 'X' || *ptr == 'x')) {
      raidex_ = 16;
      ptr++;
      len--;
    }
    return do_ret(this->template to_integral_pv<T>(ptr, len, raidex_));
  }
};

}  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_traits_LIB_HPP_FILE_