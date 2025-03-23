/*
 * thanks to msvc for some of the inspraition.
 */

#include "base_abi.hpp"
//
//
#ifndef MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_
#define MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_
namespace mjz {
template <auto>
struct err_str_vst {};
template <auto...>
struct err_vst {
  constexpr static void print(auto...) = delete;
};

}  // namespace mjz
namespace mjz ::bstr_ns::format_ns {

template <version_t version_v>
MJZ_CX_FN std::optional<uintlen_t>
parse_context_t<version_v>::parse_starting_ulen() noexcept {
  if (encoding() != encodings_e::ascii) {
    as_error(
        "[Error]parse_starting_ulen:only ascii is suppoerted!(note that this "
        "may change in later versions)");
    return nullopt;
  }
  if (!front()) {
    as_error("[Error]parse_starting_ulen:expected an unsigned integer");
    return nullopt;
  }

  constexpr auto max_val = uintlen_t(std::numeric_limits<intlen_t>::max());
  constexpr auto before_max_val = max_val / uintlen_t(10);
  uintlen_t value{};
  for (auto digit = front(); digit && '0' <= *digit && *digit <= '9';
       advance_amount(1), digit = front()) {
    if (value > before_max_val) {
      value = max_val + 1;
      break;
    }
    value = value * 10 + uintlen_t(*digit - '0');
  }
  if (value > max_val) {
    as_error("[Error]parse_starting_ulen:too many digits,big number!");
    return nullopt;
  }
  return value;
}

template <version_t version_v>
MJZ_CX_FN std::optional<hash_bytes_t<version_v>>
parse_context_t<version_v>::parse_starting_name() noexcept {
  if (encoding() != encodings_e::ascii) {
    as_error(
        "[Error]parse_starting_name:only ascii is suppoerted!(note that this "
        "may change in later versions)");
    return nullopt;
  }
  if (!front()) {
    as_error("[Error]parse_starting_name:expected a name");
    return nullopt;
  }

  uintlen_t name_len{};
  for (auto digit = at(name_len); digit && !(*digit == '}' || *digit == ':' ||
                                             *digit == ';' || *digit == ']');
       name_len++, digit = at(name_len)) {
  }
  view_t name = remaining_format_string.make_subview(0, name_len);
  if (  !advance_amount(name_len)) {
    as_error("[Error]parse_starting_name:expected an ending to name");
    return nullopt;
  }
  hash_bytes_t<version_v> ret(name.data(), name.length());
  return hash_bytes_t<version_v>{ret};
}
template <version_t version_v>
MJZ_CX_FN std::optional<uintlen_t>
parse_context_t<version_v>::parse_arg_index() noexcept {
  if (encoding() != encodings_e::ascii) {
    as_error(
        "[Error]parse_arg_index:only ascii is suppoerted!(note "
        "that this "
        "may change in later versions)");
    return nullopt;
  }
  auto charechter = front();
  if (!charechter ||* charechter == '}' || *charechter == ':' ||
      *charechter == ';') {
    return next_arg_id();
  }
  if (*charechter < '0' && *charechter > '9') {
    as_error("[Error]parse_arg_index:expected an integer for id");
    return nullopt;
  }
  uintlen_t id{};
  if (*charechter != '0') {
    auto id_ = parse_starting_ulen();
    if (!id_) return nullopt;
    id = *id_;
  } else {
    if (!advance_amount(1)) return nullopt;
  }
  charechter = front();
  if ( charechter &&* charechter != '}' && *charechter != ':' &&
          *charechter != ';') {
    as_error("[Error]parse_arg_index:expected an ending for id");
    return nullopt;
  }
  if (!check_arg_id(id)) {
    return nullopt;
  }
  return id;
}

template <version_t version_v>
MJZ_CX_FN std::optional<uintlen_t> parse_context_t<version_v>::find_name_index(
    hash_bytes_t<version_v> name, view_t name_str) noexcept {
  if (!parse_and_format_data_ptr) {
    as_error(
        "[Error]find_name_index:unexpected intanceiation of parse_context_t "
        "outside parse_and_format_data_t");
    return nullopt;
  }
  MJZ_RELEASE { base.name_ptr = nullptr; };
  typename base_context_t<version_v>::name_t name_v{name, name_str};
  base.name_ptr = &name_v;
  for (uintlen_t i{}; i < base.number_of_args; i++) {
    auto fn = base.parse_and_format_fn_of_args[i];
    auto args = base.data_of_args;
    typeless_arg_ref_t<version_v> obj{};
    obj.m = args ? args[i] : raw_storage_ref_u<version_v>{};
    obj.parse_and_format = fn;
    if (!((*fn)(obj, *parse_and_format_data_ptr))) return nullopt;
    if (&name_v != base.name_ptr) return i;
  }
  if (parse_only()) {
    as_error(
        "[Error]find_name_index:named argument not faound. most of the names "
        "are "
        "not avalible in the format checker, use indexies instead, or turn off "
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
    as_error(
        "[Error]parse_arg_id:only ascii is suppoerted!(note "
        "that this "
        "may change in later versions)");
    return nullopt;
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
  view_t name_str = remaining_format_string;
  auto name = parse_starting_name();
  if (!name) return nullopt;
  auto name_len = uintlen_t(remaining_format_string.data() - name_str.data());
  std::ignore = name_str.as_subview(0, name_len);

  auto ret = find_name_index(*name, name_str);
  if (!ret || !check_arg_id(*ret)) return nullopt;
  return *ret;
}
template <version_t version_v>
MJZ_CX_FN success_t
parse_and_format_data_t<version_v>::parse_format_replacement_field() noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]parse_format_replacement_field:only ascii is "
        "suppoerted!(note "
        "that this "
        "may change in later versions)");
    return false;
  }
  auto charechter = parse_context.front();
  if (charechter == '{') {
    return parse_context.advance_amount(1) && append_text(sview_t{"{"});
  }
  if (!charechter ||* charechter == '}') {
    // string was "{}", and we have a replacement field
    auto id = parse_context.next_arg_id();
    return id && call_argument_formatter(*id);
  }
  auto id = parse_context.parse_arg_id();
  if (!id) return false;
  charechter = parse_context.front();
  std::optional<sub_out_iter_t<version_v>> outer_sub_out_it{};
  std::optional<sub_out_iter_t<version_v>> inner_sub_out_it{};
  auto& actual_out = format_context.output_it;
  // this is the output iterator before all of the optional wrapping takes
  // place.
  out_it_t actual_out_buf = actual_out;
  bool has_filter{};
  MJZ_RELEASE {
    if (has_filter) actual_out = actual_out_buf;
  };
  if (charechter && *charechter == ';') {
    base_context.recursion_depth++;
    if (base_context.max_recursion_depth < base_context.recursion_depth) {
      format_context.as_error(
          "[Error]parse_format_replacement_field:max recursion depth reached try "
          "reducing the nested filters");
      return false;
    }
    has_filter = true;
    if (!parse_context.advance_amount(1)) return false;
    outer_sub_out_it = call_slice_parse_filter();
    if (!outer_sub_out_it) return false;
    if (*outer_sub_out_it) {
      actual_out = out_it_t(*outer_sub_out_it);
    }
    if (!call_formatted_parse_filters()) return false;
    inner_sub_out_it = call_slice_parse_filter();
    if (!inner_sub_out_it) return false;
    if (*inner_sub_out_it) {
      actual_out = out_it_t(*inner_sub_out_it);
    }
    charechter = parse_context.front();
    if (charechter && *charechter != ':') {
      format_context.as_error(
          "[Error]parse_format_replacement_field:unexpected symbol as ending "
          "of "
          "filter  note that ONLY "
          "{ID;[outer_slice]restricted_format_strings...[inner_slice]:SPEC} is "
          "valid ");
      return false;
    }
  }
  charechter = parse_context.front();
  if (charechter && *charechter == ':') {
    return parse_context.advance_amount(1) && call_argument_formatter(*id);
  }
  if (!charechter || *charechter == '}') {
    return call_argument_formatter(*id);
  }

  format_context.as_error(
      "[Error]parse_format_replacement_field:expected an replacement field "
      "end");
  return false;
}

