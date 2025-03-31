
#include "print.hpp"
//
#include "../string.hpp"
#ifndef MJZ_BYTE_FORMATTING_format_HPP_FILE_
#define MJZ_BYTE_FORMATTING_format_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
namespace print_ns {
template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformat(basic_string_view_t<version_v> fmt,
                              Ts &&...args) noexcept {
  basic_str_t<version_v, false> ret{};
  auto opt =
      print_t<version_v>::vformat_to(ret, fmt, std::forward<Ts>(args)...);
  if (!opt) {
    ret = basic_str_t<version_v, false>(unsafe_ns::unsafe_v, opt());
  }
  return ret;
}
template <typename... Ts>
MJZ_CX_FN static auto format(auto fmt, Ts &&...args) noexcept {
  constexpr version_t version_v{
      std::remove_cvref_t<decltype(fmt())>::Version_v};
  basic_str_t<version_v, false> ret{};
  auto opt = print_t<version_v>::format_to(ret, fmt, std::forward<Ts>(args)...);
  if (!opt) {
    ret = basic_str_t<version_v, false>(unsafe_ns::unsafe_v, opt());
  }
  return ret;
}

template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformatln(basic_string_view_t<version_v> fmt,
                                Ts &&...args) noexcept {
  basic_str_t<version_v, false> ret{};
  auto opt =
      print_t<version_v>::vformatln_to(ret, fmt, std::forward<Ts>(args)...);
  if (!opt) {
    ret = basic_str_t<version_v, false>(unsafe_ns::unsafe_v, opt());
  }
  return ret;
}
template <typename... Ts>
MJZ_CX_FN static auto formatln(auto fmt, Ts &&...args) noexcept {
  constexpr version_t version_v{
      std::remove_cvref_t<decltype(fmt())>::Version_v};

  basic_str_t<version_v, false> ret{};
  auto opt =
      print_t<version_v>::formatln_to(ret, fmt, std::forward<Ts>(args)...);
  if (!opt) {
    ret = basic_str_t<version_v, false>(unsafe_ns::unsafe_v, opt());
  }
  return ret;
}
}  // namespace print_ns
};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_print_HPP_FILE_