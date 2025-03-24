

#include "str_const_api.hpp"
#include "str_mut_api.hpp"
#ifndef MJZ_BYTE_STRING_string_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_LIB_HPP_FILE_
namespace mjz ::bstr_ns {
namespace litteral_ns {
MJZ_CONSTANT(version_t) version_V_var_{};
/*
 *makes a gengeric basic_str_t  that views the string
 */
template <str_litteral_t L, version_t vr, bool has_alloc>
MJZ_CE_FN basic_str_t<vr, has_alloc> operator_str() noexcept
  requires(!std::is_empty_v<basic_str_t<vr, has_alloc>>)
{
  return basic_str_t<vr, has_alloc>(operator_view<L, vr>());
};
/*
 *specialized operator for newest version
 */
/*
 *makes a basic_str_t(with custom allocator feature) that views the string
 */
template <str_litteral_t L>
MJZ_CX_FN basic_str_t<version_V_var_, true> operator""_astr() noexcept
  requires(!std::is_empty_v<basic_str_t<version_V_var_, true>>)
{
  basic_str_t<version_V_var_, true> ret{operator_view<L, version_V_var_>()};
  return ret;
}
template <char... Cs>
MJZ_CX_FN basic_str_t<version_V_var_, true> operator""_as_astr() noexcept
  requires(!std::is_empty_v<basic_str_t<version_V_var_, true>>)
{
  constexpr auto str0 = str_litteral_t{operator""_cs < Cs...>()};
  basic_str_t<version_V_var_, true> ret{operator_view<str0, version_V_var_>()};
  return ret;
}
/*
 *specialized operator for newest version
 */
/*
 *makes a basic_str_t that views the string
 */
template <str_litteral_t L>
MJZ_CX_FN basic_str_t<version_V_var_, false> operator""_str() noexcept
  requires(!std::is_empty_v<basic_str_t<version_V_var_, false>>)
{
  basic_str_t<version_V_var_, false> ret{operator_view<L, version_V_var_>()};
  return ret;
}
template <char... Cs>
MJZ_CX_FN basic_str_t<version_V_var_, false> operator""_as_astr() noexcept
  requires(!std::is_empty_v<basic_str_t<version_V_var_, false>>)
{
  constexpr auto str0 = str_litteral_t{operator""_cs < Cs...>()};
  basic_str_t<version_V_var_, false> ret{operator_view<str0, version_V_var_>()};
  return ret;
}
};  // namespace litteral_ns
};  // namespace mjz::bstr_ns
template <mjz::version_t version_v, bool has_alloc_v_>
struct std::hash<mjz::bstr_ns::basic_str_t<version_v, has_alloc_v_>> {
  std::size_t operator()(const auto& s) const noexcept {
    return std::size_t(s.hash());
  }
};

#endif  // MJZ_BYTE_STRING_string_LIB_HPP_FILE_