template <version_t version_v>
MJZ_CX_FN success_t
parse_and_format_data_t<version_v>::parse_formating_string() noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]parse_formating_string:only ascii is suppoerted!(note "
        "that this "
        "may change in later versions)");
    return false;
  }

  out_it_t /*the  copy is intentional */ output = format_context.out();
  MJZ_RELEASE {
    if (format_context) {
      format_context.advance_to(output);
    }
  };
  if (!parse_context.parse_only() &&
      !output.reserve(base_context.format_string.length(),
                      format_context.encoding())) {
    parse_context.as_error(
        "[Error]parse_formating_string: could't reserve an aproximation of the "
        "output buffer");
    return false;
  }
  for (auto charechter = parse_context.front(); output && charechter;
       charechter = parse_context.front()) {
    view_t remains = parse_context.remaining_format_string;
    const char* ptr = remains.data();
    const char* end = remains.data() + remains.length();
    for (;;) {
      if (ptr == end || *ptr == '{' || *ptr == '}') break;
      ptr++;
    }
    uintlen_t braket_location = uintlen_t(ptr - remains.data());
    std::ignore =
        output.append(remains.make_subview(0, braket_location).unsafe_handle());
    std::ignore = parse_context.advance_amount(braket_location);
    if (ptr == end) break;
    charechter = *ptr;
    if (!parse_context.advance_amount(1)) return false;
    if (*charechter == '{') {
      if (!parse_format_replacement_field()) return false;
      // the output must match the original to avoid conflicts
      format_context.advance_to(output);
      continue;
    }
    asserts(asserts.assume_rn, *charechter == '}');
    auto c = parse_context.front();
    if (!c || *c != '}') {
      parse_context.as_error(
          "[Error]parse_formating_string: unmatched '}' , expected '}' "
          "here "
          "as an escape sequence");
      return false;
    }
    if (!parse_context.advance_amount(1)) return false;
    output.push_back('}', parse_context.encoding());
  }
  return output && !base_context.err_content;
}

