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
#include <bit>
namespace mjz {

template <version_t version_v, uintlen_t n_bits>
  requires(64 * (n_bits / 64) == n_bits && !!n_bits)
struct uintN_t {
  MJZ_CONSTANT(uintlen_t) word_count = n_bits / 64;
  alignas(std::min(hardware_constructive_interference_size,
                   log2_of_val_to_val(log2_of_val_create(word_count * 8))))
      uint64_t words[word_count]{};
  MJZ_DISABLE_ALL_WANINGS_START_;
  MJZ_DEFAULTED_CLASS(uintN_t);
  MJZ_DISABLE_ALL_WANINGS_END_;

 private:
  MJZ_CX_FN uintN_t(void_struct_t, uintlen_t) noexcept : uintN_t{} {}
  template <std::integral T, std::integral... Ts>
  MJZ_CX_FN uintN_t(void_struct_t, const uintlen_t i, T lowest,
                    Ts... args) noexcept
      : uintN_t{void_struct_t{}, i + 1, args...} {
    nth_word(i) = lowest;
  }

 public:
  template <std::integral... Ts>
    requires(sizeof...(Ts) <= word_count)
  MJZ_CX_FN explicit uintN_t(/*low to high*/ Ts... args) noexcept
      : uintN_t{void_struct_t{}, 0, uint64_t(args)...} {}

  MJZ_CX_FN uint64_t& nth_word(auto i) noexcept {
    if constexpr (version_v.is_LE()) {
      return words[size_t(i)];
    } else {
      return words[size_t((word_count - 1) - i)];
    }
  }
  MJZ_CX_FN const uint64_t& nth_word(auto i) const noexcept {
    if constexpr (version_v.is_LE()) {
      return words[size_t(i)];
    } else {
      return words[size_t((word_count - 1) - i)];
    }
  }
  MJZ_CX_FN bool nth_bit(auto i) const noexcept {
    return !!(nth_word(i >> 6) & (uint64_t(1) << (i & 63)));
  }
  MJZ_CX_FN void set_nth_bit(auto i, bool val) noexcept {
    uint64_t mask = uint64_t(1) << (i & 63);
    uint64_t& word = nth_word(i >> 6);
    word &= ~mask;
    word |= val ? mask : 0;
  }
  MJZ_CX_FN uintN_t& operator>>=(uintlen_t amount) noexcept {
    intlen_t abs_amount = intlen_t(amount);
    intlen_t internal_amount{abs_amount & 63};
    intlen_t external_amount{abs_amount >> 6};

    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      uint64_t temps[2]{nth_word(i), nth_word(i)};
      nth_word(i) = 0;
      temps[1] >>= internal_amount;
      temps[0] ^= temps[1] << internal_amount;
      temps[0] <<= (64 - internal_amount) & 63;
      intlen_t j = i - external_amount;
      {
        bool branch = 0 <= j;
        intlen_t index_ = branchless_teranary<intlen_t>(branch, j, 0);
        nth_word(index_) |= branchless_teranary<uint64_t>(branch, temps[1], 0);
      }
      {
        bool branch = 0 < j;
        intlen_t index_ = branchless_teranary<intlen_t>(branch, j - 1, 0);
        nth_word(index_) |= branchless_teranary<uint64_t>(branch, temps[0], 0);
      }
    }

