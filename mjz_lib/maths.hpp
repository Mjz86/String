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

#include "traits.hpp"
#include "versions.hpp"
#ifndef MJZ_MATHS_LIB_HPP_FILE_
#define MJZ_MATHS_LIB_HPP_FILE_
MJZ_DISABLE_ALL_WANINGS_START_;
#include <cfloat>
#include <cmath>
MJZ_DISABLE_ALL_WANINGS_END_;
namespace mjz {


template <version_t version_v = version_t{}>
struct big_float_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<bit_range_t> get_sign_range() noexcept {
    std::optional<bit_range_t> b = bit_range_t::get_bit_range<T>(
        T(1), [&](uint64_t) noexcept { return true; },
        [](T f) noexcept { return f < 0; }, [](T f) noexcept { return f < 0; });
    if (!b) return b;
    b->len = 1;
    return b;
  }

  template <std::floating_point T>
  MJZ_CX_FN static std::optional<bit_range_t>
  get_coeffient_bit_range() noexcept {
    bool saw_beg{};
    bool saw_cof{};
    return bit_range_t::get_bit_range<T>(
        T(0), [&](uint64_t) noexcept { return true; },
        [&](T f) noexcept {
          if (f >= std::numeric_limits<T>().denorm_min() &&
              f < std::numeric_limits<T>().min()) {
            if (saw_beg) return false;
            saw_beg = true;
            return saw_beg;
          }
          return false;
        },
        [&](T f) noexcept {
          if (f >= std::numeric_limits<T>().denorm_min() &&
              f < std::numeric_limits<T>().min()) {
            return false;
          }
          if (!saw_beg || saw_cof || f <= T()) return false;
          saw_cof = true;
          return saw_cof;
        });
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<bit_range_t>
  get_exponent_bit_range() noexcept {
    bool saw_beg{};
    bool saw_exp{};
    return bit_range_t::get_bit_range<T>(
        T(0), [&](uint64_t) noexcept { return true; },
        [&](T f) noexcept {
          if ((f >= std::numeric_limits<T>().denorm_min() &&
               f < std::numeric_limits<T>().min()) ||
              f <= T(0) || saw_beg)
            return false;
          saw_beg = true;
          return saw_beg;
        },
        [&](T f) noexcept {
          if (saw_beg && !saw_exp &&
              (f <= T(0) || (f >= std::numeric_limits<T>().denorm_min() &&
                             f < std::numeric_limits<T>().min()))) {
            saw_exp = true;
            return saw_exp;
          }
          return false;
        });
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<std::pair<int64_t, bool>> get_exponent(
      MJZ_MAYBE_UNUSED T val) noexcept {
    constexpr std::optional<bit_range_t> exp_range =
        get_exponent_bit_range<T>();
    if constexpr (!exp_range) {
      return std::nullopt;
    } else {
      constexpr bit_range_t exp_range_v = *exp_range;
      constexpr int64_t offset{-int64_t(
          ~((uint64_t(-1) >> (exp_range_v.len - 1)) << (exp_range_v.len - 1)))};
      constexpr uint64_t bitcopy_len{
          std::min<uint64_t>(exp_range_v.len, uint64_t(63))};
      uint64_t ret{};
      std::array<char, sizeof(T)> a = get_XE_bitcast<T, false, false>(val);
      for (uint64_t ip{}, i{exp_range_v.i}; ip < bitcopy_len; ip++, i++) {
        ret |= uint64_t(!!(uint64_t(a[uintptr_t(i / 8)]) &
                           uint64_t(uint64_t(1) << (i % 8))))
               << (ip);
      }
      return std::pair<int64_t, bool>(int64_t(ret) + offset, ret == 0);
    }
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<bool> set_exponent_get_normaity(
      MJZ_MAYBE_UNUSED T &val, int64_t exp) noexcept {
    constexpr std::optional<bit_range_t> exp_range =
        get_exponent_bit_range<T>();
    if constexpr (!exp_range) {
      return std::nullopt;
    } else {
      constexpr bit_range_t exp_range_v = *exp_range;
      constexpr int64_t offset{-int64_t(
          ~((uint64_t(-1) >> (exp_range_v.len - 1)) << (exp_range_v.len - 1)))};
      exp -= offset;
      if (uint64_t(exp_range_v.len) < log2_ceil_of_val_create(uint64_t(exp))) {
        return std::nullopt;
      }

      constexpr uint64_t bitcopy_len{
          std::min<uint64_t>(exp_range_v.len, uint64_t(63))};
      std::array<char, sizeof(T)> a = get_XE_bitcast<T, false, false>(val);
      for (uint64_t ip{}, i{exp_range_v.i}; ip < bitcopy_len; ip++, i++) {
        uint8_t bit = uint8_t(uint8_t(1) << (i % 8));
        a[uintptr_t(i / 8)] =
            char((a[uintptr_t(i / 8)] & ~bit) |
                 ((uint64_t(exp) & (uint64_t(1) << ip)) ? bit : uint8_t()));
      }
      val = from_XE_bitcast<T, false, false>(a);

      return exp != 0;
    }
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<big_float_t> get_big_float_from(
      T val) noexcept {
    if constexpr (std::same_as<long double, T>) {
      return get_big_float_from_impl_<double>(double(val));
    } else {
      return get_big_float_from_impl_<T>(val);
    }
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<big_float_t> get_big_float_from_impl_(
      T val) noexcept {
    constexpr uint64_t num_exp_log{get_exponent_bit_range<T>()->len};
    constexpr auto coeffient_range{get_coeffient_bit_range<T>()};
    constexpr auto sign_bit{get_sign_range<T>()};
    if constexpr (62 < num_exp_log || !coeffient_range || !sign_bit) {
      return std::nullopt;
    }
    constexpr uint64_t bitcopy_len{
        std::min<uint64_t>(coeffient_range->len, uint64_t(62))};
    big_float_t ret{};
    bool normal_mantisa{true};
    if (auto exp = get_exponent<T>(val)) {
      ret.m_exponent = exp->first - int64_t(bitcopy_len);
      normal_mantisa = !exp->second;
    } else {
      return std::nullopt;
    }
    std::array<char, sizeof(T)> a = get_XE_bitcast<T, false, false>(val);
    uint64_t ip{};
    uint64_t i{coeffient_range->i + coeffient_range->len - bitcopy_len};
    for (; ip < bitcopy_len; ip++, i++) {
      uint8_t bit = uint8_t(uint8_t(1) << (i % uint64_t(8)));
      ret.m_coeffient |= int64_t(
          uint64_t(bool(a[uintptr_t(uint64_t(i) / uint64_t(8))] & bit)) << ip);
    }
    if (normal_mantisa) {
      ret.m_coeffient |= int64_t(uint64_t(1) << ip);
    }
    bool is_neg{!!(a[(size_t)(sign_bit->i / 8)] &
                   uint8_t(uint8_t(1) << (sign_bit->i % 8)))};
    if (is_neg) ret.m_coeffient = -ret.m_coeffient;
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_FN std::optional<T> to_float_() noexcept {
    constexpr auto exp_range{get_exponent_bit_range<T>()};
    constexpr auto coeffient_range{get_coeffient_bit_range<T>()};
    constexpr auto sign_bit{get_sign_range<T>()};
    if constexpr (!exp_range || uint64_t(62) < exp_range->len ||
                  !coeffient_range || !sign_bit) {
      return std::nullopt;
    }
    if (!m_coeffient) {
      return T();
    }
    bool is_neg{};
    if (m_coeffient < 0) {
      is_neg = true;
      m_coeffient = -m_coeffient;
    }
    normalize<std::min<uint64_t>(coeffient_range->len + 1, 62)>();
    constexpr int64_t min_exponent{
        -int64_t(~((uint64_t(-1) >> (uint64_t(exp_range->len) - 1))
                   << (uint64_t(exp_range->len) - 1)))};
    auto ceil_log_cof = log2_ceil_of_val_create(uint64_t(m_coeffient));
    auto floor_log_cof = log2_of_val_create(uint64_t(m_coeffient));
    int64_t exp = m_exponent + floor_log_cof;
    uint64_t sub_range_delta_exp = 0;
    if (exp < min_exponent) {
      if (exp + int64_t(coeffient_range->len) < min_exponent) {
        return std::nullopt;
      }
      sub_range_delta_exp = uint64_t(min_exponent - exp);
      if (63 < sub_range_delta_exp) return std::nullopt;
      exp += sub_range_delta_exp;
      m_coeffient >>= sub_range_delta_exp;
    }
    T ret_val{};
    if (!set_exponent_get_normaity(ret_val, exp)) return std::nullopt;
    std::array<char, sizeof(T)> a = get_XE_bitcast<T, false, false>(ret_val);
    uint64_t bitnum = get_end_bit_index(uint64_t(m_coeffient));
    if (!sub_range_delta_exp) {
      bitnum = std::max<uint64_t>(1, bitnum) - 1;
    }
    if (ceil_log_cof == floor_log_cof) {
      bitnum = 0;
    }
    for (uint64_t ip{}; ip < bitnum; ip++) {
      uint64_t i{coeffient_range->i + ip};
      uint8_t bit = uint8_t(uint8_t(1) << (i % 8));
      bool bit_on = !!(uint64_t(m_coeffient) & (uint64_t(1) << (ip)));
      char &c = a[uintptr_t(i / 8)];
      c &= ~bit;
      c |= char(bit_on ? bit : uint8_t());
    }
    if (is_neg) {
      a[uintptr_t(sign_bit->i / 8)] |= char(uint8_t(1) << (sign_bit->i % 8));
    }

    ret_val = from_XE_bitcast<T, false, false>(a);
    return ret_val;
  }
  template <int64_t cof_log_dest>
    requires(0 < cof_log_dest && cof_log_dest < 63)
  MJZ_CX_FN void normalize() noexcept {
    bool is_neg{};
    if (!m_coeffient) {
      m_exponent = 0;
    }
    if (m_coeffient < 0) {
      is_neg = true;
      m_coeffient = -m_coeffient;
    }
    uint64_t cof_log = log2_ceil_of_val_create(uint64_t(m_coeffient));

    int64_t cof_log_delta{cof_log_dest - int64_t(cof_log)};
    m_exponent -= cof_log_delta;
    uint64_t coeffient = uint64_t(m_coeffient);
    if (cof_log_delta < 0) {
      coeffient >>= -cof_log_delta;
    } else {
      coeffient <<= cof_log_delta;
    }
    m_coeffient = int64_t(coeffient);
    if (is_neg) {
      m_coeffient = -m_coeffient;
    }
  };
  MJZ_CX_FN static std::optional<big_float_t> add(big_float_t lhs,
                                                  big_float_t rhs) noexcept {
    if (!lhs.m_coeffient) return rhs;
    if (!rhs.m_coeffient) return lhs;
    auto f = [](big_float_t &hs) noexcept {
      bool is_neg{};
      if (hs.m_coeffient < 0) {
        is_neg = true;
        hs.m_coeffient = -hs.m_coeffient;
      }
      hs.normalize<61>();
      return std::tuple(is_neg, uint64_t(hs.m_coeffient), hs.m_exponent);
    };
    //https://stackoverflow.com/questions/46114214/lambda-implicit-capture-fails-with-variable-declared-from-structured-binding
    auto sb1_ = f(rhs);
    auto sb2_ = f(lhs);
    auto fn = [&]() noexcept {
      auto &&[r_ng, r_ce, r_xp] = sb1_;
      auto &&[l_ng, l_ce, l_xp] = sb2_; 
      int64_t delta = l_xp - r_xp;
      if (delta < 64) {
        r_ce >>= delta;
      } else {
        r_ce = 0;
      }
      l_ce = l_ng ? uint64_t(-int64_t(l_ce)) : l_ce;
      l_ce += r_ng ? uint64_t(-int64_t(r_ce)) : r_ce;
      big_float_t ret{};
      ret.m_coeffient = int64_t(l_ce);
      ret.m_exponent = int64_t(l_xp);
      return ret;
    };
    auto &&[r_ng, r_ce, r_xp] = sb1_;
    auto &&[l_ng, l_ce, l_xp] = sb2_; 
    if (r_xp < l_xp) {
      return fn();
    }
    std::swap(r_ng, l_ng);
    std::swap(r_xp, l_xp);
    std::swap(r_ce, l_ce);
    return fn();
  }

  MJZ_CX_FN static std::optional<big_float_t> muliply(
      big_float_t lhs, big_float_t rhs) noexcept {
    if (!lhs.m_coeffient || !rhs.m_coeffient) return big_float_t{};

    bool is_neg{(rhs.m_coeffient < 0) != (lhs.m_coeffient < 0)};
    rhs.m_coeffient = rhs.m_coeffient < 0 ? -rhs.m_coeffient : rhs.m_coeffient;
    lhs.m_coeffient = lhs.m_coeffient < 0 ? -lhs.m_coeffient : lhs.m_coeffient;
    uint64_t mask{uint32_t(-1)};
    uint64_t rtwo_parts[2]{uint64_t(rhs.m_coeffient) & mask,
                           uint64_t(rhs.m_coeffient) >> 32};
    uint64_t ltwo_parts[2]{uint64_t(lhs.m_coeffient) & mask,
                           uint64_t(lhs.m_coeffient) >> 32};
    uint64_t three_parts_restult[3]{
        ltwo_parts[0] * rtwo_parts[0],
        (ltwo_parts[0] * rtwo_parts[1]) + (ltwo_parts[1] * rtwo_parts[0]),
        ltwo_parts[1] * rtwo_parts[1]};
    three_parts_restult[1] += three_parts_restult[0] >> 32;
    three_parts_restult[2] += three_parts_restult[1] >> 32;
    three_parts_restult[0] &= mask;
    three_parts_restult[1] &= mask;
    uint64_t uint128_v[2]{
        three_parts_restult[0] + (three_parts_restult[1] << 32),
        three_parts_restult[2],
    };

    uint64_t sign_bit = ~(uint64_t(-1) >> 1);
    if (!uint128_v[1]) {
      while (sign_bit & uint128_v[0]) {
        uint64_t had{uint128_v[0] & 1};
        uint128_v[0] >>= 1;
        rhs.m_exponent++;
        uint128_v[0] += had;
      }
      rhs.m_coeffient = is_neg ? -int64_t(uint128_v[0]) : int64_t(uint128_v[0]);
      rhs.m_exponent += lhs.m_exponent;
      return rhs;
    }
    auto log_delta = log2_ceil_of_val_create(uint128_v[1]) + int64_t(1);
    uint64_t low_mask = uint64_t(uint64_t(-1) >> (64 - log_delta));
    uint64_t low_bits = uint128_v[0] & low_mask;

    uint128_v[0] >>= log_delta;
    uint128_v[1] <<= (64 - log_delta);
    uint128_v[0] |= uint128_v[1];
    if (low_bits & ((low_mask + 1) >> 1)) {
      uint128_v[0]++;
    }
    while (sign_bit & uint128_v[0]) {
      uint64_t had{uint128_v[0] & 1};
      uint128_v[0] >>= 1;
      rhs.m_exponent++;
      uint128_v[0] += had;
    }
    rhs.m_exponent += log_delta;
    rhs.m_coeffient = is_neg ? -int64_t(uint128_v[0]) : int64_t(uint128_v[0]);
    rhs.m_exponent += lhs.m_exponent;
    return rhs;
  }
  MJZ_CX_FN static std::optional<big_float_t> devide(big_float_t lhs,
                                                     big_float_t rhs) noexcept {
    if (!rhs.m_coeffient) return nullopt;

    bool is_neg{(rhs.m_coeffient < 0) != (lhs.m_coeffient < 0)};
    rhs.m_coeffient = rhs.m_coeffient < 0 ? -rhs.m_coeffient : rhs.m_coeffient;
    lhs.m_coeffient = lhs.m_coeffient < 0 ? -lhs.m_coeffient : lhs.m_coeffient;
    lhs.normalize<62>();
    rhs.normalize<32>();

    int64_t round((rhs.m_coeffient >> 1) <=
                  (lhs.m_coeffient % rhs.m_coeffient));
    lhs.m_coeffient /= rhs.m_coeffient;
    lhs.m_coeffient += round;
    lhs.m_coeffient = is_neg ? -lhs.m_coeffient : lhs.m_coeffient;
    lhs.m_exponent -= rhs.m_exponent;
    return lhs;
  }

 public:
  int64_t m_coeffient;
  int64_t m_exponent;

 public:
  template <std::floating_point T>
  MJZ_CX_FN std::optional<T> to_float(bool allow_nan = true) const noexcept {
    constexpr auto maxed =
        *get_big_float_from<T>(std::numeric_limits<T>().max());
    if (!allow_nan && maxed < *this) return std::nullopt;
    return big_float_t(*this).to_float_<T>();
  }
  template <std::floating_point T>
  MJZ_CX_FN static std::optional<big_float_t> float_from(
      T val, bool allow_nan = true) noexcept {
    constexpr auto maxed =
        *get_big_float_from<T>(std::numeric_limits<T>().max());
    auto ret = get_big_float_from<T>(val);
    if (!allow_nan && maxed < ret) return std::nullopt;
    return ret;
  }
  template <std::integral T>
    requires(sizeof(T) <= sizeof(int64_t))
  MJZ_CX_FN static big_float_t float_from_i(T val) noexcept {
    big_float_t ret{};
    if ((uint64_t(val) & ~(uint64_t(-1) >> 1)) && std::unsigned_integral<T>) {
      ret.m_exponent++;
      val >>= 1;
    }
    ret.m_coeffient = int64_t(val);
    return ret;
  }
  MJZ_CX_FN friend std::optional<big_float_t> operator+(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? add(*rhs, *lhs) : std::nullopt;
  }
  MJZ_CX_FN big_float_t operator-() const noexcept {
    big_float_t ret{*this};
    ret.m_coeffient = -m_coeffient;
    return ret;
  }
  MJZ_CX_FN friend std::optional<big_float_t> operator*(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? muliply(*rhs, *lhs) : std::nullopt;
  }

  MJZ_CX_FN friend std::optional<big_float_t> operator-(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? add(*rhs, -*lhs) : std::nullopt;
  }

  MJZ_CX_FN friend std::optional<big_float_t> operator/(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? devide(*rhs, *lhs) : std::nullopt;
  }
  MJZ_CX_FN friend big_float_t operator*(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return *muliply(rhs, lhs);
  }

  MJZ_CX_FN friend big_float_t operator-(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return *add(rhs, -lhs);
  }

  MJZ_CX_FN friend big_float_t operator/(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return *devide(rhs, lhs);
  }
  MJZ_CX_FN friend big_float_t operator+(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return *add(rhs, lhs);
  }

  MJZ_CX_FN friend std::partial_ordering operator<=>(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? (*rhs <=> *lhs)
                            : std::partial_ordering::unordered;
  }

  MJZ_CX_FN std::strong_ordering operator<=>(
      const big_float_t &lhs) const noexcept {
    auto r = add(*this, -lhs);
    return r->m_coeffient <=> int64_t(0);
  }
  MJZ_CX_FN friend std::optional<bool> operator==(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? (*rhs == *lhs) : nullopt;
  }

  MJZ_CX_FN bool operator==(const big_float_t &lhs) const noexcept {
    auto r = add(*this, -lhs);
    return r->m_coeffient == int64_t(0);
  }
  template <int64_t cof_log_dest>
    requires(0 < cof_log_dest && cof_log_dest < 62)
  MJZ_CX_FN big_float_t make_normal() const noexcept {
    big_float_t ret{*this};
    ret.normalize<cof_log_dest>();
    return ret;
  }

  MJZ_CX_FN std::optional<std::pair<int64_t, big_float_t>>
  to_integral_and_fraction() const noexcept {
    big_float_t ret{*this};
    bool is_negative{};
    if (ret.m_coeffient < 0) {
      is_negative = true;
      ret.m_coeffient = -ret.m_coeffient;
    }
    int64_t &exponent = ret.m_exponent;
    constexpr uint64_t sign_bit = ~(uint64_t(-1) >> 1);
    uint64_t integral_coeffient{uint64_t(ret.m_coeffient)};
    while (!(integral_coeffient & sign_bit) && integral_coeffient && exponent) {
      if (0 < exponent) {
        integral_coeffient <<= 1;
        exponent--;
      } else {
        integral_coeffient >>= 1;
        exponent++;
      }
    }
    if (integral_coeffient & sign_bit) {
      return std::nullopt;
    }
    ret.m_coeffient = is_negative ? -int64_t(integral_coeffient)
                                  : int64_t(integral_coeffient);
    return std::pair<int64_t, big_float_t>(ret.m_coeffient, *this - ret);
  }

  MJZ_CX_FN std::pair<int64_t, big_float_t> to_log_and_coeffient(
      uint64_t exp_base) const noexcept {
    auto me{*this};
    if (!me.m_coeffient) return {};
    bool is_neg{me.m_coeffient < 0};
    me.m_coeffient = is_neg ? -me.m_coeffient : me.m_coeffient;
    int64_t ceil_log{};
    big_float_t fractionic_val{};
    int64_t ceil_log2 = log2_ceil_of_val_create(uint64_t(me.m_coeffient)) +
                        int64_t(me.m_exponent);
    ceil_log = ceil_log2 / log2_of_val_create(exp_base);
    uint64_t pow = uint64_t(ceil_log < 0 ? -ceil_log : ceil_log);

    big_float_t one = big_float_t::float_from_i(1);

    big_float_t expo = big_float_t::float_from_i(exp_base);
    big_float_t coeffient = one;
    big_float_t r{expo};
    for (; pow;) {
      if (pow & 1) {
        coeffient = coeffient * r;
      }
      r = r * r;
      pow >>= 1;
    }
    if (ceil_log < 0) {
      fractionic_val = coeffient * me;
    } else {
      fractionic_val = me / coeffient;
    }
    while (expo < fractionic_val) {
      fractionic_val = fractionic_val / expo;
      ceil_log++;
    }
    while (fractionic_val < one) {
      fractionic_val = fractionic_val * expo;
      ceil_log--;
    }
    if (is_neg) {
      fractionic_val = -fractionic_val;
    }
    return {ceil_log, fractionic_val};
  }

  MJZ_CX_FN std::pair<big_float_t, big_float_t> to_big_and_fraction()
      const noexcept {
    std::optional<std::pair<int64_t, big_float_t>> small =
        to_integral_and_fraction();
    if (small)
      return std::pair<big_float_t, big_float_t>{float_from_i(small->first),
                                                 small->second};
    return std::pair<big_float_t, big_float_t>{*this, big_float_t{}};
  }
  MJZ_CX_FN big_float_t() noexcept = default;
};

namespace float_litteral_ns {
MJZ_CX_FN std::optional<big_float_t<>> operator""_bf(long double val) noexcept {
  return big_float_t<>::float_from((double)val);
}

};  // namespace float_litteral_ns

template <bool B = 0>
struct power2_helper_t {
  using maxfloat_t = double;

  MJZ_CONSTANT(maxfloat_t) two { 2 };
  MJZ_CONSTANT(maxfloat_t) one { 1 };
  MJZ_CONSTANT(maxfloat_t) zero { 0 };
  MJZ_CONSTANT(maxfloat_t) eps { DBL_EPSILON };
  MJZ_CX_FN static maxfloat_t small_exp(maxfloat_t x,
                                        uint32_t recursion_index) noexcept {
    uint64_t i_factorial{1};
    maxfloat_t retval{0};
    maxfloat_t x_to_i_val{1};
    for (uint64_t i{1}; i < recursion_index; i++) {
      retval += x_to_i_val / maxfloat_t(i_factorial);
      x_to_i_val *= x;
      i_factorial *= i;
    }
    return retval;
  };
  MJZ_CX_FN static maxfloat_t small_pow2(maxfloat_t x, uint32_t) noexcept {
    return x + 0.942695040889;
  };
  MJZ_CX_FN static maxfloat_t small_log2(maxfloat_t x, uint32_t) noexcept {
    return x - 0.942695040889;
  };

  MJZ_CX_FN static maxfloat_t get_log2(
      maxfloat_t input,
      uint32_t recursion_index = sizeof(maxfloat_t)) noexcept {
    uint64_t integer_log{};
    constexpr uint64_t sz{6};
    if (input <= zero) {
      return -DBL_MAX;
    }

    if (input < one) {
      return -get_log2(one / input, recursion_index);
    }
    while (two < input) {
      input /= 2;
      integer_log++;
    }
    if (recursion_index == 0) {
      maxfloat_t log2_aproximation_betwen_1_and_2{small_log2(input, 0)};
      return maxfloat_t(integer_log) + log2_aproximation_betwen_1_and_2 + eps;
    }
    for (uint64_t i{}; i < sz; i++) {
      input *= input;
    }

    return maxfloat_t(integer_log) +
           (get_log2(input, recursion_index - 1) / maxfloat_t(1ULL << sz));
  }

  MJZ_CONSTANT(maxfloat_t) e { 2.7182818284590452353602874713527 };
  MJZ_CONSTANT(maxfloat_t) log_e_2 { 0.69314718055994530941723212145818 };
  MJZ_CONSTANT(maxfloat_t) log_2_e { one / log_e_2 };
  MJZ_CX_FN static maxfloat_t get_pow2(
      maxfloat_t input,
      uint32_t recursion_index = sizeof(maxfloat_t)) noexcept {
    if (input < maxfloat_t{0}) {
      return one / get_pow2(-input, recursion_index);
    }
    constexpr uint32_t sz{9};
    uint32_t int_log2_of_ret{uint32_t(input)};
    input -= maxfloat_t(int_log2_of_ret & ((1ULL << sz) - 1));
    maxfloat_t ret{1};
    maxfloat_t i_power2{2};
    for (uint32_t i{}; i < sz; i++) {
      if ((1ULL << i) & int_log2_of_ret) {
        ret *= i_power2;
      }
      i_power2 *= i_power2;
    }
    if (recursion_index == 0) {
      ret *= small_pow2(input, 0);
    } else {
      ret *= small_exp(input * log_e_2, recursion_index + 4);
    }
    return ret;
  }
};

MJZ_CX_FN auto log_base_e(power2_helper_t<>::maxfloat_t x) noexcept {
  MJZ_IFN_CONSTEVAL { return std::log(x); }
  using T = power2_helper_t<>::maxfloat_t;
  T ret{};
  {
    MJZ_RELEASE { ret /= power2_helper_t<>::log_2_e; };
    big_float_t<> f = *big_float_t<>::float_from(x);
    ret += T(f.m_exponent);
    f.m_exponent = 0;
    f = f.make_normal<61>();
    ret += T(f.m_exponent + 61);
    f.m_exponent = -61;
    ret += power2_helper_t<>::get_log2(*f.to_float<T>());
  }
  return ret;
}

MJZ_CX_FN auto pow_base_e(power2_helper_t<>::maxfloat_t x) noexcept {
  MJZ_IFN_CONSTEVAL { return std::exp(x); }
  return power2_helper_t<>::get_pow2(x * power2_helper_t<>::log_2_e, 63);
}
MJZ_CX_FN auto mjz_sqrt(std::floating_point auto x) noexcept -> decltype(x) {
  MJZ_IFN_CONSTEVAL { return decltype(x)(std::sqrt((double)(x))); }
  return decltype(x)(pow_base_e(log_base_e((double)(x)) / 2));
}
MJZ_CONSTANT(double)
PI_num_{3.1415926535897932384626433832795};
MJZ_CX_FN auto mjz_sin(std::floating_point auto x) noexcept -> decltype(x) {
  MJZ_IFN_CONSTEVAL { return decltype(x)(std::sin((double)(x))); }
  if (x < 0) {
    return -mjz_sin(-x);
  }
  if (decltype(x)(PI_num_) < x) {
    return mjz_sin(x - decltype(x)(PI_num_));
  }
  if (decltype(x)(PI_num_ / 2) < x) {
    return mjz_cos(decltype(x)(PI_num_ / 2) - x);
  }
  decltype(x) factorial{1}, num{x}, ret{};
  bool positive{true};
  for (uint64_t i{1}; i < 10; i++) {
    factorial /= decltype(x)(i);
    num *= x;
    if (!(i % 2)) {
      positive = !positive;
      continue;
    }
    decltype(x) add = num * factorial;
    ret += positive ? add : -add;
  }
  return ret;
}

MJZ_CX_FN auto mjz_cos(std::floating_point auto x) noexcept -> decltype(x) {
  MJZ_IFN_CONSTEVAL { return decltype(x)(std::cos((double)(x))); }
  if (x < 0) {
    return mjz_cos(-x);
  }
  if (decltype(x)(PI_num_) < x) {
    return mjz_cos(x - decltype(x)(PI_num_));
  }
  if (decltype(x)(PI_num_ / 2) < x) {
    return mjz_sin(decltype(x)(PI_num_ / 2) - x);
  }
  decltype(x) factorial{1}, num{x}, ret{1};
  bool positive{true};
  for (uint64_t i{1}; i < 10; i++) {
    factorial /= decltype(x)(i);
    num *= x;
    if (i % 2) {
      positive = !positive;
      continue;
    }
    decltype(x) add = num * factorial;
    ret += positive ? add : -add;
  }
  return ret;
}
template <class f_t>
MJZ_CX_FN auto mjz_make_number(std::floating_point auto);
template <std::floating_point f_t>
MJZ_CX_FN auto mjz_make_number(std::floating_point auto x) noexcept {
  return f_t(x);
}

}  // namespace mjz
#endif  // MJZ_MATHS_LIB_HPP_FILE_
