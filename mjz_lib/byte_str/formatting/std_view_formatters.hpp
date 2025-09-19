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
#ifndef MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
MJZ_EXPORT namespace mjz::bstr_ns::format_ns {
  template <version_t version_v, typename T>
    requires requires() {
      { std::string_view(just_some_invalid_obj<T &&>()) };
      // to forward the span into std::basic_string_view
      requires !partial_same_as<
          decltype(std::string_view(just_some_invalid_obj<T &&>())), T>;
    }
  struct default_formatter_t<version_v, T, 30> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    // this is an empty forwarding implementation
    using decay_optimize_to_t =
        decltype(std::string_view(just_some_invalid_obj<T &&>()));
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &) noexcept {
      return false;
    };
    MJZ_CX_FN success_t format(T &&,
                               format_context_t<version_v> &) const noexcept {
      return false;
    };
  };
  template <version_t version_v, typename T>
    requires requires() {
      requires std::is_pointer_v<std::remove_cvref_t<T>>;
      requires partial_same_as<char,
                               std::remove_pointer_t<std::remove_cvref_t<T>>>;
    }
  struct default_formatter_t<version_v, T, 19> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    // this is an empty forwarding implementation
    using decay_optimize_to_t =
        decltype(std::string_view(just_some_invalid_obj<T &&>()));
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &) noexcept {
      return false;
    };
    MJZ_CX_FN success_t format(T &&,
                               format_context_t<version_v> &) const noexcept {
      return false;
    };
  };

  template <version_t version_v, partial_same_as<std::string_view> T>
  struct default_formatter_t<version_v, T, 30> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
    using view_t = base_string_view_t<version_v>;
    using CVT_pv = const view_t &;
    using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
        get_invalid_T_obj<CVT_pv>()));
    using Formatter = typename format_context_t<
        version_v>::template formatter_type<decayed_t>;
    Formatter formatter{};
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      return formatter.parse(ctx);
    };
    MJZ_CX_FN success_t
    format(const std::string_view &view,
           format_context_t<version_v> &ctx) const noexcept {
      view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
      return formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx);
    };
  };

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