    return *this;
  }
  MJZ_CX_FN uintN_t& operator<<=(uintlen_t amount) noexcept {
    intlen_t abs_amount = intlen_t(amount);
    intlen_t internal_amount{abs_amount & 63};
    intlen_t external_amount{abs_amount >> 6};
    for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
      uint64_t temps[2]{nth_word(i), nth_word(i)};
      nth_word(i) = 0;
      temps[0] <<= internal_amount;
      temps[1] ^= temps[0] >> internal_amount;
      temps[1] >>= (64 - internal_amount) & 63;
      intlen_t j = i + external_amount;
      {
        bool branch = j + 1 < intlen_t(word_count);
        intlen_t index_ = branchless_teranary<intlen_t>(branch, j + 1, 0);
        nth_word(index_) |= branchless_teranary<uint64_t>(branch, temps[1], 0);
      }
      {
        bool branch = j < intlen_t(word_count);
        intlen_t index_ = branchless_teranary<intlen_t>(branch, j, 0);
        nth_word(index_) |= branchless_teranary<uint64_t>(branch, temps[0], 0);
      }
    }
    return *this;
  }
  MJZ_CX_FN uintN_t& operator&=(const uintN_t& amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) &= amount.nth_word(i);
    }
    return *this;
  }
  MJZ_CX_FN uintN_t& operator|=(const uintN_t& amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) &= amount.nth_word(i);
    }
    return *this;
  }
  MJZ_CX_FN uintN_t& operator^=(const uintN_t& amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) &= amount.nth_word(i);
    }
    return *this;
  }

  MJZ_CX_FN uintN_t& flip() noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) = ~nth_word(i);
    }
    return *this;
  }

  MJZ_CX_FN bool add(const uintN_t& amount, bool carry = false) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      uint64_t rhs = nth_word(i);
      uint64_t lhs = amount.nth_word(i);
      uintlen_t min_rhs = rhs;
      rhs += carry;
      bool overflow = rhs < min_rhs;
      min_rhs = rhs;
      rhs += lhs;
      overflow |= rhs < min_rhs;
      carry = overflow;
      nth_word(i) = rhs;
    }
    return carry;
  }
  MJZ_CX_FN uintN_t& negate() noexcept {
    // twos compliment (~*this + 1)
    bool carry = true;
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      uint64_t rhs = ~nth_word(i);
      bool perv_carry = carry;
      carry &= rhs == uintlen_t(-1);
      rhs += perv_carry;
      nth_word(i) = rhs;
    }
    return *this;
  }
  MJZ_CX_FN bool minus(const uintN_t& amount, bool carry = false) noexcept {
    bool negate_carry = true;
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      uint64_t rhs = nth_word(i);
      uint64_t lhs = ~amount.nth_word(i);
      bool negate_perv_carry = negate_carry;
      negate_carry &= lhs == uintlen_t(-1);
      lhs += negate_perv_carry;

      uintlen_t min_rhs = rhs;
      rhs += carry;
      bool overflow = rhs < min_rhs;
      min_rhs = rhs;
      rhs += lhs;
      overflow |= rhs < min_rhs;
      carry = overflow;
      nth_word(i) = rhs;
    }
    return carry;
  }
  MJZ_CX_FN uintN_t& operator-=(const uintN_t& amount) noexcept {
    minus(amount);
    return *this;
  }
  MJZ_CX_FN uintN_t& operator+=(const uintN_t& amount) noexcept {
    add(amount);
    return *this;
  }
  MJZ_CX_FN uintN_t& operator*=(const uintN_t& amount) noexcept {
    if constexpr (word_count == 2) {
      // https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/uint128_t.h
      const uint64_t x = nth_word(0);
      const uint64_t y = amount.nth_word(0);
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
      nth_word(1) = h + x * amount.nth_word(1) + nth_word(1) * y;
      nth_word(0) = l;
      return *this;
    }
    uintN_t ret{};
    for (uintlen_t i{}; i < n_bits; i++) {
      if (nth_bit(i)) {
        ret += amount << i;
      }
    }
    return *this = ret;
  }
  MJZ_CX_FN std::strong_ordering operator<=>(
      const uintN_t& rhs) const noexcept {
    for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
      std::strong_ordering order = nth_word(i) <=> rhs.nth_word(i);
      if (order != std::strong_ordering::equal) {
        return order;
      }
    }
    return std::strong_ordering::equal;
  }

  MJZ_CX_FN bool operator==(const uintN_t& rhs) const noexcept = default;

  MJZ_CX_FN std::optional<uintlen_t> floor_log2() const noexcept {
    for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
      uint64_t word = nth_word(i);
      if (!word) continue;
      return uintlen_t(log2_of_val_create(word)) + i * 64;
    }
    return {};
  }
  MJZ_CX_FN std::optional<uintlen_t> ceil_log2() const noexcept {
    intlen_t i{intlen_t(word_count) - 1};
    for (; 0 <= i; i--) {
      uint64_t word = nth_word(i);
      if (!word) continue;
      uintlen_t awnser = uintlen_t(log2_of_val_create(word));
      bool ceil_ = (uintlen_t(1) << awnser) != word;
      awnser += i * 64;
      i--;
      for (; 0 <= i; i--) {
        ceil_ |= !!nth_word(i);
      }
      awnser += ceil_;
      return awnser;
    }
    return {};
  }

  MJZ_CX_FN uintN_t to_modulo_ret_devide(const uintN_t& rhs) noexcept {
    uintN_t intermidiate{};
    std::optional<uintlen_t> clog2_lhs_ = ceil_log2();
    if (!clog2_lhs_) {
      return intermidiate;
    }
    intlen_t clog2_lhs = intlen_t(*clog2_lhs_);
    intlen_t flog2_rhs = intlen_t(*rhs.floor_log2());
    intlen_t clog2_ret =
        std::min<intlen_t>(intlen_t(n_bits - 1), clog2_lhs) - flog2_rhs;
    if (clog2_ret < 0) {
      return intermidiate;
    }
    uintN_t temp = rhs << uintlen_t(clog2_ret);
    for (intlen_t i{clog2_ret}; 0 <= i; i--) {
      bool is_bigger{temp <= *this};
      intermidiate.set_nth_bit(i, is_bigger);
      if (is_bigger) {
        *this -= temp;
      }
      temp >>= 1;
    }
    return intermidiate;
  }

  MJZ_CX_FN uintN_t& operator%=(const uintN_t& rhs) noexcept {
    to_modulo_ret_devide(rhs);
    return *this;
  }
  MJZ_CX_FN uintN_t& operator/=(const uintN_t& rhs) noexcept {
    return *this = to_modulo_ret_devide(rhs);
  }
  MJZ_CX_FN uintN_t& operator_assign_devide_up(const uintN_t& rhs) noexcept {
    uintN_t temp = to_modulo_ret_devide(rhs);
    temp.add(uintN_t(), *this != uintN_t());
    return *this = temp;
  }
  MJZ_CX_FN friend uintN_t operator_devide_up(uintN_t x,
                                              const uintN_t& y) noexcept {
    x.operator_assign_devide_up(y);
    return x;
  }
  MJZ_CX_FN explicit operator bool() const noexcept { return *this != 0; }

  template <std::integral T>
  MJZ_CX_FN explicit operator T() const noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t));
    return static_cast<T>(nth_word(0));
  }

  MJZ_CX_FN friend uintN_t operator&(uintN_t x, const uintN_t& y) noexcept {
    x &= y;
    return x;
  }
  MJZ_CX_FN friend uintN_t operator^(uintN_t x, const uintN_t& y) noexcept {
    x ^= y;
    return x;
  }

  MJZ_CX_FN friend uintN_t operator|(uintN_t x, const uintN_t& y) noexcept {
    x |= y;
    return x;
  }

  MJZ_CX_FN friend uintN_t operator<<(uintN_t x, uintlen_t y) noexcept {
    x <<= y;
    return x;
  }

  MJZ_CX_FN friend uintN_t operator>>(uintN_t x, uintlen_t y) noexcept {
    x >>= y;
    return x;
  }

  MJZ_CX_FN uintN_t operator~() const noexcept {
    uintN_t x = *this;
    x.flip();
    return x;
  }

  MJZ_CX_FN uintN_t operator-() const noexcept {
    uintN_t x = *this;
    x.negate();
    return x;
  }

  MJZ_CX_FN friend uintN_t operator+(uintN_t x, const uintN_t& y) noexcept {
    x += y;
    return x;
  }

  MJZ_CX_FN friend uintN_t operator-(uintN_t x, const uintN_t& y) noexcept {
    x -= y;
    return x;
  }
  MJZ_CX_FN friend uintN_t operator*(uintN_t x, const uintN_t& y) noexcept {
    x *= y;
    return x;
  }
  MJZ_CX_FN friend uintN_t operator/(uintN_t x, const uintN_t& y) noexcept {
    x /= y;
    return x;
  }

  MJZ_CX_FN friend uintN_t operator%(uintN_t x, const uintN_t& y) noexcept {
    x %= y;
    return x;
  }

  MJZ_CX_FN uintN_t& operator--() noexcept { return *this -= 1; }

  MJZ_CX_FN uintN_t& operator++() noexcept { return *this += 1; }

  MJZ_CX_FN uintN_t operator++(int) noexcept {
    uintN_t temp{*this};
    *this -= 1;
    return temp;
  }

  MJZ_CX_FN uintN_t operator--(int) noexcept {
    uintN_t temp{*this};
    *this += 1;
    return temp;
  }
};

