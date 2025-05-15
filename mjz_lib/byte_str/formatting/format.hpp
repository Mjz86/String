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

#include "print.hpp"
//
#include "../string.hpp"
#ifndef MJZ_BYTE_FORMATTING_format_HPP_FILE_
#define MJZ_BYTE_FORMATTING_format_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
namespace print_ns {


template <version_t version_v>
MJZ_CX_NL_FN void vformat_err_handle_impl_pv_(basic_str_t<version_v>&ret,const status_view_t<version_v>&opt) noexcept {
  ret = basic_str_t<version_v>(unsafe_ns::unsafe_v, opt());
}

template <version_t version_v>
MJZ_CX_AL_FN void vformat_err_handle_pv_(
    basic_str_t<version_v> &ret, const status_view_t<version_v> &opt) noexcept {
  if (!opt) MJZ_IS_UNLIKELY{
    vformat_err_handle_impl_pv_(ret, opt);
  }
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN static auto vformat(basic_string_view_t<version_v> fmt,
                              Ts &&...args) noexcept {
  basic_str_t<version_v> ret{};
  auto opt =
      print_t<version_v>::vformat_to(ret, fmt, std::forward<Ts>(args)...);
  vformat_err_handle_pv_(ret, opt);
  return ret;
}
template <typename... Ts>
MJZ_CX_AL_FN static auto format(auto fmt, Ts &&...args) noexcept {
  constexpr version_t version_v{
      std::remove_cvref_t<decltype(fmt())>::Version_v};
  basic_str_t<version_v> ret{};
  auto opt = print_t<version_v>::format_to(ret, fmt, std::forward<Ts>(args)...);

  vformat_err_handle_pv_(ret, opt);
  return ret;
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN static auto vformatln(basic_string_view_t<version_v> fmt,
                                Ts &&...args) noexcept {
  basic_str_t<version_v> ret{};
  auto opt =
      print_t<version_v>::vformatln_to(ret, fmt, std::forward<Ts>(args)...);
  vformat_err_handle_pv_(ret, opt);
  return ret;
}
template <typename... Ts>
MJZ_CX_AL_FN static auto formatln(auto fmt, Ts &&...args) noexcept {
  constexpr version_t version_v{
      std::remove_cvref_t<decltype(fmt())>::Version_v};

  basic_str_t<version_v> ret{};
  auto opt =
      print_t<version_v>::formatln_to(ret, fmt, std::forward<Ts>(args)...);
  vformat_err_handle_pv_(ret, opt);
  return ret;
}
}  // namespace print_ns
};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_print_HPP_FILE_