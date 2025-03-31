
#include "base.hpp"
//
#ifndef MJZ_BYTE_FORMATTING_named_args_HPP_FILE_
#define MJZ_BYTE_FORMATTING_named_args_HPP_FILE_

namespace mjz::bstr_ns::format_ns {
MJZ_DISABLE_WANINGS_START_;
template <version_t version_v, typename T>
struct named_arg_t {
  using view_t = basic_string_view_t<version_v>;
  using hash_t = hash_bytes_t<version_v>;
  std::remove_reference_t<T> obj;
  view_t name;
  hash_t hash;
  template <typename U>
  MJZ_CX_FN named_arg_t(U &&arg, view_t name_, hash_t hash_) noexcept
      : obj{std::forward<U>(arg)}, name{name_}, hash{hash_} {}

  MJZ_CX_FN success_t
  operator()(hash_context_t<version_v> &ctx) const noexcept {
    auto opt = ctx.name();
    if (!opt) return false;
    argument_name_t<version_v> arg_name{*opt};

    if (arg_name.hash == hash && arg_name.name_str == name) {
      return ctx.matched();
    }
    return true;
  }
  MJZ_CX_FN T operator()() const noexcept { return obj; }
};

MJZ_DISABLE_WANINGS_END_;
template <version_t version_v, typename T>
struct is_named_arg_t {};

template <version_t version_v, typename T>
struct is_named_arg_t<version_v, named_arg_t<version_v, T>> {
  using type = T;
};
template <version_t version_v>
struct make_named_arg_t {
  using view_t = basic_string_view_t<version_v>;
  using hash_t = hash_bytes_t<version_v>;
  view_t name;
  hash_t hash;
  MJZ_CX_FN make_named_arg_t(view_t val) noexcept
      : name(val), hash(val.data(), val.length()) {}
  template <typename T>
  MJZ_CX_FN auto operator=(T &&arg) noexcept {
    return named_arg_t<version_v, typed_arg_ref_final_type_t<version_v, T>>{
        to_final_type_fn<version_v, T>(std::forward<T>(arg)), name, hash};
  }
};
namespace fmt_litteral_ns {
template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L>
MJZ_CX_FN make_named_arg_t<version_v> operator_arg() noexcept {
  return bstr_ns::litteral_ns::operator_view<L, version_v>();
}
template <bstr_ns::litteral_ns::str_litteral_t L>
MJZ_CX_FN auto operator""_arg() noexcept {
  return operator_arg<version_t{}, L>();
}
}  // namespace fmt_litteral_ns

template <version_t version_v, class T_>
  requires requires() {
    typename is_named_arg_t<version_v, std::remove_cvref_t<T_>>::type;
  }
struct default_formatter_t<version_v, T_, 20> {
  using T = typename is_named_arg_t<version_v, std::remove_cvref_t<T_>>::type;
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<T>;
  Formatter formatter{};

  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept {
    return formatter.parse(ctx);
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::remove_reference_t<T_> &arg,
      format_context_t<version_v> &ctx) const noexcept {
    return formatter.format(arg(), ctx);
  };

  MJZ_CX_FN static success_t arg_name(const std::remove_reference_t<T_> &arg,
                                      hash_context_t<version_v> &ctx) noexcept {
    return arg(ctx);
  }
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_named_args_HPP_FILE_

/**/