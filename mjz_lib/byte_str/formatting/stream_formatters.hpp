
#include "basic_formatters.hpp"
#include "../../allocs/pmr_adaptor.hpp"
#if MJZ_WITH_iostream
#include <memory_resource>
#include <sstream>
#else
#define MJZ_BYTE_FORMATTING_stream_formatters_HPP_FILE_
#endif
#ifndef MJZ_BYTE_FORMATTING_stream_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_stream_formatters_HPP_FILE_

namespace mjz::bstr_ns::format_ns {

template <version_t version_v, typename T>
concept is_stream_formatter_c =
    requires(std::ostream& buffer, const std::remove_reference_t<T>& arg) {
      buffer << arg;
    }; 
template <version_t version_v, class T_>
  requires is_stream_formatter_c<version_v, T_>
struct default_formatter_t<version_v, T_, 100> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using view_t = base_string_view_t<version_v>;
  using CVT_pv = const view_t&;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<decayed_t>;
  Formatter formatter{};
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v>& ctx) noexcept {
    return formatter.parse(ctx);
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
      std::basic_stringstream<char, std::char_traits<char>,std::pmr::polymorphic_allocator<char>>
          buffer{std::pmr::string{pmr}, std::ios_base::in};

      buffer.imbue(std::locale::classic());
      buffer.exceptions(std::ios_base::failbit);
      buffer << arg;
      std::string_view view = buffer.view();

      view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
      good&=!! ctx.advance_to(
          formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx));
    };
    if (good) return ctx.out();

    ctx.as_error(
        "[Error]default_formatter_t<is_stream_formatter_c>: an error was thrown "
        "during output");
    return nullptr;
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_stream_formatters_HPP_FILE_