template <version_t version_v>
struct devide_fast_t {
  uint64_t devide_val{};
  uint64_t inverse{};
  MJZ_DEFAULTED_CLASS(devide_fast_t);
  using uint128_t_mjz_ = uintN_t<version_v, 128>;
  MJZ_CX_FN static uint64_t devide_with_inverse_impl_(
      const uint64_t amount, const uint64_t inverse_) noexcept {
    // https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/uint128_t.h
    const uint64_t x = amount;
    const uint64_t y = inverse_;
    const uint64_t xl = x & 0xffffffff;
    const uint64_t xh = x >> 32;
    const uint64_t yl = y & 0xffffffff;
    const uint64_t yh = y >> 32;
    const uint64_t ll = xl * yl;
    const uint64_t lh = xl * yh;
    const uint64_t hl = xh * yl;
    const uint64_t hh = xh * yh;
    const uint64_t m = (ll >> 32) + lh + (hl & 0xffffffff);
    const uint64_t h = (m >> 32) + (hl >> 32) + hh;
    return h;
  }

  MJZ_CX_FN devide_fast_t(void_struct_t, uint64_t devidition_mount) noexcept
      : devide_val(devidition_mount), inverse([devidition_mount]() noexcept {
          asserts(asserts.assume_rn, 2 <= devidition_mount);
          return uint64_t(operator_devide_up(uint128_t_mjz_{0, 1},
                                             uint128_t_mjz_(devidition_mount)));
        }()) {}
  MJZ_CE_FN devide_fast_t(uint64_t devidition_mount) noexcept
      : devide_fast_t(void_struct_t{}, devidition_mount) {}
  MJZ_CX_FN friend uint64_t operator/(const uint64_t amount,
                                      const devide_fast_t rhs) noexcept {
    return devide_with_inverse_impl_(amount, rhs.inverse);
  }
  MJZ_CX_FN std::pair<uint64_t, uint64_t> divide_modulo(
      const uint64_t amount) const noexcept {
    const uint64_t value = devide_with_inverse_impl_(amount, inverse);
    const uint64_t modulo = amount - value * devide_val;
    return {value, modulo};
  }
};

