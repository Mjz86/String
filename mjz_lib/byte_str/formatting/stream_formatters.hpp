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

#include "../../allocs/pmr_adaptor.hpp"
#include "basic_formatters.hpp"

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
    requires(std::ostream &buffer, const std::remove_reference_t<T> &arg) {
      buffer << arg;
    };
template <version_t version_v, class T_>
  requires is_stream_formatter_c<version_v, T_>
struct default_formatter_t<version_v, T_, 100> {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = true;
  using view_t = base_string_view_t<version_v>;
  using CVT_pv = const view_t &;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using Formatter =
      typename format_context_t<version_v>::template formatter_type<decayed_t>;
  Formatter formatter{};
  MJZ_CX_FN success_t parse(
      parse_context_t<version_v> &ctx) noexcept {
    return formatter.parse(ctx);
  };
  MJZ_CX_FN success_t format(
      const std::remove_reference_t<T_> &arg,
      format_context_t<version_v> &ctx) const noexcept {
    bool good{true};
    auto blk_0_ =
        ctx.fn_alloca(conversion_buffer_size_v<T_>, alignof(uintlen_t));
    if (!blk_0_.size()) {
      ctx.as_error(
          "[Error]default_formatter_t<is_stream_formatter_c>::format:cannot allocate more "
          "memory");
      return false;
    }
    MJZ_RELEASE { ctx.fn_dealloca(std::move(blk_0_), alignof(uintlen_t)); };
    good &= MJZ_NOEXCEPT {
      allocs_ns::pmr_adaptor_t<version_v> alloc{ctx.allocator()}; 
      std::pmr::monotonic_buffer_resource mbr{blk_0_.data(), blk_0_.size(),
                                              &alloc};
      std::pmr::polymorphic_allocator<char> pmr{&mbr};
      std::basic_stringstream<char, std::char_traits<char>,
                              std::pmr::polymorphic_allocator<char>>
          buffer{std::pmr::string{pmr}, std::ios_base::in};

      buffer.imbue(std::locale::classic());
      buffer.exceptions(std::ios_base::failbit);
      buffer << arg;
      std::string_view view = buffer.view();

      view_t v = view_t::make(view.data(), view.size(), encodings_e::ascii);
      good &= !!ctx.advance_to(
          formatter.format(to_final_type_fn<version_v, CVT_pv>(v), ctx));
    };
    if (good) return true;

    ctx.as_error(
        "[Error]default_formatter_t<is_stream_formatter_c>: an error "
        "was thrown "
        "during output");
    return false;
  };
};

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_stream_formatters_HPP_FILE_
