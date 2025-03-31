
#include "../../allocs/pmr_adaptor.hpp"
#include "basic_formatters.hpp"
#if MJZ_WITH_iostream
#include <string>
#include <string_view>
#else
#define MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
#endif
#ifndef MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
template <version_t version_v, typename T>
  requires requires() {
    { std::string_view(just_some_invalid_obj<T &&>()) };
    // to forward the span into std::basic_string_view
    requires !partial_same_as<
        decltype(std::string_view(just_some_invalid_obj<T &&>())), T>;
  }
struct default_formatter_t<version_v, T, 30> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  // this is an empty forwarding implementation
  using decay_optimize_to_t =
      decltype(std::string_view(just_some_invalid_obj<T &&>()));
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &) noexcept {
    return nullptr;
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      T &&, format_context_t<version_v> &) const noexcept {
    return nullptr;
  };
};
template <version_t version_v, typename T>
  requires requires() {
    requires std::is_pointer_v<std::remove_cvref_t<T>>;
    requires partial_same_as<char,
                             std::remove_pointer_t<std::remove_cvref_t<T>>>;
  }
struct default_formatter_t<version_v, T, 19> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  // this is an empty forwarding implementation
  using decay_optimize_to_t =
      decltype(std::string_view(just_some_invalid_obj<T &&>()));
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &) noexcept {
    return nullptr;
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      T &&, format_context_t<version_v> &) const noexcept {
    return nullptr;
  };
};

template <version_t version_v, partial_same_as<std::string_view> T>
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
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::string_view &view,
      format_context_t<version_v> &ctx) const noexcept {
    view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
    return formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx);
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_std_view_formatters_HPP_FILE_
