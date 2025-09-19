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

#include "base_abi.hpp"
//
//
#ifndef MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_
#define MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz {
  template <auto>
  struct err_str_vst {};
  template <auto...>
  struct err_vst {
    constexpr static void print(auto...) = delete;
  };

}  // namespace mjz
MJZ_EXPORT namespace mjz ::bstr_ns::format_ns {
  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t>
  parse_context_t<version_v>::parse_starting_ulen() noexcept {
    if (encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        as_error(
            "[Error]parse_starting_ulen:only ascii is suppoerted!(note that "
            "this "
            "may change in later versions)");
        return nullopt;
      }
    }
    if (!front()) {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_starting_ulen:expected an unsigned integer");
        return nullopt;
      }
    }

    constexpr auto max_val = uintlen_t(std::numeric_limits<intlen_t>::max());
    constexpr auto before_max_val = max_val / uintlen_t(10);
    uintlen_t value{};
    for (auto digit = front(); digit && '0' <= *digit && *digit <= '9';
         unchecked_advance_amount_(1), digit = front()) {
      if (value > before_max_val) {
        value = max_val + 1;
        break;
      }
      value = value * 10 + uintlen_t(*digit - '0');
    }
    if (value > max_val) {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_starting_ulen:too many digits,big number!");
        return nullopt;
      }
    }
    return value;
  }

  template <version_t version_v>
  MJZ_CX_FN std::optional<hash_bytes_t<version_v>>
  parse_context_t<version_v>::parse_starting_name() noexcept {
    if (encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        as_error(
            "[Error]parse_starting_name:only ascii is suppoerted!(note that "
            "this "
            "may change in later versions)");
        return nullopt;
      }
    }
    if (!front()) {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_starting_name:expected a name");
        return nullopt;
      }
    }

    uintlen_t name_len{};
    for (auto digit = at(name_len); digit && !(*digit == '}' || *digit == ':' ||
                                               *digit == ';' || *digit == ']');
         name_len++, digit = at(name_len)) {
    }
    view_t name = view().make_subview(0, name_len);
    if (!advance_amount(name_len)) {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_starting_name:expected an ending to name");
        return nullopt;
      }
    }
    hash_bytes_t<version_v> ret(name.data(), name.length());
    return hash_bytes_t<version_v>{ret};
  }
  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t>
  parse_context_t<version_v>::parse_arg_index() noexcept {
    if (encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        as_error(
            "[Error]parse_arg_index:only ascii is suppoerted!(note "
            "that this "
            "may change in later versions)");
        return nullopt;
      }
    }
    auto charechter = front();
    if (!charechter || *charechter == '}' || *charechter == ':' ||
        *charechter == ';') {
      return next_arg_id();
    }
    if (*charechter < '0' && *charechter > '9') {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_arg_index:expected an integer for id");
        return nullopt;
      }
    }
    uintlen_t id{};
    if (*charechter != '0') {
      auto id_ = parse_starting_ulen();
      if (!id_) return nullopt;
      id = *id_;
    } else {
      if (!unchecked_advance_amount_(1)) return nullopt;
    }
    charechter = front();
    if (charechter && *charechter != '}' && *charechter != ':' &&
        *charechter != ';') {
      MJZ_IS_UNLIKELY {
        as_error("[Error]parse_arg_index:expected an ending for id");
        return nullopt;
      }
    }
    if (!check_arg_id(id)) return nullopt;
    return id;
  }

  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t>
  parse_context_t<version_v>::find_name_index(hash_bytes_t<version_v> name,
                                              view_t name_str) noexcept {
    MJZ_RELEASE { main_ctx().name_ptr = nullptr; };
    typename base_context_t<version_v>::name_t name_v{name, name_str};
    main_ctx().name_ptr = &name_v;
    for (uintlen_t i{}; i < main_ctx().number_of_args; i++) {
      if (!main_ctx().parse_and_format_call_at(i, true)) return nullopt;
      if (&name_v != main_ctx().name_ptr) return i;
    }
    if (parse_only()) {
      as_error(
          "[Error]find_name_index:named argument not faound. most of the names "
          "are "
          "not avalible in the format checker, use indexies instead, or turn "
          "off "
          "the format checking");
    } else {
      as_error("[Error]find_name_index:named argument not faound");
    }
    return nullopt;
  }

  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t>
  parse_context_t<version_v>::parse_arg_id() noexcept {
    if (encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        as_error(
            "[Error]parse_arg_id:only ascii is suppoerted!(note "
            "that this "
            "may change in later versions)");
        return nullopt;
      }
    }
    auto charechter = front();
    if (!charechter || *charechter == '}' || *charechter == ':' ||
        *charechter == ';') {
      auto ret = next_arg_id();
      if (!ret) return nullopt;
      return *ret;
    }
    if ('0' <= *charechter && *charechter <= '9') {
      auto ret = parse_arg_index();
      if (!ret) return nullopt;
      return *ret;
    }
    view_t name_str = view();
    auto name = parse_starting_name();
    if (!name) return nullopt;
    auto name_len = uintlen_t(data_left() - name_str.data());
    std::ignore = name_str.as_subview(0, name_len);

    auto ret = find_name_index(*name, name_str);
    if (!ret || !check_arg_id(*ret)) return nullopt;
    return *ret;
  }
  template <version_t version_v>
  MJZ_CX_FN success_t parse_and_format_data_t<
      version_v>::parse_format_replacement_field() noexcept {
    if (parse_ctx().encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]parse_format_replacement_field:only ascii is "
            "suppoerted!(note "
            "that this "
            "may change in later versions)");
        return false;
      }
    }
    auto charechter = parse_ctx().front();
    if (charechter == '{') {
      return parse_ctx().unchecked_advance_amount_(1) &&
             append_text(sview_t{"{"});
    }
    if (parse_only()) {
      if (this->cx_parse_storage_of_args) {
        const uintlen_t arg_i{uintlen_t(-1) - base_ctx().err_index};
        cx_formatter_storage_base_ref_t<version_v> &fs_ref =
            const_cast<cx_formatter_storage_base_ref_t<version_v> &>(
                base_ctx().cx_parse_storage_of_args[arg_i]);
        fs_ref.formatting_str_index_begin =
            base_ctx().remaining_format_string_index - 1;
      }
    }

    if (!charechter || *charechter == '}') {
      // string was "{}", and we have a replacement field
      auto id = parse_ctx().next_arg_id();
      return id && call_argument_formatter(*id);
    }
    auto id = parse_ctx().parse_arg_id();
    if (!id) return false;
    charechter = parse_ctx().front();

    auto &actual_out = main_ctx().output_it;
    // this is the output iterator before all of the optional wrapping takes
    // place.
    out_it_t actual_out_buf = actual_out;
    bool has_filter{};
    MJZ_RELEASE {
      if (has_filter) actual_out = actual_out_buf;
    };
    if (charechter && *charechter == ':') {
      return parse_ctx().unchecked_advance_amount_(1) &&
             call_argument_formatter(*id);
    }
    if (!charechter || *charechter == '}') {
      MJZ_IS_LIKELY { return call_argument_formatter(*id); }
    }

    format_ctx().as_error(
        "[Error]parse_format_replacement_field:expected an replacement field "
        "end");
    return false;
  }

  template <version_t version_v>
  MJZ_CX_FN success_t parse_and_format_data_t<
      version_v>::parse_formating_string_nocache() noexcept {
    out_it_t output = format_ctx().out();
    for (auto charechter = parse_ctx().front(); output && charechter;
         charechter = parse_ctx().front()) {
      view_t remains = parse_ctx().view();
      const char *ptr = remains.data();
      const char *end = remains.data() + remains.length();
      for (;;) {
        if (ptr == end || *ptr == '{' || *ptr == '}') break;
        ptr++;
      }
      uintlen_t braket_location = uintlen_t(ptr - remains.data());
      std::ignore = output.append(
          remains.make_subview(0, braket_location).unsafe_handle());
      std::ignore = parse_ctx().unchecked_advance_amount_(braket_location);
      if (ptr == end) break;
      charechter = *ptr;
      if (!parse_ctx().unchecked_advance_amount_(1)) return false;
      if (*charechter == '{') {
        if (!parse_format_replacement_field()) return false;
        // the output must match the original to avoid conflicts
        format_ctx().advance_to(output);
        continue;
      }
      asserts(asserts.assume_rn, *charechter == '}');
      auto c = parse_ctx().front();
      if (!c || *c != '}') {
        MJZ_IS_UNLIKELY {
          parse_ctx().as_error(
              "[Error]parse_formating_string: unmatched '}' , expected '}' "
              "here "
              "as an escape sequence");
          return false;
        }
      }
      if (!parse_ctx().unchecked_advance_amount_(1)) return false;
      output.push_back('}', parse_ctx().encoding());
    }

    return output && !base_ctx().err_content;
  }
  template <version_t version_v>
  MJZ_CX_FN success_t
  parse_and_format_data_t<version_v>::parse_formating_string_cache() noexcept {
    out_it_t output = format_ctx().out();
    const uintlen_t feilds = base_ctx().number_of_cx_parse_storage_of_args;
    const cx_formatter_storage_base_ref_t<version_v> *feild_arr =
        base_ctx().cx_parse_storage_of_args;
    for (uintlen_t i{}; i < feilds; i++) {
      const cx_formatter_storage_base_ref_t<version_v> val = feild_arr[i];
      std::ignore = output.append_u_(
          main_ctx()
              .format_string(main_ctx().remaining_format_string_index,
                             val.formatting_str_index_begin)
              .unsafe_handle());
      main_ctx().remaining_format_string_index = val.formatting_str_index_end;
      if (!base_ctx().cache_format_call_at(
              val.index_of_element, val.formatter_ptr, uint8_t(val.type))) {
        return false;
      }
    }
    std::ignore = output.append_u_(parse_ctx().view().unsafe_handle());
    main_ctx().remaining_format_string_index =
        base_ctx().format_string.length();
    return !!output;
  }
  template <version_t version_v>
  MJZ_CX_FN success_t
  parse_and_format_data_t<version_v>::parse_formating_string() noexcept {
    out_it_t /*the  copy is intentional */ output{};
    success_t ret = [this, &output]() noexcept {
      format_context_t<version_v> &fctx_ = format_ctx();
      if (parse_ctx().encoding() != encodings_e::ascii) {
        MJZ_IS_UNLIKELY {
          fctx_.as_error(
              "[Error]parse_formating_string:only ascii is suppoerted!(note "
              "that this "
              "may change in later versions)");
          return false;
        }
      }
      base_ctx().buf_view.encoding = base_ctx().format_string.get_encoding();
      base_ctx().buf_view.length = 0;
      if (!fctx_.advance_to(fctx_.out())) return false;
      output = fctx_.out();
      if (!parse_ctx().parse_only() &&
          !output.reserve(base_ctx().format_string.length(),
                          fctx_.encoding())) {
        MJZ_IS_UNLIKELY {
          parse_ctx().as_error(
              "[Error]parse_formating_string: could't reserve an aproximation "
              "of "
              "the "
              "output buffer");
          return false;
        }
      }

      if (base_ctx().number_of_cx_parse_storage_of_args)
        return parse_formating_string_cache();
      return parse_formating_string_nocache();
    }();
    if (ret) format_ctx().advance_to(output);
    return ret;
  }

  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t>
  parse_and_format_data_t<version_v>::get_parse_filter_numeric(
      uintlen_t defult) noexcept {
    if (parse_ctx().encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_parse_filter_numeric:only ascii is "
            "suppoerted!(note "
            "that this "
            "may change in later versions)");
        return nullopt;
      }
    }
    auto charechter = parse_ctx().front();
    if (!charechter) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_parse_filter_numeric:unexpected end of feild");
        return nullopt;
      }
    }
    std::optional<uintlen_t> ret{};
    if ('0' <= *charechter && *charechter <= '9')
      return parse_ctx().parse_starting_ulen();
    if (*charechter != '{') return defult;
    if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
    auto ret_id = parse_ctx().parse_arg_id();
    if (!ret_id) return nullopt;
    charechter = parse_ctx().front();
    if (!charechter || *charechter != '}') {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_parse_filter_numeric:expected '}' for the dynamic "
            "filter_numeric argument id end");
        return nullopt;
      }
    }
    if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
    if (!parse_ctx()
             .type_v(*ret_id)
             .template is_one_of_xcr<uint8_t, uint16_t, uint32_t,
                                     uintlen_t>()) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]basic_format_specs_t::get_parse_filter_numeric():invalid "
            "dynamic filter_numeric "
            "type(must be unsigned integral (of type uintlen_t))");
        return nullopt;
      }
    }
    if (!parse_ctx().parse_only() &&
        !format_ctx()
             .arg(*ret_id)
             .template visit_xcr<uint8_t, uint16_t, uint32_t, uintlen_t>(
                 [&](uintlen_t val) noexcept { ret = uintlen_t(val); })) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]basic_format_specs_t::get_parse_filter_numeric():couldnt "
            "get "
            "filter_numeric");
        return nullopt;
      }
    }
    if (!ret) ret = defult;
    return ret;
  }
  template <version_t version_v>
  MJZ_CX_FN std::optional<uintlen_t> parse_context_t<version_v>::get_numeric(
      uintlen_t defult) noexcept {
    if (!main_ctx().parse_and_format_data_ptr) {
      MJZ_IS_UNLIKELY {
        as_error(
            "parse_context_t::get_numeric:unexpected intanceiation of "
            "parse_context_t "
            "outside parse_and_format_data_t");
        return nullopt;
      }
    }
    return main_ctx().parse_and_format_data_ptr->get_parse_filter_numeric(
        defult);
  }
  template <version_t version_v>
  MJZ_CX_FN std::optional<pair_t<uintlen_t /*index*/, uintlen_t /*length*/>>

  parse_context_t<version_v>::get_slice() noexcept {
    return main_ctx().get_slice_parse_filter(false);
  }

  template <version_t version_v>
  MJZ_CX_FN std::optional<pair_t<uintlen_t /*index*/, uintlen_t /*length*/>>
  parse_and_format_data_t<version_v>::get_slice_parse_filter(
      bool check_after_slice) noexcept {
    if (parse_ctx().encoding() != encodings_e::ascii) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_slice_parse_filter:only ascii is "
            "suppoerted!(note "
            "that this "
            "may change in later versions)");
        return nullopt;
      }
    }

    auto charechter = parse_ctx().front();
    std::optional<uintlen_t> beg_loc = nullopt;
    std::optional<uintlen_t> end_loc = nullopt;
    if (!charechter) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error("[Error]get_slice_parse_filter: unexpected end");
        return nullopt;
      }
    }
    if (*charechter != '[')
      return pair_t<uintlen_t, uintlen_t>{0, uintlen_t(-1)};
    if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
    charechter = parse_ctx().front();
    if (!charechter) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error("[Error]get_slice_parse_filter: unexpected end");
        return nullopt;
      }
    }
    if (*charechter == ']') {
      if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
      //"[]"no output at all
      return pair_t<uintlen_t, uintlen_t>{uintlen_t(-1), uintlen_t(0)};
    }
    beg_loc = get_parse_filter_numeric(uintlen_t(0));
    if (!beg_loc) return nullopt;
    charechter = parse_ctx().front();
    if (!charechter) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error("[Error]get_slice_parse_filter: unexpected end");
        return nullopt;
      }
    }
    if (*charechter == ']') {
      if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
      //"[i]" index
      return pair_t<uintlen_t, uintlen_t>{*beg_loc, uintlen_t(1)};
    }
    if (*charechter != ':') {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_slice_parse_filter: expected slice symbol(':') for "
            "range "
            "or end symbol (']') for indexed");
        return nullopt;
      }
    }
    if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
    end_loc = get_parse_filter_numeric(uintlen_t(-1));
    if (!end_loc) return nullopt;
    charechter = parse_ctx().front();
    if (!charechter || *charechter != ']') {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_slice_parse_filter: expected ']' as slice end");
        return nullopt;
      }
    }
    if (!parse_ctx().unchecked_advance_amount_(1)) return nullopt;
    charechter = parse_ctx().front();
    if (check_after_slice &&
        (!charechter ||
         (*charechter != ':' && *charechter != '{' && *charechter != '['))) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_slice_parse_filter:unexpected symbol in filter ");
        return nullopt;
      }
    }
    beg_loc = beg_loc ? *beg_loc : 0;
    end_loc = end_loc ? *end_loc : uintlen_t(-1);
    if (*end_loc < *beg_loc) {
      MJZ_IS_UNLIKELY {
        format_ctx().as_error(
            "[Error]get_slice_parse_filter: end index must not be less than "
            "begin "
            "index ");
        return nullopt;
      }
    }
    return pair_t<uintlen_t, uintlen_t>{*beg_loc, *end_loc - *beg_loc};
  }
  template <version_t version_v>
  MJZ_CX_FN success_t parse_and_format_data_t<version_v>::append_text(
      view_t text) noexcept {
    bool success{true};
    out_it_t it = format_ctx().out();
    it.format_back_insert_append_pv_fn_(unsafe_ns::unsafe_v,
                                        text.unsafe_handle());
    return success && format_ctx().advance_to(it) && !base_ctx().err_content;
  }
  template <version_t version_v>
  MJZ_CX_FN success_t
  parse_and_format_data_t<version_v>::call_argument_formatter(
      uintlen_t id) noexcept {
    if (!parse_ctx().main_ctx().parse_and_format_call_at(id, false))
      return false;
    if (auto c = parse_ctx().front(); !c || *c == '}') {
      MJZ_IS_LIKELY {
        if (!(!c || parse_ctx().unchecked_advance_amount_(1))) return false;
        if (!base_ctx().parse_only()) return true;
        if (!base_ctx().cx_parse_storage_of_args) return true;
        cx_formatter_storage_base_ref_t<version_v> &fs_ref =
            const_cast<cx_formatter_storage_base_ref_t<version_v> &>(
                base_ctx().cx_parse_storage_of_args[(uintlen_t(-2) -
                                                     base_ctx().err_index)]);
        fs_ref.formatting_str_index_end =
            base_ctx().remaining_format_string_index;
        return true;
      }
    }
    format_ctx().as_error(
        "[Error]call_argument_formatter(): expected '}' at the end of "
        "replacement feild ");
    return false;
  }

  template <typename T, version_t version_v>
  concept valid_formatter_format_c =
      requires(T &obj, parse_context_t<version_v> &pctx) {
        { obj.~T() } noexcept;
        { T() } noexcept;
        { obj.parse(pctx) } noexcept -> std::same_as<success_t>;
        { obj = std::as_const(obj) } noexcept;
      };

  template <version_t version_v, valid_formatter_format_c<version_v> T>
  struct formatter_formatter_base_t {
    using view_t = basic_string_view_t<version_v>;
    MJZ_DEFAULTED_CLASS(formatter_formatter_base_t);
    MJZ_CX_FN formatter_formatter_base_t(view_t name_) noexcept : name(name_) {}
    view_t input{};
    view_t name{};
    uintlen_t index_in_arragment{uintlen_t(-1)};
    T object{};
    template <class L>
    MJZ_CX_FN auto make(L &&) noexcept;
  };
  template <version_t version_v, valid_formatter_format_c<version_v> T,
            class Lambda_t = decltype([]<class... Ts>(Ts &&...) noexcept
                                          -> success_t { return true; })>
    requires callable_c<
        const Lambda_t,
        success_t(optional_ref_t<formatter_formatter_base_t<version_v, T>> obj,
                  format_context_t<version_v> & ctx) noexcept>
  struct formatter_formatter_ref_t {
    using view_t = basic_string_view_t<version_v>;
    using base_t = formatter_formatter_base_t<version_v, T>;
    optional_ref_t<base_t> base{};
    Lambda_t l{};
  };

  template <version_t version_v, valid_formatter_format_c<version_v> T>
  template <class L>
  MJZ_CX_FN auto formatter_formatter_base_t<version_v, T>::make(
      L && lambda) noexcept {
    return formatter_formatter_ref_t<version_v, T, std::remove_cvref_t<L>>{
        this, std::forward<L>(lambda)};
  }
  template <version_t version_v, class T>
    requires requires() {
      {
        formatter_formatter_ref_t(std::declval<std::remove_cvref_t<T>>())
      } noexcept -> std::same_as<std::remove_cvref_t<T>>;
    }
  struct default_formatter_t<version_v, T, 55> {
    MJZ_MCONSTANT(bool) no_perfect_forwarding_v = false;
    MJZ_MCONSTANT(bool) can_bitcast_optimize_v = false;
    MJZ_MCONSTANT(bool) can_have_cx_formatter_v = false;
    using sview_t = static_string_view_t<version_v>;
    using view_t = basic_string_view_t<version_v>;
    std::remove_cvref_t<decltype(*std::declval<T>().base)> value{};
    MJZ_CX_FN success_t parse(parse_context_t<version_v> &ctx) noexcept {
      view_t v = ctx.view();
      if (!value.object.parse(ctx)) return false;
      v.remove_suffix(ctx.view().size());
      value.input = v;
      return true;
    }
    MJZ_CX_FN success_t
    format(T &obj, format_context_t<version_v> &ctx) const noexcept {
      if (obj.base) {
        obj.base->object = value.object;
        obj.base->input = value.input;
      }
      return obj.l(obj.base, ctx);
    }
    MJZ_CX_FN static success_t arg_name(
        const std::remove_reference_t<T> &arg,
        hash_context_t<version_v> &ctx) noexcept {
      auto opt = ctx.name();
      if (!opt) return false;
      argument_name_t<version_v> arg_name{*opt};
      if (arg.base && arg_name.name_str == arg.base->name) return ctx.matched();
      return true;
    }
  };

  template <version_t version_v>
  struct formatting_object_t : parse_and_format_data_t<version_v> {
    MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx()
        const noexcept {
      return *this;
    }
    MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept {
      return *this;
    }
    MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept { return *this; }

    MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept { return *this; }

    MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept {
      return *this;
    }

    MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept { return *this; }

    MJZ_NO_MV_NO_CPY(formatting_object_t);
    template <class>
    friend class mjz_private_accessed_t;
    using out_it_t = base_out_it_t<version_v>;
    using view_t = basic_string_view_t<version_v>;
    using stack_alloc_t =
        allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>;
    using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;

    MJZ_CX_FN formatting_object_t(alloc_ref_t alloc, std::span<char> cache_ref,
                                  uintlen_t cache_ref_align) noexcept {
      asserts(asserts.expect_rn,
              stack_alloc_t::align <= cache_ref_align &&
                  stack_alloc_t::align * 2 <= cache_ref.size());
      base_ctx().alloc = std::move(alloc);
      base_ctx().format_cache_ref = cache_ref;
      base_ctx().stack_alloc =
          stack_alloc_t{base_ctx().format_cache_ref, stack_alloc_t::align};
      std::span<char> blk =
          base_ctx().stack_alloc.fn_alloca(cache_ref.size() >> 1);
      asserts(asserts.assume_rn, !!blk.size());
      base_ctx().buf_view.begin_ptr = blk.data();
      base_ctx().buf_view.capacity = blk.size();
    }
    MJZ_CX_FN ~formatting_object_t() noexcept {
      base_ctx().stack_alloc.fn_dealloca(std::span<char>{
          base_ctx().buf_view.begin_ptr, base_ctx().buf_view.capacity});
    }

    MJZ_CX_FN void reset() noexcept {
      main_ctx().remaining_format_string_index = 0;
      main_ctx().next_arg_index = 0;
      main_ctx().output_it = out_it_t{};
      main_ctx().err_output = out_it_t{};
      main_ctx().err_content = view_t{};
      main_ctx().err_index = 0;
      main_ctx().data_of_args = nullptr;
      main_ctx().cx_parse_storage_of_args = nullptr;
      main_ctx().parse_and_format_fn_of_args = nullptr;
      main_ctx().number_of_args = 0;
      main_ctx().format_string = view_t{};
      main_ctx().name_ptr = nullptr;
    }

    template <typename... Ts>
    MJZ_CX_FN success_t vformat_to(out_it_t iter, view_t format_str,
                                   Ts &&...args) noexcept {
      return vformat_to_pv(
          iter, format_str,
          to_final_type_fn<version_v, Ts>(std::forward<Ts>(args))...);
    }

    template <typename L_v, typename... Ts>
    MJZ_CX_FN success_t format_to(out_it_t iter, L_v format_str,
                                  Ts &&...args) noexcept {
      return format_to_pv(
          iter, L_v(format_str),
          ((typed_arg_ref_final_type_t<version_v, Ts>)
               to_final_type_fn<version_v, Ts>(std::forward<Ts>(args)))...);
    }

   private:
    template <class... Ts>
    MJZ_CX_FN success_t vformat_to_pv(out_it_t iter, view_t format_str,
                                      Ts &&...args) noexcept {
      if constexpr ((!!sizeof...(Ts))) {
        MJZ_IF_CONSTEVAL {
          basic_cx_format_args_t<version_v, Ts...> buf{
              std::forward<Ts>(args)...};
          base_ctx().number_of_args = buf.arg_no_typed.count_args;
          base_ctx().data_of_args = buf.arg_no_typed.data_of_args;
          base_ctx().parse_and_format_fn_of_args =
              buf.arg_no_typed.parse_and_format_fn_of_args;
          // base_ctx().cx_parse_storage_of_args
          base_ctx().format_string = format_str;

          main_ctx().main_ctx().output_it = iter;
          return run_format();
        }
        else {
          basic_format_args_t<version_v, sizeof...(Ts)> buf{
              basic_format_args_ncx_tag{}, std::forward<Ts>(args)...};
          base_ctx().number_of_args = buf.count_args;
          base_ctx().data_of_args = buf.data_of_args;
          base_ctx().parse_and_format_fn_of_args =
              buf.parse_and_format_fn_of_args;
          // base_ctx().cx_parse_storage_of_args
          base_ctx().format_string = format_str;
          main_ctx().main_ctx().output_it = iter;
          return run_format();
        }
      } else {
        base_ctx().format_string = format_str;

        main_ctx().main_ctx().output_it = iter;
        return run_format();
      }
    }

    template <typename L_v, is_formatted_c<version_v>... Ts>
    MJZ_CX_FN success_t format_to_pv(out_it_t iter, L_v, Ts &&...args) noexcept;
    MJZ_CX_FN success_t run_format() noexcept {
      success_t s = main_ctx().parse_formating_string();
      return s;
    }
    template <version_t, typename...>
    friend struct cx_parser_t;

   public:
    template <class... Ts>
    MJZ_CX_FN success_t format_formatters(
        uintlen_t &count_of_used_args, view_t &format_in_last_out,
        formatter_formatter_base_t<version_v, Ts> &...args) noexcept {
      count_out_buf_t<version_v> out_{};
      uintlen_t per_cnt{};
      count_of_used_args = 0;
      if (!vformat_to(out_, format_in_last_out,
                      (args.make([&](auto obj, format_context_t<version_v>
                                                   &ctx) noexcept -> success_t {
                        if (!ctx.out().flush_buffer() || !ctx.advance_to(out_))
                          return false;
                        if (uintlen_t(-1) != obj->index_in_arragment) {
                          ctx.as_error(
                              "[Error]formatting_object_t::format_formatters: "
                              "duplicated the format entery");
                          return false;
                        }
                        uintlen_t cnt = out_.count;
                        out_.count += obj->input.size() + 2;
                        obj->input = format_in_last_out.make_subview(
                            per_cnt, cnt - per_cnt);
                        per_cnt = out_.count;
                        obj->index_in_arragment = count_of_used_args++;
                        return true;
                      }))...)) {
        if (auto c = format_in_last_out.at(base_ctx().err_index - 1);
            c && *c != '}')
          return false;
      }
      if (out_.invalid) return false;
      if (!format_ctx().out().flush_buffer()) return false;
      format_in_last_out.as_subview(per_cnt, out_.count - per_cnt);
      return true;
    }

    MJZ_DISABLE_ALL_WANINGS_START_;
  };
  MJZ_DISABLE_ALL_WANINGS_END_;

  template <version_t version_v, typename... Ts>
  struct cx_parser_t : public formatting_object_t<version_v> {
    MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx()
        const noexcept {
      return *this;
    }
    MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept {
      return *this;
    }
    MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept { return *this; }

    MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept { return *this; }

    MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept {
      return *this;
    }

    MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept {
      return *this;
    }
    MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept { return *this; }
    template <class>
    friend class mjz_private_accessed_t;
    using out_it_t = base_out_it_t<version_v>;
    using view_t = basic_string_view_t<version_v>;
    using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;
    count_out_buf_t<version_v> out{};
    success_t successful{true};
    MJZ_NO_MV_NO_CPY(cx_parser_t);
    template <size_t... Is>
      requires(sizeof...(Is) == sizeof...(Ts))
    MJZ_CX_FN cx_parser_t(view_t fmt_str, alloc_ref_t alloc,
                          std::span<char> cache_ref, uintlen_t cache_ref_align,
                          std::index_sequence<Is...>,
                          cx_formatter_storage_base_ref_t<version_v>
                              *cx_parse_storage_ptr = nullptr) noexcept
        : formatting_object_t<version_v>{std::move(alloc), cache_ref,
                                         cache_ref_align} {
      if constexpr (sizeof...(Ts)) {
        basic_format_args_t<version_v, sizeof...(Ts)> buf{
            basic_format_args_parse_tag{}, alias_t<void (*)(Ts &&...)>{}};
        base_ctx().number_of_args = buf.count_args;
        base_ctx().parse_and_format_fn_of_args =
            buf.parse_and_format_fn_of_args;

        /* cx_formatter_storage_ref_t<version_v>
            cx_parse_storage_of_args_buf[sizeof...(Ts)]{
                cx_formatter_storage_ref_t<version_v>(std::get<Is>(cx_store))...};
           cx_parse_storage_of_args_buf;*/
        base_ctx().cx_parse_storage_of_args = cx_parse_storage_ptr;
        base_ctx().err_index = uintlen_t(-1);
        base_ctx().format_string = fmt_str;

        main_ctx().main_ctx().output_it = out;
        successful = this->run_format();
      } else {
        base_ctx().err_index = uintlen_t(-1);
        base_ctx().format_string = fmt_str;

        main_ctx().main_ctx().output_it = out;
        successful = this->run_format();
      }

#if !MJZ_VERBOSE_FORMAT_ERROR
      asserts(successful);
#endif
    }
  };

  template <version_t version_v>
  template <typename L_v, is_formatted_c<version_v>... Ts>
  MJZ_CX_FN success_t formatting_object_t<version_v>::format_to_pv(
      out_it_t iter, L_v value, Ts && ...args) noexcept {
    if constexpr (requires() {
                    { int(L_v::format_optimization_lvl_v) } noexcept;
                  }) {
      [&]() noexcept {
        constexpr const int format_optimization_lvl_v =
            L_v::format_optimization_lvl_v;
        static_assert(
            requires() {
              { view_t(L_v()()) } noexcept;
            }, "see if you used \"...\"_fmt or not (you should) ");
        static_assert(0 <= format_optimization_lvl_v);
        //
        // cx_formatter_storages_t<version_v, Ts &&...> cx_store{};
        auto run_function_cx_f = []() noexcept
            -> pair_t<uintlen_t, std::optional<pair_t<uintlen_t, view_t>>> {
          constexpr auto align_v_ =
              allocs_ns::stack_alloc_ns::stack_allocator_meta_t<
                  version_v>::align;
          alignas(align_v_) char buffer_[format_stack_size_v<version_v>]{};
          MJZ_MAYBE_UNUSED cx_parser_t<version_v, Ts...> checker{
              view_t(L_v()()), alloc_ref_t{}, buffer_, align_v_,
              std::make_index_sequence<sizeof...(Ts)>{}};
          if (checker.successful)
            return {uintlen_t(-1) - checker.base_ctx().err_index, nullopt};
          return {0, pair_t<uintlen_t, view_t>{checker.base_ctx().err_index,
                                               checker.base_ctx().err_content}};
        };
        using run_function_cx_ft = static_data_t<decltype(run_function_cx_f)>;

        if constexpr ((sizeof...(Ts)) && format_optimization_lvl_v) {
          using give_first_fn_t_ = decltype([]() noexcept -> uintlen_t {
            constexpr uintlen_t f = run_function_cx_ft{}().first;
            return f;
          });
          if constexpr (!run_function_cx_ft{}().second &&
                        give_first_fn_t_{}()) {
            [&]<int = 0>() noexcept {
              using type_info_array_t =
                  std::array<cx_formatter_storage_base_ref_t<version_v>,
                             give_first_fn_t_{}()>;
              auto generate_table_f = []() noexcept {
                type_info_array_t ret{};
                constexpr auto align_v_ =
                    allocs_ns::stack_alloc_ns::stack_allocator_meta_t<
                        version_v>::align;
                alignas(
                    align_v_) char buffer_[format_stack_size_v<version_v>]{};
                MJZ_MAYBE_UNUSED cx_parser_t<version_v, Ts...> checker{
                    view_t(L_v()()),
                    alloc_ref_t{},
                    buffer_,
                    align_v_,
                    std::make_index_sequence<sizeof...(Ts)>{},
                    ret.data()};
                return ret;
              };
              using tuple_types = tuple_t<cx_formatter_storage_t<
                  version_v, formatter_type_t<version_v, Ts>>...>;
              using generate_table_ft =
                  static_data_t<decltype(generate_table_f)>;
              auto generate_format_table_f = []() noexcept {
                return []<size_t... Is>(std::index_sequence<Is...>) noexcept {
                  tuple_t<std::tuple_element_t<
                      generate_table_ft{}()[Is].index_of_element,
                      tuple_types>...>
                      formatters{};
                  type_info_array_t ret{
                      cx_formatter_storage_base_ref_t<version_v>{
                          .formatter_ptr{&std::get<Is>(formatters)}}...};
                  constexpr auto align_v_ =
                      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<
                          version_v>::align;
                  alignas(
                      align_v_) char buffer_[format_stack_size_v<version_v>]{};
                  MJZ_MAYBE_UNUSED cx_parser_t<version_v, Ts...> checker{
                      view_t(L_v()()),
                      alloc_ref_t{},
                      buffer_,
                      align_v_,
                      std::make_index_sequence<sizeof...(Ts)>{},
                      ret.data()};

                  return formatters;
                }(std::make_index_sequence<give_first_fn_t_{}()>{});
              };

              using generate_format_table_ft =
                  static_data_t<decltype(generate_format_table_f)>;
              auto link_tables_f = []() noexcept {
                type_info_array_t ret = generate_table_ft{}();
                auto &formatters = generate_format_table_ft{}();
                [&]<size_t... Is>(std::index_sequence<Is...>) noexcept {
                  std::ignore =
                      ((ret[Is].formatter_ptr = &std::get<Is>(formatters),
                        true) &&
                       ...);
                }(std::make_index_sequence<give_first_fn_t_{}()>{});
                return ret;
              };
              using link_tables_ft = static_data_t<decltype(link_tables_f)>;
              base_ctx().cx_parse_storage_of_args = link_tables_ft{}().data();
              base_ctx().number_of_cx_parse_storage_of_args =
                  link_tables_ft{}().size();
            }();
          };
        }
        auto failure_f =
            []() noexcept -> std::optional<pair_t<uintlen_t, view_t>> {
          return run_function_cx_ft{}().second;
        };
        using failure_ft = static_data_t<decltype(failure_f)>;
#if MJZ_VERBOSE_FORMAT_ERROR
        if constexpr (failure_ft{}()) {
          constexpr bool failed = !failure_ft{}();
          auto failure_fv = []() noexcept { return *failure_ft{}(); };
          using failure_t = static_data_t<decltype(failure_fv)>;
          auto format_str_len_f = []() noexcept {
            return view_t(L_v()()).length();
          };
          using format_str_len_t = static_data_t<decltype(format_str_len_f)>;
          auto second_part_fn =
              []() noexcept -> std::array<char, format_str_len_t {}() -
                                                    failure_t{}().first + 1> {
            std::array<char, format_str_len_t{}() - failure_t{}().first + 1>
                ret{};
            memcpy(ret.data(), view_t(L_v()()).data() + failure_t{}().first,
                   format_str_len_t{}() - failure_t{}().first);
            return ret;
          };
          auto error_part_fn = []() noexcept
              -> std::array<char, failure_t {}().second.length() + 1> {
            std::array<char, failure_t{}().second.length() + 1> ret{};
            memcpy(ret.data(), failure_t{}().second.data(),
                   failure_t{}().second.length());
            return ret;
          };
          auto first_part_fn =
              []() noexcept -> std::array<char, failure_t {}().first + 1> {
            std::array<char, failure_t{}().first + 1> ret{};
            memcpy(ret.data(), view_t(L_v()()).data(), failure_t{}().first);
            return ret;
          };
          constexpr auto &first_part =
              static_data_t<decltype(first_part_fn)>{}();
          constexpr auto &second_part =
              static_data_t<decltype(second_part_fn)>{}();
          constexpr auto &error_part =
              static_data_t<decltype(error_part_fn)>{}();
          static_assert(failed, "error masaage-> ");
          err_vst<>::print(err_str_vst<error_part>{});
          static_assert(failed, "at index -> ");
          err_vst<>::print(err_str_vst<failure_t{}().first>{});
          static_assert(failed, "before error-> ");
          err_vst<>::print(err_str_vst<first_part>{});
          static_assert(failed, "after error-> ");
          err_vst<>::print(err_str_vst<second_part>{});
          return false;
        }
#else
        MJZ_UNUSED failure_ft check{};
#endif
      }();
    }
    return vformat_to_pv(iter, value(), std::forward<Ts>(args)...);
  }

  namespace fmt_litteral_ns {

  template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L,
            int format_optimization_lvl>
  struct operator_fmt_t : static_string_view_t<version_v> {
    constexpr static const int format_optimization_lvl_v =
        format_optimization_lvl;
    MJZ_CX_FN operator_fmt_t() noexcept
        : static_string_view_t<version_v>{
              bstr_ns::litteral_ns::operator_view<L, version_v>()} {}
  };

  template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L,
            int format_optimization_lvl = 1>
  MJZ_CX_FN auto operator_fmt() noexcept {
    return operator_fmt_t<version_v, L, format_optimization_lvl>{};
  }
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_fmt() noexcept {
    return operator_fmt<version_t{}, L>();
  }
  // check only
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_fmto0() noexcept {
    return operator_fmt<version_t{}, L, 0>();
  }
  // no only parse of format text
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_fmto1() noexcept {
    return operator_fmt<version_t{}, L, 1>();
  }
  // no only parse of format text, of forrmatter object
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_fmto2() noexcept {
    return operator_fmt<version_t{}, L, 2>();
  }
  // no only parse of format text, of forrmatter object, no lookup
  template <bstr_ns::litteral_ns::str_litteral_t L>
  MJZ_CX_FN auto operator""_fmto3() noexcept {
    return operator_fmt<version_t{}, L, 3>();
  }
  }  // namespace fmt_litteral_ns

}  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_
