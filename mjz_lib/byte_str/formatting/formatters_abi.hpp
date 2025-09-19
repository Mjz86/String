
/*
 * thanks to msvc for some of the inspraition.
 */
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
#ifndef MJZ_BYTE_FORMATTING_formatters_abi_HPP_FILE_
#define MJZ_BYTE_FORMATTING_formatters_abi_HPP_FILE_
MJZ_EXPORT namespace mjz ::bstr_ns::format_ns {
  template <version_t version_v, partial_same_as<void_struct_t> T>
  struct default_formatter_t<version_v, T, 20> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &) noexcept {
      return true;
    };
    MJZ_CX_FN success_t format(const auto &,
                               format_context_t<version_v> &) const noexcept {
      return true;
    };
  };
  template <version_t version_v, partial_same_as<std::nullopt_t> T>
  struct default_formatter_t<version_v, T, 20>
      : formatter_type_t<version_v, void_struct_t> {};
  template <version_t version_v, typename>
  MJZ_MCONSTANT(uintlen_t)
  basic_format_specs_conversion_buffer_size_v{
      format_basic_buffer_size_v<version_v>};

  template <version_t version_v>
  struct basic_format_specs_format_fn_obj_t {
    base_lazy_view_t<version_v> as_string{};
    std::optional<uintlen_t> length{};
    uintlen_t offset{};
    uintlen_t buf_size{};
    char *buf_arr{};
    char *buf{};
    bool is_string{};
    bool add_preffix{};
    bool check_neg{};
    uint8_t my_type{};
    byte_traits_t<version_v> bt{};
  };
  template <version_t version_v>
  struct basic_format_specs_t {
    using format_fn_obj = basic_format_specs_format_fn_obj_t<version_v>;
    template <typename T>
    MJZ_MCONSTANT(uintlen_t)
    conversion_buffer_size_v{
        basic_format_specs_conversion_buffer_size_v<version_v, T>};
    enum class align_e : uint8_t { none, left, right, center };
    enum class sign_e : uint8_t { none, plus, minus, space };
    enum class type_e : uint8_t {
      Default = 0,
      raidex_mask_ = (1 << 5) - 1,
      auto_int = 1 << 5,
      auto_float = 2 << 5,
      auto_ptr = 3 << 5,
      auto_str = 4 << 5,
      auto_char = 5 << 5,
      auto_bool = 6 << 5,
      invalid_ = 7 << 5,
      type_mask_ = invalid_,
      binary = 2 | auto_int,
      Octal = 8 | auto_int,
      Dec = 10 | auto_int,
      Hex = 16 | auto_int,
      true_false = auto_bool,
      string = auto_str,
      Char = auto_char,
      escaped_string = auto_str | (raidex_mask_ & Hex),
      sientific = auto_float | uint8_t(floating_format_e::scientific),
      fixed = auto_float | uint8_t(floating_format_e::fixed),
      Hex_float = auto_float | uint8_t(floating_format_e::hex),
      general = auto_float | uint8_t(floating_format_e::general),
      hex_pointer = auto_ptr | (raidex_mask_ & Hex),
    };
    uintlen_t width = 0;
    uintlen_t precision = uintlen_t(-1);
    bool dyn_width = false;
    bool dyn_precision = false;
    type_e type = type_e::Default;
    align_e alignment = align_e::none;
    sign_e sign = sign_e::none;
    bool alt = false;
    bool uppser_case = true;
    bool leading_zero = false;
    // may change on encoding suppport, particularly
    char fill_char{' '};

    MJZ_CX_FN success_t in_range(format_context_t<version_v> &cntx,
                                 auto &&v) const noexcept;
    MJZ_CX_FN success_t parse_align(parse_context_t<version_v> &cntx) noexcept;
    MJZ_CX_FN success_t parse_type(parse_context_t<version_v> &cntx) noexcept;
    MJZ_CX_FN success_t parse_width(parse_context_t<version_v> &cntx) noexcept;

    MJZ_CX_FN success_t
    parse_precision(parse_context_t<version_v> &cntx) noexcept;

   private:
    template <typename T>
    MJZ_MCONSTANT(bool)
    is_numeric = partial_is_one_of_c<
        T, signed char, unsigned char, short, unsigned short, int, unsigned int,
        long, unsigned long, long long, unsigned long long>;
    template <typename T>
    MJZ_MCONSTANT(bool)
    is_numeric_or_ptr =
        is_numeric<T> || std::is_pointer_v<std::remove_cvref_t<T>> ||
        partial_same_as<nullptr_t, T>;
    template <typename T>
    MJZ_MCONSTANT(bool)
    has_persitioned_argument = partial_is_one_of_c<
        T, base_lazy_view_t<version_v>, float, double, long double>;
    using sview_t = static_string_view_t<version_v>;

   public:
    MJZ_CX_FN success_t parse_specs(parse_context_t<version_v> &cntx) noexcept;
    template <typename T>
    MJZ_CX_FN success_t check_specs(parse_context_t<version_v> &cntx) noexcept;
    template <typename T>
    MJZ_CX_FN success_t format_specs(T &&arg,
                                     format_context_t<version_v> &ctx) noexcept;
    template <typename>
    MJZ_CX_FN success_t format_fill(
        auto &&fill_up, format_context_t<version_v> &ctx) const noexcept;

   private:
    MJZ_CX_FN success_t format_fill_pv(
        success_t (*)(void_struct_t &) noexcept, void_struct_t &, char *buffer,
        uintlen_t buffer_size, format_context_t<version_v> &ctx) const noexcept;
    MJZ_CX_FN success_t format_specs_finish(format_context_t<version_v> &cntx,
                                            format_fn_obj &) noexcept;
    MJZ_CX_FN success_t format_specs_start(format_context_t<version_v> &cntx,
                                           format_fn_obj &) noexcept;
  };
  template <typename T, version_t version_v>
  concept uses_basic_spec_c =
      !std::is_volatile_v<T> &&
      (partial_is_one_of_c<T, bool, char, signed char, unsigned char, short,
                           unsigned short, int, unsigned int, long,
                           unsigned long, long long, unsigned long long, float,
                           double, long double, nullptr_t> ||
       std::is_pointer_v<std::remove_cvref_t<T>>);

  template <version_t version_v, uses_basic_spec_c<version_v> T>
  struct default_formatter_t<version_v, T, 20> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;

    ////

    using decay_optimize_to_t = std::conditional_t<
        std::is_pointer_v<std::remove_cvref_t<T>> ||
            partial_same_as<nullptr_t, T>,
        const void *,
        std::conditional_t<
            (sizeof(T) <= sizeof(uintptr_t) &&
             partial_is_one_of_c<T, signed char, unsigned char, short,
                                 unsigned short, int, unsigned int, long,
                                 unsigned long, long long, unsigned long long>),
            std::conditional_t<std::is_signed_v<std::remove_cvref_t<T>>,
                               intptr_t, uintptr_t>,
            T>>;
    basic_format_specs_t<version_v> data{};
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      if (!data.parse_specs(ctx)) return false;
      if (!data.template check_specs<std::remove_cvref_t<T>>(ctx)) return false;
      return true;
    };
    MJZ_CX_FN success_t
    format(std::remove_cvref_t<T> arg,
           format_context_t<version_v> &ctx) const noexcept {
      if (!basic_format_specs_t<version_v>(data)
               .template format_specs<const std::remove_cvref_t<T> &>(arg, ctx))
        return false;
      return true;
    };
  };

  template <version_t version_v, class T>
  concept mjz_str_c = requires(const T &obj) {
    {
      obj.to_base_lazy_pv_fn_(unsafe_ns::unsafe_v)
    } noexcept -> std::same_as<base_lazy_view_t<version_v>>;
  };
  template <version_t version_v, class T>
    requires mjz_str_c<version_v, std::remove_cvref_t<T>>
  struct default_formatter_t<version_v, T, 20> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;

    using decay_optimize_to_t = base_string_view_arg_t<version_v>;
    basic_format_specs_t<version_v> data{};
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      if (!data.parse_specs(ctx)) return false;
      if (!data.template check_specs<base_lazy_view_t<version_v>>(ctx))
        return false;
      return true;
    };
    MJZ_CX_FN success_t
    format(auto &&arg, format_context_t<version_v> &ctx) const noexcept {
      if (!basic_format_specs_t<version_v>(data).format_specs(
              base_lazy_view_t<version_v>(decay_optimize_to_t(arg)), ctx))
        return false;
      return true;
    };
  };
};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_formatters_abi_HPP_FILE_