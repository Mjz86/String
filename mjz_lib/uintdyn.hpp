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
#include "uintN.hpp"
#ifndef MJZ_LIB_FILE_UINT_DYN_HPP_
#define MJZ_LIB_FILE_UINT_DYN_HPP_

MJZ_EXPORT namespace mjz {

  inline namespace uint_ex_t {
  template <version_t version_v_>
  MJZ_CX_FN uintlen_t /*byte count*/
  to_ascii_impl_(uint_dyn_t<version_v_, true> input,
                 std::span<uint64_t> aligned8_output_and_temp_buffer) noexcept;

  template <version_t version_v_>
  MJZ_CX_FN uint_dyn_t<version_v_, false>
  from_ascii_impl_(uint_dyn_t<version_v_, false> uninit_output_and_temp_buffer,
                   std::span<const char> ascii_input) noexcept;
  template <version_t version_v, bool is_const_v> struct uint_dyn_t {

    using mjz_uint_dyn_t_id_val_t_2354675648764874753789 = uint_dyn_t;
    using words_t =
        std::span<std::conditional_t<is_const_v, const uint64_t, uint64_t>>;
    words_t words{};
    MJZ_CX_AL_FN uint_dyn_t() noexcept : words{} {}
    MJZ_CX_AL_FN uint_dyn_t(std::span<const uint64_t> w) noexcept
      requires(is_const_v)
        : words{w} {}
    MJZ_CX_AL_FN uint_dyn_t(std::span<uint64_t> w) noexcept : words{w} {}
    MJZ_CX_AL_FN uint64_t &nth_word(auto i) noexcept
      requires(!is_const_v)
    {
      if constexpr (version_v.is_LE()) {
        return words[size_t(i)];
      } else {
        return words[size_t((words.size() - 1) - i)];
      }
    }
    MJZ_CX_AL_FN const uint64_t &nth_word(auto i) const noexcept {
      if constexpr (version_v.is_LE()) {
        return words[size_t(i)];
      } else {
        return words[size_t((words.size() - 1) - i)];
      }
    }
    MJZ_CX_AL_FN bool nth_bit(auto i) const noexcept {
      return !!(nth_word(i >> 6) & (uint64_t(1) << (i & 63)));
    }
    MJZ_CX_AL_FN void set_nth_bit(auto i, bool val) noexcept
      requires(!is_const_v)
    {
      uint64_t mask = uint64_t(1) << (i & 63);
      uint64_t &word = nth_word(i >> 6);
      word &= ~mask;
      word |= val ? mask : 0;
    }

    MJZ_CX_AL_FN uint_dyn_t &operator>>=(uintlen_t amount) noexcept
      requires(!is_const_v)
    {
      operator_sr(amount);
      return *this;
    }

    MJZ_CX_AL_FN uint_dyn_t &operator<<=(uintlen_t amount) noexcept
      requires(!is_const_v)
    {
      operator_sl(amount);
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &
    operator&=(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(std::min(words.size(), amount.n_word()));
           i++) {
        words[size_t(i)] &= amount.words[size_t(i)];
      }
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &
    operator|=(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(std::min(words.size(), amount.n_word()));
           i++) {
        words[size_t(i)] |= amount.words[size_t(i)];
      }
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &
    operator^=(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(std::min(words.size(), amount.n_word()));
           i++) {
        words[size_t(i)] ^= amount.words[size_t(i)];
      }
      return *this;
    }

    MJZ_CX_AL_FN uint_dyn_t &
    copy_from(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      for (uintlen_t i{}; i < std::min(words.size(), amount.n_word()); i++) {
        nth_word(size_t(i)) = amount.nth_word(size_t(i));
      }
      for (uintlen_t i{amount.n_word()}; i < words.size(); i++) {
        nth_word(size_t(i)) = 0;
      }

      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &flip() noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(words.size()); i++) {
        nth_word(i) = ~nth_word(i);
      }
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &broadcast(uint64_t u64) noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(words.size()); i++) {
        nth_word(i) = u64;
      }
      return *this;
    }

    MJZ_CX_AL_FN bool add(uint_dyn_t<version_v, true> amount,
                          bool carry = false) noexcept
      requires(!is_const_v)
    {
      for (intlen_t i{}; i < intlen_t(std::min(words.size(), amount.n_word()));
           i++) {
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
      for (intlen_t i{intlen_t(amount.n_word())};
           carry && i < intlen_t(words.size()); i++) {
        uint64_t rhs = nth_word(i);
        uintlen_t min_rhs = rhs;
        rhs++;
        bool overflow = rhs < min_rhs;
        carry = overflow;
        nth_word(i) = rhs;
      }
      return carry;
    }
    MJZ_CX_AL_FN uint_dyn_t &negate() noexcept
      requires(!is_const_v)
    {
      // twos compliment (~*this + 1)
      bool carry = true;
      for (intlen_t i{}; i < intlen_t(words.size()); i++) {
        uint64_t rhs = ~nth_word(i);
        bool perv_carry = carry;
        carry &= rhs == uintlen_t(-1);
        rhs += perv_carry;
        nth_word(i) = rhs;
      }
      return *this;
    }
    MJZ_CX_AL_FN bool minus(uint_dyn_t<version_v, true> amount,
                            bool carry = false) noexcept
      requires(!is_const_v)
    {
      bool negate_carry = true;
      for (intlen_t i{}; i < intlen_t(std::min(words.size(), amount.n_word()));
           i++) {
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
      for (intlen_t i{intlen_t(amount.n_word())}; i < intlen_t(words.size());
           i++) {
        uint64_t rhs = nth_word(i);
        uint64_t lhs = uint64_t(-1);
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
    MJZ_CX_AL_FN uint_dyn_t &
    operator-=(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      minus(amount);
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &
    operator+=(uint_dyn_t<version_v, true> amount) noexcept
      requires(!is_const_v)
    {
      add(amount);
      return *this;
    }
    MJZ_CX_AL_FN std::strong_ordering
    operator<=>(uint_dyn_t<version_v, false> rhs) const noexcept {
      return operator<=>(uint_dyn_t<version_v, true>(rhs));
    }
    MJZ_CX_AL_FN std::strong_ordering
    operator<=>(uint_dyn_t<version_v, true> rhs) const noexcept {
      return compare(uint_dyn_t<version_v, true>(*this), rhs);
    }
    MJZ_CX_AL_FN static std::strong_ordering
    compare(uint_dyn_t<version_v, true> lhs,
            uint_dyn_t<version_v, true> rhs) noexcept {
      uintlen_t bwr = rhs.bit_width();
      uintlen_t bwl = lhs.bit_width();
      if (bwr != bwl)
        return (bwl <=> bwr);
      lhs.shrink_to_width(bwl);
      rhs.shrink_to_width(bwr);
      for (intlen_t i{intlen_t(rhs.n_word()) - 1}; 0 <= i; i--) {
        std::strong_ordering order = lhs.nth_word(i) <=> rhs.nth_word(i);
        if (order != std::strong_ordering::equal) {
          return order;
        }
      }
      return std::strong_ordering::equal;
    }

    MJZ_CX_AL_FN bool
    operator==(uint_dyn_t<version_v, true> rhs) const noexcept {
      return (*this <=> rhs) == std::strong_ordering::equal;
    }

    MJZ_CX_AL_FN std::optional<uintlen_t> floor_log2() const noexcept {
      for (intlen_t i{intlen_t(words.size()) - 1}; 0 <= i; i--) {
        uint64_t word = nth_word(i);
        if (!word)
          continue;
        return uintlen_t(log2_of_val_create(word)) + uintlen_t(i * 64);
      }
      return {};
    }
    MJZ_CX_AL_FN std::optional<uintlen_t> ceil_log2() const noexcept {
      intlen_t i{intlen_t(words.size()) - 1};
      for (; 0 <= i; i--) {
        uint64_t word = nth_word(i);
        if (!word)
          continue;
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
    MJZ_CX_AL_FN uintlen_t countl_one() const noexcept {
      uintlen_t ret{};

      for (uint64_t n : std::views::iota(uintlen_t(0), words.size() * 64 / 64) |
                            std::views::reverse) {
        n = uintlen_t(std::countl_one(nth_word(n)));
        ret += n;
        if (n != 64)
          break;
      }
      return ret;
    }

    MJZ_CX_AL_FN uintlen_t countr_zero() const noexcept {
      uintlen_t ret{};

      for (uint64_t n :
           std::views::iota(uintlen_t(0), words.size() * 64 / 64)) {
        n = uintlen_t(std::countr_zero(nth_word(n)));
        ret += n;
        if (n != 64)
          break;
      }
      return ret;
    }
    MJZ_CX_AL_FN uintlen_t countl_zero() const noexcept {
      uintlen_t ret{};

      for (uint64_t n : std::views::iota(uintlen_t(0), words.size() * 64 / 64) |
                            std::views::reverse) {
        n = uintlen_t(std::countl_zero(nth_word(n)));
        ret += n;
        if (n != 64)
          break;
      }
      return ret;
    }
    MJZ_CX_AL_FN uintlen_t countr_one() const noexcept {
      uintlen_t ret{};

      for (uint64_t n :
           std::views::iota(uintlen_t(0), words.size() * 64 / 64)) {
        n = uintlen_t(std::countr_one(nth_word(n)));
        ret += n;
        if (n != 64)
          break;
      }
      return ret;
    }
    MJZ_CX_AL_FN bool has_single_bit() const noexcept {
      return popcount() == 1;
    }
    MJZ_CX_AL_FN uintlen_t bit_width() const noexcept {
      return words.size() * 64 - countl_zero();
    }

    MJZ_CX_AL_FN uintlen_t popcount() const noexcept {
      uintlen_t ret{};
      for (uint64_t n :
           std::views::iota(uintlen_t(0), words.size() * 64 / 64)) {
        ret += uintlen_t(std::popcount(nth_word(n)));
      }
      return ret;
    }

    template <std::integral T>
    MJZ_CX_AL_FN explicit operator T() const noexcept {
      if (!words.size())
        return {};
      static_assert(sizeof(T) <= sizeof(uint64_t));
      return static_cast<T>(nth_word(0));
    }

    MJZ_CX_AL_FN uint_dyn_t &operator--() noexcept
      requires(!is_const_v)
    {
      return *this -= 1;
    }

    MJZ_CX_AL_FN uint_dyn_t &operator++() noexcept
      requires(!is_const_v)
    {
      return *this += 1;
    }

    MJZ_CX_AL_FN explicit operator bool() const noexcept {
      return *this != uint_dyn_t{};
    }

    template <std::unsigned_integral UT>
    MJZ_CX_AL_FN explicit operator UT() const noexcept {
      if (!words.size())
        return {};
      return UT(nth_word(0));
    }
    MJZ_CX_AL_FN bool operator!() const noexcept {
      return *this == uint_dyn_t{};
    }
    template <uintlen_t n2_bits>
    MJZ_CX_AL_FN explicit
    operator uintN_t<version_v, n2_bits>() const noexcept {
      uintN_t<version_v, n2_bits> ret{};
      for (uintlen_t i{}; i < (std::min(n2_bits, words.size() * 64) >> 6);
           i++) {
        ret.nth_word(i) = nth_word(i);
      }
      return ret;
    }

    MJZ_CX_AL_FN operator uint_dyn_t<version_v, true>() const noexcept {
      return uint_dyn_t<version_v, true>(words);
    }
    MJZ_CX_AL_FN uintlen_t dec_width_aprox_ceil() const noexcept {
      return dec_width_aprox_ceil(bit_width());
    }

    MJZ_CX_AL_FN void shrink_to_bit_width() noexcept {
      return shrink_to_width(bit_width());
    }
    MJZ_CX_AL_FN void shrink_to_width(uintlen_t bw) noexcept {
      uintlen_t len = (std::min(n_bit(), bw) + 63) >> 6;
      if constexpr (version_v.is_LE()) {
        words = words.subspan(0, len);
      } else {
        words = words.subspan(words.size() - len);
      }
    }
    MJZ_CX_AL_FN uintlen_t n_bit() const noexcept { return words.size() * 64; }
    MJZ_CX_AL_FN uintlen_t n_word() const noexcept { return words.size(); }

  public:
    MJZ_CX_AL_FN uintlen_t /*byte count*/
    to_ascii(
        std::span<uint64_t> aligned8_output_and_temp_buffer) const noexcept {
      return to_ascii_impl_(uint_dyn_t<version_v, true>(*this),
                            aligned8_output_and_temp_buffer);
    }
    MJZ_CX_AL_FN void /*byte count*/
    from_ascii(std::span<const char> ascii_input) noexcept
      requires(!is_const_v)
    {
      *this = from_ascii_impl_(*this, ascii_input);
    }

    MJZ_CX_AL_FN uintlen_t /*u64 count*/
    stack_shrinked_ceil_size_for_ascii() noexcept {
      uintlen_t bw = bit_width();
      shrink_to_width(bw);
      bw = n_bit();
      return (bw >> 3) + (bw >> 4);
    }
    // argument must not overlap (/*restrict span*/ )
    MJZ_CX_FN static pair_t<success_t, bool /*overflow-cutoff of devide_out*/>
    operator_modulo_and_devide(
        /*restrict span*/ uint_dyn_t devide_out,
        /*restrict span*/ uint_dyn_t modulo_out_val_in,
        /*restrict span*/ /*from outside it can be considered non-thread safe
                             const , but mofified witith function so its not
                             const */
        uint_dyn_t rhs) noexcept
      requires(!is_const_v)
    {

      uintlen_t rhs_lz = rhs.countl_zero();
      uintlen_t rhs_bw = rhs.n_bit() - rhs_lz;
      intlen_t bit_index_max = intlen_t(modulo_out_val_in.bit_width() - rhs_bw);

      if (bit_index_max < 0) {
        return {true, false};
      }
      bool cut_off{devide_out.n_bit() <= uintlen_t(bit_index_max)};
      if (rhs_lz < uintlen_t(bit_index_max) ||
          !rhs_bw) { // rhs shall not overflow, nor be zero
        return {false, cut_off};
      }
      rhs <<= uintlen_t(bit_index_max);
      if (cut_off) {
        for (intlen_t i{bit_index_max}; intlen_t(devide_out.n_bit()) <= i;) {
          bool is_bigger{rhs <= modulo_out_val_in};
          if (is_bigger) {
            modulo_out_val_in -= rhs;
          };
          i--;
          if (intlen_t(devide_out.n_bit()) <= i)
            rhs >>= uintlen_t(1);
        }
        bit_index_max = intlen_t(devide_out.n_bit());
      }
      for (intlen_t i{bit_index_max}; 0 <= i;) {
        bool is_bigger{rhs <= modulo_out_val_in};
        if (is_bigger) {
          devide_out.set_nth_bit(i, is_bigger);
          modulo_out_val_in -= rhs;
        };
        i--;
        if (0 <= i)
          rhs >>= uintlen_t(1);
      }
      // rhs is back to original
      return {true, cut_off};
    }
    MJZ_DEPRECATED_R("O(3*n) memory , use operator_modulo_and_devide")
    MJZ_CX_AL_FN uint_dyn_t &
    operator%=(uint_dyn_t<version_v, true> rhs) noexcept
      requires(!is_const_v)
    = delete;
    MJZ_DEPRECATED_R("O(3*n) memory , use operator_modulo_and_devide")
    MJZ_CX_AL_FN uint_dyn_t &
    operator/=(uint_dyn_t<version_v, true> rhs) noexcept
      requires(!is_const_v)
    = delete;
    MJZ_DEPRECATED_R("3*n) memory , use operator_muleq")
    MJZ_CX_AL_FN uint_dyn_t &
    operator*=(uint_dyn_t<version_v, true> rhs) noexcept
      requires(!is_const_v)
    = delete;

    MJZ_CX_AL_FN uint_dyn_t &operator*=(uint64_t amount) noexcept {
      using u128_t0_ = uintN_t<version_v, 128>;
      u128_t0_ carry_lvl0{};
      for (uintlen_t i{}; i < n_word(); i++) {
        uint64_t carry_lvl2{};
        carry_lvl2 += carry_lvl0.add(u128_t0_(nth_word(i)) * u128_t0_(amount));
        nth_word(i) =
            std::exchange(carry_lvl0.nth_word(0),
                          std::exchange(carry_lvl0.nth_word(1), carry_lvl2));
      }
      return *this;
    }
    // argument must not overlap (/*restrict span*/ )
    MJZ_CX_AL_FN pair_t<success_t, bool /*overflow-cutoff of devide_out*/>
    operator_modulo_in_to_devide(uint_dyn_t modulo_out_val_in,
                                 uint_dyn_t rhs) noexcept
      requires(!is_const_v)
    {
      return operator_modulo_and_devide(*this, modulo_out_val_in, rhs);
    }
    // argument must not overlap (/*restrict span*/ )
    MJZ_CX_AL_FN pair_t<success_t, bool /*overflow-cutoff of devide_out*/>
    operator_devide_to_modulo_in(uint_dyn_t devide_out, uint_dyn_t rhs) noexcept
      requires(!is_const_v)
    {
      return operator_modulo_and_devide(devide_out, *this, rhs);
    }

    // *ptr argument must not overlap (/*restrict span*/ )
    MJZ_CX_FN static uint_dyn_t
    operator_mul(uint64_t *MJZ_restrict ptr, uintlen_t word_n,
                 uint_dyn_t<version_v, true> lhs,
                 uint_dyn_t<version_v, true> rhs) noexcept
      requires(!is_const_v)
    {
      std::span<uint64_t> playground_space{ptr, word_n};
      uint_dyn_t dyn_int = playground_space;
      using u128_t0_ = uintN_t<version_v, 128>;
      uintlen_t rhs_bw = rhs.bit_width();
      uintlen_t lhs_bw = lhs.bit_width();
      rhs.shrink_to_width(rhs_bw);
      lhs.shrink_to_width(lhs_bw);
      dyn_int.shrink_to_width(rhs_bw + lhs_bw);
      if constexpr (version_v.is_BE()) {
        playground_space = playground_space.subspan(0, playground_space.size() -
                                                           dyn_int.n_word());
      } else {
        playground_space = playground_space.subspan(dyn_int.n_word());
      }
      u128_t0_ carry_lvl0{};
      for (uintlen_t k{}; k < dyn_int.n_word(); k++) {
        uint64_t carry_lvl2{};
        for (uintlen_t i{}; i <= k; i++) {
          uintlen_t j = k - i;
          if (j < rhs.n_word() && i < lhs.n_word())
            carry_lvl2 += carry_lvl0.add(u128_t0_(lhs.nth_word(i)) *
                                         u128_t0_(rhs.nth_word(j)));
        }
        dyn_int.nth_word(k) =
            std::exchange(carry_lvl0.nth_word(0),
                          std::exchange(carry_lvl0.nth_word(1), carry_lvl2));
      }
      return dyn_int;
    }
    MJZ_CX_AL_FN pair_t<uint_dyn_t, uint_dyn_t>
    split(uintlen_t n_low) const noexcept {
      n_low = std::min(n_low, n_word());
      if constexpr (version_v.is_BE()) {
        auto [high, low] = pair_t(words.subspan(0, n_word() - n_low),
                                  words.subspan(n_word() - n_low, n_low));
        return {low, high};
      } else {
        auto [low, high] =
            pair_t(words.subspan(0, n_low), words.subspan(n_low));
        return {low, high};
      }
    }

    MJZ_CX_AL_FN pair_t<uint_dyn_t, uint_dyn_t>
    split(/*middle*/) const noexcept {
      return split((n_word() + version_v.is_BE()) >> 1);
    }

    MJZ_CX_FN uint_dyn_t word_sr(uintlen_t n_words_) const noexcept {
      n_words_ = std::min(n_words_, words.size());
      if constexpr (version_v.is_BE()) {
        return words.subspan(0, words.size() - n_words_);
      } else {
        return words.subspan(n_words_);
      }
    }
    MJZ_CX_FN uint_dyn_t word_sl(uintlen_t n_words_) const noexcept {
      n_words_ = std::min(n_words_, words.size());
      if constexpr (version_v.is_LE()) {
        return words.subspan(0, words.size() - n_words_);
      } else {
        return words.subspan(n_words_);
      }
    }

    MJZ_CX_FN static uint_dyn_t
    operator_muladd(uint64_t *const MJZ_restrict ptr, const uintlen_t word_n,
                    uint_dyn_t<version_v, true> lhs,
                    uint_dyn_t<version_v, true> rhs,
                    uint64_t *const MJZ_restrict stack_ptr_,
                    const uintlen_t stack_word_size_) noexcept
      requires(!is_const_v)
    {
      std::span<uint64_t> playground_space =
          std::span<uint64_t>{stack_ptr_, stack_word_size_};
      using u128_t0_ = uintN_t<version_v, 128>;
      uint_dyn_t dyn_int = std::span<uint64_t>{ptr, word_n};
      {
        uintlen_t rhs_bw = rhs.bit_width();
        uintlen_t lhs_bw = lhs.bit_width();
        rhs.shrink_to_width(rhs_bw);
        lhs.shrink_to_width(lhs_bw);
        dyn_int.shrink_to_width(rhs_bw + lhs_bw);
      };

      enum operation_code_e : char {
        return_v = 0,
        muladd_v,
        lowh_callret,
        highh_callret,
        last_callret,
      };
      struct alignas(8) stack_frame_ {
        uint_dyn_t<version_v, true> lhs;
        uint_dyn_t<version_v, true> rhs;
        uint_dyn_t dyn_int;
        uint_dyn_t z3;
        stack_frame_ *stack_pointer{};
        operation_code_e op;
      };
      operation_code_e op{muladd_v};
      uint_dyn_t z3{};
      stack_frame_ *stack_pointer{};
      stack_frame_ callee_frame{};

      do {
        const uintlen_t middle = std::max(rhs.n_word(), lhs.n_word()) >> 1;
        switch (std::exchange(op, return_v)) {
        case muladd_v: {
          uintlen_t n_words_ =
              std::max(rhs.n_word() + lhs.n_word() + 2, dyn_int.n_word());
          if (std::min(rhs.n_word() + lhs.n_word(), dyn_int.n_word()) <= 4 ||
              playground_space.size() < n_words_ + (sizeof(stack_frame_) / 8)) {
            u128_t0_ carry_lvl0{};
            for (uintlen_t k{}; k < dyn_int.n_word(); k++) {
              uint64_t carry_lvl2{};
              for (uintlen_t i{}; i <= k; i++) {
                uintlen_t j = k - i;
                if (j < rhs.n_word() && i < lhs.n_word())
                  carry_lvl2 += carry_lvl0.add(u128_t0_(lhs.nth_word(i)) *
                                               u128_t0_(rhs.nth_word(j)));
              }

              uintN_t<version_v, 64> temp = std::array{std::exchange(
                  carry_lvl0.nth_word(0),
                  std::exchange(carry_lvl0.nth_word(1), carry_lvl2))};
              carry_lvl2 += carry_lvl0.add(u128_t0_{
                  uint64_t(temp.add(std::array{dyn_int.nth_word(k)}))});
              dyn_int.nth_word(k) = temp.nth_word(0);
            }
            break;
          } else {
            const auto [lowl0, highl0] = rhs.split(middle);
            const auto [lowr0, highr0] = lhs.split(middle);
            z3 = playground_space.subspan(playground_space.size() - n_words_);
            playground_space = playground_space.subspan(
                0, playground_space.size() - z3.n_word());
            z3.broadcast(uint64_t(-1));
            callee_frame.dyn_int = z3;
            callee_frame.lhs = lowl0;
            callee_frame.rhs = lowr0;
            op = lowh_callret;
          }
        }
        ////////////////////
        break;
        case lowh_callret: {
          const auto [lowl0, highl0] = rhs.split(middle);
          const auto [lowr0, highr0] = lhs.split(middle);
          z3.flip();
          callee_frame.dyn_int = z3.word_sr(middle);
          callee_frame.lhs = highl0;
          callee_frame.rhs = highr0;
          op = highh_callret;
        } break;
        case highh_callret: {
          const auto [lowl0, highl0] = rhs.split(middle);
          const auto [lowr0, highr0] = lhs.split(middle);
          const uintlen_t lhsn = std::max(lowl0.n_word(), highl0.n_word()) + 1;
          const uintlen_t rhsn = std::max(lowr0.n_word(), highr0.n_word()) + 1;
          callee_frame.dyn_int = dyn_int.word_sr(middle);
          auto old_z3 = z3;
          playground_space =
              std::span<uint64_t>(playground_space.data(),
                                  playground_space.size() + z3.words.size());
          z3 =
              playground_space.subspan(playground_space.size() - (rhsn + lhsn));
          playground_space = playground_space.subspan(
              0, playground_space.size() - z3.n_word());

          auto l = z3.word_sl(rhsn);
          auto r = z3.word_sr(lhsn);
          callee_frame.lhs = l;
          callee_frame.rhs = r;
          // now run everything
          dyn_int -= old_z3;
          callee_frame.dyn_int += old_z3;
          l.copy_from(highl0);
          r.copy_from(highr0);
          l += lowl0;
          r += lowr0;
          op = last_callret;
        } break;
        case last_callret: {
          playground_space =
              std::span<uint64_t>(playground_space.data(),
                                  playground_space.size() + z3.words.size());
        } break;
        case return_v:
          MJZ_FALLTHROUGH;
        default:
          asserts.unreachable();
          break;
        };
        if (op != return_v) {
          stack_frame_ caller_frame{.lhs{lhs},
                                    .rhs{rhs},
                                    .dyn_int{dyn_int},
                                    .z3{z3},
                                    .stack_pointer{stack_pointer},
                                    .op{op}};
          uint_dyn_t frame_store_ = playground_space.subspan(
              playground_space.size() - (sizeof(stack_frame_) / 8));
          uint64_t *ptr_stack_{frame_store_.words.data()};

          playground_space = playground_space.subspan(
              0, playground_space.size() - frame_store_.n_word());
          MJZ_IFN_CONSTEVAL {
            callee_frame.stack_pointer =
                new (ptr_stack_) stack_frame_{caller_frame};
          }
          else {
            callee_frame.stack_pointer = new stack_frame_{caller_frame};
          }
          callee_frame.z3 = uint_dyn_t{};
          callee_frame.op = muladd_v;
        } else {
          if (!stack_pointer)
            return dyn_int;
          callee_frame = *stack_pointer;
          MJZ_IFN_CONSTEVAL { std::destroy_at(stack_pointer); }
          else {
            delete stack_pointer;
          }

          playground_space = std::span<uint64_t>(
              playground_space.data(),
              playground_space.size() + (sizeof(stack_frame_) / 8));

          stack_pointer = nullptr;
        }
        std::swap(callee_frame.lhs, lhs);
        std::swap(callee_frame.rhs, rhs);
        std::swap(callee_frame.dyn_int, dyn_int);
        std::swap(callee_frame.z3, z3);
        std::swap(callee_frame.stack_pointer, stack_pointer);
        std::swap(callee_frame.op, op);
        callee_frame = stack_frame_{};
      } while (true);
      asserts.unreachable();
    }

    // *this argument must not overlap (/*restrict span*/ )
    MJZ_CX_AL_FN uint_dyn_t &
    operator_muleq(uint_dyn_t<version_v, true> lhs,
                   uint_dyn_t<version_v, true> rhs) noexcept
      requires(!is_const_v)
    {
      for (uintlen_t i{
               operator_mul(words.data(), words.size(), lhs, rhs).n_word()};
           i < words.size(); i++) {
        nth_word(size_t(i)) = 0;
      };
      return *this;
    }
    MJZ_CX_AL_FN uint_dyn_t &operator_muleqadd(
        uint_dyn_t<version_v, true> lhs, uint_dyn_t<version_v, true> rhs,
        std::span<uint64_t> stack = std::span<uint64_t>{}) noexcept
      requires(!is_const_v)
    {
      for (uintlen_t i{operator_muladd(words.data(), words.size(), lhs, rhs,
                                       stack.data(), stack.size())
                           .n_word()};
           i < words.size(); i++) {
        nth_word(size_t(i)) = 0;
      };
      return *this;
    }
    MJZ_CX_AL_FN void operator_sl(uintlen_t amount) noexcept
      requires(!is_const_v)
    {
      if (n_bit() <= amount) {
        for (uint64_t &w : words)
          w = 0;
        return;
      }
      uintlen_t word_zeroed{(amount) >> 6};
      uintlen_t word_working{words.size() - word_zeroed};
      amount &= 63;
      if constexpr (version_v.is_BE()) {
        memomve_overlap(words.data(), words.data() + word_zeroed, word_working);
        for (uint64_t &w : words.subspan(word_working))
          w = 0;
        if (amount) {
          uint64_t temp{};
          for (uint64_t &w :
               words.subspan(0, word_working) | std::views::reverse) {
            w = (w << amount) | std::exchange(temp, w >> (64 - amount));
          }
        }
      } else {
        memomve_overlap(words.data() + word_zeroed, words.data(), word_working);
        for (uint64_t &w : words.subspan(0, word_zeroed))
          w = 0;
        if (amount) {
          uint64_t temp{};
          for (uint64_t &w : words.subspan(word_zeroed)) {
            w = (w << amount) | std::exchange(temp, w >> (64 - amount));
          }
        }
      }
    }
    MJZ_CX_AL_FN void operator_sr(uintlen_t amount) noexcept
      requires(!is_const_v)
    {
      if (n_bit() <= amount) {
        for (uint64_t &w : words)
          w = 0;
        return;
      }
      uintlen_t word_zeroed{(amount) >> 6};
      uintlen_t word_working{words.size() - word_zeroed};
      amount &= 63;
      if constexpr (version_v.is_LE()) {
        memomve_overlap(words.data(), words.data() + word_zeroed, word_working);
        for (uint64_t &w : words.subspan(word_working))
          w = 0;
        if (amount) {
          uint64_t temp{};
          for (uint64_t &w :
               words.subspan(0, word_working) | std::views::reverse) {
            w = (w >> amount) | std::exchange(temp, w << (64 - amount));
          }
        }
      } else {
        memomve_overlap(words.data() + word_zeroed, words.data(), word_working);
        for (uint64_t &w : words.subspan(0, word_zeroed))
          w = 0;
        if (amount) {
          uint64_t temp{};
          for (uint64_t &w : words.subspan(word_zeroed)) {
            w = (w >> amount) | std::exchange(temp, w << (64 - amount));
          }
        }
      }
    }
    MJZ_CX_AL_FN void to_bit_ceil() noexcept
      requires(!is_const_v)
    {
      if (!words.size())
        return;
      bool low_had_value{};
      intlen_t last_byte_value_i{};
      uint64_t last_byte_value{};
      for (intlen_t i{intlen_t(words.size()) - 1}; 0 <= i; i--) {
        if (!nth_word(i)) {
          continue;
        }
        last_byte_value_i = i;
        last_byte_value = std::exchange(nth_word(i), 0);
        i--;
        for (; 0 <= i; i--) {
          low_had_value |= !std::exchange(nth_word(i), 0);
        }
      }
      last_byte_value += low_had_value;
      nth_word(last_byte_value_i) = std::bit_ceil(last_byte_value);
      return;
    }
    MJZ_CX_AL_FN void to_bit_floor() noexcept
      requires(!is_const_v)
    {

      for (intlen_t i{intlen_t(words.size()) - 1}; 0 <= i; i--) {
        if (!nth_word(i)) {
          continue;
        }
        nth_word(i) = bit_floor(nth_word(i));
        i--;
        for (; 0 <= i; i--) {
          nth_word(i) = 0;
        }
        break;
      }
    }
    MJZ_CX_AL_FN void to_rotl(intlen_t r) noexcept
      requires(!is_const_v)
    {
      return to_rotr(-r);
    }
    MJZ_CX_AL_FN void to_rotr(intlen_t r) noexcept
      requires(!is_const_v)
    {
      uintlen_t n_bits_ = words.size() * 64;

      if (n_bits_ <= uintlen_t(std::max(r, -r)))
        r %= intlen_t(n_bits_);
      if (!r)
        return;
      if (r < 0) {
        r = intlen_t(words.size()) + r;
      }
      uint64_t bit_rot = uint64_t(r) & 63;
      for (uint64_t &w : words) {
        w = std::rotr(w, int(bit_rot));
      }
      std::ranges::rotate(words, words.begin() + (uint64_t(r) >> 6));
    }
    MJZ_CX_AL_FN void to_byteswap() noexcept
      requires(!is_const_v)
    {
      // mem_byteswap(words.data(),words.byte_size())
      for (uint64_t &w : words) {
        w = byteswap(w);
      }
      std::ranges::reverse(words);
    }
    MJZ_CX_AL_FN void to_bitswap() noexcept
      requires(!is_const_v)
    {
      // mem_bitswap(words.data(),words.byte_size())
      for (uint64_t &w : words) {
        w = bitswap(w);
      }
      std::ranges::reverse(words);
    }

    MJZ_CX_AL_FN uintlen_t countl_one(uintlen_t offset) const noexcept {
      auto temp = word_sl(offset >> 6);
      if (!temp.words.size())
        return 0;
      uintlen_t cnt = uint_ex_t::countl_one(
          temp.nth_word(temp.words.size() - 1), (offset &= 63));
      temp = temp.word_sl(1);
      if (64 != cnt + offset)
        return cnt;
      return cnt + temp.countl_one();
    }
    MJZ_CX_AL_FN uintlen_t countl_zero(uintlen_t offset) const noexcept {
      auto temp = word_sl(offset >> 6);
      if (!temp.words.size())
        return 0;
      uintlen_t cnt = uint_ex_t::countl_zero(
          temp.nth_word(temp.words.size() - 1), (offset &= 63));
      temp = temp.word_sl(1);
      if (64 != cnt + offset)
        return cnt;
      return cnt + temp.countl_zero();
    }
    MJZ_CX_AL_FN uintlen_t countr_one(uintlen_t offset) const noexcept {
      auto temp = word_sr(offset >> 6);
      if (!temp.words.size())
        return 0;
      uintlen_t cnt = uint_ex_t::countr_one(temp.nth_word(0), (offset &= 63));
      temp = temp.word_sr(1);
      if (64 != cnt + offset)
        return cnt;
      return cnt + temp.countr_one();
    }
    MJZ_CX_AL_FN uintlen_t countr_zero(uintlen_t offset) const noexcept {
      auto temp = word_sr(offset >> 6);
      if (!temp.words.size())
        return 0;
      uintlen_t cnt = uint_ex_t::countr_zero(temp.nth_word(0), (offset &= 63));
      temp = temp.word_sr(1);
      if (64 != cnt + offset)
        return cnt;
      return cnt + temp.countr_zero();
    }
    MJZ_CX_AL_FN uintlen_t
    popcount(uintlen_t offset, uintlen_t count = uintlen_t(-1)) const noexcept {
      if (n_bit() < count)
        count = n_bit();
      if (n_bit() <= offset || !count)
        return 0;

      auto temp = word_sr(offset >> 6);
      temp = word_sl(temp.n_word() - ((count + 63) >> 6));
      offset &= ~uintlen_t(63);
      uintlen_t cnt = uint_ex_t::popcount(temp.nth_word(0), offset, count);
      temp = temp.word_sr(1);
      count -= 64 - offset;
      if (count & 63) {
        cnt += uint_ex_t::popcount(temp.nth_word(count >> 6), 0, count & 63);
        temp.word_sl(1);
        count &= ~uintlen_t(63);
      }
      return cnt + temp.popcount();
    }

    template <bool set>
    MJZ_CX_AL_FN void set_bit_patch(uintlen_t index, uintlen_t count) noexcept
      requires(!is_const_v)
    {

      if (!count)
        return;
      uintlen_t i{index};
      uintlen_t cnt = count + index;
      uint64_t mask{};
      if (i & 63) {
        mask = ((uint64_t(-1) >> (64 - std::min<uintlen_t>(cnt - i, 64))))
               << (i & 63);
        if constexpr (set) {
          nth_word(i >> 6) |= mask;
        } else {

          nth_word(i >> 6) &= ~mask;
        }
        i += 63;
        i &= ~uint64_t(63);
        if (cnt <= i)
          return;
      }
      uintlen_t j = (i >> 6);
      uintlen_t sz = ((cnt - i) >> 6);
      i += sz << 6;
      sz += j;
      if (i != cnt) {
        mask = uint64_t(-1) >> (64 - (cnt - i));
        if constexpr (set) {
          nth_word(i >> 6) |= mask;
        } else {
          nth_word(i >> 6) &= ~mask;
        }
      }
      mask = uint64_t(-int64_t(set));

      for (; j < sz; j++) {
        nth_word(j) = mask;
      }
    }
    template <bool needle, bool smallest = true, bool right_to_left = true>
    MJZ_CX_AL_FN uintlen_t
    find_bit_patch(uintlen_t bit_needle_cound, uintlen_t bit_hey_offset = 0,
                   uintlen_t bit_hey_count = uintlen_t(-1)) const noexcept {
      uintlen_t count_blk{smallest ? uintlen_t(-1) : 0};
      uintlen_t index_blk{};
      bit_hey_count = std::min(bit_hey_count, n_bit());
      bit_hey_offset =
          right_to_left ? bit_hey_offset : n_bit() - 1 - bit_hey_offset;
      for (uintlen_t i{bit_hey_offset}; i < bit_hey_count;) {
        uintlen_t len{};
        if (!needle) {
          len = right_to_left ? countr_zero(i) : countl_zero(i);
        } else {
          len = right_to_left ? countr_one(i) : countl_one(i);
        }
        len = std::min(bit_hey_count, len + i) - i;
        if (bit_needle_cound <= len &&
            (smallest ? len < count_blk : count_blk < len)) {
          count_blk = len;
          index_blk = i;
          if (smallest && len == bit_needle_cound)
            break;
        }
        i += len;
        if (needle) {
          i += right_to_left ? countr_zero(i) : countl_zero(i);
        } else {
          i += right_to_left ? countr_one(i) : countl_one(i);
        }
      }
      if (count_blk == (smallest ? uintlen_t(-1) : 0))
        return uintlen_t(-1);
      return right_to_left ? index_blk : n_bit() - 1 - index_blk;
    }

    // log()+1 or log()+2 or for 0, 1
    MJZ_CX_AL_FN static uintlen_t
    dec_width_aprox_ceil(uintlen_t bit_width_) noexcept {
      // IEEE 754
      constexpr uint64_t log2_v_ =
          std::bit_cast<uint64_t>(0.30102999566398119521373889472449);
      int shift = std::bit_width(bit_width_);
      uintlen_t log2_muled_ceil =
          uintlen_t(std::bit_cast<double>(log2_v_ + (uint64_t(shift) << 52))) +
          1;
      uintlen_t log_or_1_plus_log = ((log2_muled_ceil * bit_width_) >> shift);
      return log_or_1_plus_log + 1;
    }
    MJZ_CX_AL_FN static uintlen_t
    bit_width_aprox_ceil(uintlen_t dec_width_) noexcept {
      return 1 +
             uintlen_t(double(dec_width_) * 3.3219280948873623478703194294894);
    }
  };

  template <version_t version_v, uintlen_t n_bits>
    requires(64 * (n_bits / 64) == n_bits && !!n_bits)
  MJZ_CX_AL_FN uint_dyn_t<version_v, true>
  uintN_t<version_v, n_bits>::as_dyn() const noexcept {
    return std::span<const uint64_t>(words);
  }
  template <version_t version_v, uintlen_t n_bits>
    requires(64 * (n_bits / 64) == n_bits && !!n_bits)
  MJZ_CX_AL_FN uint_dyn_t<version_v, false>
  uintN_t<version_v, n_bits>::as_dyn() noexcept {
    return std::span<uint64_t>(words);
  }
  template <version_t version_v>
  MJZ_CX_FN uint_dyn_t<version_v, false>
  pow10_dyn_uint_u64(std::span<uint64_t> out_and_temp, uintlen_t pow) noexcept {
    if (out_and_temp.size() * 6 < pow) // low memory
      return {};
    for (uint64_t &w : out_and_temp)
      w = 0;
    uint_dyn_t<version_v, false> out{out_and_temp.subspan(
        0,
        (uint_dyn_t<version_v, false>::bit_width_aprox_ceil(pow) + 63) >> 6)};
    std::span<uint64_t> temp{out_and_temp.subspan(out.n_word())};
    out_and_temp = temp;
    uint_dyn_t<version_v, false> temp_pow{temp.subspan(0, out.n_word())};
    temp = temp.subspan(out.n_word());
    temp_pow.nth_word(0) = 5;
    out.set_nth_bit(pow, true);
    do {
      if (pow & 1) {
        uint_dyn_t<version_v, false> temp_out_cpy{temp};
        out.operator_muleq(temp_out_cpy.copy_from(out), temp_pow);
      }
      pow >>= 1;
      if (!pow)
        break;
      uint_dyn_t<version_v, false> temp_pow_cpy{temp};
      temp_pow_cpy.copy_from(temp_pow);
      temp_pow.operator_muleq(temp_pow_cpy, temp_pow_cpy);
    } while (true);
    return out;
  }

  template <version_t version_v>
  struct conert_extended_dyn_uint_to_base10p8_u64_t {
    uint64_t *begin_of_buf_pointer{};
    std::span<uint64_t> output{};
    std::span<uint64_t> val_buffer{};
    uint_dyn_t<version_v, false> val_input{};
    uintlen_t max_dec_width{};
    uintlen_t bw{};
    uint64_t *old_frame_ptr{};

  private:
    struct offset_span_t {
      intlen_t offset;
      uintlen_t size;
    };
    struct frame_obj_t {
      int64_t old_frame_ptr{};
      offset_span_t output{};
      offset_span_t val_buffer{};
      offset_span_t val_input{};
      uintlen_t max_dec_width{};
      uintlen_t bw{};
    };
    MJZ_CX_AL_FN offset_span_t conv(std::span<uint64_t> s) const noexcept {
      return {s.data() - begin_of_buf_pointer, s.size()};
    }
    MJZ_CX_AL_FN std::span<uint64_t> conv(offset_span_t s) const noexcept {
      return {s.offset + begin_of_buf_pointer, s.size};
    }
    MJZ_CX_AL_FN success_t
    push_frame(conert_extended_dyn_uint_to_base10p8_u64_t frame) noexcept {
      if (old_frame_ptr)
        return false;
      if (val_buffer.size() < sizeof(frame_obj_t) / 8)
        return false;
      old_frame_ptr = val_buffer.data();
      val_buffer = val_buffer.subspan(sizeof(frame_obj_t) / 8);
      frame_obj_t frame_obj{
          .old_frame_ptr{frame.old_frame_ptr
                             ? frame.old_frame_ptr - frame.begin_of_buf_pointer
                             : -1},
          .output{conv(frame.output)},
          .val_buffer{conv(frame.val_buffer)},
          .val_input{conv(frame.val_input.words)},
          .max_dec_width{frame.max_dec_width},
          .bw{frame.bw}};
      auto b = std::bit_cast<std::array<uint64_t, sizeof(frame_obj_t) / 8>>(
          frame_obj);
      auto *p{old_frame_ptr};
      for (uint64_t u : b) {
        *p++ = u;
      }
      return true;
    }
    MJZ_CX_AL_FN success_t pop_frame() noexcept {
      if (!old_frame_ptr) {
        return true;
      }
      frame_obj_t frame{};
      auto b =
          std::bit_cast<std::array<uint64_t, sizeof(frame_obj_t) / 8>>(frame);
      auto *p{old_frame_ptr};
      for (uint64_t &u : b) {
        u = *p++;
      }
      frame = std::bit_cast<frame_obj_t>(b);
      *this = conert_extended_dyn_uint_to_base10p8_u64_t{
          .begin_of_buf_pointer{begin_of_buf_pointer},
          .output{conv(frame.output)},
          .val_buffer{conv(frame.val_buffer)},
          .val_input{conv(frame.val_input)},
          .max_dec_width{frame.max_dec_width},
          .bw{frame.bw},
          .old_frame_ptr{frame.old_frame_ptr < 0
                             ? nullptr
                             : frame.old_frame_ptr + begin_of_buf_pointer}};
      return false;
    }

    MJZ_CX_AL_FN success_t quick_small_conv() noexcept {
      if (!max_dec_width)
        return true;
      constexpr uint64_t v10p8 = 100000000;
      constexpr uint64_t v10p16 = v10p8 * v10p8;
      constexpr uint64_t bw10p8 = bit_width(v10p8);
      constexpr uint64_t bw10p16 = bit_width(v10p16);
      if (max_dec_width <= 8) {
        asserts(asserts.assume_rn, bw <= bw10p8);
        uint64_t res = uint64_t(val_input);
        output[0] = res;
        return true;
      }
      if (max_dec_width <= 16) {
        asserts(asserts.assume_rn, bw <= bw10p16);
        uint64_t res = uint64_t(val_input);
        asserts(asserts.assume_rn, res < v10p16);
        if (bw10p8 <= bw) {
          output[0] = res / v10p8;
          res %= v10p8;
        }
        output[1] = res;
        return true;
      }
      if (bw <= 64) {
        uint64_t res = uint64_t(val_input);
        if (bw10p16 <= bw) {
          output[0] = res / v10p16;
          res %= v10p16;
        }
        asserts(asserts.assume_rn, res < v10p16);
        if (bw10p8 <= bw) {
          output[1] = res / v10p8;
          res %= v10p8;
        }
        output[2] = res;
        return true;
      }
      return false;
    }

    MJZ_CX_AL_FN success_t
    big_conv_impl(uint_dyn_t<version_v, false> val_devide_out,
                  uintlen_t pow_of_ten_index) noexcept {
      uint_dyn_t<version_v, false> dyn_pow_div =
          pow10_dyn_uint_u64<version_v>(val_buffer, pow_of_ten_index);
      if (!dyn_pow_div.n_word())
        return false;

      if (val_devide_out.n_word() < dyn_pow_div.n_word())
        return false;

      val_buffer = val_buffer.subspan(val_devide_out.n_word());
      uint_dyn_t<version_v, false> dyn_pow_dest =
          val_buffer.subspan(0, val_devide_out.n_word());
      for (uint64_t &w : dyn_pow_dest.words)
        w = 0;
      dyn_pow_dest.copy_from(std::exchange(dyn_pow_div, {}));
      std::span<uint64_t> val_buffer_old_0 = std::exchange(
          val_buffer, val_buffer.subspan(val_devide_out.n_word()));
      for (uint64_t &w : val_devide_out.words)
        w = 0;

      val_input.shrink_to_width(bw);
      auto [good_, cuted_] =
          uint_dyn_t<version_v, false>::operator_modulo_and_devide(
              val_devide_out, val_input, dyn_pow_dest);
      if (!good_ || cuted_)
        return false;

      val_buffer = val_buffer_old_0;
      return true;
    }
    MJZ_CX_AL_FN success_t big_conv() noexcept {
      std::span<uint64_t> val_buffer_old = val_buffer;
      uintlen_t pow_of_ten_index_wide = (max_dec_width / 16);
      uintlen_t pow_of_ten_index = pow_of_ten_index_wide * 8;
      if (val_buffer.size() < (pow_of_ten_index_wide + 1) * 2)
        return false;
      uint_dyn_t<version_v, false> val_devide_out{
          val_buffer.subspan(0, pow_of_ten_index_wide + 1)};
      if (!big_conv_impl(val_devide_out, pow_of_ten_index))
        return false;
      auto other_routine = conert_extended_dyn_uint_to_base10p8_u64_t(
          begin_of_buf_pointer,
          output.subspan(0, output.size() - pow_of_ten_index_wide), val_buffer,
          val_devide_out, max_dec_width - pow_of_ten_index,
          val_devide_out.bit_width());
      bw = val_input.bit_width();
      max_dec_width = pow_of_ten_index;
      val_buffer = val_buffer_old;
      output = output.subspan(output.size() - pow_of_ten_index_wide);
      return push_frame(std::exchange(*this, other_routine));
    }

  public:
    MJZ_CX_AL_FN success_t run() noexcept {
      while (true) {
        if (quick_small_conv()) {
          if (pop_frame()) {
            return true;
          }
          continue;
        }
        if (!big_conv())
          MJZ_IS_UNLIKELY
        return false;
      };
      return true;
    }
  };

  template <version_t version_v>
  MJZ_CX_FN uintlen_t /*byte count*/
  to_ascii_impl_(uint_dyn_t<version_v, true> input,
                 std::span<uint64_t> aligned8_output_and_temp_buffer) noexcept {
    if (!aligned8_output_and_temp_buffer.size())
      return {};
    uintlen_t bw = input.bit_width();
    if (bw == 0) {
      aligned8_output_and_temp_buffer[0] = details_ns::ascii_offset;
      return 1;
    }

    uint64_t *const beg_of_buf_ptr{aligned8_output_and_temp_buffer.data()};
    input.shrink_to_width(bw);
    uintlen_t max_dec_width = input.dec_width_aprox_ceil(bw);
    uintlen_t out_buf_width = (max_dec_width + 7) >> 3;
    if (aligned8_output_and_temp_buffer.size() < out_buf_width + input.n_word())
      return {};
    std::span<uint64_t> vals =
        aligned8_output_and_temp_buffer.subspan(0, out_buf_width);

    uint_dyn_t<version_v, false> input_ =
        aligned8_output_and_temp_buffer.subspan(out_buf_width, input.n_word());
    input_.copy_from(input);
    for (uint64_t &elem : vals) {
      elem = 0;
    }
    if (!conert_extended_dyn_uint_to_base10p8_u64_t<version_v>(
             beg_of_buf_ptr, vals,
             aligned8_output_and_temp_buffer.subspan(out_buf_width +
                                                     input.n_word()),
             input_, max_dec_width, bw)
             .run()) {
      return {};
    }
    while (!vals[0])
      vals = vals.subspan(1);
    vectorizable_conv_base_impl<version_v>.base10p8_to_base10_u64(vals);
    uint64_t first = (vals[0] & ~details_ns::ascii_offset);
    uintlen_t count = vals.size() * 8;
    uintlen_t offset{};
    if constexpr (std::endian::big == std::endian::native) {
      offset = countl_zero(first) >> 3;

      uint_dyn_t<version_v, false>{vals} <<= offset << 3;
    } else {
      offset = countr_zero(first) >> 3;
      uint_dyn_t<version_v, false>{vals} >>= offset << 3;
    }
    count -= offset;
    memomve_overlap(beg_of_buf_ptr, vals.data(), vals.size());
    return count;
  }

  template <version_t version_v>
  MJZ_CX_FN uint_dyn_t<version_v, false>
  from_ascii_impl_(uint_dyn_t<version_v, false> uninit_output_and_temp_buffer,
                   std::span<const char> ascii_input) noexcept {
    while (ascii_input.size() && ascii_input[0] == '0')
      ascii_input = ascii_input.subspan(1);
    uintlen_t sz16in = ascii_input.size() >> 4;
    uintlen_t ret_wsz = (ascii_input.size() + 15) >> 4;
    const auto ret0 = uninit_output_and_temp_buffer;
    auto ret = ret0;
    ret.broadcast(uint64_t());
    ret.shrink_to_width(ret_wsz * 64);
    uintlen_t szm16in = ascii_input.size() & 15;
    constexpr auto f = vectorizable_conv_base_impl<version_v>.b10p1_2_10p8;
    uintlen_t i{};
    if (!ret_wsz)
      return ret0;
    if (szm16in) {

      uint64_t word{};
      std::array<char, 16> temp{};
      memcpy(16 - szm16in + temp.data(), ascii_input.data(), szm16in);
      auto [w0, w1] = std::bit_cast<std::array<uint64_t, 2>>(temp);
      word = f(w0) * uint64_t(100000000) + f(w1);
      ret += uint_dyn_t<version_v, false>{std::span<uint64_t>{&word, 1}};
      if (sz16in)
        ret *= uint64_t(10000000000000000);
    }
    if (sz16in) {
      do {
        uint64_t word{};
        auto [w0, w1] = cpy_bitcast<std::array<uint64_t, 2>>(
            ascii_input.data() + szm16in + i * 16);
        word = f(w0) * uint64_t(100000000) + f(w1);
        ret += uint_dyn_t<version_v, false>{std::span<uint64_t>{&word, 1}};
        i++;
        if (i == sz16in)
          break;
        ret *= uint64_t(10000000000000000);
      } while (true);
    }
    return ret0;
  }
  template <version_t version_v, uintlen_t n_bits>
    requires(64 * (n_bits / 64) == n_bits && !!n_bits)
  MJZ_CX_AL_FN void uintN_t<version_v, n_bits>::from_ascii(
      std::span<const char> ascii_input) noexcept {
    *this = uintN_t<version_v, n_bits>(
        from_ascii_impl_(as_dyn(), ascii_input).n_word());
  }
  }; // namespace uint_ex_t
};
#endif