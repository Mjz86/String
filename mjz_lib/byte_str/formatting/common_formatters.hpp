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

#include "../../tuple.hpp"
#include "../string.hpp"
#include "base.hpp"
#include "opt_formatters.hpp"

#ifndef MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_

MJZ_EXPORT namespace mjz::bstr_ns::format_ns {
  template <version_t version_v> struct common_data_t {
    using sview_t = static_string_view_t<version_v>;
    using view_t = basic_string_view_t<version_v>;
    using big_str_t =
        basic_str_t<version_v, basic_str_props_t<version_v>{.sso_min_cap = 30}>;
    MJZ_NO_MV_NO_CPY(common_data_t);
    format_context_t<version_v> &ctx;
    view_t input{};
    sview_t err_view{std::nullopt};
    MJZ_CX_FN common_data_t(format_context_t<version_v> &base,
                            view_t input_) noexcept
        : ctx{base}, input{input_} {}
    MJZ_CX_FN ~common_data_t() noexcept {}

    MJZ_CX_FN common_data_t &operator_ll(const auto &v) noexcept
      requires std::same_as<
          success_t, decltype(base_out_it_t<version_v>{}.append_obj_impl_(v))>
    {
      if (ctx.out().append_obj_impl_(v))
        return *this;
      err_view = sview_t{
          "[Error]common_data_t&operator<<(view_t view):could not output the "
          "view"};
      return *this;
    }

    MJZ_CX_FN common_data_t &operator<<(const auto &val) noexcept
      requires requires() { this->operator_ll(val); }
    {
      return operator_ll(val);
    }
  };

  template <version_t version_v, typename T>
  concept common_formatted_c = requires(const std::remove_cvref_t<T> &obj,
                                        common_data_t<version_v> &cf) {
    { cf << obj } noexcept;
    requires(!requires() { cf.operator_ll(obj); });
  };
  template <version_t version_v, typename T>
    requires common_formatted_c<version_v, T>
  struct default_formatter_t<version_v, T, 15> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    using sview_t = static_string_view_t<version_v>;
    using view_t = basic_string_view_t<version_v>;
    view_t input{};
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      view_t view = ctx.view();
      uintlen_t pos = view.find_first_of(sview_t{"}"});
      if (pos == view.nops)
        return true;
      input = view(0, pos);
      if (ctx.advance_amount(pos))
        return true;
      return false;
    }
    MJZ_CX_FN success_t
    format(const std::remove_reference_t<T> &arg,
           format_context_t<version_v> &ctx) const noexcept {
      common_data_t<version_v> cd{ctx, input};
      cd << arg;
      if (!cd.err_view.data())
        return true;
      ctx.as_error(cd.err_view);
      return false;
    }
  };

} // namespace mjz::bstr_ns::format_ns

#endif // MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_