template <version_t version_v, uint64_t devidition_mount>
  requires(!!devidition_mount)
MJZ_CX_FN uint64_t devide_with_inverse(const uint64_t amount) noexcept {
  if constexpr (devidition_mount == 1) {
    return amount;
  } else {
    constexpr auto rhs = devide_fast_t<version_v>(devidition_mount);
    return amount / rhs;
  }
}
template <version_t version_v, bool shifts_before = true,
          uintlen_t cached_count = 16>
alignas(hardware_constructive_interference_size) static inline constexpr const
    std::array<uint64_t, cached_count> math_helper_t_cached_vals_ =
        []() noexcept {
          std::array<uint64_t, cached_count> ret{};
          for (uint64_t i{}; i < cached_count; i++) {
            uint64_t rhs{};
            if constexpr (shifts_before) {
              rhs = (i + 1) * 2 + 1;
            } else {
              rhs = i + 2;
            }
            ret[i] = devide_fast_t<version_v>{void_struct_t{}, rhs}.inverse;
          }
          return ret;
        }();

template <version_t version_v, bool shifts_before = true,
          uintlen_t cached_count = 16>
struct math_helper_t_ {
  MJZ_CX_FN static std::pair<uint64_t, uint64_t> divide_modulo(
      const uint64_t lhs, const uint64_t rhs) noexcept {
    asserts(asserts.assume_rn, rhs != 0);
    constexpr auto&& cached_inverses =
        math_helper_t_cached_vals_<version_v, shifts_before, cached_count>;
    if constexpr (!shifts_before) {
      if (rhs == 1) {
        return {lhs, 0};
      }
      const uint64_t index_cache = rhs - 2;
      if (cached_count <= index_cache) {
        return {lhs / rhs, lhs % rhs};
      }
      devide_fast_t<version_v> div_{};
      div_.devide_val = rhs;
      div_.inverse = cached_inverses[index_cache];
      return div_.divide_modulo(lhs);
    }
    const int num_zero_bits = std::countr_zero(rhs);
    const uint64_t rhs_reduced = rhs >> num_zero_bits;
    const uint64_t lhs_reduced = lhs >> num_zero_bits;
    const uint64_t round_val = (lhs_reduced << num_zero_bits) ^ lhs;
    if (rhs_reduced == 1) {
      return {lhs_reduced, round_val};
    }
    const uint64_t index_cache = (rhs_reduced >> 1) - 1;
    if (cached_count <= index_cache) {
      return {lhs_reduced / rhs_reduced,
              ((lhs_reduced % rhs_reduced) << num_zero_bits) + round_val};
    }
    devide_fast_t<version_v> div_{};
    div_.devide_val = rhs_reduced;
    div_.inverse = cached_inverses[index_cache];
    auto [mul, r] = div_.divide_modulo(lhs_reduced);
    return {mul, (r << num_zero_bits) + round_val};
  }

