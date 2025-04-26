
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

#include "formatters_abi.hpp"
#ifndef MJZ_BYTE_FORMATTING_basic_formatters_HPP_FILE_
#define MJZ_BYTE_FORMATTING_basic_formatters_HPP_FILE_
namespace mjz ::bstr_ns::format_ns {

template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::parse_align(
    parse_context_t<version_v> &cntx) noexcept {
  if (cntx.encoding() != encodings_e::ascii) {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_align():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }
  auto first = cntx.begin();
  auto last = cntx.end();
  if (first == last || *first == '}') {
    return true;
  }
  auto align_it = first + std::min<intlen_t>(last - first, 1);

  if (align_it == last) {
    align_it = first;
  }
  auto alignment_ = alignment;
  for (;;) {
    switch (*align_it) {
      case '<':
        alignment_ = align_e ::left;
        break;
      case '>':
        alignment_ = align_e::right;
        break;
      case '^':
        alignment_ = align_e::center;
        break;
      default:
        break;
    }
    if (alignment_ == align_e::none) {
      if (align_it == first) {
        break;
      }
      align_it = first;
      continue;
    }
    if (align_it == first) {
      ++first;
      alignment = alignment_;
      break;
    }
    if (*first == '{') {
      cntx.as_error(
          "[Error]basic_format_specs_t::parse_align():  invalid fill "
          "character '{' ");
      return false;
    }
    fill_char = *first;
    first = align_it + 1;
    alignment = alignment_;
    break;
  }

  return cntx.advance_to(first);
}
template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::parse_type(
    parse_context_t<version_v> &cntx) noexcept {
  if (cntx.encoding() != encodings_e::ascii) {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_type():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }
  std::optional<char> ch = cntx.front();
  if (!ch || *ch == '}') {
    return true;
  }
  if (!cntx.advance_amount(1)) return false;
  char c = *ch;
  if (c >= 'A' && c <= 'Z') {
    uppser_case = true;
  }
  if (c >= 'a' && c <= 'z') {
    uppser_case = false;
  }
  switch (c) {
    case 's':
      type = type_e::string;
      break;
    case '?':
      type = type_e::escaped_string;
      break;
    case 'B':
      MJZ_FALLTHROUGH;
    case 'b':
      type = type_e::binary;
      break;
    case 'o':
      type = type_e::Octal;
      break;
    case 'd':
      type = type_e::Dec;
      break;
    case 'X':
      MJZ_FALLTHROUGH;
    case 'x':
      type = type_e::Hex;
      break;
    case 'c':
      type = type_e::Char;
      break;
    case 'a':
      MJZ_FALLTHROUGH;
    case 'A':
      type = type_e::Hex_float;
      break;
    case 'E':
      MJZ_FALLTHROUGH;
    case 'e':
      type = type_e::sientific;
      break;
    case 'f':
      MJZ_FALLTHROUGH;
    case 'F':
      type = type_e::fixed;
      break;
    case 'g':
      MJZ_FALLTHROUGH;
    case 'G':
      type = type_e::general;
      break;
    case 'p':
      MJZ_FALLTHROUGH;
    case 'P':
      type = type_e::hex_pointer;
      break;
    case 'D':
      type = type_e::Default;
      break;
    default:
      type = type_e::invalid_;
      break;
  }
  if (type != type_e::invalid_) return true;
  cntx.as_error(
      "[Error]basic_format_specs_t::parse_type():invalid type charachter");
  return false;
}
template <version_t version_v>
template <typename T_t>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::in_range(
    format_context_t<version_v> &cntx, T_t &&v) const noexcept {
  using T = std::remove_cvref_t<T_t>;
  if ((uint8_t(type) & uint8_t(type_e::type_mask_)) !=
      uint8_t(type_e::auto_char)) {
    return true;
  }
  if constexpr (std::integral<T>) {
    if (uint64_t(char(v)) == uint64_t(v)) {
      return true;
    }
  }
  cntx.as_error(
      "[Error]basic_format_specs_t::in_ch_range(): the value dose "
      "not fit in a char");
  return false;
}
template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::parse_width(
    parse_context_t<version_v> &cntx) noexcept {
  if (cntx.encoding() != encodings_e::ascii) {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_width():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }
  auto ch = cntx.front();
  if (!ch || *ch == '}') {
    return true;
  }
  if ('0' <= *ch && *ch <= '9') {
    auto num = cntx.parse_starting_ulen();
    if (!num) return false;
    width = *num;
    return true;
  }
  if (*ch != '{') {
    return true;
  }
  if (!cntx.advance_amount(1)) return false;
  ch = cntx.front();
  if (ch) {
    auto width_id = cntx.parse_arg_id();
    if (!width_id) return false;
    width = *width_id;
    dyn_width = true;
    if (!cntx.type_v(width)
             .template is_one_of_xcr<uint8_t, uint16_t, uint32_t, uint64_t>()) {
      cntx.as_error(
          "[Error]basic_format_specs_t::parse_width():invalid width "
          "type(must be unsigned integral )");
      return false;
    }
  }
  ch = cntx.front();
  if (!ch || *ch != '}') {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_width(): expected end of "
        "width/feild");
    return false;
  }
  return cntx.advance_amount(1);
}
template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::parse_precision(
    parse_context_t<version_v> &cntx) noexcept {
  if (cntx.encoding() != encodings_e::ascii) {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_precision():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }
  auto ch = cntx.front();
  if (!ch || *ch == '}') return true;
  if (*ch != '.') return true;
  if (!cntx.advance_amount(1)) return false;
  ch = cntx.front();
  auto consume_braket = [&](bool bad = false) noexcept {
    ch = cntx.front();
    if (!bad && *ch == '}') return cntx.advance_amount(1);
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_precision(): expected end of "
        "precision/feild with '}'");
    return false;
  };
  if ('0' <= *ch && *ch <= '9') {
    auto num = cntx.parse_starting_ulen();
    if (!num) return false;
    precision = *num;
    return true;
  }
  if (*ch != '{') {
    return consume_braket(true);
  }
  if (!cntx.advance_amount(1)) return false;
  if (!cntx.front()) {
    return consume_braket(true);
  }
  auto precision_id = cntx.parse_arg_id();
  if (!precision_id) return false;
  precision = *precision_id;
  dyn_precision = true;
  if (cntx.type_v(precision)
          .template is_one_of_xcr<uint8_t, uint16_t, uint32_t, uint64_t>()) {
    return consume_braket();
  }
  cntx.as_error(
      "[Error]basic_format_specs_t::parse_precision():invalid "
      "precision type(must be unsigned integral )");
  return false;
}

