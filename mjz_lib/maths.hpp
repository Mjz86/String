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

#include "mjz_uintconv.hpp"
#include "traits.hpp"
#include "tuple.hpp"
#include "versions.hpp"
#ifndef MJZ_MATHS_LIB_HPP_FILE_
#define MJZ_MATHS_LIB_HPP_FILE_
MJZ_DISABLE_ALL_WANINGS_START_;
#include <cfloat>
#include <cmath>
MJZ_DISABLE_ALL_WANINGS_END_;

#ifdef __SIZEOF_INT128__
#define MJZ_uint128_t_impl_t_ unsigned __int128
#elif 1 < _MSC_VER
#define MJZ_uint128_t_impl_t_ std::_Unsigned128
#else
#endif

#include <bit>
namespace mjz {

template <version_t version_v, uintlen_t n_bits>
  requires(64 * (n_bits / 64) == n_bits && !!n_bits)
struct uintN_t {
  MJZ_CONSTANT(uintlen_t) word_count = n_bits / 64;
  alignas(std::min<uintlen_t>(
      hardware_constructive_interference_size,
      log2_of_val_to_val(uint8_t(std::countr_zero(word_count * 8)))))
      uint64_t words[word_count]{};
  MJZ_DISABLE_ALL_WANINGS_START_;
  MJZ_DEFAULTED_CLASS(uintN_t);
  MJZ_DISABLE_ALL_WANINGS_END_;

 private:
  MJZ_CX_AL_FN uintN_t(void_struct_t, uintlen_t) noexcept : uintN_t{} {}
  template <std::integral T, std::integral... Ts>
  MJZ_CX_AL_FN uintN_t(void_struct_t, const uintlen_t i, T lowest,
                       Ts... args) noexcept
      : uintN_t{void_struct_t{}, i + 1, args...} {
    nth_word(i) = lowest;
  }

 public:
  template <std::integral... Ts>
    requires(sizeof...(Ts) <= word_count)
  MJZ_CX_AL_FN explicit uintN_t(/*low to high*/ Ts... args) noexcept
      : uintN_t{void_struct_t{}, 0, uint64_t(args)...} {}