template <version_t version_v>
MJZ_CX_FN success_t parse_and_format_data_t<version_v>::parse_formating_filter(
    bool& has_filter) noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]parse_formating_filter:only ascii is suppoerted!(note "
        "that this "
        "may change in later versions)");
    return false;
  }

  out_it_t& /*the  ref is intentional, we want propgation of output iterators */
      output = format_context.output_it;
  for (auto charechter = parse_context.front(); output && charechter;
       charechter = parse_context.front()) {
    view_t remains = parse_context.remaining_format_string;
    const char* ptr = remains.data();
    const char* end = remains.data() + remains.length();
    for (;;) {
      if (ptr == end || *ptr == '{' || *ptr == '}' || *ptr == '[' ||
          *ptr == ']' || *ptr == ':')
        break;
      ptr++;
    }
    uintlen_t braket_location = uintlen_t(ptr - remains.data());
    std::ignore =
        output.append(remains.make_subview(0, braket_location).unsafe_handle());
    std::ignore = parse_context.advance_amount(braket_location);
    if (ptr == end) break;
    charechter = *ptr;
    // [[=>[ or ]]=>] as escape of slices and :: to secape : beacaue ':' is used
    // as the terminator of the filter
    if (*charechter == '[' || *charechter == ']' || *charechter == ':') {
      std::optional<char> escape{parse_context.at(1)};
      if (!escape) {
        parse_context.as_error(
            "[Error]parse_formating_filter: slice vs terminator vs text "
            "ambigouity");
        return false;
      }
      if (*charechter != *escape) {
        has_filter = false;
        // go and parse the slice
        return true;
      }
      if (!parse_context.advance_amount(2)) return false;
      output.push_back(*escape, parse_context.encoding());
      continue;
    }
    if (!parse_context.advance_amount(1)) return false;
    if (*charechter == '{') {
      if (!parse_format_replacement_field()) return false;
      continue;
    }
    asserts(asserts.assume_rn, *charechter == '}');
    auto c = parse_context.front();
    if (!c || *c != '}') {
      parse_context.as_error(
          "[Error]parse_formating_filter: unmatched '}' , expected '}' "
          "here "
          "as an escape sequence");
      return false;
    }
    if (!parse_context.advance_amount(1)) return false;
    output.push_back('}', parse_context.encoding());
  }
  return output && !base_context.err_content;
}

