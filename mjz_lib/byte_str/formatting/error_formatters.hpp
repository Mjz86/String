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

#include "../../allocs/pmr_adaptor.hpp"
#include "basic_formatters.hpp"
#if MJZ_WITH_iostream
#include <exception>
#include <string>
#include <string_view>
#include <system_error>

#else
#define MJZ_BYTE_FORMATTING_error_formatters_HPP_FILE_
#endif
#ifndef MJZ_BYTE_FORMATTING_error_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_error_formatters_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
template <version_t version_v, class T>
  requires std::derived_from<std::remove_cvref_t<T>, std::exception>
struct default_formatter_t<version_v, T, 30> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using decay_optimize_to_t = const std::exception &;
  using view_t = base_string_view_t<version_v>;
  using CVT_pv = const view_t &;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<decayed_t>;
  Formatter formatter{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept {
    return formatter.parse(ctx);
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::exception &err,
      format_context_t<version_v> &ctx) const noexcept {
    bool ret{};
    MJZ_NOEXCEPT {
      std::string_view view{err.what()};
      view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
      ret = ctx.advance_to(
          formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx));
    };
    if (ret) return ctx.out();
    return nullptr;
  };
};

template <version_t version_v, partial_same_as<std::error_code> T>
struct default_formatter_t<version_v, T, 30> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using view_t = base_string_view_t<version_v>;
  using CVT_pv = const view_t &;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<decayed_t>;
  Formatter formatter{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept {
    return formatter.parse(ctx);
  };
  MJZ_NCX_FN base_out_it_t<version_v> format(
      const std::error_code &err,
      format_context_t<version_v> &ctx) const noexcept {
    bool ret{};
    MJZ_NOEXCEPT {
      auto view{err.message()};
      view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
      ret = ctx.advance_to(
          formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx));
    };
    if (ret) return ctx.out();
    return nullptr;
  };
};
}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_error_formatters_HPP_FILE_
