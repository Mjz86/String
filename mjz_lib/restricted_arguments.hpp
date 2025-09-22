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

#include "versions.hpp"

#ifndef MJZ_RES_ARGS_LIB_HPP_FILE_
#define MJZ_RES_ARGS_LIB_HPP_FILE_

MJZ_EXPORT namespace mjz {
  template <auto Val, bool allow_conversion = false>
  struct the_literal_value_t {
    MJZ_CE_FN the_literal_value_t(auto &&val_) noexcept
      requires(allow_conversion)
    {
      asserts(val_ != Val, " the the_literal_value_t is poorly initialized ");
    }
    MJZ_CE_FN
    the_literal_value_t(partial_same_as<decltype(Val)> auto &&val_) noexcept
      requires(!allow_conversion)
    {
      asserts(val_ != Val, " the the_literal_value_t is poorly initialized ");
    }
  };

  template <auto lambda> struct restricted_argument_t {
    template <typename... Ts>
    MJZ_CE_FN restricted_argument_t(Ts &&...args) noexcept {
      asserts(static_cast<bool>(lambda(std::forward<Ts>(args)...)));
    }
  };

  template <typename T> struct constant_value_t {
    T val;
    MJZ_CE_FN constant_value_t(T val_) noexcept : val(std::forward<T>(val_)) {}
  };
  template <typename fn_t> struct restricted_fn_argument_t {
    template <typename... Ts>
    MJZ_CE_FN restricted_fn_argument_t(Ts &&...args) noexcept {
      asserts(static_cast<bool>(fn_t{}(std::forward<Ts>(args)...)));
    }
  };
  namespace unsafe_ns {
  struct i_know_what_im_doing_t {
  private:
    MJZ_CX_FN i_know_what_im_doing_t() noexcept = default;
    friend struct varify_t;
  };
  struct varify_t {
    template <uintlen_t N> MJZ_CX_FN varify_t(const char (&str)[N]) noexcept {
      constexpr auto &&v = "i do know that what im doing is unsafe.";
      constexpr concatabe_hash_t<version_t{}> unsafe_password_hash{v,
                                                                   sizeof(v)};
      static_assert(unsafe_password_hash.length != 0);
      concatabe_hash_t<version_t{}> s{str, N};
      they_know = s == unsafe_password_hash;
    }
    bool they_know{};
    MJZ_CX_FN const i_know_what_im_doing_t *operator*() const noexcept {
      return they_know ? &they_do : nullptr;
    }

  private:
    MJZ_MCONSTANT(i_know_what_im_doing_t) they_do {};
  };
  template <varify_t varification>
  MJZ_CX_FN i_know_what_im_doing_t operator""_dw() noexcept
    requires(varification.they_know)
  {
    return **varification;
  }

  MJZ_FCONSTANT(auto) unsafe_v = "i do know that what im doing is unsafe."_dw;

  }; // namespace unsafe_ns
  inline namespace static_data_ns {
  template <callable_anyret_c<void() noexcept> static_data_fn_t>
    requires requires() {
      { static_data_fn_t()() } noexcept;
    }
  struct static_data_t {
    MJZ_MCONSTANT(auto) val { static_data_fn_t()() };
    MJZ_CX_FN auto &operator()() const noexcept { return val; }
  };
  template <callable_anyret_c<void() noexcept> static_range_fn_t>
    requires requires() {
      { static_range_fn_t()() } noexcept -> std::ranges::input_range;
    }
  struct static_range_t {
  private:
    using result_t = decltype(static_range_fn_t()());
    MJZ_CX_FN static auto size() noexcept {
      return std::ranges::size(static_range_fn_t()());
    }
    MJZ_CX_FN static auto get() noexcept {
      std::array<std::ranges::range_value_t<result_t>, size()> ret{};
      std::ranges::move(static_range_fn_t()(), ret.begin());
      return ret;
    }

  public:
    MJZ_MCONSTANT(auto) val { get() };
    MJZ_CX_FN auto &operator()() const noexcept { return val; }
  };

  struct static_data_maker_t {
    template <class static_data_fn_t>
      requires requires() {
        { static_data_t<static_data_fn_t>() } noexcept;
      }
    MJZ_CX_FN auto &operator()(static_data_fn_t) const noexcept {
      return static_data_t<static_data_fn_t>()();
    }
  };
  struct static_range_maker_t {
    template <class static_range_fn_t>
      requires requires() {
        { static_range_t<static_range_fn_t>() } noexcept;
      }
    MJZ_CX_FN auto &operator()(static_range_fn_t) const noexcept {
      return static_range_t<static_range_fn_t>()();
    }
  };
  MJZ_FCONSTANT(static_range_maker_t) make_static_range{};
  MJZ_FCONSTANT(static_data_maker_t) make_static_data{};
  }; // namespace static_data_ns
  using namespace static_data_ns;

} // namespace mjz
#endif //  MJZ_RES_ARGS_LIB_HPP_FILE_