template <version_t version_v>
MJZ_CX_FN std::optional<uintlen_t>
parse_and_format_data_t<version_v>::get_parse_filter_numeric(
    uintlen_t defult) noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]get_parse_filter_numeric:only ascii is "
        "suppoerted!(note "
        "that this "
        "may change in later versions)");
    return nullopt;
  }
  auto charechter = parse_context.front();
  if (!charechter) {
    format_context.as_error(
        "[Error]get_parse_filter_numeric:unexpected end of feild");
    return nullopt;
  }
  std::optional<uintlen_t> ret{};
  if ('0' <= *charechter && *charechter <= '9') {
    return parse_context.parse_starting_ulen();
  }
  if (*charechter != '{') return defult;
  if (!parse_context.advance_amount(1)) return nullopt;
  auto ret_id = parse_context.parse_arg_id();
  if (!ret_id) return nullopt;
  charechter = parse_context.front();
  if (!charechter || *charechter != '}') {
    format_context.as_error(
        "[Error]get_parse_filter_numeric:expected '}' for the dynamic "
        "filter_numeric argument id end");
    return nullopt;
  }
  if (!parse_context.advance_amount(1)) return nullopt;
  if (!parse_context.type_v(*ret_id)
           .template is_one_of_xcr<uint8_t, uint16_t, uint32_t, uintlen_t>()) {
    format_context.as_error(
        "[Error]basic_format_specs_t::get_parse_filter_numeric():invalid "
        "dynamic filter_numeric "
        "type(must be unsigned integral (of type uintlen_t))");
    return nullopt;
  }
  if (!parse_context.parse_only() &&
      !format_context.arg(*ret_id)
           .template visit_xcr<uint8_t, uint16_t, uint32_t, uintlen_t>(
               [&](uintlen_t val) noexcept { ret = uintlen_t(val); })) {
    format_context.as_error(
        "[Error]basic_format_specs_t::get_parse_filter_numeric():couldnt get "
        "filter_numeric");
    return nullopt;
  }
  if (!ret) ret = defult;
  return ret;
}
template <version_t version_v>
MJZ_CX_FN std::optional<uintlen_t> parse_context_t<version_v>::get_numeric(
    uintlen_t defult) noexcept {
  if (!this->parse_and_format_data_ptr) {
    as_error(
        "parse_context_t::get_numeric:unexpected intanceiation of "
        "parse_context_t "
        "outside parse_and_format_data_t");
    return nullopt;
  }
  return this->parse_and_format_data_ptr->get_parse_filter_numeric(defult);
}
template <version_t version_v>
MJZ_CX_FN std::optional<std::pair<uintlen_t /*index*/, uintlen_t /*length*/>>

parse_context_t<version_v>::get_slice() noexcept {
  if (!this->parse_and_format_data_ptr) {
    as_error(
        "parse_context_t::get_slice:unexpected intanceiation of "
        "parse_context_t "
        "outside parse_and_format_data_t");
    return nullopt;
  }
  return this->parse_and_format_data_ptr->get_slice_parse_filter(false);
}

