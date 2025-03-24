
#include "base.hpp"
//
#ifndef MJZ_BYTE_FORMATTING_opt_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_opt_formatters_HPP_FILE_

namespace mjz::bstr_ns::format_ns {

template <version_t version_v, typename T>
struct is_opt_arg_t {};

template <version_t version_v, typename T>
struct is_opt_arg_t<version_v, optional_ref_t<T>> {
  using type = T&;
};

template <version_t version_v, typename T>
struct is_opt_arg_t<version_v, std::optional<T>> {
  using type = T;
};
MJZ_DISABLE_ALL_WANINGS_START_;
template <version_t version_v, typename T>
struct decay_optimize_opt_to_t {
  std::optional<T> obj;
  template <typename U>
  MJZ_CX_FN decay_optimize_opt_to_t(const std::optional<U>& arg) noexcept
      : obj(arg ? std::optional<T>(
                      to_final_type_fn<version_v,
                                       const std::remove_const_t<U>&>(*arg))
                : std::optional<T>{}) {}
  template <typename U>
  MJZ_CX_FN decay_optimize_opt_to_t(const optional_ref_t<U>& arg) noexcept
      : obj(arg ? std::optional<T>(
                      to_final_type_fn<version_v,
                                       const std::remove_const_t<U>&>(*arg))
                : std::optional<T>{}) {}
  MJZ_CX_FN explicit operator bool() const noexcept { return bool(obj); }
  MJZ_CX_FN decltype(auto) operator*() const noexcept { return *obj; }
};
MJZ_DISABLE_ALL_WANINGS_END_;

template <version_t version_v, typename T>
struct is_opt_arg_t<version_v, decay_optimize_opt_to_t<version_v, T>> {
  using type = T;
};
template <version_t version_v, typename T_>
  requires(!!std::is_reference_v<T_>)
struct decay_optimize_opt_to_t<version_v, T_> {
  using T = std::remove_reference_t<T_>;
  optional_ref_t<T> obj;
  MJZ_CX_FN decay_optimize_opt_to_t(auto&& arg) noexcept
      : obj(arg ? optional_ref_t<T>(*std::addressof(
                      to_final_type_fn<version_v, decltype(*arg)>(*arg)))
                : optional_ref_t<T>{}) {}
  MJZ_CX_FN explicit operator bool() const noexcept { return bool(obj); }
  MJZ_CX_FN decltype(auto) operator*() const noexcept { return *obj; }
};
MJZ_DISABLE_ALL_WANINGS_START_;
template <version_t version_v, class T_>
  requires requires() {
    typename is_opt_arg_t<version_v, std::remove_cvref_t<T_>>::type;
  }
struct default_formatter_t<version_v, T_, 20> {
  using T1 = typename is_opt_arg_t<version_v, std::remove_cvref_t<T_>>::type;
  using T = const std::remove_const_t<T1>;
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using CVT_pv = const std::remove_reference_t<T>&;
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<CVT_pv>;
  Formatter formatter{};
  using decay_optimize_to_t =
      decay_optimize_opt_to_t<version_v,
                              decltype(to_final_type_fn<version_v, T>(
                                  get_invalid_T_obj<T>()))>;
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v>& ctx) noexcept {
    return formatter.parse(ctx);
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      auto&& arg, format_context_t<version_v>& ctx) const noexcept {
    if (!arg) return ctx.out();
    return formatter.format(*arg, ctx);
  };

  MJZ_CX_FN static success_t arg_name(auto&& arg,
                                      hash_context_t<version_v>& ctx) noexcept
    requires(noexcept(Formatter::arg_name(*arg, ctx)))
  {
    if (!arg) return true;
    return Formatter::arg_name(*arg, ctx);
  }
};
MJZ_DISABLE_ALL_WANINGS_END_;
}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_opt_formatters_HPP_FILE_

/**/