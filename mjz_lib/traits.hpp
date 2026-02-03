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

#include "memories.hpp"

#ifndef MJZ_TRAIS_LIB_HPP_FILE_
#define MJZ_TRAIS_LIB_HPP_FILE_

MJZ_EXPORT namespace mjz {
  using nullopt_t = std::nullopt_t;
  MJZ_FCONSTANT(nullopt_t) nullopt{std::nullopt};
  template <class T>
  concept noexcept_destructable_c = requires(T obj) {
    { obj.~T() } noexcept;
  };

  template <class T, typename... Ts>
  concept noexcept_constructable_c = requires(Ts &&...args) {
    { T(std::forward<Ts>(args)...) } noexcept;
  };
  template <class T, typename U>
  concept noexcept_assignment_c = requires(T obj, U &&arg) {
    { obj = std::forward<U>(arg) } noexcept -> std::convertible_to<T &>;
  };
  template <class T, typename... Ts>
  concept noexcept_de_or_con_structable_c =
      noexcept_constructable_c<T, Ts...> && noexcept_destructable_c<T>;

  template <class T, typename Allocator_ptr_t>
  concept noexcept_de_or_con_structable_or_mv_or_cpy_assignment_c = requires() {
    requires noexcept_de_or_con_structable_c<T>;
    requires noexcept_de_or_con_structable_c<T, const T &>;
    requires noexcept_de_or_con_structable_c<T, T &&>;
    requires noexcept_de_or_con_structable_c<T, Allocator_ptr_t>;
    requires noexcept_de_or_con_structable_c<T, const T &, Allocator_ptr_t>;
    requires noexcept_de_or_con_structable_c<T, T &&, Allocator_ptr_t>;
    requires noexcept_assignment_c<T, const T &>;
    requires noexcept_assignment_c<T, T &&>;
  };
  /* we use this approach(const T*) because it dose not allow r-values */
  template <typename T> using lvalue_const_no_null_ptr_t = const T *const;
  template <typename T, typename U, typename... Us>
  concept is_totaly_trivial_sub_subs = requires(T t) {
    requires std::is_trivial_v<T>;
    requires std::is_trivially_move_constructible_v<T>;
    requires std::same_as<U, void> || std::is_trivially_assignable_v<T, U>;
    requires std::is_trivially_constructible_v<T, Us...>;
    requires std::is_trivially_copyable_v<T>;
    requires std::is_trivially_copy_assignable_v<T>;
    requires std::is_trivially_default_constructible_v<T>;
    requires std::is_trivially_move_assignable_v<T>;
    requires std::is_trivially_destructible_v<T>;
  };
  template <typename T, typename U>
  concept is_totaly_trivial_sub_sub = requires(T t) {
    requires std::is_trivial_v<T>;
    requires std::is_trivially_move_constructible_v<T>;
    requires std::is_trivially_copyable_v<T>;
    requires std::is_trivially_copy_assignable_v<T>;
    requires std::is_trivially_default_constructible_v<T>;
    requires std::is_trivially_move_assignable_v<T>;
    requires std::is_trivially_destructible_v<T>;
  };

  template <typename T, typename... Us>
  concept is_totaly_trivial_subs = requires(T t) {
    requires is_totaly_trivial_sub_subs<T, T, Us...>;
    requires is_totaly_trivial_sub_subs<T, const T, Us...>;
    requires is_totaly_trivial_sub_subs<T, std::remove_cvref_t<T> &, Us...>;
    requires is_totaly_trivial_sub_subs<T, std::remove_cvref_t<T> &&, Us...>;
    requires is_totaly_trivial_sub_subs<T, const std::remove_cvref_t<T> &,
                                        Us...>;
    requires is_totaly_trivial_sub_subs<T, const std::remove_cvref_t<T> &&,
                                        Us...>;
  };
  template <typename T, typename... Us>
  concept is_totaly_trivial_sub = requires(T t) {
    requires is_totaly_trivial_sub_sub<T, T>;
    requires is_totaly_trivial_sub_sub<T, const T>;
    requires is_totaly_trivial_sub_sub<T, std::remove_cvref_t<T> &>;
    requires is_totaly_trivial_sub_sub<T, std::remove_cvref_t<T> &&>;
    requires is_totaly_trivial_sub_sub<T, const std::remove_cvref_t<T> &>;
    requires is_totaly_trivial_sub_sub<T, const std::remove_cvref_t<T> &&>;
  };
  template <typename T, typename... Us>
  concept is_totaly_trivials = requires(T t, const T ct) {
    requires is_totaly_trivial_subs<T, T, Us...>;
    {
      T {}
    } noexcept;
    { t.~T() } noexcept;
    {
      t = T {}
    } noexcept;
    { t = t } noexcept;
    { t = ct } noexcept;
  };
  template <typename T, typename... Us>
  concept is_totaly_trivial = requires(T t, const T ct) {
    requires is_totaly_trivial_sub<T, T>;
    {
      T {}
    } noexcept;
    { t.~T() } noexcept;
    {
      t = T {}
    } noexcept;
    { t = t } noexcept;
    { t = ct } noexcept;
  };

  template <typename T>
  concept is_totaly_trivial_after_init = requires(T t, const T ct) {
    requires std::is_trivial_v<T>;
    requires std::is_trivially_move_constructible_v<T>;
    requires std::is_trivially_copyable_v<T>;
    requires std::is_trivially_copy_assignable_v<T>;
    requires std::is_trivially_move_assignable_v<T>;
    requires std::is_trivially_destructible_v<T>;
    { t.~T() } noexcept;
    { t = t } noexcept;
    { t = ct } noexcept;
  };

  template <auto integral_constant_maker_lambda>
    requires requires { integral_constant_maker_lambda(); }
  struct mjz_integral_constant_t {
    MJZ_MCONSTANT(auto) value = integral_constant_maker_lambda();
    using value_type = decltype(value);
    using type = mjz_integral_constant_t; // using injected-class-name
    MJZ_CX_FN operator value_type &() const noexcept { return value; }
    MJZ_CX_FN value_type &operator()() const noexcept { return value; }
  };
  template <auto... integral_constant_maker_lambdas>
  struct mjz_integral_constants_t {
    template <uintlen_t i>
    using type_at = std::tuple_element_t<
        i,
        tuple_t<mjz_integral_constant_t<integral_constant_maker_lambdas>...>>;
    template <uintlen_t i> using value_type = decltype(type_at<i>{}());
    using type = mjz_integral_constants_t;
    template <uintlen_t i> MJZ_CX_FN static auto &get() noexcept {
      return type_at<i>{}();
    }
    MJZ_MCONSTANT(size_t)
    MJZ_STATIC_tuple_len_{sizeof...(integral_constant_maker_lambdas)};
  };

  template <typename T> MJZ_CX_FN T get_invalid_T_obj() noexcept {
    MJZ_UNREACHABLE();
  }

  template <typename T>
  MJZ_NORETURN MJZ_CX_FN T just_some_invalid_obj() noexcept {
    MJZ_NEVER_RUN_PATH();
  }

  template <class inheritable_or_void>
  using make_inheritable_or_empty_t =
      std::conditional_t<std::same_as<void, inheritable_or_void>,
                         totally_empty_type_t, inheritable_or_void>;

  template <typename T> struct lvalue_const_ref_t {
    const T &obj;
    MJZ_CX_FN lvalue_const_ref_t(no_null_t<const T *const> That) noexcept
        : obj(*That) {}
    MJZ_CX_FN lvalue_const_ref_t(const T &That) noexcept : obj(That) {}
    MJZ_CX_FN lvalue_const_ref_t(T &That) noexcept : obj(That) {}
    MJZ_CX_FN lvalue_const_ref_t(const T &&That) noexcept = delete;
    MJZ_CX_FN lvalue_const_ref_t(T &&That) noexcept = delete;
  };
  template <typename T, typename FN_t, bool strict_return_v>
  struct is_callable_helper : std::false_type {};
  template <typename fn_t, bool strict_return_v, typename ret_t,
            typename... args_t>
    requires requires(fn_t f, args_t... args) {
      requires !strict_return_v || requires() {
        {
          std::forward<fn_t>(f)(std::forward<args_t>(args)...)
        } -> std::same_as<ret_t>;
      };
      requires strict_return_v || requires() {
        {
          std::forward<fn_t>(f)(std::forward<args_t>(args)...)
        } -> std::convertible_to<ret_t>;
      };
    }
  struct is_callable_helper<fn_t, ret_t(args_t...), strict_return_v>
      : std::true_type {};
  template <typename fn_t, bool strict_return_v, typename ret_t,
            typename... args_t>
    requires requires(fn_t f, args_t... args) {
      requires !strict_return_v || requires() {
        {
          std::forward<fn_t>(f)(std::forward<args_t>(args)...)
        } noexcept -> std::same_as<ret_t>;
      };
      requires strict_return_v || requires() {
        {
          std::forward<fn_t>(f)(std::forward<args_t>(args)...)
        } noexcept -> std::convertible_to<ret_t>;
      };
    }
  struct is_callable_helper<fn_t, ret_t(args_t...) noexcept, strict_return_v>
      : std::true_type {};
  template <typename T, typename FN_t>
  struct is_callable_anyret_helper : std::false_type {};
  template <typename fn_t, typename ret_t, typename... args_t>
  struct is_callable_anyret_helper<fn_t, ret_t(args_t...) noexcept>
      : is_callable_helper<
            fn_t,
            decltype(std::forward<fn_t>(
                *reinterpret_cast<std::remove_reference_t<fn_t> *>(0))(
                std::forward<args_t>(
                    *reinterpret_cast<std::remove_reference_t<args_t> *>(
                        0))...))(args_t...) noexcept,
            false> {};

  template <typename T, typename FN_t, bool strict_return_v>
  struct is_callable_helper_ : is_callable_helper<T, FN_t, strict_return_v> {};
  template <typename T, typename FN_t>
  struct is_callable_anyret_helper_ : is_callable_anyret_helper<T, FN_t> {};

  template <typename fn_t, typename ret_t, typename... args_t>
  struct is_callable_anyret_helper_<fn_t, ret_t(args_t...) const>
      : is_callable_anyret_helper<const fn_t, ret_t(args_t...) const> {};
  template <typename fn_t, typename ret_t, bool strict_return_v,
            typename... args_t>
  struct is_callable_helper_<fn_t, ret_t(args_t...) const, strict_return_v>
      : is_callable_helper<const fn_t, ret_t(args_t...) const,
                           strict_return_v> {};

  template <typename fn_t, typename ret_t, typename... args_t>
  struct is_callable_anyret_helper_<fn_t, ret_t(args_t...) const noexcept>
      : is_callable_anyret_helper<const fn_t, ret_t(args_t...) const noexcept> {
  };
  template <typename fn_t, typename ret_t, bool strict_return_v,
            typename... args_t>
  struct is_callable_helper_<fn_t, ret_t(args_t...) const noexcept,
                             strict_return_v>
      : is_callable_helper<const fn_t, ret_t(args_t...) const noexcept,
                           strict_return_v> {};

  template <typename T, typename FN_t, bool strict_return_v = false>
  concept callable_c = !!is_callable_helper_<T, FN_t, strict_return_v>()();
  template <typename T, typename FN_t>
  concept callable_anyret_c = !!is_callable_anyret_helper_<T, FN_t>()();
  template <typename T, typename U>
  concept partial_same_as =
      std::same_as<std::remove_cvref_t<U>, std::remove_cvref_t<T>>;

  template <class the_class, typename func_t> struct funcptr_maker_helper;

  template <class the_class, typename ret_t, typename... args_t>
  struct funcptr_maker_helper<the_class, ret_t(args_t...)> {
    using type = ret_t (the_class::*)(args_t...);
  };

  template <class the_class, typename ret_t, typename... args_t>
  struct funcptr_maker_helper<the_class, ret_t(args_t...) const> {
    using type = ret_t (the_class::*)(args_t...) const;
  };
  template <class the_class, typename ret_t, typename... args_t>
  struct funcptr_maker_helper<the_class, ret_t(args_t...) noexcept> {
    using type = ret_t (the_class::*)(args_t...) noexcept;
  };
  template <class the_class, typename ret_t, typename... args_t>
  struct funcptr_maker_helper<the_class, ret_t(args_t...) const noexcept> {
    using type = ret_t (the_class::*)(args_t...) const noexcept;
  };
  template <typename ret_t, typename... args_t>
  struct funcptr_maker_helper<void, ret_t(args_t...)> {
    using type = ret_t (*)(args_t...);
  };
  template <typename ret_t, typename... args_t>
  struct funcptr_maker_helper<void, ret_t(args_t...) const> {
    using type = ret_t (*const)(args_t...);
  };
  template <typename ret_t, typename... args_t>
  struct funcptr_maker_helper<void, ret_t(args_t...) noexcept> {
    using type = ret_t (*)(args_t...) noexcept;
  };
  template <typename ret_t, typename... args_t>
  struct funcptr_maker_helper<void, ret_t(args_t...) const noexcept> {
    using type = ret_t (*const)(args_t...) noexcept;
  };

  template <typename func_t, class type_t = void>
  using funcptr_of_t =
      typename funcptr_maker_helper<std::remove_cvref_t<type_t>, func_t>::type;

  template <class the_class, typename func_t> struct like_funcptr_maker_helper;

  template <class the_class, typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<the_class, ret_t(args_t...)> {
    using type = ret_t (*)(the_class *, args_t...);
  };
  template <class the_class, typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<the_class, ret_t(args_t...) const> {
    using type = ret_t (*)(const the_class *, args_t...);
  };
  template <class the_class, typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<the_class, ret_t(args_t...) noexcept> {
    using type = ret_t (*)(the_class *, args_t...) noexcept;
  };
  template <class the_class, typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<the_class, ret_t(args_t...) const noexcept> {
    using type = ret_t (*)(const the_class *, args_t...) noexcept;
  };
  template <typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<void, ret_t(args_t...)> {
    using type = ret_t (*)(args_t...);
  };
  template <typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<void, ret_t(args_t...) const> {
    using type = ret_t (*const)(args_t...);
  };
  template <typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<void, ret_t(args_t...) noexcept> {
    using type = ret_t (*)(args_t...) noexcept;
  };
  template <typename ret_t, typename... args_t>
  struct like_funcptr_maker_helper<void, ret_t(args_t...) const noexcept> {
    using type = ret_t (*const)(args_t...) noexcept;
  };

  template <typename func_t, class type_t = void>
  using like_funcptr_of_t =
      typename like_funcptr_maker_helper<std::remove_cvref_t<type_t>,
                                         func_t>::type;

  template <typename T, typename... Ts>
  concept is_one_of_c = ((std::same_as<T, Ts>) || ...);
  template <typename T, typename... Ts>
  concept partial_is_one_of_c = ((partial_same_as<T, Ts>) || ...);

  MJZ_CX_FN char bit_reverse_bitcast(char ch) noexcept {
    uint8_t ret{};
    for (uint8_t i{}; i < 8; i++) {
      ret |= uint8_t(uint8_t(!!(uint8_t(ch) & uint8_t(1 << i))) << (7 - i));
    }
    return char(ret);
  }

  template <typename T, bool to_BE, bool do_reverse_if_NE>
  MJZ_CX_FN std::array<char, sizeof(T)> get_XE_bitcast(T val) noexcept {
    if constexpr (SYSTEM_is_little_endian_ != to_BE) {
      return std::bit_cast<std::array<char, sizeof(T)>>(val);
    }
    auto r_ret = std::bit_cast<std::array<char, sizeof(T)>>(val);
    std::array<char, sizeof(T)> ret{};
    for (uintlen_t i{}; i < sizeof(T); i++) {
      ret[(size_t)i] =
          do_reverse_if_NE
              ? bit_reverse_bitcast(r_ret[(size_t)(sizeof(T) - 1 - i)])
              : r_ret[(size_t)(sizeof(T) - 1 - i)];
    }
    return ret;
  }
  template <typename T, bool was_BE, bool do_reverse_if_NE>
  MJZ_CX_FN T from_XE_bitcast(std::array<char, sizeof(T)> val) noexcept {
    if constexpr (SYSTEM_is_little_endian_ != was_BE) {
      return std::bit_cast<T>(val);
    }
    std::array<char, sizeof(T)> ret{};
    for (uintlen_t i{}; i < sizeof(T); i++) {
      ret[(size_t)i] =
          do_reverse_if_NE
              ? bit_reverse_bitcast(val[(size_t)(sizeof(T) - 1 - i)])
              : val[(size_t)(sizeof(T) - 1 - i)];
    }
    return std::bit_cast<T>(ret);
  }
  struct bit_range_t {
    uintlen_t i;
    uintlen_t len;
    template <is_totaly_trivial T>
    MJZ_CX_FN static std::optional<bit_range_t>
    get_bit_range(const T strating_val,
                  callable_c<bool(uintlen_t i) noexcept> auto &&bit_to,
                  callable_c<bool(const T &) noexcept> auto &&is_begin,
                  callable_c<bool(const T &) noexcept> auto &&is_end) noexcept {
      uintlen_t numbits{sizeof(T) * 8};
      bit_range_t range{uintlen_t(-1), uintlen_t(-1)};
      std::array<char, sizeof(T)> a_s_var{
          get_XE_bitcast<T, false, false>(strating_val)};
      for (uintlen_t i{}; i < numbits / 8; i++) {
        for (uintlen_t j{}; j < 8; j++) {
          std::array<char, sizeof(T)> a_var{a_s_var};
          a_var[(size_t)i] = bit_to(8 * i + j) ? char(uint8_t(1 << j)) : 0;
          const T var = from_XE_bitcast<T, false, false>(a_var);
          if (is_begin(var)) {
            if (~range.i) {
              return std::nullopt; // multiple is_begin;
            }
            range.i = 8 * i + j;
          }
          if (is_end(var)) {
            if (!~range.i || ~range.len) {
              return std::nullopt; //   no is_begin || multiple is_end;
            }
            range.len = 8 * i + j - range.i;
          }
        }
      }
      if (!~range.len) {
        range.len = numbits - range.i;
      }
      if (!~range.i) {
        return std::nullopt; //   no is_begin ;
      }
      return range;
    }
  };
  template <auto val> struct ce_val_t {
    using type = decltype(val);
    MJZ_CX_FN auto operator()() const noexcept { return val; }
  };

  template <typename T>
  using total_decay_t = std::remove_cvref_t<std::decay_t<T>>;
}; // namespace mjz
#endif // MJZ_TRAIS_LIB_HPP_FILE_