template <version_t version_v>
MJZ_CX_FN std::optional<std::pair<uintlen_t /*index*/, uintlen_t /*length*/>>
parse_and_format_data_t<version_v>::get_slice_parse_filter(
    bool check_after_slice) noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]call_slice_parse_filter:only ascii is "
        "suppoerted!(note "
        "that this "
        "may change in later versions)");
    return nullopt;
  }

  auto charechter = parse_context.front();
  std::optional<uintlen_t> beg_loc = nullopt;
  std::optional<uintlen_t> end_loc = nullopt;
  if (!charechter) {
    format_context.as_error("[Error]call_slice_parse_filter: unexpected end");
    return nullopt;
  }
  if (*charechter != '[') {
    return std::pair<uintlen_t, uintlen_t>{0, uintlen_t(-1)};
  }
  if (!parse_context.advance_amount(1)) return nullopt;
  charechter = parse_context.front();
  if (!charechter) {
    format_context.as_error("[Error]call_slice_parse_filter: unexpected end");
    return nullopt;
  }
  if (*charechter == ']') {
    if (!parse_context.advance_amount(1)) return nullopt;
    //"[]"no output at all
    return std::pair<uintlen_t, uintlen_t>{uintlen_t(-1), uintlen_t(0)};
  }
  beg_loc = get_parse_filter_numeric(uintlen_t(0));
  if (!beg_loc) return nullopt;
  charechter = parse_context.front();
  if (!charechter) {
    format_context.as_error("[Error]call_slice_parse_filter: unexpected end");
    return nullopt;
  }
  if (*charechter == ']') {
    if (!parse_context.advance_amount(1)) return nullopt;
    //"[i]" index
    return std::pair<uintlen_t, uintlen_t>{*beg_loc, uintlen_t(1)};
  }
  if (*charechter != ':') {
    format_context.as_error(
        "[Error]call_slice_parse_filter: expected slice symbol(':') for range "
        "or end symbol (']') for indexed");
    return nullopt;
  }
  if (!parse_context.advance_amount(1)) return nullopt;
  end_loc = get_parse_filter_numeric(uintlen_t(-1));
  if (!end_loc) return nullopt;
  charechter = parse_context.front();
  if (!charechter || *charechter != ']') {
    format_context.as_error(
        "[Error]call_slice_parse_filter: expected ']' as slice end");
    return nullopt;
  }
  if (!parse_context.advance_amount(1)) return nullopt;
  charechter = parse_context.front();
  if (check_after_slice &&
      (!charechter ||
       (*charechter != ':' && *charechter != '{' && *charechter != '['))) {
    format_context.as_error(
        "[Error]call_slice_parse_filter:unexpected symbol in filter ");
    return nullopt;
  }
  beg_loc = beg_loc ? *beg_loc : 0;
  end_loc = end_loc ? *end_loc : uintlen_t(-1);
  if (*end_loc < *beg_loc) {
    format_context.as_error(
        "[Error]call_slice_parse_filter: end index must not be less than begin "
        "index ");
    return nullopt;
  }
  return std::pair<uintlen_t, uintlen_t>{*beg_loc, *end_loc - *beg_loc};
}
template <version_t version_v>
MJZ_CX_FN std::optional<sub_out_iter_t<version_v>>
parse_and_format_data_t<version_v>::call_slice_parse_filter() noexcept {
  std::optional<std::pair<uintlen_t /*index*/, uintlen_t /*length*/>> slice =
      get_slice_parse_filter();
  if (!slice) return nullopt;
  if (!slice->first && slice->second == uintlen_t(-1)) {
    return sub_out_iter_t<version_v>{nullptr, parse_context.encoding()};
  }
  return sub_out_iter_t<version_v>{format_context.out(),
                                   parse_context.encoding(), slice->first,
                                   slice->second};
}
template <version_t version_v>
MJZ_CX_FN success_t
parse_and_format_data_t<version_v>::call_formatted_parse_filters() noexcept {
  if (parse_context.encoding() != encodings_e::ascii) {
    format_context.as_error(
        "[Error]call_formatted_parse_filters:only ascii is "
        "suppoerted!(note "
        "that this "
        "may change in later versions)");
    return false;
  }
  bool has_filter{true};
  auto charechter = parse_context.front();
  for (; charechter && has_filter; charechter = parse_context.front()) {
    /*this is the filter replacement feild,
     * this can be used to swap the perivous output_it(initially
     * outer_slice.output_it) with another wrapper that filters it before giving
     * the data to the perivous output_it, this only lasts for the duration of
     * the upper parse_format_replacement_field , multiple filter feilds and
     * nested ones can be used, the input of the last one is sliced by
     * inner_slice of replacement.
     */
    if (!parse_formating_filter(has_filter)) return false;
  }
  return true;
}
template <version_t version_v>
MJZ_CX_FN success_t
parse_and_format_data_t<version_v>::append_text(view_t text) noexcept {
  bool success{true};
  out_it_t it = format_context.out();
  it.format_back_insert_append_pv_fn_(unsafe_ns::unsafe_v,
                                      text.unsafe_handle());
  return success && format_context.advance_to(it) && !base_context.err_content;
}
template <version_t version_v>
MJZ_CX_FN success_t parse_and_format_data_t<version_v>::call_argument_formatter(
    uintlen_t id) noexcept {
  auto fn = base_context.parse_and_format_fn_of_args[id];
  auto args = base_context.data_of_args;
  typeless_arg_ref_t<version_v> obj{};
  obj.m = args ? args[id] : raw_storage_ref_u<version_v>{};
  obj.parse_and_format = fn;
  if (!((*fn)(obj, *this))) return false;
  if (auto c = parse_context.front(); !c || *c == '}') {
    return !c || parse_context.advance_amount(1);
  }
  format_context.as_error(
      "[Error]call_argument_formatter(): expected '}' at the end of "
      "replacement feild ");
  return false;
}
template <version_t version_v>
struct formatting_object_t : void_struct_t {
  MJZ_NO_MV_NO_CPY(formatting_object_t);
  template <class>
  friend class mjz_private_accessed_t;
  using out_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;
  base_context_t<version_v> base_context{};
  parse_and_format_data_t<version_v> context_data{};

