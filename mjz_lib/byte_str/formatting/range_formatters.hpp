
/*
 * thanks to msvc for some of the inspraition.
 */
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

#include "../../aggregate_tuple.hpp"
#include "basic_formatters.hpp"

//
#ifndef MJZ_BYTE_FORMATTING_range_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_range_formatters_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
template <typename T>
concept is_tuple_c = requires {
  typename mjz::tuple_size<std::remove_cvref_t<T>>;
  mjz::tuple_size<std::remove_cvref_t<T>>::value;
  {
    mjz::tuple_size_v<std::remove_cvref_t<T>>
  } noexcept -> std::convertible_to<uintlen_t>;
};

template <version_t version_v, is_tuple_c T_>
struct default_formatter_t<version_v, T_, 60> {
  MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
  MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
  using T = std::remove_cvref_t<T_>;
  MJZ_MCONSTANT(bool) a_tuple_thingy_v { true };
  using sview = static_string_view_t<version_v>;
  using view = basic_string_view_t<version_v>;
  template <std::size_t I>
  using CVT_pv = const std::remove_reference_t<mjz::tuple_element_t<I, T> &> &;
  template <std::size_t I>
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv<I>>(
      get_invalid_T_obj<CVT_pv<I>>()));
  template <std::size_t... I>
  MJZ_CX_FN auto static F_t_fn(std::index_sequence<I...>) noexcept
      -> tuple_t<typename format_context_t<version_v>::template formatter_type<
          decayed_t<I>>...> * {
    return nullptr;
  }

  using IS_t = std::make_index_sequence<mjz::tuple_size_v<T>>;

  using F_t = std::remove_pointer_t<decltype(F_t_fn(IS_t{}))>;

 public:
  F_t formatters{};
  view open_braket{sview("(")};
  view close_braket{sview(")")};
  view separator{sview(", ")};
  uintlen_t slice_index{};
  uintlen_t slice_length{uintlen_t(-1)};
  basic_format_specs_t<version_v> partial_spec{};
  bool is_m{};

  template <std::size_t I_v>
  MJZ_CX_FN auto inner_loop_fn(auto &&fn_v, uintlen_t &len,
                               success_t &result) const -> bool {
    if (I_v < slice_index) {
      return true;
    }
    if (len == slice_length) {
      return false;
    }
    len++;
    result &= fn_v.template operator()<I_v>();
    if (!result) {
      return false;
    }
    return true;
  }

  template <std::size_t... I>
  MJZ_CX_FN success_t on_types_Is(auto &&fn_v,
                                  std::index_sequence<I...>) const {
    success_t result{true};
    uintlen_t len{};
    MJZ_UNUSED bool b = (inner_loop_fn<I>(fn_v, len, result) && ...);
    return result;
  }
  MJZ_CX_FN success_t on_types(auto &&fn_v) const noexcept {
    success_t ret{};
    MJZ_NOEXCEPT { ret = on_types_Is(fn_v, IS_t{}); };
    return ret;
  }

  MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
    if (ctx.encoding() != encodings_e::ascii) {
      ctx.as_error(
          "[Error]default_formatter_t::parse(tuple,auto):"
          "only ascii is "
          "suppoerted!(note that this "
          "may change in later versions)");
      return false;
    }
    if (!partial_spec.parse_align(ctx)) {
      return false;
    }
    if (!partial_spec.parse_width(ctx)) {
      return false;
    }
    auto ch = ctx.front();
    if (ch && *ch == '[') {
      std::optional<pair_t<uintlen_t /*index*/, uintlen_t /*length*/>> slice =
          ctx.get_slice();
      if (!slice) {
        return false;
      }
      slice_index = slice->first;
      slice_length = slice->second;
      ch = ctx.front();
    }
    if (ch && *ch == 'n') {
      open_braket = sview("");
      close_braket = sview("");
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    }
    if (is_m || (ch && *ch == 'm')) {
      if (2 + slice_index != std::min(mjz::tuple_size_v<std::remove_cvref_t<T>>,
                                      slice_length + slice_index)) {
        ctx.as_error(
            "[Error]default_formatter_t::parse(tuple,"
            "auto):"
            "invalid range spec of m with size other than 2");
        return false;
      }
      open_braket = sview("");
      close_braket = sview("");
      separator = sview(": ");
      if (!is_m) {
        if (!ctx.unchecked_advance_amount_(1)) return false;
        ch = ctx.front();
      }
    }
    if (ch && *ch == 's') {
      open_braket = sview("");
      close_braket = sview("");
      separator = sview("");
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    }

    if (!on_types([&]<std::size_t I>() {
          ch = ctx.front();
          if (ch && *ch != '}' && ch != '{') {
            ctx.as_error(
                "[Error]default_formatter_t::parse(tuple,"
                "auto):"
                "invalid range spec");
            return false;
          }
          bool had_scope{};
          if (ch == '{') {
            if (!ctx.unchecked_advance_amount_(1)) return false;
            ch = ctx.front();
            had_scope = true;
          }
          if (!(std::get<I>(formatters).parse(ctx))) return false;
          if (ch && *ch != '}') {
            ctx.as_error(
                "[Error]default_formatter_t::parse(tuple,"
                "auto):"
                "invalid range spec");
            return false;
          }
          if (had_scope) {
            if (!ctx.unchecked_advance_amount_(1)) return false;
          }
          return true;
        }))
      return false;
    if (ch && *ch != '}') {
      ctx.as_error(
          "[Error]default_formatter_t::parse(tuple,auto):"
          "invalid range spec");
      return false;
    }
    return true;
  };
  MJZ_CX_FN success_t format(const T &arg,
                             format_context_t<version_v> &ctx) const noexcept {
    return partial_spec.template format_fill<T>(
        [&]() noexcept {
          bool not_first{false};
          auto it = ctx.out();
          it.append(open_braket.unsafe_handle());
          if (!on_types([&]<std::size_t I>() {
                const auto &val = mjz::get<I>(arg);
                if (not_first) {
                  it.append(separator.unsafe_handle());
                }
                if (!(mjz::get<I>(formatters)
                          .format(to_final_type_fn<version_v, CVT_pv<I>>(val),
                                  ctx)))
                  return false;
                not_first = true;
                it = ctx.out();
                return true;
              }))
            return false;
          it.append(close_braket.unsafe_handle());
          return true;
        },
        ctx);
  };
};
template <version_t version_v, std::ranges::input_range T>
struct default_formatter_t<version_v, T, 50> {
  using T_range = std::remove_const_t<std::remove_reference_t<T>>;
  using CT_range = std::conditional_t<requires(const T_range obj) {
    *std::ranges::begin(obj);
    std::ranges::begin(obj) == std::ranges::end(obj);
  }, const T_range, T_range>;
  using CRT_range = CT_range &;
  MJZ_MCONSTANT(bool)
  no_perfect_forwarding_v = std::is_const_v<CT_range>;
  MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
  MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
  using sview = static_string_view_t<version_v>;
  using view = basic_string_view_t<version_v>;
  using value_t =
      decltype(*std::ranges::begin(just_some_invalid_obj<CRT_range>()));
  using CVT_pv = const std::remove_reference_t<value_t> &;
  using decayed_t = decltype(to_final_type_fn<version_v, CVT_pv>(
      get_invalid_T_obj<CVT_pv>()));
  using F_t =
      typename format_context_t<version_v>::template formatter_type<decayed_t>;

