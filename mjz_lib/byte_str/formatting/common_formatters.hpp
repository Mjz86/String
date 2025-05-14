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
#ifndef MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_

namespace mjz::bstr_ns::format_ns {

template <version_t version_v>
struct common_data_t {
  using sview_t = static_string_view_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  MJZ_NO_MV_NO_CPY(common_data_t);
  format_context_t<version_v> &ctx;
  view_t input{};
  sview_t err_view{""};
  MJZ_CX_FN common_data_t(format_context_t<version_v> &base,
                          view_t input_) noexcept
      : ctx{base}, input{input_} {}
};

template <version_t version_v, typename T>
concept common_formatted_c =
    requires(const std::remove_cvref_t<T> &obj, common_data_t<version_v> &cf) {
      { cf << obj } noexcept;
    };
template <version_t version_v, typename T>
  requires common_formatted_c<version_v, T>
struct default_formatter_t<version_v, T, 15> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  MJZ_CONSTANT(bool) can_have_cx_formatter_v = true;
  using sview_t = static_string_view_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  view_t input{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept {
    view_t view = ctx.view();
    uintlen_t pos = view.find_first_of(sview_t{"}"});
    if (pos == view.nops) {
      return ctx.begin();
    }
    input = view(0, pos);
    if (ctx.advance_amount(pos)) return ctx.begin();
    return nullptr;
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::remove_reference_t<T> &arg,
      format_context_t<version_v> &ctx) const noexcept {
    common_data_t<version_v> cd{ctx, input};
    cd << arg;
    if (!cd.err_view) {
      return ctx.out();
    }
    ctx.as_error(cd.err_view);
    return nullptr;
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_common_formatters_HPP_FILE_