  MJZ_CX_FN formatting_object_t(alloc_ref_t alloc = alloc_ref_t{}) noexcept
      : base_context{}, context_data{base_context} {
    base_context.alloc = std::move(alloc);
  }

  MJZ_CX_FN void reset() noexcept {
    context_data.parse_context.remaining_format_string = view_t{};
    context_data.parse_context.next_arg_index = 0;
    context_data.format_context.output_it = out_it_t{};
    context_data.base_context.err_output = out_it_t{};
    context_data.base_context.err_content = view_t{};
    context_data.base_context.err_index = 0;
    context_data.base_context.data_of_args = nullptr;
    context_data.base_context.parse_and_format_fn_of_args = nullptr;
    context_data.base_context.number_of_args = 0;
    context_data.base_context.format_string = view_t{};
    context_data.base_context.name_ptr = nullptr;
  }

  template <typename... Ts>
  MJZ_CX_FN success_t vformat_to(out_it_t iter, view_t format_str,
                                 Ts&&... args) noexcept {
    return vformat_to_pv(
        iter, format_str,
        to_final_type_fn<version_v, Ts>(std::forward<Ts>(args))...);
  }

  template <typename L_v, typename... Ts>
  MJZ_CX_FN success_t format_to(out_it_t iter, L_v format_str,
                                Ts&&... args) noexcept {
    return format_to_pv(
        iter, L_v(format_str),
       ((typed_arg_ref_final_type_t<version_v, Ts>)to_final_type_fn<version_v, Ts>(std::forward<Ts>(args)))...);
  }

 private:
  template <class... Ts >
  MJZ_CX_FN success_t vformat_to_pv(out_it_t iter, view_t format_str,
                                    Ts&&... args) noexcept {
    if constexpr ((!!sizeof...(Ts))) {
      MJZ_IF_CONSTEVAL {
        basic_cx_format_args_t<version_v, Ts...> buf{std::forward<Ts>(args)...};
        base_context.number_of_args = buf.arg_no_typed.count_args;
        base_context.data_of_args = buf.arg_no_typed.data_of_args;
        base_context.parse_and_format_fn_of_args =
            buf.arg_no_typed.parse_and_format_fn_of_args;
        base_context.format_string = format_str;
        context_data.parse_context.remaining_format_string = format_str;
        context_data.format_context.output_it = iter;
        return run_format();
      }
      else {
        basic_format_args_t<version_v, sizeof...(Ts)> buf{
            basic_format_args_ncx_tag{}, std::forward<Ts>(args)...};
        base_context.number_of_args = buf.count_args;
        base_context.data_of_args = buf.data_of_args;
        base_context.parse_and_format_fn_of_args =
            buf.parse_and_format_fn_of_args;
        base_context.format_string = format_str;
        context_data.parse_context.remaining_format_string = format_str;
        context_data.format_context.output_it = iter;
        return run_format();
      }
    } else {
      base_context.format_string = format_str;
      context_data.parse_context.remaining_format_string = format_str;
      context_data.format_context.output_it = iter;
      return run_format();
    }
  }

  template <typename L_v, is_formatted_c<version_v>... Ts>
  MJZ_CX_FN success_t format_to_pv(out_it_t iter, L_v, Ts&&... args) noexcept;
  MJZ_CX_FN success_t run_format() noexcept {
    return context_data.parse_formating_string();
  }
  template <version_t, typename...>
  friend struct cx_parser_t;
};

