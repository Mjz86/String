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

#include "base.hpp"
//
#ifndef MJZ_BYTE_FORMATTING_named_args_HPP_FILE_
#define MJZ_BYTE_FORMATTING_named_args_HPP_FILE_

MJZ_EXPORT namespace mjz::bstr_ns::format_ns {
  MJZ_DISABLE_ALL_WANINGS_START_;
  template <version_t version_v, typename T> struct named_arg_t {
    using view_t = basic_string_view_t<version_v>;
    using hash_t = hash_bytes_t<version_v>;
    std::remove_reference_t<T> obj;
    view_t name;
    hash_t hash;
    template <typename U>
    MJZ_CX_FN named_arg_t(U &&arg, view_t name_, hash_t hash_) noexcept
        : obj{std::forward<U>(arg)}, name{name_}, hash{hash_} {}

    MJZ_CX_FN success_t
    operator()(hash_context_t<version_v> &ctx) const noexcept {
      auto opt = ctx.name();
      if (!opt)
        return false;
      argument_name_t<version_v> arg_name{*opt};

      if (arg_name.hash == hash && arg_name.name_str == name) {
        return ctx.matched();
      }
      return true;
    }
    MJZ_CX_FN T operator()() const noexcept { return obj; }
  };

  MJZ_DISABLE_ALL_WANINGS_END_;
  template <version_t version_v, typename T> struct is_named_arg_t {};

  template <version_t version_v, typename T>
  struct is_named_arg_t<version_v, named_arg_t<version_v, T>> {
    using type = T;
  };
  template <version_t version_v> struct make_named_arg_t {
    using view_t = basic_string_view_t<version_v>;
    using hash_t = hash_bytes_t<version_v>;
    view_t name;
    hash_t hash;
    MJZ_CX_FN make_named_arg_t(view_t val) noexcept
        : name(val), hash(val.data(), val.length()) {}
    template <typename T> MJZ_CX_FN auto operator=(T &&arg) noexcept {
      return named_arg_t<version_v, typed_arg_ref_final_type_t<version_v, T>>{
          to_final_type_fn<version_v, T>(std::forward<T>(arg)), name, hash};
    }
  };
  namespace fmt_litteral_ns {
  template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN make_named_arg_t<version_v> operator_arg() noexcept {
    return bstr_ns::litteral_ns::operator_view<L, version_v>();
  }
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_arg() noexcept {
    return operator_arg<version_t{}, L>();
  }
  } // namespace fmt_litteral_ns

  template <version_t version_v, class T_>
    requires requires() {
      typename is_named_arg_t<version_v, std::remove_cvref_t<T_>>::type;
    }
  struct default_formatter_t<version_v, T_, 20> {
    using T = typename is_named_arg_t<version_v, std::remove_cvref_t<T_>>::type;
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    using Formatter =
        typename format_context_t<version_v>::template formatter_type<T>;
    Formatter formatter{};

    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      return formatter.parse(ctx);
    };
    MJZ_CX_FN success_t
    format(const std::remove_reference_t<T_> &arg,
           format_context_t<version_v> &ctx) const noexcept {
      return formatter.format(arg(), ctx);
    };

    MJZ_CX_FN static success_t
    arg_name(const std::remove_reference_t<T_> &arg,
             hash_context_t<version_v> &ctx) noexcept {
      return arg(ctx);
    }
  };

} // namespace mjz::bstr_ns::format_ns

#endif // MJZ_BYTE_FORMATTING_named_args_HPP_FILE_

/**/