
#include "mjz_uintconv.hpp"
#include "traits.hpp"
#include "tuple.hpp"
#include "versions.hpp"
#ifndef MJZ_LIB_FILE_UINTN_HPP_
#define MJZ_LIB_FILE_UINTN_HPP_

MJZ_EXPORT namespace mjz {
  template <class T>
  concept fixed_extended_ingeral_c = requires() {
    typename std::remove_cvref_t<
        T>::mjz_uintN_t_id_val_t_2354675648764874753789;
  };
  template <class T>
  concept dyn_extended_ingeral_c = requires() {
    typename std::remove_cvref_t<
        T>::mjz_uint_dyn_t_id_val_t_2354675648764874753789;
  };
  template <class T>
  concept extended_ingeral_c =
      dyn_extended_ingeral_c<T> || fixed_extended_ingeral_c<T>;
  template <version_t version_v, bool is_const_v> struct uint_dyn_t;
  template <version_t version_v, uintlen_t n_bits>
  MJZ_CX_FN uintlen_t to_ascii_impl_pv_uintN_conv(
      std::span<uint64_t, n_bits / 64> var, char *output, uintlen_t len,
      uintlen_t bw) noexcept;
  template <version_t version_v, uintlen_t n_bits>
    requires(64 * (n_bits / 64) == n_bits && !!n_bits)
  struct uintN_t {
    MJZ_MCONSTANT(uintlen_t) word_count = n_bits / 64;
    alignas(std::min<uintlen_t>(
        hardware_constructive_interference_size,
        log2_of_val_to_val(uint8_t(std::countr_zero(word_count * 8)))))
        std::array<uint64_t, word_count> words{};
    MJZ_DISABLE_ALL_WANINGS_START_;
    MJZ_DEFAULTED_CLASS(uintN_t);
    MJZ_DISABLE_ALL_WANINGS_END_;
    using mjz_uintN_t_id_val_t_2354675648764874753789 = uintN_t;

    MJZ_CX_AL_FN uintN_t(std::array<uint64_t, word_count> w) noexcept
        : words{w} {}

  private:
    MJZ_CX_AL_FN uintN_t(void_struct_t, uintlen_t) noexcept : uintN_t{} {}

    template <std::integral T, std::integral... Ts>
    MJZ_CX_AL_FN uintN_t(void_struct_t, const uintlen_t i, T lowest,
                         Ts... args) noexcept
        : uintN_t{void_struct_t{}, i + 1, args...} {
      nth_word(i) = lowest;
    }

  public:
    template <uintlen_t n2_bits>
    MJZ_CX_AL_FN explicit uintN_t(
        const uintN_t<version_v, n2_bits> &other) noexcept
        : uintN_t{} {
      for (uintlen_t i{}; i < std::min(other.word_count, word_count); i++) {
        nth_word(i) = other.nth_word(i);
      }
    }

    template <std::integral... Ts>
      requires(sizeof...(Ts) <= word_count)
    MJZ_CX_AL_FN explicit uintN_t(/*low to high*/ Ts... args) noexcept
        : uintN_t{void_struct_t{}, 0, uint64_t(args)...} {}

    MJZ_CX_AL_FN uint64_t &nth_word(auto i) noexcept {
      if constexpr (version_v.is_LE()) {
        return words[size_t(i)];
      } else {
        return words[size_t((word_count - 1) - i)];
      }
    }
    MJZ_CX_AL_FN const uint64_t &nth_word(auto i) const noexcept {
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
      uint64_t &word = nth_word(i >> 6);
      word &= ~mask;
      word |= val ? mask : 0;
    }
    MJZ_CX_AL_FN void operator_sr(uintlen_t amount) noexcept {
      bool zero_out = amount < n_bits;
      const uint64_t zero_out_mask64 = (~uint64_t(zero_out)) + 1;
      for (uint64_t &word : words)
        word &= zero_out_mask64;
      amount &= zero_out_mask64;
      intlen_t abs_amount = intlen_t(amount);
      intlen_t internal_amount{abs_amount & 63};
      intlen_t external_amount{abs_amount >> 6};
      MJZ_JUST_ASSUME_(external_amount < intlen_t(word_count));
      MJZ_JUST_ASSUME_(amount < n_bits);
      const uint64_t ub_annoying_mask = (~uint64_t(internal_amount != 0)) + 1;

      uintN_t upper_half{*this}, lower_half{*this};
      for (uint64_t &word : upper_half.words)
        word = (word >> internal_amount);
      internal_amount |= 0 == internal_amount;
      for (uint64_t &word : lower_half.words)
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
    MJZ_CX_AL_FN uintN_t &operator>>=(uintlen_t amount) noexcept {
      operator_sr(amount);

      return *this;
    }

    MJZ_CX_AL_FN void operator_sl(uintlen_t amount) noexcept {
      bool zero_out = amount < n_bits;
      const uint64_t zero_out_mask64 = (~uint64_t(zero_out)) + 1;
      for (uint64_t &word : words)
        word &= zero_out_mask64;
      amount &= zero_out_mask64;
      intlen_t abs_amount = intlen_t(amount);
      intlen_t internal_amount{abs_amount & 63};
      intlen_t external_amount{abs_amount >> 6};
      MJZ_JUST_ASSUME_(external_amount < intlen_t(word_count));
      MJZ_JUST_ASSUME_(amount < n_bits);
      const uint64_t ub_annoying_mask = (~uint64_t(internal_amount != 0)) + 1;
      uintN_t upper_half{*this}, lower_half{*this};
      for (uint64_t &word : lower_half.words)
        word = (word << internal_amount);
      internal_amount |= 0 == internal_amount;
      for (uint64_t &word : upper_half.words)
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
    MJZ_CX_AL_FN uintN_t &operator<<=(uintlen_t amount) noexcept {
      operator_sl(amount);
      return *this;
    }
    MJZ_CX_AL_FN uintN_t &operator&=(const uintN_t amount) noexcept {
      for (intlen_t i{}; i < intlen_t(word_count); i++) {
        words[size_t(i)] &= amount.words[size_t(i)];
      }
      return *this;
    }
    MJZ_CX_AL_FN uintN_t &operator|=(const uintN_t amount) noexcept {
      for (intlen_t i{}; i < intlen_t(word_count); i++) {
        words[size_t(i)] |= amount.words[size_t(i)];
      }
      return *this;
    }
    MJZ_CX_AL_FN uintN_t &operator^=(const uintN_t amount) noexcept {
      for (intlen_t i{}; i < intlen_t(word_count); i++) {
        words[size_t(i)] ^= amount.words[size_t(i)];
      }
      return *this;
    }

    MJZ_CX_AL_FN uintN_t &flip() noexcept {
      for (intlen_t i{}; i < intlen_t(word_count); i++) {
        nth_word(i) = ~nth_word(i);
      }
      return *this;
    }

    MJZ_CX_AL_FN bool add(const uintN_t &amount, bool carry = false) noexcept {
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
    MJZ_CX_AL_FN uintN_t &negate() noexcept {
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
    MJZ_CX_AL_FN bool minus(const uintN_t &amount,
                            bool carry = false) noexcept {
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
    MJZ_CX_AL_FN uintN_t &operator-=(const uintN_t &amount) noexcept {
      minus(amount);
      return *this;
    }
    MJZ_CX_AL_FN uintN_t &operator+=(const uintN_t &amount) noexcept {
      add(amount);
      return *this;
    }
    template <int = 0>
      requires(1 < word_count)
    MJZ_CX_AL_FN auto split() const noexcept

    {
      using ret_t = pair_t<uintN_t<version_v, ((word_count) / 2) * 64>,
                           uintN_t<version_v, ((word_count + 1) / 2) * 64>>;
      if constexpr (version_v.is_BE()) {
        auto [high, low] =
            std::bit_cast<pair_t<std::array<uint64_t, ((word_count + 1) / 2)>,
                                 std::array<uint64_t, ((word_count) / 2)>>>(
                words);
        return ret_t{low, high};
      } else {
        auto [low, high] =
            std::bit_cast<pair_t<std::array<uint64_t, ((word_count) / 2)>,
                                 std::array<uint64_t, ((word_count + 1) / 2)>>>(
                *this);
        return ret_t{low, high};
      }
    }
    template <int = 0>
      requires(1 == word_count)
    MJZ_CX_AL_FN pair_t<uint32_t, uint32_t> split() const noexcept {
      return {uint32_t(words[0]), uint32_t(words[0] >> 32)};
    }
    // buggy rn
    MJZ_CX_FN auto acumilate_halfs() const noexcept {
      using ret_t = uintN_t<version_v, ((word_count + 1) / 2) * 64 + 64>;
      auto [l, h] = split();
      return ret_t{l} + ret_t{h};
    }
    MJZ_CX_FN uintN_t<version_v, n_bits * 2>
    operator_mul_ret(const uintN_t &amount) const noexcept {
      using ret_t = uintN_t<version_v, n_bits * 2>;
      if constexpr (word_count == 1) {
        using u128_t0_ = uintN_t<version_v, 128>;
        return u128_t0_(*this) * u128_t0_{amount};
      } else if constexpr (word_count <= 4) {
        using u128_t0_ = uintN_t<version_v, 128>;
        ret_t ret{};
        u128_t0_ carry_lvl0{};
        for (uintlen_t k{}; k < ret.word_count; k++) {
          uint64_t carry_lvl2{};
          for (uintlen_t i{}; i <= k; i++) {
            uintlen_t j = k - i;
            if (i < word_count && j < amount.word_count)
              carry_lvl2 += carry_lvl0.add(u128_t0_(amount.nth_word(j)) *
                                           u128_t0_(nth_word(i)));
          }
          ret.nth_word(k) =
              std::exchange(carry_lvl0.nth_word(0),
                            std::exchange(carry_lvl0.nth_word(1), carry_lvl2));
        }
        return ret;
      } else {
        // Karatsuba multiplication
        const auto [lowl0, highl0] = split();
        const auto [lowr0, highr0] = amount.split();
        const auto z1 =
            ret_t(acumilate_halfs().operator_mul_ret(amount.acumilate_halfs()));
        const auto z0 = ret_t(lowl0.operator_mul_ret(lowr0));
        const auto z2 = ret_t(highr0.operator_mul_ret(highl0));
        const auto n1 = lowr0.word_count * 64;
        const auto n2 = n1 * 2;
        return (z2 << n2) + ((z1 - (z2 + z0)) << n1) + (z0);
      }
    }
    MJZ_CX_AL_FN uintN_t &operator*=(const uintN_t &amount) noexcept {
      if constexpr (word_count == 1) {
        return *this = uintN_t(nth_word(0) * amount.nth_word(0));
      } else if constexpr (word_count == 2) {

#ifdef MJZ_uint128_t_impl_t_
        return *this = std::bit_cast<uintN_t>(
                   std::bit_cast<MJZ_uint128_t_impl_t_>(amount) *
                   std::bit_cast<MJZ_uint128_t_impl_t_>(*this));
#endif //  MJZ_uint128_t_impl_t_

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
      } else if constexpr (0) {
        uintN_t ret{};
        for (uintlen_t i{}; i < n_bits; i++) {
          if (nth_bit(i)) {
            ret += amount << i;
          }
        }

        return *this = ret;
      } else if constexpr (0) {

        uintN_t ret{};
        using u128_t0_ = uintN_t<version_v, 128>;
        for (uintlen_t i{}; i < word_count; i++) {
          for (uintlen_t j{}; j < word_count; j++) {
            if (i + j < word_count) {
              u128_t0_ x{nth_word(i)};
              x *= u128_t0_(amount.nth_word(j));
              uintN_t temp_{};
              temp_.nth_word(i + j) = x.nth_word(0);
              if (i + j + 1 < word_count) {
                temp_.nth_word(i + j + 1) = x.nth_word(1);
              }
              ret += temp_;
            }
          }
        }
        return *this = ret;
      } else if constexpr (word_count <= 4) { // convolotion
        using u128_t0_ = uintN_t<version_v, 128>;
        uintN_t ret{};
        u128_t0_ carry_lvl0{};
        for (uintlen_t k{}; k < word_count; k++) {
          uint64_t carry_lvl2{};
          for (uintlen_t i{}; i <= k; i++) {
            uintlen_t j = k - i;
            carry_lvl2 += carry_lvl0.add(u128_t0_(amount.nth_word(j)) *
                                         u128_t0_(nth_word(i)));
          }

          ret.nth_word(k) =
              std::exchange(carry_lvl0.nth_word(0),
                            std::exchange(carry_lvl0.nth_word(1), carry_lvl2));
        }

        return *this = ret;
      } else {
        auto [lowl0, highl0] = operator_mul_ret(amount).split();
        return *this = lowl0;
      }
    }
    MJZ_CX_AL_FN std::strong_ordering
    operator<=>(const uintN_t &rhs) const noexcept {
      for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
        std::strong_ordering order = nth_word(i) <=> rhs.nth_word(i);
        if (order != std::strong_ordering::equal) {
          return order;
        }
      }
      return std::strong_ordering::equal;
    }

    MJZ_CX_AL_FN uintlen_t n_bit() const noexcept { return words.size() * 64; }
    MJZ_CX_AL_FN uintlen_t n_word() const noexcept { return words.size(); }
    MJZ_CX_AL_FN bool operator==(const uintN_t &rhs) const noexcept = default;

    MJZ_CX_AL_FN std::optional<uintlen_t> floor_log2() const noexcept {
      for (intlen_t i{intlen_t(word_count) - 1}; 0 <= i; i--) {
        uint64_t word = nth_word(i);
        if (!word)
          continue;
        return uintlen_t(log2_of_val_create(word)) + uintlen_t(i * 64);
      }
      return {};
    }
    MJZ_CX_AL_FN std::optional<uintlen_t> ceil_log2() const noexcept {
      intlen_t i{intlen_t(word_count) - 1};
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
      bool no_brk{true};
      for (uint64_t n :
           std::views::iota(uintlen_t(0), n_bits / 64) | std::views::reverse) {
        n = uintlen_t(std::countl_one(nth_word(n)));
        ret += uint64_t(-int64_t(no_brk)) & n;
        no_brk &= n == 64;
      }
      return ret;
    }
    MJZ_CX_AL_FN uintlen_t countr_zero() const noexcept {
      uintlen_t ret{};
      bool no_brk{true};
      for (uint64_t n : std::views::iota(uintlen_t(0), n_bits / 64)) {
        n = uintlen_t(std::countr_zero(nth_word(n)));
        ret += uint64_t(-int64_t(no_brk)) & n;
        no_brk &= n == 64;
      }
      return ret;
    }
    MJZ_CX_AL_FN uintlen_t countl_zero() const noexcept {
      uintlen_t ret{};
      bool no_brk{true};
      for (uint64_t n :
           std::views::iota(uintlen_t(0), n_bits / 64) | std::views::reverse) {
        n = uintlen_t(std::countl_zero(nth_word(n)));
        ret += uint64_t(-int64_t(no_brk)) & n;
        no_brk &= n == 64;
      }
      return ret;
    }
    MJZ_CX_AL_FN uintlen_t countr_one() const noexcept {
      uintlen_t ret{};
      bool no_brk{true};
      for (uint64_t n : std::views::iota(uintlen_t(0), n_bits / 64)) {
        n = uintlen_t(std::countr_one(nth_word(n)));
        ret += uint64_t(-int64_t(no_brk)) & n;
        no_brk &= n == 64;
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
      if (!r)
        return *this;
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

    MJZ_CX_AL_FN uintN_t byteswap() const noexcept {
      auto ret = make_bitcast(*this);
      mem_byteswap(ret.data(), ret.size());
      return std::bit_cast<uintN_t>(ret);
    }
    MJZ_CX_AL_FN uintN_t to_modulo_ret_devide(const uintN_t &rhs) noexcept {
      asserts(asserts.assume_rn, rhs != uintN_t{},
              " it is undefined behaviour to call with rhs of 0");
      uintN_t intermidiate{};
      for (intlen_t i{intlen_t(rhs.countl_zero() - this->countl_zero())};
           0 <= i; i--) {
        uintN_t temp = rhs << uintlen_t(i);
        bool is_bigger{temp <= *this};
        if (is_bigger) {
          intermidiate.set_nth_bit(i, is_bigger);
          *this -= temp;
        };
      }
      return intermidiate;
    }

    MJZ_CX_AL_FN uintN_t &operator%=(const uintN_t &rhs) noexcept {
      to_modulo_ret_devide(rhs);
      return *this;
    }
    MJZ_CX_AL_FN uintN_t &operator/=(const uintN_t &rhs) noexcept {
      return *this = to_modulo_ret_devide(rhs);
    }
    MJZ_CX_AL_FN uintN_t &
    operator_assign_devide_up(const uintN_t &rhs) noexcept {
      uintN_t temp = to_modulo_ret_devide(rhs);
      temp.add(uintN_t(), *this != uintN_t());
      return *this = temp;
    }
    MJZ_CX_AL_FN friend uintN_t operator_devide_up(uintN_t x,
                                                   const uintN_t &y) noexcept {
      x.operator_assign_devide_up(y);
      return x;
    }

    template <std::integral T>
    MJZ_CX_AL_FN explicit operator T() const noexcept {
      static_assert(sizeof(T) <= sizeof(uint64_t));
      return static_cast<T>(nth_word(0));
    }

    MJZ_CX_AL_FN friend uintN_t operator&(uintN_t x,
                                          const uintN_t &y) noexcept {
      x &= y;
      return x;
    }
    MJZ_CX_AL_FN friend uintN_t operator^(uintN_t x,
                                          const uintN_t &y) noexcept {
      x ^= y;
      return x;
    }

    MJZ_CX_AL_FN friend uintN_t operator|(uintN_t x,
                                          const uintN_t &y) noexcept {
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

    MJZ_CX_AL_FN friend uintN_t operator+(uintN_t x,
                                          const uintN_t &y) noexcept {
      x += y;
      return x;
    }

    MJZ_CX_AL_FN friend uintN_t operator-(uintN_t x,
                                          const uintN_t &y) noexcept {
      x -= y;
      return x;
    }
    MJZ_CX_AL_FN friend uintN_t operator*(uintN_t x,
                                          const uintN_t &y) noexcept {
      x *= y;
      return x;
    }
    MJZ_CX_AL_FN friend uintN_t operator/(uintN_t x,
                                          const uintN_t &y) noexcept {
      x /= y;
      return x;
    }

    MJZ_CX_AL_FN friend uintN_t operator%(uintN_t x,
                                          const uintN_t &y) noexcept {
      x %= y;
      return x;
    }

    MJZ_CX_AL_FN uintN_t &operator--() noexcept { return *this -= 1; }

    MJZ_CX_AL_FN uintN_t &operator++() noexcept { return *this += 1; }

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

    template <std::unsigned_integral UT>
    MJZ_CX_AL_FN explicit operator UT() const noexcept {
      return UT(nth_word(0));
    }
    MJZ_CX_AL_FN bool operator!() const noexcept { return *this == uintN_t{0}; }
    template <uintlen_t n2_bits>
    MJZ_CX_AL_FN explicit
    operator uintN_t<version_v, n2_bits>() const noexcept {
      uintN_t<version_v, n2_bits> ret{};
      for (uintlen_t i{}; i < (std::min(n2_bits, n_bits) >> 6); i++) {
        ret.nth_word(i) = nth_word(i);
      }
      return ret;
    }
    MJZ_CX_AL_FN explicit
    operator uint_dyn_t<version_v, true>() const noexcept {
      return as_dyn();
    }
    MJZ_CX_AL_FN explicit operator uint_dyn_t<version_v, false>() noexcept {
      return as_dyn();
    }

    MJZ_CX_AL_FN uint_dyn_t<version_v, true> as_dyn() const noexcept;
    MJZ_CX_AL_FN uint_dyn_t<version_v, false> as_dyn() noexcept;

    static_assert(0 == int64_t(std::bit_cast<double>(0x3FEFFFFFFFFFFFFF)));
    //  double(n_bits) shouldnt really need more precision , this is a stack
    //  variable....
    constexpr static inline double log2_v_ = 0.30102999566398119521373889472449;
    constexpr static inline double low_limit1_v_ =
        std::bit_cast<double>(0x3FEFFFFFFFFFFFFF);
    constexpr static inline int64_t max_dec_width =
        int64_t(log2_v_ * double(n_bits) + low_limit1_v_);

    MJZ_CX_AL_FN uintlen_t to_ascii(char *output,
                                    uintlen_t len) const noexcept {
      if (!len || !output)
        return 0;
      return to_ascii_impl_pick_pv_<0, n_bits>(output, len, bit_width());
    }

    template <uintlen_t low_bit_bound, uintlen_t high_bit_bound,
              bool no_bloat_v = true>
    MJZ_CX_AL_FN uintlen_t to_ascii_impl_pick_pv_(char *output, uintlen_t len,
                                                  uintlen_t bw) const noexcept {
      constexpr auto middle =
          ((low_bit_bound + high_bit_bound + 127) / 128) * 64;
      if constexpr (low_bit_bound / 64 == high_bit_bound / 64 || no_bloat_v) {
        if constexpr (64 < high_bit_bound) {
          uintN_t<version_v, ((high_bit_bound + 63) & ~uintlen_t(63))> v(*this);
          return to_ascii_impl_pv_uintN_conv<version_v, ((high_bit_bound + 63) &
                                                         ~uintlen_t(63))>(
              v.words, output, len, bw);
        } else {
          return int_to_dec(
              output, len,
              uint_size_of_t<(high_bit_bound + 7) / 8>(nth_word(0)));
        }
      } else {
        if (middle < bw) {
          return to_ascii_impl_pick_pv_<low_bit_bound, middle>(output, len, bw);
        } else {
          return to_ascii_impl_pick_pv_<middle, high_bit_bound>(output, len,
                                                                bw);
        }
      }
    }
    MJZ_CX_AL_FN uintlen_t dec_width_aprox_ceil() const noexcept {
      return dec_width_aprox_ceil(bit_width());
    }

  private:
    // log()+1 or log()+2 or for 0, 1
    MJZ_CX_AL_FN static uintlen_t
    dec_width_aprox_ceil(uintlen_t bit_width_) noexcept {
      constexpr int shift = (std::bit_width(n_bits));

      constexpr uintlen_t multiplier_of_log2 = uintlen_t(1) << shift;
      constexpr double log2_muled_dbl_ceil =
          log2_v_ * double(multiplier_of_log2) + low_limit1_v_;
      constexpr uintlen_t log2_muled_ceil = uintlen_t(log2_muled_dbl_ceil);
      uintlen_t log_or_1_plus_log = ((log2_muled_ceil * bit_width_) >> shift);
      return log_or_1_plus_log + 1;
    }
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

  MJZ_CX_AL_FN auto to_modulo_ret_devide(auto &lhs, const auto &rhs) noexcept {
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
  using uint_min_N_t = std::conditional_t<
      (n <= 64), uint_size_of_t<std::min<uintlen_t>(8, (n + 7) >> 3)>,
      uintN_t<version_v, ((((n | (n == 0)) + 63) >> 6) << 6)>>;

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
        result_bit_width - 1 - fraction_bit_count;
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
            << most_significant_bit_is_set_comparasion_shift) -
           decltype(temp)(1);
    return tuple_t{ret, uintlen_t(fraction_bit_count - zcnt_),
                   uintlen_t(bit_count - zcnt_), second_shift, temp};
  }
  template <version_t version_v>
  MJZ_CX_FN void vectorizable_convert_base10p8_to_base10_u64(
      std::span<uint64_t> vals) noexcept {
    constexpr uint64_t mask = (uint64_t(1) << 47) - 1;
    constexpr uint64_t inv10p6_47b = 140737489;
    constexpr uint64_t mask_upper_6b = 0xfc00'fc00'fc00'fc00;
    constexpr uint64_t inv10p1_b10 = 103;

    for (uint64_t &elem : vals) {
      asserts(asserts.assume_rn, elem < 100000000ull);
    }
    for (uint64_t &elem : vals) {
      uint64_t val = elem;
      MJZ_ASSUME_(val < 100000000ull);
      val *= inv10p6_47b;
      const auto ret0 = val >> 47;
      val &= mask;
      val *= 25;
      const auto ret1 = val >> 45;
      val &= mask >> 2;
      val *= 25;
      const auto ret2 = val >> 43;
      val &= mask >> 4;
      val *= 25;
      const auto ret3 = val >> 41;
      MJZ_ASSUME_(ret0 < 100);
      MJZ_ASSUME_(ret1 < 100);
      MJZ_ASSUME_(ret2 < 100);
      MJZ_ASSUME_(ret3 < 100);
      if constexpr (std::endian::big == std::endian::native) {
        val = ret3 | (ret2 << 16) | (ret1 << 32) | (ret0 << 48);
      } else {
        val = ret0 | (ret1 << 16) | (ret2 << 32) | (ret3 << 48);
      }
      val *= inv10p1_b10;
      const auto high = val & mask_upper_6b;
      const auto low = (((val & ~mask_upper_6b) * 10) & mask_upper_6b);
      if constexpr (std::endian::big == std::endian::native) {
        val = (high >> 2) | (low >> 10);
      } else {
        val = (high >> 10) | (low >> 2);
      }
      elem = val | details_ns::ascii_offset;
    }
  }

  MJZ_CX_FN void memcpy_u64_to_char_buffer(
      std::span<char> out, const uint64_t *MJZ_restrict vals) noexcept {
    MJZ_IFN_CONSTEVAL {
      mjz::memcpy(out.data(), reinterpret_cast<const char *>(vals), out.size());
      return;
    }
    uintlen_t sz = out.size();
    uintlen_t sz_no_chunk = sz & 7;
    uintlen_t sz_chunk = sz >> 3;
    for (uintlen_t i{}; i < sz_chunk; i++) {
      cpy_bitcast(&out[i * 8], vals[i]);
    }
    for (uintlen_t i{}; i < sz_no_chunk; i++) {
      out[i + sz_chunk * 8] = make_bitcast(vals[sz_chunk])[i];
    }
  }
  template <version_t version_v, uintlen_t max_dec_width, uintlen_t n_bits>
    requires(!(n_bits & 63))
  MJZ_CX_FN void conert_extended_uint_to_base10p8_u64(
      std::span<uint64_t> vals, std::span<uint64_t, n_bits / 64> right_var_ref,
      uintlen_t bw_) noexcept {
    using uintn_t_ = uintN_t<version_v, n_bits>;
    uintn_t_ var{};

    {
      uintlen_t i{};
      for (uint64_t v : right_var_ref)
        var.words[i++] = v;
    }
    constexpr uint64_t v10p8 = 100000000;
    constexpr uint64_t v10p16 = v10p8 * v10p8;
    constexpr uint64_t bw10p8 = bit_width(v10p8);
    constexpr uint64_t bw10p16 = bit_width(v10p16);
    if constexpr (max_dec_width <= 8) {
      asserts(asserts.assume_rn, bw_ <= bw10p8);
      uint64_t res = uint64_t(var);
      vals[0] = res;
      return;
    } else if constexpr (max_dec_width <= 16) {
      asserts(asserts.assume_rn, bw_ <= bw10p16);
      uint64_t res = uint64_t(var);
      asserts(asserts.assume_rn, res < v10p16);
      if (bw10p8 <= bw_) {
        vals[0] = res / v10p8;
        res %= v10p8;
      }
      vals[1] = res;
      return;
    } else if constexpr (n_bits <= 64 && max_dec_width < 24) {

      uint64_t res = uint64_t(var);
      if (bw10p16 <= bw_) {
        vals[0] = res / v10p16;
        res %= v10p16;
      }
      asserts(asserts.assume_rn, res < v10p16);
      if (bw10p8 <= bw_) {
        vals[1] = res / v10p8;
        res %= v10p8;
      }
      vals[2] = res;
    } else {
      constexpr auto pow_fn = [](uint64_t pow_) {
        uintn_t_ ret{1};
        uintn_t_ temp{10};
        do {
          if (pow_ & 1) {
            ret *= temp;
          }
          pow_ >>= 1;
          if (!pow_)
            return ret;
          temp *= temp;
        } while (true);
        return ret;
      };
      constexpr uintlen_t max_dec_width_ = max_dec_width;

      constexpr uintlen_t pow_of_ten_index =
          max_dec_width_ - (max_dec_width_ / 16) * 8;
      constexpr uintlen_t pow_of_ten_index_wide = ((pow_of_ten_index + 7) / 8);
      constexpr uintn_t_ power_of_ten = pow_fn(pow_of_ten_index);
      constexpr uintn_t_ leftover_pow_ten =
          pow_fn(max_dec_width_ - pow_of_ten_index);
      constexpr uintlen_t bw1 = power_of_ten.bit_width();
      constexpr uintlen_t bw2 = leftover_pow_ten.bit_width();

      constexpr const auto _0pack =
          get_devide_inverse_and_shift<version_v, n_bits>(~uintn_t_(),
                                                          leftover_pow_ten);
      const auto &[inv_val, shift_val, _0, _1, _2] = _0pack;
      using divN_t_ = std::remove_cvref_t<decltype(inv_val)>;
      constexpr divN_t_ max_input_v_ =
          (divN_t_(leftover_pow_ten) * divN_t_(power_of_ten));
      constexpr uintlen_t max_input_bw_v_ = max_input_v_.bit_width();
      constexpr auto mask_val = ((divN_t_(1)) << get<1>(_0pack)) - divN_t_(1);
      constexpr auto p10_ = divN_t_(leftover_pow_ten);

      asserts(asserts.assume_rn, bw_ <= max_input_bw_v_);
      intlen_t bw_left_ = intlen_t(bw_) - intlen_t(bw2 - 1);

      using right_val_t = uintN_t<version_v, ((bw2 + 63) / 64) * 64>;
      std::span right_val_ref_new = [&]() noexcept {
        if constexpr (std::endian::big == std::endian::native) {
          return right_var_ref
              .template subspan<((n_bits / 64) - right_val_t::word_count),
                                right_val_t::word_count>();
        } else {
          return right_var_ref.template subspan<0, right_val_t::word_count>();
        }
      }();

      if (0 < bw_left_) {
        divN_t_ inversed = divN_t_(var) * inv_val;
        auto r1 = inversed;
        auto r2 = inversed;
        r2 &= mask_val;
        r2 *= p10_;
        r2 >>= shift_val;
        r1 >>= shift_val;
        auto left_val = uintN_t<version_v, ((bw1 + 63) / 64) * 64>(r1);
        bw_left_--;
        bw_left_ += left_val.nth_bit(bw_left_);
        bw_ = r2.bit_width();
        uintlen_t bw_left = left_val.bit_width();
        {
          uintlen_t i{};
          if constexpr (std::endian::big == std::endian::native) {
            i += r2.word_count - right_val_ref_new.size();
          }
          for (uint64_t &v : right_val_ref_new)
            v = r2.words[i++];
        }
        asserts(asserts.assume_rn, bw_left == uintlen_t(bw_left_));
        bw_left = uintlen_t(bw_left_);
        conert_extended_uint_to_base10p8_u64<version_v, pow_of_ten_index,
                                             ((bw1 + 63) / 64) * 64>(
            vals.subspan(0, pow_of_ten_index_wide), left_val.words, bw_left);
      }

      return conert_extended_uint_to_base10p8_u64<
          version_v, max_dec_width_ - pow_of_ten_index,
          right_val_t::word_count * 64>(vals.subspan(pow_of_ten_index_wide),
                                        right_val_ref_new, bw_);
    }
  }

  template <version_t version_v, uintlen_t n_bits>
  MJZ_CX_FN uintlen_t to_ascii_impl_pv_uintN_conv(
      std::span<uint64_t, n_bits / 64> var, char *output, uintlen_t len,
      uintlen_t bw) noexcept {
    if (bw == 0) {
      if (!len)
        return 0;
      *output = 0;
      return 1;
    }
    using uintn_t_ = uintN_t<version_v, n_bits>;
    uintN_t<version_v, (((uintn_t_ ::max_dec_width + 7) / 8)) * 64> buffer{};
    std::span<uint64_t> vals = buffer.words;
    conert_extended_uint_to_base10p8_u64<version_v, uintn_t_ ::max_dec_width,
                                         n_bits>(vals, var, bw);
    while (!vals[0])
      vals = vals.subspan(1);
    vectorizable_convert_base10p8_to_base10_u64<version_v>(vals);
    uint64_t first = (vals[0] & ~details_ns::ascii_offset);
    uintlen_t count = vals.size() * 8;
    uintlen_t offset{};
    if constexpr (std::endian::big == std::endian::native) {
      offset = countl_zero(first) >> 3;
      buffer <<= offset << 3;
    } else {
      offset = countr_zero(first) >> 3;
      buffer >>= offset << 3;
    }
    count -= offset;
    if (len < count) {
      return 0;
    }
    memcpy_u64_to_char_buffer(std::span{output, count}, vals.data());

    return count;
  }
};

#endif /* MJZ_LIB_FILE_UINTN_HPP_ */