template <version_t version_v, typename... Ts>
struct cx_parser_t : public formatting_object_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;
  using out_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;
  count_out_buf_t<version_v> out{};
  using formatting_object_t<version_v>::base_context;
  using formatting_object_t<version_v>::context_data;
  success_t successful{true};
  MJZ_NO_MV_NO_CPY(cx_parser_t);
  MJZ_CX_FN cx_parser_t(view_t fmt_str) noexcept
      : formatting_object_t<version_v>{} {
    if constexpr (sizeof...(Ts)) {
      basic_format_args_t<version_v, sizeof...(Ts)> buf{
          basic_format_args_parse_tag{}, alias_t<void (*)(Ts&&...)>{}};
      base_context.number_of_args = buf.count_args;
      base_context.parse_and_format_fn_of_args =
          buf.parse_and_format_fn_of_args;
      base_context.err_index = uintlen_t(-1);
      base_context.format_string = fmt_str;
      context_data.parse_context.remaining_format_string =
          base_context.format_string;
      context_data.format_context.output_it = out;
      successful = this->run_format();
    } else {
      base_context.err_index = uintlen_t(-1);
      base_context.format_string = fmt_str;
      context_data.parse_context.remaining_format_string =
          base_context.format_string;
      context_data.format_context.output_it = out;
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
    out_it_t iter, L_v value, Ts&&... args) noexcept {
  static_assert(
      requires() {
        { view_t(L_v()()) } noexcept;
      }, "see if you used \"...\"_fmt or not (you should) ");
  auto failure_f =
      []() noexcept -> std::optional<std::pair<uintlen_t, view_t>> {
    MJZ_MAYBE_UNUSED cx_parser_t<version_v, Ts...> checker{view_t(L_v()())};
    if (checker.successful) return nullopt;
    return std::pair<uintlen_t, view_t>{checker.base_context.err_index,
                                        checker.base_context.err_content};
  };
  using failure_ft = static_data_t<decltype(failure_f)>;
#if MJZ_VERBOSE_FORMAT_ERROR
  if constexpr (failure_ft{}()) {
    constexpr bool failed = !failure_ft{}();
    auto failure_fv = []() noexcept { return (*failure_ft{}()); };
    using failure_t = static_data_t<decltype(failure_fv)>;
    auto format_str_len_f = []() noexcept { return view_t(L_v()()).length(); };
    using format_str_len_t = static_data_t<decltype(format_str_len_f)>;
    auto second_part_fn = []() noexcept
        -> std::array<char, format_str_len_t {}() - failure_t{}().first + 1> {
      std::array<char, format_str_len_t{}() - failure_t{}().first + 1> ret{};
      memcpy(ret.data(), view_t(L_v()()).data() + failure_t{}().first,
             format_str_len_t{}() - failure_t{}().first);
      return ret;
    };
    auto error_part_fn =
        []() noexcept -> std::array<char, failure_t {}().second.length() + 1> {
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
    constexpr auto& first_part = static_data_t<decltype(first_part_fn)>{}();
    constexpr auto& second_part = static_data_t<decltype(second_part_fn)>{}();
    constexpr auto& error_part = static_data_t<decltype(error_part_fn)>{}();
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
MJZ_UNUSED  failure_ft check{};
#endif
  return vformat_to(iter, value(), std::forward<Ts>(args)...);
}

namespace fmt_litteral_ns {

template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L>
struct operator_fmt_t: static_string_view_t<version_v> {
  MJZ_CX_FN operator_fmt_t() noexcept : static_string_view_t<version_v>{bstr_ns::litteral_ns::operator_view<L, version_v>()}{}
};

template <version_t version_v, bstr_ns::litteral_ns::str_litteral_t L>
MJZ_CX_FN auto operator_fmt() noexcept {
  return operator_fmt_t<version_v,L>{};
}
template <bstr_ns::litteral_ns::str_litteral_t L>
MJZ_CX_FN auto operator""_fmt() noexcept {
  return operator_fmt<version_t{}, L>();
}
}  // namespace fmt_litteral_ns

}  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_base_LIB_HPP_FILE_