 public:
  F_t formatter{};
  view open_braket{sview("[")};
  view close_braket{sview("]")};
  view separator{sview(", ")};
  uintlen_t slice_index{};
  uintlen_t slice_length{uintlen_t(-1)};
  basic_format_specs_t<version_v> partial_spec{};

  MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
    if (ctx.encoding() != encodings_e::ascii) {
      ctx.as_error(
          "[Error]default_formatter_t::parse(std::ranges::input_range,auto):"
          "only ascii is "
          "suppoerted!(note that this "
          "may change in later versions)");
      return false;
    }
    if (!partial_spec.parse_align(ctx)) {
      return false;
    }
    if (!partial_spec.parse_width(ctx)) {
      return false;
    }
    auto ch = ctx.front();
    if (ch && *ch == '[') {
      std::optional<pair_t<uintlen_t /*index*/, uintlen_t /*length*/>> slice =
          ctx.get_slice();
      if (!slice) return false;
      slice_index = slice->first;
      slice_length = slice->second;
      ch = ctx.front();
    }
    if (ch && *ch == 'n') {
      open_braket = sview("");
      close_braket = sview("");
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    }
    if (ch && *ch == 'm') {
      if constexpr (!requires() {
                      {
                        F_t::a_tuple_thingy_v
                      } noexcept -> partial_same_as<bool>;
                      requires bool(F_t::a_tuple_thingy_v);
                    }) {
        ctx.as_error(
            "[Error]default_formatter_t::parse(std::ranges::input_range,"
            "auto):"
            "invalid range spec of m with size other than 2");
        return false;
      } else {
        formatter.is_m = true;
      }

      open_braket = sview("{");
      close_braket = sview("}");
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    }
    if (ch && *ch == 's') {
      open_braket = sview("");
      close_braket = sview("");
      separator = sview("");
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    }
    if (ch && *ch == ':') {
      if (!ctx.unchecked_advance_amount_(1)) return false;
      ch = ctx.front();
    } else if (ch && *ch != '}') {
      ctx.as_error(
          "[Error]default_formatter_t::parse(std::ranges::input_range,auto):"
          "invalid range spec");
      return false;
    }
    return formatter.parse(ctx);
  };
  MJZ_CX_FN success_t format(auto &&arg,
                             format_context_t<version_v> &ctx) const noexcept {
    auto may_throw = [&]() noexcept(false) {
      const auto &f = formatter;
      auto beg = std::ranges::begin(arg);
      auto end = std::ranges::end(arg);
      bool not_first{false};
      auto it = ctx.out();
      it.append(open_braket.unsafe_handle());
      uintlen_t index{};
      uintlen_t len{};
      uintlen_t slice_index_var{slice_index};
      uintlen_t slice_index_len{slice_length};

      if constexpr (std::ranges::sized_range<T>) {
        uintlen_t length_of_range = uintlen_t(std::ranges::distance(arg));
        slice_index_var = std::min(slice_index_var, length_of_range);
        slice_index_len =
            std::min(slice_index_var + slice_index_len, length_of_range) -
            slice_index_var;
        it.reserve(slice_index_len * sizeof(uintlen_t), ctx.encoding());
      }
      if constexpr (std::integral<std::remove_cvref_t<
                        std::ranges::range_difference_t<T>>>) {
        std::ranges::advance(
            beg, std::ranges::range_difference_t<T>(slice_index_var));
        index = slice_index_var;
      }

      while (beg != end) {
        if (index++ < slice_index_var) {
          (void)++beg;
          continue;
        }
        if (len == slice_index_len) {
          break;
        }
        if (not_first) {
          it.append(separator.unsafe_handle());
        }
        value_t v = *beg;
        if (!f.format(to_final_type_fn<version_v, CVT_pv>(v), ctx))
          return false;
        (void)++beg;
        len++;
        not_first = true;
        it = ctx.out();
      }
      it.append(close_braket.unsafe_handle());
      return true;
    };
    return partial_spec.template format_fill<T>(
        [&]() noexcept {
          success_t ret{true};
          bool not_thrown = MJZ_NOEXCEPT { ret &= may_throw(); };
          if (!not_thrown) {
            ctx.as_error(
                "[Error]default_formatter_t::format(std::ranges::input_range,"
                "auto): exception detected while executing iterations ( take a "
                "look at run_and_block_exeptions where its catch happened)");
          }
          ret &= not_thrown;
          return ret;
        },
        ctx);
  };
};
template <version_t version_v, typename T>
  requires requires() {
    { std::span(just_some_invalid_obj<T &&>()) } noexcept;
    // to forward the span into std::ranges::input_range
    requires !partial_same_as<
        decltype(std::span(just_some_invalid_obj<T &&>())), T>;
  }
struct default_formatter_t<version_v, T, 40> {
  MJZ_MCONSTANT(bool) no_perfect_forwarding_v = true;
  MJZ_MCONSTANT(bool) can_bitcast_optimize_v = true;
  MJZ_MCONSTANT(bool) can_have_cx_formatter_v = true;
  // this is an empty forwarding implementation
  using decay_optimize_to_t =
      std::span<std::remove_reference_t<typename decltype(std::span(
          just_some_invalid_obj<T &&>()))::reference>>;
  MJZ_CX_FN success_t parse(parse_context_t<version_v> &) noexcept {
    return false;
  };
  MJZ_CX_FN success_t format(T &&,
                             format_context_t<version_v> &) const noexcept {
    return false;
  };
};

};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_range_formatters_HPP_FILE_