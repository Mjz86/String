
#include "print.hpp"
#if MJZ_WITH_iostream
#include <iostream>
#else
#define MJZ_BYTE_FORMATTING_formated_stream_HPP_FILE_
#endif
#ifndef MJZ_BYTE_FORMATTING_formated_stream_HPP_FILE_
#define MJZ_BYTE_FORMATTING_formated_stream_HPP_FILE_

namespace mjz::bstr_ns::format_ns {
template <version_t version_v, typename T>
concept is_streamed_c_helper_ =
    is_formatted_c<T, version_v> &&
    !requires(std::ostream& buffer, const std::remove_reference_t<T>& arg) {
      buffer << arg;
    } && is_formatted_exact_c<T, version_v>;

template <typename T, version_t version_v>
  requires is_streamed_c_helper_<version_v, T>
MJZ_NCX_FN std::ostream& operator_shift_left(std::ostream& cout_v, T&& arg) {
  using printer_t_ = print_t<version_v>;
  stream_output_it_t<version_v> it{cout_v};
  status_view_t<version_v> status = printer_t_::format_to(
      alias_t<typename printer_t_::meta_data_t>{false, nullptr, false}, it,
      fmt_litteral_ns::operator_fmt<version_v, "{}">(), std::forward<T>(arg));
  if (status) return cout_v;
  std::ostream& ref = *it.Stream;
  MJZ_RELEASE {
    MJZ_NOEXCEPT {
      ref.width(0);
      ref.setstate(std::istream::failbit | std::istream::badbit);
    };
  };
  if (ref.exceptions() & std::ios_base::badbit) {
    auto view = status.sview();
    if (view.as_c_str()) {
      throw std::runtime_error{view.as_c_str()};
    }
    throw std::runtime_error{std::string{view.data(), view.size()}};
  }
  return cout_v;
}

namespace fmt_litteral_ns {
namespace shl_litteral_ns {
  template <typename T>
    requires is_streamed_c_helper_<version_t{}, T>
  MJZ_NCX_FN std::ostream& operator<<(std::ostream& cout_v, T&& arg) {
    return operator_shift_left<T, version_t{}>(cout_v, std::forward<T>(arg));
  }
}  // namespace fmt_litteral_ns
}
}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_formated_stream_HPP_FILE_
