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
#ifndef MJZ_BYTE_FORMATTING_easy_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_easy_formatters_HPP_FILE_

namespace mjz::bstr_ns::format_ns {

template <version_t version_v>
struct easy_output_data_t;
template <version_t version_v>
struct easy_output_it {
 public:
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using pointer = void;
  using reference = void;
  using container_type = easy_output_data_t<version_v>;
  using difference_type = intlen_t;
  MJZ_CX_FN easy_output_it &operator=(char ch) noexcept;
  MJZ_CX_FN easy_output_data_t<version_v> &operator()() noexcept;
  MJZ_CX_FN easy_output_it &operator*() noexcept { return *this; }
  MJZ_CX_FN easy_output_it &operator++() noexcept { return *this; }
  MJZ_CX_FN easy_output_it operator++(int) noexcept { return *this; }
  easy_output_data_t<version_v> *ptr{};
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept;
  MJZ_CX_ND_FN base_out_it_t<version_v> out() const noexcept;
};
template <version_t version_v>
struct easy_output_data_t {
  MJZ_CX_FN easy_output_data_t(format_context_t<version_v> &ctx) noexcept
      : iter(ctx.out()), encoding{ctx.encoding()} {}
  MJZ_CX_FN easy_output_it<version_v> it() noexcept {
    easy_output_it<version_v> ret{};
    ret.ptr = this;
    return ret;
  };
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept { return encoding; }
  MJZ_CX_FN easy_output_data_t &operator=(char ch) noexcept {
    out().push_back(ch, encoding);
    return *this;
  }
  MJZ_CX_ND_FN base_out_it_t<version_v> out() noexcept {
    return base_out_it_t<version_v>{iter};
  }

 protected:
  MJZ_NO_MV_NO_CPY(easy_output_data_t); 
  base_out_it_t<version_v> iter{};
  encodings_e encoding{};
};

template <version_t version_v>
MJZ_CX_FN easy_output_data_t<version_v> &
easy_output_it<version_v>::operator()() noexcept {
  return *ptr;
}
template <version_t version_v>
MJZ_CX_FN base_out_it_t<version_v> easy_output_it<version_v>::out()
    const noexcept {
  return ptr->out();
}

template <version_t version_v>
MJZ_CX_FN encodings_e easy_output_it<version_v>::get_encoding() const noexcept {
  return ptr->get_encoding();
}
template <version_t version_v>
MJZ_CX_FN easy_output_it<version_v> &easy_output_it<version_v>::operator=(
    char ch) noexcept {
  *ptr = ch;
  return *this;
}
template <typename T>
struct easy_formatter_t {
  MJZ_CONSTANT(version_t) version_v {};
  MJZ_NO_MV_NO_CPY(easy_formatter_t);
  MJZ_CX_FN easy_formatter_t() noexcept = delete;
  MJZ_CX_FN ~easy_formatter_t() noexcept = delete;
  MJZ_CX_FN status_view_t<version_v> parse(
      basic_string_view_t<version_v> format_input) noexcept = delete;
  MJZ_CX_FN status_view_t<version_v> format(
      const T &s, easy_output_it<version_v> it) const noexcept = delete;
};

template <version_t version_v, typename T, class F_t>
concept easy_formatted_helper_c = requires(
    F_t obj, const F_t cobj, T &&arg,
    basic_string_view_t<version_v> format_input, easy_output_it<version_v> it) {
  { obj.~F_t() } noexcept;
  { F_t() } noexcept;
  { F_t(cobj) } noexcept;
  { obj = cobj } noexcept;
  { F_t(std::move(obj)) } noexcept;
  { obj = std::move(obj) } noexcept;
  {
    obj.parse(format_input)
  } noexcept -> std::same_as<status_view_t<version_v>>;
  {
    cobj.format(std::forward<T>(arg), it)
  } noexcept -> std::same_as<status_view_t<version_v>>;
};
template <version_t version_v, typename T>
concept easy_formatted_c =
    easy_formatted_helper_c<version_v, const std::remove_cvref_t<T> &,
                            easy_formatter_t<std::remove_cvref_t<T>>>;

template <version_t version_v, typename T>
  requires easy_formatted_c<version_v, T>
struct default_formatter_t<version_v, T, 10> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using sview_t = static_string_view_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  easy_formatter_t<std::remove_cvref_t<T>> easy_formatter{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept {
    view_t view = ctx.view();
    uintlen_t pos = view.find_first_of(sview_t{"}"});
    if (pos == view.nops) {
      return ctx.begin();
    }
    if (!ctx.advance_amount(pos)) return nullptr;
    status_view_t<version_v> status = easy_formatter.parse(view(0, pos));
    if (status) {
      return ctx.begin();
    }
    ctx.as_error(status.sview());
    return nullptr;
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::remove_reference_t<T> &arg,
      format_context_t<version_v> &ctx) const noexcept {
    easy_output_data_t<version_v> data{ctx};
    status_view_t<version_v> status = easy_formatter.format(arg, data.it());
    if (status) {
      return ctx.out();
    }
    ctx.as_error(status.sview());
    return nullptr;
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_easy_formatters_HPP_FILE_