template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::parse_specs(
    parse_context_t<version_v> &cntx) noexcept {
  if (cntx.encoding() != encodings_e::ascii) {
    cntx.as_error(
        "[Error]basic_format_specs_t::parse_specs():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }
  if (!parse_align(cntx)) {
    return false;
  }
  auto ch = cntx.front();
  if (!ch) return true;
  switch (*ch) {
    case '+':
      sign = sign_e::plus;
      if (!cntx.advance_amount(1)) return false;
      break;
    case '-':
      sign = sign_e::minus;
      if (!cntx.advance_amount(1)) return false;
      break;
    case ' ':
      sign = sign_e::space;
      if (!cntx.advance_amount(1)) return false;
      break;
    default:
      break;
  }
  ch = cntx.front();
  if (!ch) return true;

  if (*ch == '#') {
    alt = true;
    if (!cntx.advance_amount(1)) return false;
    ch = cntx.front();
    if (!ch) return true;
  }

  if (*ch == '0') {
    leading_zero = true;
    if (!cntx.advance_amount(1)) return false;
    ch = cntx.front();
    if (!ch) return true;
  }
  return parse_width(cntx) && parse_precision(cntx) && parse_type(cntx);
}
template <version_t version_v>
template <typename T>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::check_specs(
    parse_context_t<version_v> &cntx) noexcept {
  bool need_numeric_argument{};
  bool numeric_or_pointer_argument{};
  bool need_persitioned_argument{};
  need_numeric_argument |= sign != sign_e::none;
  numeric_or_pointer_argument |= leading_zero;
  need_persitioned_argument |= precision != uintlen_t(-1);
  auto err_fn = [&]() noexcept -> success_t {
    cntx.as_error(
        "[Error]basic_format_specs_t::check_specs:invalid format type and Type "
        "match");
    return false;
  };
  if constexpr (partial_same_as<bool, T>) {
    if (type == type_e::string) {
      type = type_e::true_false;
    }
  }
  switch (type) {
    case type_e::Default: {
      if constexpr (partial_same_as<bool, T>) {
        type = type_e::true_false;
      } else if constexpr (partial_same_as<char, T>) {
        type = type_e::Char;
      } else if constexpr (std::floating_point<T>) {
        type = type_e::general;
      } else if constexpr (std::integral<T>) {
        type = type_e::Dec;
      } else if constexpr (std::derived_from<T, base_lazy_view_t<version_v>>) {
        type = type_e::string;
      } else if constexpr (std::is_pointer_v<T>) {
        type = type_e::hex_pointer;
      } else {
        cntx.as_error(
            "[Error]basic_format_specs_t::sorry :(  idk what is this (maybe no "
            "string yet)");
        return false;
      }
    } break;

    case type_e::binary:
      MJZ_FALLTHROUGH;
    case type_e::Octal:
      MJZ_FALLTHROUGH;
    case type_e::Dec:
      MJZ_FALLTHROUGH;
    case type_e::Hex:
      MJZ_FALLTHROUGH;
    case type_e::auto_char:
      MJZ_FALLTHROUGH;
    case type_e::auto_int: {
      if constexpr (!std::integral<T>) return err_fn();
    } break;
    case type_e::sientific:
      MJZ_FALLTHROUGH;
    case type_e::fixed:
      MJZ_FALLTHROUGH;
    case type_e::Hex_float:
      MJZ_FALLTHROUGH;
    case type_e::general:
      MJZ_FALLTHROUGH;
    case type_e::auto_float: {
      if constexpr (!std::floating_point<T>) return err_fn();
    } break;
    case type_e::hex_pointer:
      MJZ_FALLTHROUGH;
    case type_e::auto_ptr: {
      if constexpr (!std::is_pointer_v<T> || partial_same_as<nullptr_t, T>)
        return err_fn();
    } break;
    case type_e::escaped_string:
      MJZ_FALLTHROUGH;
    case type_e::auto_str: {
      if constexpr (!std::derived_from<T, base_lazy_view_t<version_v>> &&
                    !partial_same_as<bool, T> && !partial_same_as<char, T>)
        return err_fn();
    } break;
    case type_e::auto_bool: {
      if constexpr (!partial_same_as<bool, T>) return err_fn();
    } break;
    case type_e::raidex_mask_:
      MJZ_FALLTHROUGH;
    case type_e::invalid_: {
      return err_fn();
    } break;
  }
  //-no type check :(-//
  if (alt && !std::integral<T> && !std::floating_point<T>) {
    cntx.as_error(
        "[Error]basic_format_specs_t::check():type must be an integral or "
        "floating to use alt('#')");
    return false;
  }
  if (need_numeric_argument && !is_numeric<T>) {
    cntx.as_error(
        "[Error]basic_format_specs_t::check():type must be an integral");
    return false;
  }
  if (numeric_or_pointer_argument && !is_numeric_or_ptr<T>) {
    cntx.as_error(
        "[Error]basic_format_specs_t::check():type must be an integral or "
        "pointer");
    return false;
  }
  if (need_persitioned_argument && !has_persitioned_argument<T>) {
    cntx.as_error(
        "[Error]basic_format_specs_t::check():type must be a positional "
        "type");
    return false;
  }
  return true;
}
template <version_t version_v>
template <typename T_t>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::format_specs(
    T_t &&arg, format_context_t<version_v> &ctx) noexcept {
  using T = std::remove_cvref_t<T_t>;
  using bview = base_lazy_view_t<version_v>;
  if (!this->in_range(ctx, arg)) {
    return false;
  }
  char obj_buf[std::max<uintlen_t>(8, conversion_buffer_size_v<T>)]{};
  format_fn_obj stack{};
  stack.buf_arr = obj_buf;
  stack.buf_size = sizeof(obj_buf);
  if (!format_specs_start(ctx, stack)) return false;
  if constexpr (partial_same_as<T, bview>) {
    stack.as_string = arg;
    if (ctx.encoding() != stack.as_string.get_encoding()) {
      ctx.as_error(
          "[Error]basic_format_specs_t::format_specs():encoding mismatch in "
          "string");
      return false;
    }
    stack.is_string = true;
    stack.buf -= stack.offset;
    stack.buf_size += stack.offset;
  } else if constexpr (std::is_pointer_v<std::remove_cvref_t<T>> ||
                       partial_same_as<nullptr_t, T>) {
    stack.add_preffix = true;
    this->alignment =
        this->alignment == align_e::none ? align_e ::right : this->alignment;
    MJZ_IF_CONSTEVAL {
      ctx.as_error(
          "[Error]basic_format_specs_t::format_specs(): reinterpret_cast of "
          "pointer to integral failed at compile-time");

      return false;
    }
    this->alignment =
        this->alignment == align_e::none ? align_e ::right : this->alignment;

    stack.length = stack.bt.from_integral_fill(

        stack.buf, stack.buf_size,
        reinterpret_cast<uintptr_t>(alias_t<const void *>(arg)), uppser_case,
        uint8_t(uint8_t(type) & uint8_t(type_e::raidex_mask_)));
    if (!stack.length) {
      ctx.as_error(
          "[Error]basic_format_specs_t::format_specs():couldnt convert to "
          "pointer");

      return false;
    }
  } else if constexpr (partial_is_one_of_c<T, float, double, long double>) {
    stack.check_neg = true;
    this->alignment =
        this->alignment == align_e::none ? align_e ::right : this->alignment;

    stack.length = stack.bt.from_float_format_fill(

        stack.buf, stack.buf_size, arg,
        precision == uintlen_t(-1) ? sizeof(std::remove_cvref_t<T>) : precision,
        uppser_case,
        floating_format_e(
            uint8_t(uint8_t(type) & uint8_t(type_e::raidex_mask_))),
        '.', alt);
    if (!stack.length) {
      ctx.as_error(
          "[Error]basic_format_specs_t::format_specs():couldnt convert to "
          "floating");

      return false;
    }
  } else if constexpr (partial_same_as<bool, T>) {
    do {
      if (stack.my_type != uint8_t(type_e::auto_bool)) {
        stack.add_preffix = true;

        stack.buf[0] = arg ? '1' : '0';

        stack.length = 1;
        break;
      }
      if (arg) {
        memcpy(stack.buf, "true", 4);

        stack.length = 4;
        break;
      }
      memcpy(stack.buf, "false", 5);

      stack.length = 5;
    } while (0);
  } else if constexpr (std::integral<T>) {
    if (stack.my_type == uint8_t(type_e::auto_char)) {
      this->alignment =
          this->alignment == align_e::none ? align_e ::left : this->alignment;

      stack.buf[0] = static_cast<char>(arg);

      stack.length = 1;
    } else {
      stack.check_neg = true;

      stack.add_preffix = true;
      this->alignment =
          this->alignment == align_e::none ? align_e ::right : this->alignment;

      stack.length = stack.bt.from_integral_fill(

          stack.buf, stack.buf_size, arg, uppser_case,
          uint8_t(uint8_t(type) & uint8_t(type_e::raidex_mask_)));
    }
    if (!stack.length) {
      ctx.as_error(
          "[Error]basic_format_specs_t::format_specs():couldnt convert to "
          "integral");
      return false;
    }
  } else {
    ctx.as_error(
        "[Error]basic_format_specs_t::format_specs():type not suppoerted");

    return false;
  }
  return format_specs_finish(ctx, stack);
}

template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::format_specs_finish(
    format_context_t<version_v> &ctx, format_fn_obj &stack) noexcept {
  using bcview = base_string_view_t<version_v>;
  stack.add_preffix &= alt;
  stack.length = stack.length ? *stack.length : 0;
  const char *numeric_begin{stack.buf};
  if (*stack.length) {
    bool is_neg{};
    auto raidex = uint8_t(uint8_t(type) & uint8_t(type_e::raidex_mask_));
    if (!stack.add_preffix) raidex = 0;
    if (stack.check_neg && stack.buf[0] == '-') {
      numeric_begin++;
      stack.buf++;
      is_neg = true;
    }
    if (type == type_e::Hex_float) {
      numeric_begin += uintlen_t(alt) << 1;
    }
    if (raidex == 8) {
      *--stack.buf = '0';
    } else if (raidex == 16) {
      *--stack.buf = uppser_case ? 'X' : 'x';
      *--stack.buf = '0';
    } else if (raidex == 2) {
      *--stack.buf = uppser_case ? 'B' : 'b';
      *--stack.buf = '0';
    }
    if (is_neg) {
      *--stack.buf = '-';
    } else if (stack.check_neg) {
      if (sign == sign_e::plus) *--stack.buf = '+';
      if (sign == sign_e::space) *--stack.buf = ' ';
    }
    *stack.length += stack.offset - (stack.buf - stack.buf_arr);
  }

  char *empty_part_end{stack.buf_arr + stack.buf_size};
  char *empty_part_begin{stack.buf + *stack.length};
  base_out_it_t<version_v> actual_it = ctx.out();
  out_buf_it_t<version_v> output{actual_it, empty_part_begin,
                                 uintlen_t(empty_part_end - empty_part_begin),
                                 ctx.encoding()};
  base_out_it_t<version_v> it{output};
  if (stack.is_string) {
    *stack.length = stack.as_string.len;
  }
  width = std::max(width, *stack.length);
  std::ignore = actual_it.reserve(width, ctx.encoding());
  uintlen_t delta{};
  if (this->leading_zero &&
      (alignment == align_e::right || alignment == align_e::none)) {
    uintlen_t prefix_len = uintlen_t(numeric_begin - stack.buf);
    std::ignore =
        it.append(bcview::make(stack.buf, prefix_len, ctx.encoding()));
    delta = width - *stack.length;
    *stack.length -= prefix_len;
    std::ignore =
        it.multi_push_back('0', std::exchange(delta, {}), ctx.encoding());

  } else {
    numeric_begin = stack.buf;
    delta = width - *stack.length;
  }
  if (delta && (alignment == align_e::right || alignment == align_e::center)) {
    uintlen_t rigth_delta{delta >> uint8_t(alignment == align_e::center)};
    delta -= rigth_delta;
    std::ignore = it.multi_push_back(fill_char, std::exchange(rigth_delta, {}),
                                     ctx.encoding());
  }

  std::ignore = output.flush();
  if (stack.is_string) {
    std::ignore = actual_it.format_back_insert_append_pv_fn_(
        unsafe_ns::unsafe_v, stack.as_string);
  } else {
    std::ignore = actual_it.append(
        bcview::make(numeric_begin, *stack.length, ctx.encoding()));
  }
  if (delta && (alignment == align_e::left || alignment == align_e::center)) {
    std::ignore =
        it.multi_push_back(fill_char, std::exchange(delta, {}), ctx.encoding());
  }
  if (output.flush() && actual_it) {
    return true;
  }
  return ctx.advance_to(nullptr);
}

template <version_t version_v>
MJZ_CX_FN success_t basic_format_specs_t<version_v>::format_specs_start(
    format_context_t<version_v> &ctx, format_fn_obj &stack) noexcept {
  stack.offset = 3;
  stack.buf_size -= stack.offset;
  stack.buf = stack.buf_arr + 3;
  stack.my_type = uint8_t(uint8_t(type) & uint8_t(type_e::type_mask_));
  if (ctx.encoding() != encodings_e::ascii) {
    ctx.as_error(
        "[Error]basic_format_specs_t::format_specs():only ascii is "
        "suppoerted!(note that this "
        "may change in later versions)");
    return false;
  }

  if (dyn_width &&
      !ctx.arg(width).template visit_xcr<uint8_t, uint16_t, uint32_t, uint64_t>(
          [&](uint64_t val) noexcept {
            width = uintlen_t(val);
            dyn_width = false;
          })) {
    ctx.as_error(
        "[Error]basic_format_specs_t::format_specs():couldnt get width");
    return false;
  }
  if (dyn_precision &&
      !ctx.arg(precision)
           .template visit_xcr<uint8_t, uint16_t, uint32_t, uint64_t>(
               [&](uint64_t val) noexcept {
                 precision = uintlen_t(val);
                 dyn_precision = false;
               })) {
    ctx.as_error(
        "[Error]basic_format_specs_t::format_specs():couldnt get width");
    return false;
  }
  return true;
}

template <version_t version_v>
template <typename T>
MJZ_CX_FN base_out_it_t<version_v> basic_format_specs_t<version_v>::format_fill(
    auto &&place_stuff, format_context_t<version_v> &ctx) const noexcept {
  struct Place_t : void_struct_t {
    std::remove_reference_t<decltype(place_stuff)> *function{};
    MJZ_CX_FN static success_t place_fn(void_struct_t &obj) noexcept {
      return (*static_cast<Place_t &>(obj).function)();
    }
    MJZ_CX_FN Place_t(decltype(function) ptr) noexcept : function(ptr) {}
  };
  Place_t p{&place_stuff};
  char buffer[std::max<uintlen_t>(8, conversion_buffer_size_v<T>)]{};
  return format_fill_pv(Place_t::place_fn, p, buffer, sizeof(buffer), ctx);
}

template <version_t version_v>
MJZ_CX_FN base_out_it_t<version_v>
basic_format_specs_t<version_v>::format_fill_pv(
    success_t (*place_fn)(void_struct_t &) noexcept, void_struct_t &place_obj,
    char *buffer, uintlen_t buffer_size,
    format_context_t<version_v> &ctx) const noexcept {
  auto place_stuff = [&]() noexcept { return place_fn(place_obj); };
  auto real_output = ctx.out();
  MJZ_RELEASE {
    if (ctx.out()) ctx.advance_to(real_output);
  };
  base_out_it_t<version_v> it{real_output};
  uintlen_t my_width{};
  if (dyn_width &&
      !ctx.arg(width).template visit_xcr<uint8_t, uint16_t, uint32_t, uint64_t>(
          [&](uint64_t val) noexcept { my_width = uintlen_t(val); })) {
    ctx.as_error(
        "[Error]basic_format_specs_t::format_specs():couldnt get width");
    return nullptr;
  } else {
    my_width = width;
  }
  bool output_buffer_was_used{};
  buffer_out_buf_t<version_v> output_buffer{};
  if (my_width) {
    output_buffer.encoding = ctx.encoding();
    output_buffer.buffer = buffer;
    output_buffer.capacity = buffer_size >> 1;
    buffer += output_buffer.capacity;
    buffer_size -= output_buffer.capacity;
    output_buffer.info.is_thread_safe = false;
    output_buffer.alloc = ctx.allocator();
    out_buf_it_t<version_v> output_iter{output_buffer, buffer, buffer_size,
                                        ctx.encoding()};
    my_width = width;
    auto output = ctx.out();
    MJZ_RELEASE {
      if (ctx.out()) ctx.advance_to(output);
    };

    if (!ctx.advance_to(output_iter)) return nullptr;
    if (!place_stuff()) return nullptr;
    output_buffer_was_used = true;
  };
  if (output_buffer.invalid) {
    ctx.as_error(
        "[Error]default_formatter_t::format(tuple,auto):"
        "error at counting the output");
    return nullptr;
  }
  base_out_it_t<version_v> actual_it = ctx.out();
  out_buf_it_t<version_v> output{actual_it, buffer, buffer_size,
                                 ctx.encoding()};
  uintlen_t length = output_buffer.length;
  my_width = std::max(my_width, length);
  it = output;
  uintlen_t delta = my_width - length;
  std::ignore = actual_it.reserve(my_width, ctx.encoding());
  if (!ctx.advance_to(it)) return nullptr;
  if (alignment == align_e::right || alignment == align_e::center) {
    uintlen_t rigth_delta{delta >> uint8_t(alignment == align_e::center)};
    delta -= rigth_delta;
    std::ignore = it.multi_push_back(fill_char, rigth_delta, ctx.encoding());
  }
  if (output_buffer_was_used) {
    std::ignore = it.append(output_buffer.bview());
  } else {
    if (!place_stuff()) return nullptr;
  }
  if (alignment == align_e::left || alignment == align_e::center) {
    std::ignore = it.multi_push_back(fill_char, delta, ctx.encoding());
  }
  if (output.flush() && actual_it) {
    return actual_it;
  }
  return nullptr;
}

};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_basic_formatters_HPP_FILE_