  MJZ_CX_ND_FN static std::pair<int64_t, int64_t> signed_divide_modulo(
      int64_t lhs, int64_t rhs) noexcept {
    bool is_neg = int(lhs < 0) != int(rhs < 0);
    lhs = std::max(lhs, -lhs);
    rhs = std::max(rhs, -rhs);
    auto [mul, r] = divide_modulo(uint64_t(lhs), uint64_t(rhs));
    lhs = int64_t(r);
    rhs = int64_t(mul);
    rhs = is_neg ? -rhs : rhs;
    lhs = is_neg ? -lhs : lhs;
    return {rhs, lhs};
  }
};

template <version_t version_v, uint64_t... exclude_>
struct exclusive_math_helper_t_ {
  template <devide_fast_t<version_v> rhs_v>
  MJZ_CX_FN static bool divide_modulo_impl(std::pair<uint64_t, uint64_t>& ret,
                                           const uint64_t lhs,
                                           const uint64_t rhs) noexcept {
    if (rhs != rhs_v.devide_val) {
      return false;
    }
    if constexpr (std::has_single_bit(rhs_v.devide_val)) {
      ret = std::pair<uint64_t, uint64_t>{lhs / rhs_v.devide_val,
                                          lhs % rhs_v.devide_val};
      return true;
    } else if constexpr (MJZ_MSVC_ONLY_CODE_(true) MJZ_GCC_ONLY_CODE_(false)) {
      ret = rhs_v.divide_modulo(lhs);
    }
    ret = std::pair<uint64_t, uint64_t>{lhs / rhs_v.devide_val,
                                        lhs % rhs_v.devide_val};
    return true;
  }

  MJZ_CX_FN static std::pair<uint64_t, uint64_t> divide_modulo(
      const uint64_t lhs, const uint64_t rhs) noexcept {
    std::pair<uint64_t, uint64_t> ret{};
    if ((divide_modulo_impl<exclude_>(ret, lhs, rhs) || ...)) return ret;
    return {lhs / rhs, lhs % rhs};
  }
  MJZ_CX_ND_FN static std::pair<int64_t, int64_t> signed_divide_modulo(
      int64_t lhs, int64_t rhs) noexcept {
    bool is_neg = int(lhs < 0) != int(rhs < 0);
    lhs = std::max(lhs, -lhs);
    rhs = std::max(rhs, -rhs);
    auto [mul, r] = divide_modulo(uint64_t(lhs), uint64_t(rhs));
    lhs = int64_t(r);
    rhs = int64_t(mul);
    rhs = is_neg ? -rhs : rhs;
    lhs = is_neg ? -lhs : lhs;
    return {rhs, lhs};
  }
};
template <version_t version_v>
using parse_math_helper_t_ =
    exclusive_math_helper_t_<version_v, 10, 2, 4, 8, 16, 32>;