  MJZ_CX_AL_FN uint64_t& nth_word(auto i) noexcept {
    if constexpr (version_v.is_LE()) {
      return words[size_t(i)];
    } else {
      return words[size_t((word_count - 1) - i)];
    }
  }
  MJZ_CX_AL_FN const uint64_t& nth_word(auto i) const noexcept {
    if constexpr (version_v.is_LE()) {
      return words[size_t(i)];
    } else {
      return words[size_t((word_count - 1) - i)];
    }
  }
  MJZ_CX_AL_FN bool nth_bit(auto i) const noexcept {
    return !!(nth_word(i >> 6) & (uint64_t(1) << (i & 63)));
  }
  MJZ_CX_AL_FN void set_nth_bit(auto i, bool val) noexcept {
    uint64_t mask = uint64_t(1) << (i & 63);
    uint64_t& word = nth_word(i >> 6);
    word &= ~mask;
    word |= val ? mask : 0;
  }
  MJZ_CX_AL_FN void operator_sr(uintlen_t amount) noexcept {
    bool zero_out = amount < n_bits;
    const uint64_t zero_out_mask64 = (~uint64_t(zero_out)) + 1;
    for (uint64_t& word : words) word &= zero_out_mask64;
    amount &= zero_out_mask64;
    intlen_t abs_amount = intlen_t(amount);
    intlen_t internal_amount{abs_amount & 63};
    intlen_t external_amount{abs_amount >> 6};
    MJZ_JUST_ASSUME_(external_amount < intlen_t(word_count));
    MJZ_JUST_ASSUME_(amount < n_bits);
    const uint64_t ub_annoying_mask = (~uint64_t(internal_amount != 0)) + 1;

    uintN_t upper_half{*this}, lower_half{*this};
    for (uint64_t& word : upper_half.words) word = (word >> internal_amount);
    internal_amount |= 0 == internal_amount;
    for (uint64_t& word : lower_half.words)
      word = ub_annoying_mask & (word << (64 - internal_amount));

    for (intlen_t i{1}; i < intlen_t(word_count); i++) {
      upper_half.nth_word(i - 1) |= lower_half.nth_word(i);
    }
    std::array<uint64_t, word_count * 2> temp{};
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      temp[size_t(i)] = upper_half.nth_word(i);
    }
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) = temp[size_t(i + external_amount)];
    }
  }
  MJZ_CX_AL_FN uintN_t& operator>>=(uintlen_t amount) noexcept {
    operator_sr(amount);

    return *this;
  }

  MJZ_CX_AL_FN void operator_sl(uintlen_t amount) noexcept {
    bool zero_out = amount < n_bits;
    const uint64_t zero_out_mask64 = (~uint64_t(zero_out)) + 1;
    for (uint64_t& word : words) word &= zero_out_mask64;
    amount &= zero_out_mask64;
    intlen_t abs_amount = intlen_t(amount);
    intlen_t internal_amount{abs_amount & 63};
    intlen_t external_amount{abs_amount >> 6};
    MJZ_JUST_ASSUME_(external_amount < intlen_t(word_count));
    MJZ_JUST_ASSUME_(amount < n_bits);
    const uint64_t ub_annoying_mask = (~uint64_t(internal_amount != 0)) + 1;
    uintN_t upper_half{*this}, lower_half{*this};
    for (uint64_t& word : lower_half.words) word = (word << internal_amount);
    internal_amount |= 0 == internal_amount;
    for (uint64_t& word : upper_half.words)
      word = ub_annoying_mask & (word >> (64 - internal_amount));
    for (intlen_t i{}; i < intlen_t(word_count) - 1; i++) {
      lower_half.nth_word(i + 1) |= upper_half.nth_word(i);
    }
    std::array<uint64_t, word_count * 2> temp{};
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      intlen_t dest = i + external_amount;
      temp[size_t(dest)] = lower_half.nth_word(i);
    }

    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) = temp[size_t(i)];
    }
  }
  MJZ_CX_AL_FN uintN_t& operator<<=(uintlen_t amount) noexcept {
    operator_sl(amount);
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator&=(const uintN_t amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      words[i] &= amount.words[i];
    }
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator|=(const uintN_t amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      words[i] |= amount.words[i];
    }
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator^=(const uintN_t amount) noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      words[i] ^= amount.words[i];
    }
    return *this;
  }

  MJZ_CX_AL_FN uintN_t& flip() noexcept {
    for (intlen_t i{}; i < intlen_t(word_count); i++) {
      nth_word(i) = ~nth_word(i);
    }
    return *this;
  }

  MJZ_CX_AL_FN bool add(const uintN_t& amount, bool carry = false) noexcept {
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
  MJZ_CX_AL_FN uintN_t& negate() noexcept {
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
  MJZ_CX_AL_FN bool minus(const uintN_t& amount, bool carry = false) noexcept {
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
  MJZ_CX_AL_FN uintN_t& operator-=(const uintN_t& amount) noexcept {
    minus(amount);
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator+=(const uintN_t& amount) noexcept {
    add(amount);
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator*=(const uintN_t& amount) noexcept {
    if constexpr (word_count == 2) {
#ifdef MJZ_uint128_t_impl_t_
      return *this = std::bit_cast<uintN_t>(
                 std::bit_cast<MJZ_uint128_t_impl_t_>(amount) *
                 std::bit_cast<MJZ_uint128_t_impl_t_>(*this));
#endif  //  MJZ_uint128_t_impl_t_

      // https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/uint128_t.h
      const uint64_t x = nth_word(0);
      const uint64_t y = amount.nth_word(0);
      const uint64_t ah = amount.nth_word(1);
      const uint64_t th = nth_word(1);
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
      nth_word(1) = mh;
      nth_word(0) = ml;
      return *this;
    } else

        if constexpr (0) {
      uintN_t ret{};
      for (uintlen_t i{}; i < n_bits; i++) {
        if (nth_bit(i)) {
          ret += amount << i;
        }
      }

      return *this = ret;
    } else {
      uintN_t ret{};
      using u128_t0_ = uintN_t<version_v, 128>;
      for (uintlen_t i{}; i < n_bits / 64; i++) {
        for (uintlen_t j{}; j < n_bits / 64; j++) {
          u128_t0_ x{nth_word(i)};
          x *= u128_t0_(amount.nth_word(j));
          ret += uintN_t{x.nth_word(0), x.nth_word(1)} << ((i + j) * 64);
        }
      }
      return *this = ret;
    }
  }
  MJZ_CX_AL_FN std::strong_ordering operator<=>(
      const uintN_t& rhs) const noexcept {
    for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
      std::strong_ordering order = nth_word(i) <=> rhs.nth_word(i);
      if (order != std::strong_ordering::equal) {
        return order;
      }
    }
    return std::strong_ordering::equal;
  }

  MJZ_CX_AL_FN bool operator==(const uintN_t& rhs) const noexcept = default;

  MJZ_CX_AL_FN std::optional<uintlen_t> floor_log2() const noexcept {
    for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
      uint64_t word = nth_word(i);
      if (!word) continue;
      return uintlen_t(log2_of_val_create(word)) + uintlen_t(i * 64);
    }
    return {};
  }
  MJZ_CX_AL_FN std::optional<uintlen_t> ceil_log2() const noexcept {
    intlen_t i{intlen_t(word_count) - 1};
    for (; 0 <= i; i--) {
      uint64_t word = nth_word(i);
      if (!word) continue;
      uintlen_t awnser = uintlen_t(log2_of_val_create(word));
      bool ceil_ = (uintlen_t(1) << awnser) != word;
      awnser += uintlen_t(i * 64);
      i--;
      for (; 0 <= i; i--) {
        ceil_ |= !!nth_word(i);
      }
      awnser += ceil_;
      return awnser;
    }
    return {};
  }
  MJZ_CX_AL_FN uintlen_t countr_zero() const noexcept {
    uintlen_t ret{};
    for (uint64_t n : std::views::iota(uintlen_t(0), n_bits / 64)) {
      n = uintlen_t(std::countr_zero(nth_word(n)));
      if (n == 64) {
        ret += 64;
        continue;
      }
      ret += n;
      break;
    }
    return ret;
  }
  MJZ_CX_AL_FN uintlen_t countl_zero() const noexcept {
    uintlen_t ret{};
    for (uint64_t n :
         std::views::iota(uintlen_t(0), n_bits / 64) | std::views::reverse) {
      n = uintlen_t(std::countl_zero(nth_word(n)));
      if (n == 64) {
        ret += 64;
        continue;
      }
      ret += n;
      break;
    }
    return ret;
  }
  MJZ_CX_AL_FN uintlen_t countr_one() const noexcept {
    uintlen_t ret{};
    for (uint64_t n : std::views::iota(uintlen_t(0), n_bits / 64)) {
      n = uintlen_t(std::countr_one(nth_word(n)));
      if (n == 64) {
        ret += 64;
        continue;
      }
      ret += n;
      break;
    }
    return ret;
  }
  MJZ_CX_AL_FN bool has_single_bit() const noexcept {
    if constexpr (false) {
      return !!*this && !(*this & (*this - uintN_t(1)));
    } else {
      return popcount() == 1;
    }
  }
  MJZ_CX_AL_FN uintN_t bit_ceil() const noexcept {
    if (*this <= uintN_t(1)) {
      return uintN_t(1);
    }
    return uintN_t(1) << (n_bits - (*this - uintN_t(1)).countl_zero());
  }
  MJZ_CX_AL_FN uintN_t bit_floor() const noexcept {
    if (!*this) {
      return uintN_t{};
    }
    return uintN_t(1) << (n_bits - 1 - countl_zero());
  }
  MJZ_CX_AL_FN uintlen_t bit_width() const noexcept {
    return n_bits - countl_zero();
  }
  MJZ_CX_AL_FN uintN_t rotr(intlen_t r) const noexcept {
    r %= n_bits;
    if (!r) return *this;
    if (r < 0) {
      r = -r;
      return (*this << uintlen_t(r)) | (*this >> (n_bits - uintlen_t(r)));
    } else {
      return (*this >> uintlen_t(r)) | (*this << (n_bits - uintlen_t(r)));
    }
  }
  MJZ_CX_AL_FN uintN_t rotl(intlen_t r) const noexcept { return rotr(-r); }

  MJZ_CX_AL_FN uintlen_t popcount() const noexcept {
    uintlen_t ret{};
    for (uint64_t n : std::views::iota(uintlen_t(0), n_bits / 64)) {
      ret += uintlen_t(std::popcount(nth_word(n)));
    }
    return ret;
  }
  MJZ_CX_AL_FN uintlen_t countl_one() const noexcept {
    uintlen_t ret{};
    for (uint64_t n :
         std::views::iota(uintlen_t(0), n_bits / 64) | std::views::reverse) {
      n = uintlen_t(std::countl_one(nth_word(n)));
      if (n == 64) {
        ret += 64;
        continue;
      }
      ret += n;
      break;
    }
    return ret;
  }
  MJZ_CX_AL_FN uintN_t byteswap() const noexcept {
    auto ret = make_bitcast(*this);
    mem_byteswap(ret.data(), ret.size());
    return std::bit_cast<uintN_t>(ret);
  }
  MJZ_CX_AL_FN uintN_t to_modulo_ret_devide(const uintN_t& rhs) noexcept {
    asserts(asserts.assume_rn, rhs != uintN_t{},
            " it is undefined behaviour to call with rhs of 0");
    uintN_t intermidiate{};
    for (intlen_t i{intlen_t(rhs.countl_zero() - this->countl_zero())}; 0 <= i;
         i--) {
      uintN_t temp = rhs << uintlen_t(i);
      bool is_bigger{temp <= *this};
      if (is_bigger) {
        intermidiate.set_nth_bit(i, is_bigger);
        *this -= temp;
      };
    }
    return intermidiate;
  }

  MJZ_CX_AL_FN uintN_t& operator%=(const uintN_t& rhs) noexcept {
    to_modulo_ret_devide(rhs);
    return *this;
  }
  MJZ_CX_AL_FN uintN_t& operator/=(const uintN_t& rhs) noexcept {
    return *this = to_modulo_ret_devide(rhs);
  }
  MJZ_CX_AL_FN uintN_t& operator_assign_devide_up(const uintN_t& rhs) noexcept {
    uintN_t temp = to_modulo_ret_devide(rhs);
    temp.add(uintN_t(), *this != uintN_t());
    return *this = temp;
  }
  MJZ_CX_AL_FN friend uintN_t operator_devide_up(uintN_t x,
                                                 const uintN_t& y) noexcept {
    x.operator_assign_devide_up(y);
    return x;
  }

  template <std::integral T>
  MJZ_CX_AL_FN explicit operator T() const noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t));
    return static_cast<T>(nth_word(0));
  }

  MJZ_CX_AL_FN friend uintN_t operator&(uintN_t x, const uintN_t& y) noexcept {
    x &= y;
    return x;
  }
  MJZ_CX_AL_FN friend uintN_t operator^(uintN_t x, const uintN_t& y) noexcept {
    x ^= y;
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator|(uintN_t x, const uintN_t& y) noexcept {
    x |= y;
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator<<(uintN_t x, uintlen_t y) noexcept {
    x <<= y;
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator>>(uintN_t x, uintlen_t y) noexcept {
    x >>= y;
    return x;
  }

  MJZ_CX_AL_FN uintN_t operator~() const noexcept {
    uintN_t x = *this;
    x.flip();
    return x;
  }

  MJZ_CX_AL_FN uintN_t operator-() const noexcept {
    uintN_t x = *this;
    x.negate();
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator+(uintN_t x, const uintN_t& y) noexcept {
    x += y;
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator-(uintN_t x, const uintN_t& y) noexcept {
    x -= y;
    return x;
  }
  MJZ_CX_AL_FN friend uintN_t operator*(uintN_t x, const uintN_t& y) noexcept {
    x *= y;
    return x;
  }
  MJZ_CX_AL_FN friend uintN_t operator/(uintN_t x, const uintN_t& y) noexcept {
    x /= y;
    return x;
  }

  MJZ_CX_AL_FN friend uintN_t operator%(uintN_t x, const uintN_t& y) noexcept {
    x %= y;
    return x;
  }

  MJZ_CX_AL_FN uintN_t& operator--() noexcept { return *this -= 1; }

  MJZ_CX_AL_FN uintN_t& operator++() noexcept { return *this += 1; }

  MJZ_CX_AL_FN uintN_t operator++(int) noexcept {
    uintN_t temp{*this};
    *this -= 1;
    return temp;
  }

  MJZ_CX_AL_FN uintN_t operator--(int) noexcept {
    uintN_t temp{*this};
    *this += 1;
    return temp;
  }
  MJZ_CX_AL_FN explicit operator bool() const noexcept {
    return *this != uintN_t{0};
  }
  template <uintlen_t n2_bits>
  MJZ_CX_AL_FN explicit operator uintN_t<version_v, n2_bits>() const noexcept {
    uintN_t<version_v, n2_bits> ret{};
    for (uintlen_t i{}; i < (std::min(n2_bits, n_bits) >> 6); i++) {
      ret.nth_word(i) = nth_word(i);
    }
    return ret;
  }
  template <std::unsigned_integral UT>
  MJZ_CX_AL_FN explicit operator UT() const noexcept {
    return UT(nth_word(0));
  }
  MJZ_CX_AL_FN bool operator!() const noexcept { return *this == uintN_t{0}; }
};

MJZ_CX_AL_FN uintlen_t countr_zero(auto v) noexcept {
  if constexpr (requires() { std::countr_zero(v); }) {
    return uintlen_t(std::countr_zero(v));
  } else {
    return v.countr_zero();
  }
}
MJZ_CX_AL_FN uintlen_t countl_zero(auto v) noexcept {
  if constexpr (requires() { std::countl_zero(v); }) {
    return (uintlen_t)std::countl_zero(v);
  } else {
    return v.countl_zero();
  }
}
MJZ_CX_AL_FN uintlen_t countr_one(auto v) noexcept {
  if constexpr (requires() { std::countr_one(v); }) {
    return (uintlen_t)std::countr_one(v);
  } else {
    return v.countr_one();
  }
}
MJZ_CX_AL_FN bool has_single_bit(auto v) noexcept {
  if constexpr (requires() { std::has_single_bit(v); }) {
    return std::has_single_bit(v);
  } else {
    return v.has_single_bit();
  }
}
MJZ_CX_AL_FN auto bit_ceil(auto v) noexcept {
  if constexpr (requires() { std::bit_ceil(v); }) {
    return std::bit_ceil(v);
  } else {
    return v.bit_ceil();
  }
}
MJZ_CX_AL_FN auto bit_floor(auto v) noexcept {
  if constexpr (requires() { std::bit_floor(v); }) {
    return std::bit_floor(v);
  } else {
    return v.bit_floor();
  }
}
MJZ_CX_AL_FN uintlen_t bit_width(auto v) noexcept {
  if constexpr (requires() { std::bit_width(v); }) {
    return (uintlen_t)std::bit_width(v);
  } else {
    return v.bit_width();
  }
}
MJZ_CX_AL_FN auto rotr(auto v, intlen_t r) noexcept {
  if constexpr (requires() { std::rotr(v, int(r)); }) {
    return std::rotr(v, int(uint8_t(r)));
  } else {
    return v.rotr(r);
  }
}
MJZ_CX_AL_FN auto rotl(auto v, intlen_t r) noexcept {
  if constexpr (requires() { std::rotl(v, int(r)); }) {
    return std::rotl(v, int(uint8_t(r)));
  } else {
    return v.rotl(r);
  }
}

MJZ_CX_AL_FN uintlen_t popcount(auto v) noexcept {
  if constexpr (requires() { std::popcount(v); }) {
    return (uintlen_t)std::popcount(v);
  } else {
    return v.popcount();
  }
}
MJZ_CX_AL_FN uintlen_t countl_one(auto v) noexcept {
  if constexpr (requires() { std::countl_one(v); }) {
    return (uintlen_t)std::countl_one(v);
  } else {
    return v.countl_one();
  }
}

MJZ_CX_AL_FN auto to_modulo_ret_devide(auto& lhs, const auto& rhs) noexcept {
  if constexpr (requires() {
                  lhs.to_modulo_ret_devide(
                      static_cast<std::remove_cvref_t<decltype(lhs)>>(rhs));
                }) {
    return lhs.to_modulo_ret_devide(
        static_cast<std::remove_cvref_t<decltype(lhs)>>(rhs));
  } else {
    auto temp = lhs / rhs;
    lhs %= rhs;
    return temp;
  }
}

template <version_t version_v, size_t n>
using uint_min_N_t =
    std::conditional_t<(n <= 64),
                       uint_size_of_t<std::min<uintlen_t>(8, (n + 7) >> 3)>,
                       uintN_t<version_v, ((((n | (n == 0)) + 63) >> 6) << 6)>>;

template <version_t version_v, uint64_t... exclude_>
struct exclusive_math_helper_t_ {
  template <uint64_t rhs_v>
  MJZ_CX_FN static bool divide_modulo_impl(std::pair<uint64_t, uint64_t>& ret,
                                           const uint64_t lhs,
                                           const uint64_t rhs) noexcept {
    if (rhs != rhs_v) {
      return false;
    }
    if constexpr (std::has_single_bit(rhs_v)) {
      ret = std::pair<uint64_t, uint64_t>{lhs / rhs_v, lhs % rhs_v};
      return true;
    }
    ret = std::pair<uint64_t, uint64_t>{lhs / rhs_v, lhs % rhs_v};
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
      exp += intlen_t(sub_range_delta_exp);
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
    const int shift_amount =
        std::countl_zero(is_neg ? uint64_t(-hs.m_coeffient)
                                : uint64_t(hs.m_coeffient)) -
        2;
    hs.m_coeffient = shift_amount < 0 ? hs.m_coeffient >> (-shift_amount)
                                      : hs.m_coeffient << shift_amount;
    hs.m_exponent -= shift_amount;
    return hs;
  }
  MJZ_CX_FN static big_float_t add(big_float_t lhs, big_float_t rhs) noexcept {
    lhs = normalize_add_impl_(lhs);
    rhs = normalize_add_impl_(rhs);
    const int64_t delta_exp = lhs.m_exponent - rhs.m_exponent;
    std::swap(lhs, delta_exp < 0 ? rhs : lhs);
    const int64_t abs_delta_exp = lhs.m_exponent - rhs.m_exponent;
    lhs.m_coeffient +=
        abs_delta_exp < 63 ? rhs.m_coeffient >> abs_delta_exp : 0;
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
    rhs.m_exponent += int64_t(66 - shift_amount);
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

template <version_t version_v = version_t{}, uintlen_t n_ = 64>
MJZ_CX_AL_FN auto get_devision_by_mul_rs_shift_and_bit_count(
    uint_min_N_t<version_v, n_> max_value_,
    uint_min_N_t<version_v, n_> devisor_,
    uint_min_N_t<version_v, n_> modulos_percition,
    bool two_shifts_ = false) noexcept {
  max_value_ = std::max(max_value_, devisor_);
  uint32_t factor2d = 0;
  if (two_shifts_) {
    factor2d = uint32_t(countr_zero(devisor_));
    max_value_ >>= factor2d;
  } else {
    factor2d = 0;
  }
  const auto mvbw_ = uint32_t(bit_width(max_value_));
  const auto floor_log2_plus1_divisor = uint32_t(bit_width(devisor_));
  const auto bwd = floor_log2_plus1_divisor - factor2d;
  const auto bwmv = mvbw_ - factor2d;
  const auto ceil_log2_mv = bwmv - has_single_bit(max_value_);
  const auto ceil_log2_d = bwd - has_single_bit(devisor_);
  const auto fraction_bit_count =
      floor_log2_plus1_divisor +
      std::max(uint32_t(bit_width(modulos_percition)), ceil_log2_mv);
  const auto result_bit_width =
      fraction_bit_count + ceil_log2_mv - ceil_log2_d + 1;
  // v\ge\operatorname{ceil}\left(\frac{m}{2^{\left(k-N\right)}}\right)
  //  ---------
  //  or
  //  --------
  //  v>\operatorname{ceil}\left(\frac{m}{2^{\left(k-N\right)}}\right)-1
  //  -----------
  // or
  //-----------
  //\frac{2^{k}}{m}v\ge2^{N}
  const auto most_significant_bit_is_set_comparasion_shift =
      result_bit_width-1 - fraction_bit_count;
  return tuple_t{fraction_bit_count, result_bit_width, factor2d,
                 most_significant_bit_is_set_comparasion_shift};
}
template <version_t version_v = version_t{}, uintlen_t n_ = 64>
MJZ_CX_AL_FN auto get_devide_inverse_and_shift(
    uint_min_N_t<version_v, n_> max_value_,
    uint_min_N_t<version_v, n_> devisor_, bool reduce_ = true,
    uint_min_N_t<version_v, n_> modulos_percition =
        uint_min_N_t<version_v, n_>{},
    bool two_shifts_ = false) noexcept {
  uint_min_N_t<version_v, n_ * 2 + 1> ret{1};
  const auto [fraction_bit_count, bit_count, second_shift,
              most_significant_bit_is_set_comparasion_shift] =
      get_devision_by_mul_rs_shift_and_bit_count<version_v, n_>(
          max_value_, devisor_, modulos_percition, two_shifts_);
  asserts(asserts.assume_rn, bit_count < sizeof(ret) * 8);
  ret <<= uintlen_t(fraction_bit_count);
  auto temp = to_modulo_ret_devide(ret, devisor_);
  ret = temp + decltype(temp)(!!ret);
  uintlen_t zcnt_{};
  if (reduce_) {
    zcnt_ = countr_zero(ret);
    ret >>= zcnt_;
  }
  temp = (decltype(temp)(devisor_)
          << most_significant_bit_is_set_comparasion_shift)- decltype(temp)(1);
  return tuple_t{ret, uintlen_t(fraction_bit_count - zcnt_),
                 uintlen_t(bit_count - zcnt_), second_shift, temp};
}

}  // namespace mjz

#endif  // MJZ_MATHS_LIB_HPP_FILE_
