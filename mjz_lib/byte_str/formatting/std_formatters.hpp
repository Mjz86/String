
#include "../../allocs/pmr_adaptor.hpp"
#include "basic_formatters.hpp"

#ifndef __cpp_lib_format
#define MJZ_BYTE_FORMATTING_std_formatters_HPP_FILE_
#endif
#if !MJZ_WITH_iostream
#define MJZ_BYTE_FORMATTING_std_formatters_HPP_FILE_
#endif
#ifndef MJZ_BYTE_FORMATTING_std_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_std_formatters_HPP_FILE_
#include <format>
#include <sstream>

#include "../string.hpp"

namespace mjz::bstr_ns::format_ns {
template <version_t version_v, typename T>
concept is_std_formatter_c = requires(const std::remove_reference_t<T>& arg) {
  std::formatter<T, char>();
  std::make_format_args(arg);
};
template <version_t version_v, class T_>
  requires is_std_formatter_c<version_v, T_>
struct default_formatter_t<version_v, T_, 90> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using bview_t = base_string_view_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using sview_t = static_string_view_t<version_v>;
  using CVT_pv = const view_t&;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using str_t = basic_str_t<version_v, true>;
  str_t parse_content{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v>& ctx) noexcept {
    view_t view = ctx.view();
    uintlen_t pos = view.find_first_of(sview_t{"}"});
    if (pos == view.nops) {
      return ctx.begin();
    }
    replace_flags_t<version_v> rp{};
    rp.change_alloc_v = decltype(rp)::change_e::always_force_change;
    if (!(parse_content.append_data_with_char(1, '{', ctx.allocator(), rp) &&
          parse_content.append_data_with_range(view(0, pos + 1)))) {
      ctx.as_error(
          "[Error]default_formatter_t<is_std_formatter_c>parse:allocation "
          "failed");
      return nullptr;
    }
    if (ctx.advance_amount(pos)) return ctx.begin();
    return nullptr;
  };
  MJZ_CX_FN base_out_it_t<version_v> format(
      const std::remove_reference_t<T_>& arg,
      format_context_t<version_v>& ctx) const noexcept {
    bool good{true};
    good &= MJZ_NOEXCEPT {
      allocs_ns::pmr_adaptor_t<version_v> alloc{ctx.allocator()};
      char buff[1024]{};
      std::pmr::monotonic_buffer_resource mbr{buff, sizeof(buff), &alloc};
      std::pmr::polymorphic_allocator<char> pmr{&mbr};
      std::pmr::string buffer{pmr};
      std::string_view fmt{parse_content.data(), parse_content.length()};
      try {
        std::vformat_to(std::back_insert_iterator(buffer),
                        std::locale::classic(), fmt,
                        std::make_format_args(arg));
      } catch (std::format_error err) {
        good = false;
        std::string_view err_view(err.what());
        bview_t v =
            bview_t::make(err_view.data(), err_view.size(), encodings_e::ascii);
        good &= ctx.as_error(
                       "[Error]default_formatter_t<is_std_formatter_c>:  a std "
                       "format error was thrown "
                       "during output")
                    .append(v);
      }
      if (!good) return;
      bview_t v =
          bview_t::make(buffer.data(), buffer.size(), encodings_e::ascii);
      good &= ctx.out().append(v);
    };
    if (good) return ctx.out();
    ctx.as_error(
        "[Error]default_formatter_t<is_std_formatter_c>: an  exeption was "
        "thrown during the output");
    return nullptr;
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_std_formatters_HPP_FILE_