template <version_t version_v = version_t{}>
struct big_float_t : parse_math_helper_t_<version_v> {
  using parse_math_helper_t_<version_v>::divide_modulo;
  using parse_math_helper_t_<version_v>::signed_divide_modulo;
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
      MJZ_MAYBE_UNUSED T& val, int64_t exp) noexcept {
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
      char& c = a[uintptr_t(i / 8)];
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
  MJZ_CX_AL_FN void normalize() noexcept {
    bool is_neg{};
    m_exponent = !m_coeffient ? 0 : m_exponent;
    is_neg = m_coeffient < 0;
    m_coeffient = is_neg ? -m_coeffient : m_coeffient;
    uint64_t cof_log = log2_ceil_of_val_create(uint64_t(m_coeffient));
    int64_t cof_log_delta{cof_log_dest - int64_t(cof_log)};
    m_exponent -= cof_log_delta;
    uint64_t coeffient = uint64_t(m_coeffient);
    coeffient = cof_log_delta < 0 ? coeffient >> (-cof_log_delta)
                                  : coeffient << cof_log_delta;
    m_coeffient = int64_t(coeffient);
    m_coeffient = is_neg ? -m_coeffient : m_coeffient;
  };
  MJZ_CX_FN static big_float_t normalize_add_impl_(big_float_t hs) noexcept {
    bool is_neg = hs.m_coeffient < 0;
   const int shift_amount = std::countl_zero(is_neg ? uint64_t(-hs.m_coeffient)
                                                  : uint64_t(hs.m_coeffient))-2;
    hs.m_coeffient = shift_amount < 0 ? hs.m_coeffient >> (-shift_amount)
                                      : hs.m_coeffient << shift_amount;
   hs.m_exponent -= shift_amount;
    return hs;
  }
  MJZ_CX_FN static big_float_t add(  big_float_t lhs,
                                     big_float_t rhs) noexcept {
    lhs = normalize_add_impl_(lhs);
    rhs = normalize_add_impl_(rhs);
    const int64_t delta_exp = lhs.m_exponent - rhs.m_exponent;
    std::swap(lhs, delta_exp<0?rhs: lhs);
    const int64_t abs_delta_exp = lhs.m_exponent - rhs.m_exponent;
    lhs.m_coeffient += abs_delta_exp<63 ? rhs.m_coeffient >> abs_delta_exp : 0;
    return lhs;
  }

  MJZ_CX_FN static big_float_t muliply(big_float_t lhs,
                                       big_float_t rhs) noexcept {
    const bool is_neg{(rhs.m_coeffient < 0) != (lhs.m_coeffient < 0)};
    rhs.m_coeffient = rhs.m_coeffient < 0 ? -rhs.m_coeffient : rhs.m_coeffient;
    lhs.m_coeffient = lhs.m_coeffient < 0 ? -lhs.m_coeffient : lhs.m_coeffient;
    uintN_t<version_v, 128> i1{rhs.m_coeffient}, i2{lhs.m_coeffient},
        i3{i1 * i2};
    int shift_amount = std::countl_zero(i3.nth_word(1));
    shift_amount = branchless_teranary<int>(
        shift_amount == 64, std::countl_zero(i3.nth_word(0)) + 64,
        shift_amount);
    i3 <<= uintlen_t(shift_amount);
    i3 >>= 2;
    i3.nth_word(1) += !!i3.nth_word(0);
    rhs.m_exponent += lhs.m_exponent;
    rhs.m_exponent += uint64_t(66) - shift_amount;
    rhs.m_coeffient = int64_t(i3.nth_word(1));
    rhs.m_coeffient = is_neg ? -rhs.m_coeffient : rhs.m_coeffient;
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

    auto [coff_, rem_] = signed_divide_modulo(lhs.m_coeffient, rhs.m_coeffient);
    int64_t round((rhs.m_coeffient >> 1) <= (rem_));
    lhs.m_coeffient = coff_;
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
    return (!!rhs && !!lhs) ? std::optional<big_float_t>(muliply(*rhs, *lhs))
                            : std::nullopt;
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
    return muliply(rhs, lhs);
  }

  MJZ_CX_FN friend big_float_t operator-(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return add(rhs, -lhs);
  }

  MJZ_CX_FN friend big_float_t operator/(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return *devide(rhs, lhs);
  }
  MJZ_CX_FN friend big_float_t operator+(big_float_t rhs,
                                         big_float_t lhs) noexcept {
    return add(rhs, lhs);
  }

  MJZ_CX_FN friend std::partial_ordering operator<=>(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? (*rhs <=> *lhs)
                            : std::partial_ordering::unordered;
  }

  MJZ_CX_FN std::strong_ordering operator<=>(
      const big_float_t& lhs) const noexcept {
    auto r = add(*this, -lhs);
    return r.m_coeffient <=> int64_t(0);
  }
  MJZ_CX_FN friend std::optional<bool> operator==(
      std::optional<big_float_t> rhs, std::optional<big_float_t> lhs) noexcept {
    return (!!rhs && !!lhs) ? (*rhs == *lhs) : nullopt;
  }

  MJZ_CX_FN bool operator==(const big_float_t& lhs) const noexcept {
    auto r = add(*this, -lhs);
    return r.m_coeffient == int64_t(0);
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
    int64_t& exponent = ret.m_exponent;
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
    ceil_log =
        signed_divide_modulo(ceil_log2, log2_of_val_create(exp_base)).first;
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
