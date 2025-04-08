

#include "string_api.hpp"
#ifndef MJZ_BYTE_STRING_string_mut_ABI_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_mut_ABI_LIB_HPP_FILE_

namespace mjz ::bstr_ns {

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &&
basic_str_t<version_v, has_alloc_v_>::move_v() & noexcept {
  return uniqe_inout_v();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    cheap_str_info &info) noexcept
    : basic_str_t() {
  reset_to_error_on_fail(
      reset(info),
      "[Error]basic_str_t(cheap_str_info):couldn't initilize string!");
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(nullptr_t) noexcept
    : m{nullptr} {
  m.fast_uninitilized_constructor_to_empty_sso();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t() noexcept
    : m{nullptr} {
  m.construct_non_sso_from_invalid_fast_no_keep();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN basic_str_t<version_v, has_alloc_v_>::~basic_str_t() noexcept {
  asserts(asserts.assume_rn, m.deconstruct_to_invalid());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    cheap_str_info &&info) noexcept
    : basic_str_t(info) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &, owned_stack_buffer &&stack_buffer,
    cheap_str_info &&info, uintlen_t byte_offset, uintlen_t byte_count) noexcept
    : basic_str_t() {
  MJZ_RELEASE { stack_buffer = owned_stack_buffer{}; };
  bool success{true};
  MJZ_RELEASE {
    reset_to_error_on_fail(
        success,
        "[Error]basic_str_t(owned_stack_buffer):couldn't initilize string!");
  };
  if (max_size() < byte_count || max_size() < byte_offset ||
      max_size() < stack_buffer.buffer_size ||
      max_size() < info.reserve_capacity ||
      stack_buffer.buffer_size < byte_count + byte_offset) {
    success = false;
  }
  if (info.reserve_capacity <= stack_buffer.buffer_size) {
    info.reserve_capacity = 0;
  }
  success = success && reset(info);
  if (!success) return;
  if (stack_buffer.buffer_size < m.get_capacity()) {
    success &= resize_and_overwrite(
        byte_count, [&](const mut_ref_t &&range) noexcept -> success_t {
          memcpy(range.data(), stack_buffer.buffer + byte_offset,
                 range.length());
          return true;
        });
    return;
  }
  success &= m.construct_non_sso_from_invalid(
      stack_buffer.buffer + byte_offset, byte_count, stack_buffer.buffer,
      stack_buffer.buffer_size, false, true);
  return;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &ok, owned_stack_buffer &&stack_buffer,
    uintlen_t byte_offset, uintlen_t byte_count) noexcept
    : basic_str_t(ok, std::move(stack_buffer), cheap_str_info{}, byte_offset,
                  byte_count) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &, generic_string_view view) noexcept
    : basic_str_t() {
  reset_to_error_on_fail(
      init_view(view),
      "[Error]basic_str_t(const "
      "basic_string_view_t<version_v>&):couldn't init view!");
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &, generic_string_view view,
    cheap_str_info &&info) noexcept
    : basic_str_t() {
  bool success{true};
  MJZ_RELEASE {
    reset_to_error_on_fail(
        success,
        "[Error]basic_str_t(const basic_string_view_t<version_v>&,cheap_str_"
        "info&&):couldn't init view!");
  };
  if (max_size() < view.length() || max_size() < info.reserve_capacity) {
    success = false;
  }
  if (info.reserve_capacity < view->len) {
    info.reserve_capacity = 0;
  }

  success = success && reset(info);
  if (!success) return;
  view.unsafe_handle().encodings = uint8_t(info.encoding);
  success &= init_view(view);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    static_string_view view, cheap_str_info &&info) noexcept
    : basic_str_t(dont_mess_up, basic_string_view_t<version_v>(view),
                  std::move(info)) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    static_string_view view) noexcept
    : basic_str_t(dont_mess_up, basic_string_view_t<version_v>(view)) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &ok, dynamic_string_view view,
    cheap_str_info &&info) noexcept
    : basic_str_t(ok, basic_string_view_t<version_v>(view), std::move(info)) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const dont_mess_up_t &ok, dynamic_string_view view) noexcept
    : basic_str_t(ok, basic_string_view_t<version_v>(view)) {}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    basic_str_t<version_v, has_alloc_v_> &&obj) noexcept
    : basic_str_t{} {
  reset_to_error_on_fail(
      move_init(std::move(obj)),
      "[Error]basic_str_t(basic_str_t&&):couldn't move string!");
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const basic_str_t<version_v, has_alloc_v_> &obj) noexcept
    : basic_str_t{} {
  reset_to_error_on_fail(
      copy_assign(obj),
      "[Error]basic_str_t(const basic_str_t&):couldn't copy string!");
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const basic_str_t<version_v, has_alloc_v_> &&obj) noexcept
    : basic_str_t{} {
  reset_to_error_on_fail(
      share_init(obj),
      "[Error]basic_str_t(const basic_str_t&&):couldn't share string!");
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    const basic_str_t<version_v, has_alloc_v_> &obj,
    cheap_str_info &&info) noexcept
    : basic_str_t{} {
  if (info.reserve_capacity < obj.m.length) info.reserve_capacity = 0;
  reset_to_error_on_fail(reset(info) && copy_assign(obj),
                         "[Error]basic_str_t(const basic_str_t& obj, "
                         "cheap_str_info&& info):couldn't copy string!");
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(
    char c, cheap_str_info &&info) noexcept
    : basic_str_t() {
  reset_to_error_on_fail(reset(info) && push_back(c),
                         "[Error]basic_str_t(char):couldnt make str!");
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>::basic_str_t(char c) noexcept
    : basic_str_t() {
  reset_to_error_on_fail(push_back(c),
                         "[Error]basic_str_t(char):couldnt make str!");
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::operator=(char c) noexcept {
  char b[2]{c};
  reset_to_error_on_fail(
      assign_data_with_range(&b[0], &b[1]),
      "[Error]basic_str_t&operator=( char c):couldn't assign string!");
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::operator=(
    basic_str_t<version_v, has_alloc_v_> &&obj) noexcept {
  if (this == &obj) return *this;
  reset_to_error_on_fail(move_init(std::move(obj)),
                         "[Error]basic_str_t&operator=( basic_str_t&& "
                         "obj):couldn't move string!");
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::operator=(
    const basic_str_t<version_v, has_alloc_v_> &obj) noexcept {
  if (this == &obj) return *this;
  reset_to_error_on_fail(copy_assign(obj),
                         "[Error]basic_str_t&operator=(const basic_str_t& "
                         "obj):couldn't copy string!");
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN void basic_str_t<version_v, has_alloc_v_>::reset_to_error_fail(
    static_string_view view) noexcept {
  asserts(asserts.assume_rn, m.deconstruct_to_invalid());
  asserts(asserts.assume_rn,
          m.construct_non_sso_from_invalid(view->ptr, view->len, nullptr, 0,
                                           view->is_static, false));
  m.d_set_cntrl(my_details::encodings_bits, encodings_e::err_ascii);
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN void basic_str_t<version_v, has_alloc_v_>::reset_to_error_on_fail(
    success_t op, static_string_view view) noexcept {
  asserts(asserts.assume_rn,
          encodings_e(view->encodings) == encodings_e::ascii ||
              encodings_e(view->encodings) == encodings_e::err_ascii);
  if (op) MJZ_MOSTLY_LIKELY return;
  if constexpr (MJZ_IN_DEBUG_MODE) {
    reset_to_error_fail(view);
  } else {
    reset_to_error_fail();
  }
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::copy_assign_data_fast(
    const basic_str_t &str, uintlen_t offset, uintlen_t count) noexcept {
  auto set_fast_to_dest = [&]() noexcept {
    m.d_set_cntrl(
        my_details::encodings_bits,
        str.m.template d_get_cntrl<encodings_e>(my_details::encodings_bits));
    m.d_set_cntrl(my_details::as_not_threaded_bit, !str.get_threaded());
    auto *p = m.get_alloc_ptr();
    if (p) *p = str.get_alloc();
  };
  if (count <= m.mut_data.sso_cap) {
    asserts(asserts.assume_rn, total_reset(true));
    m.construct_sso_from_invalid(str.data() + offset, count, false);
    set_fast_to_dest();
    return true;
  }
  if (is_stacked() && count <= m.get_capacity()) {
    m.length = count;
    uintlen_t delta_count =
        replace_flags{}.buffer_offset(m.get_capacity(), count);
    m.begin = &m.buffer_location_ptr()[delta_count];
    memcpy(m.mut_begin(), str.data() + offset, count);
    set_fast_to_dest();
    return true;
  }
  return false;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::copy_assign_data(
    const basic_str_t &str, bool no_allocate, uintlen_t offset,
    uintlen_t count) noexcept {
  if (&str == this) return as_substring(offset, count);
  /* this is early check becacuse replace_data is probably not inline*/
  std::ignore = str.make_right_then_give_has_null(offset, count);
  if (copy_assign_data_fast(str, offset, count)) return true;
  if (no_allocate && m.mut_data.sso_cap < str.length() &&
      (m.get_capacity() < str.length() || !m.is_owner())) {
    return false;
  }
  if (!(replace_data_with_char_il(
            0, nops, count, nullopt, str.get_alloc(),
            replace_flags{
                .can_choose_back = true,
                .can_choose_front = true,
                .no_encoding_check = true,
                .no_allocation = no_allocate,
                .can_change_to_ts = !no_allocate,
                .to_is_threaded_v = !str.m.template d_get_cntrl<bool>(
                    my_details::as_not_threaded_bit),
                .force_ownership = true,
                .change_alloc_v{replace_flags::change_e::always_force_change},
                .change_threaded_v =
                    replace_flags::change_e::always_force_change}) &&
        [&]() noexcept {
          m.d_set_cntrl(my_details::encodings_bits,
                        str.m.template d_get_cntrl<encodings_e>(
                            my_details::encodings_bits));

          return true;
        }())) {
    return false;
  }
  if (m.length) {
    memcpy(m.mut_begin(), str.data() + offset, count);
  }
  return true;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::reset(cheap_str_info &info) noexcept {
  if (!replace_data_with_char_il(
          0, nops, info.reserve_capacity, std::nullopt,
          info.alloc_ptr ? *info.alloc_ptr : get_alloc(),
          replace_flags{
              .to_is_threaded_v = info.is_threaded,
              .change_alloc_v = replace_flags::change_e::always_force_change,
              .change_threaded_v =
                  replace_flags::change_e::always_force_change}))
    return false;
  m.length = 0;
  m.d_set_cntrl(my_details::encodings_bits, info.encoding);
  return true;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::total_reset(bool keep_flags) noexcept {
  if (!m.is_sso()) {
    if (!m.deallocate_non_sso()) return false;
  }
  m.construct_sso_from_invalid_fast(keep_flags);
  return true;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::share_init(
    const basic_str_t &str, bool no_allocate, uintlen_t offset,
    uintlen_t count) noexcept {
  if (&str == this) {
    return as_substring(offset, count);
  }

  if (!str.m.template d_get_cntrl<bool>(my_details::is_sharable) ||
      str.m.template d_get_cntrl<bool>(my_details::is_ownerized) ||
      m.template d_get_cntrl<bool>(my_details::is_ownerized)) {
    return copy_assign_data(str, no_allocate, offset, count);
  }

  if (str.m.non_sso_buffer_location_ptr()) {
    str_heap_manager hm = str.m.non_sso_my_heap_manager_no_own();
    if (!hm.can_add_shareholder()) {
      return copy_assign_data(str, no_allocate, offset, count);
    }
    if (!total_reset(true)) return false;
    if (!hm.add_shareholder()) return false;
  } else {
    if (!total_reset(true)) return false;
  }
  if (m.get_alloc_ptr()) {
    *m.get_alloc_ptr() = *str.m.get_alloc_ptr();
  }
  m.begin = str.m.begin;
  m.length = str.m.length;
  m.mut_data.non_sso = str.m.mut_data.non_sso;
  return as_substring(offset, count);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::move_init(basic_str_t &&str) noexcept {
  if (&str == this) return true;
  if (!m.deconstruct_to_invalid()) return false;
  if (m.get_alloc_ptr()) {
    *m.get_alloc_ptr() = *str.m.get_alloc_ptr();
  }
  bool is_sso = str.m.is_sso();
  m.length = str.m.length;
  if (is_sso) {
    m.begin = m.sso_buffer_location_ptr();
    m.mut_data.sso_raw_buffer_and_cntrl[0] = 0;
    memcpy(m.mut_data.sso_raw_buffer_and_cntrl,
           str.m.mut_data.sso_raw_buffer_and_cntrl,
           sizeof(m.mut_data.sso_raw_buffer_and_cntrl));
  } else {
    m.begin = str.m.begin;
    m.mut_data.non_sso = str.m.mut_data.non_sso;
  }
  str.m.construct_non_sso_from_invalid_fast_no_keep();
  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::copy_assign(
    const basic_str_t &obj, bool no_allocate, uintlen_t offset,
    uintlen_t count) noexcept {
  return copy_assign_data(obj, true, offset, count) ||
         share_init(obj, no_allocate, offset, count);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::init_view(
    const generic_string_view &view) noexcept {
  if (max_size() < view.length()) return false;
  if (view->len > m.get_capacity() || m.is_sso() || !m.is_owner()) {
    if (!total_reset(true)) {
      return false;
    }
    asserts(asserts.assume_rn,
            m.construct_non_sso_from_invalid(
                view.data(), view.length(), nullptr, 0, view->is_static, true));
    m.d_set_cntrl(my_details::encodings_bits, encodings_e(view->encodings));
    return true;
  }
  return replace_data_with_range(0, nops, view);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::m_t &
basic_str_t<version_v, has_alloc_v_>::unsafe_handle(
    const dont_mess_up_t &, const dont_mess_up_t &) noexcept {
  return m;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &&
basic_str_t<version_v, has_alloc_v_>::uniqe_inout_v() & noexcept {
  return static_cast<basic_str_t<version_v, has_alloc_v_> &&>(*this);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::format_back_insert_append_pv_fn_(
    unsafe_ns::i_know_what_im_doing_t, blazy_t view,
    replace_flags rp) noexcept {
  bool succsess{};
  MJZ_RELEASE {
    reset_to_error_on_fail(
        succsess, "[Error](?):back_insert_iterator cannot work anymore");
  };
  rp.better_back();

  if (view.is_invalid()) {
    return false;
  }
  if (view.is_resurve()) {
    succsess =
        reserve(view.len + length(), length() + view.len, get_alloc(), rp);
    return succsess;
  }
  if (view.get_encoding() != get_encoding()) return false;
  uintlen_t at_offset{m.length};
  if (max_size() < view.len ||
      !append_data_with_char(view.len, nullopt, get_alloc(), rp))
    return false;

  uintlen_t offset_of_substr{};
  char *append_begin = at_offset + m.mut_begin();
  MJZ_RELEASE {
    if (succsess) asserts(asserts.condition_rn, offset_of_substr == view.len);
  };
  succsess = view.get_value(
      [&](base_string_view_t<version_v> read_slice) noexcept -> success_t {
        uintlen_t new_offset_of_substr{offset_of_substr + read_slice.len};
        asserts(asserts.condition_rn, new_offset_of_substr <= view.len);
        memcpy(&append_begin[offset_of_substr], read_slice.ptr, read_slice.len);
        offset_of_substr = new_offset_of_substr;
        return true;
      });
  return succsess;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<uintlen_t> erase_if(
    basic_str_t<version_v, has_alloc_v_> &obj,
    callable_c<bool(const char &) noexcept> auto &&pred) noexcept {
  uintlen_t ret{};
  if (!obj.resize_and_overwrite(
          obj.length(),
          [&](const typename basic_str_t<version_v, has_alloc_v_>::mut_ref_t
                  &range) noexcept -> success_t {
            return MJZ_NOEXCEPT {
              auto it = std::remove_if(range.begin(), range.end(), pred);
              ret = range.end() - it;
            };
          }) ||
      !obj.remove_suffix(ret))
    return std::nullopt;
  return ret;
}
template <version_t version_v, bool has_alloc_v_, class U>
MJZ_CX_FN std::optional<uintlen_t> erase(
    basic_str_t<version_v, has_alloc_v_> &obj, const U &value) noexcept {
  return erase_if(
      obj, [&value](const char ch) noexcept -> bool { return value == ch; });
}

template <version_t version_v, bool has_alloc_v_>
template <std::floating_point T>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::as_floating(
    T val, uintlen_t accuracacy, bool upper_case,
    floating_format_e floating_format, char point_ch) noexcept {
  uintlen_t new_len = 0;
  auto fn = [&](const mut_ref_t &range) noexcept -> success_t {
    auto ret = traits_type{}.template from_float_format_fill<T>(
        range.data(), range.length(), val, accuracacy, upper_case,
        floating_format, point_ch);
    if (!ret) return false;
    new_len = *ret;
    return true;
  };
  bool success = resize_and_overwrite(m.mut_data.sso_cap, fn, get_alloc(),
                                      replace_flags{.no_allocation = true}) ||
                 resize_and_overwrite(accuracacy + 2 * sizeof(T), fn);
  m.length = new_len;
  if (!success) return false;
  set_encoding(dont_mess_up, encodings_e::ascii);
  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::append_with_insert_iter(
    callable_c<success_t(back_insert_iterator_t iter) noexcept> auto
        &&fn) noexcept {
  return !is_error() && fn(back_insert_iterator_t(*this)) && !is_error();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN success_t
basic_str_t<version_v, has_alloc_v_>::make_substrview_helper_(
    const basic_str_t<version_v, has_alloc_v_> &obj, uintlen_t byte_offset,
    uintlen_t byte_count, bool propgate_alloc,
    bool unsafe_assume_static_) noexcept {
  bool conviniently_has_null =
      obj.make_right_then_give_has_null(byte_offset, byte_count);
  allocs_ns::alloc_base_ref_t<version_v> alloc(
      propgate_alloc ? obj.m.get_alloc() : obj.m.empty_alloc);

  m.d_set_cntrl(
      my_details::as_not_threaded_bit,
      obj.m.template d_get_cntrl<bool>(my_details::as_not_threaded_bit));
  m.d_set_cntrl(
      my_details::encodings_bits,
      obj.m.template d_get_cntrl<encodings_e>(my_details::encodings_bits));
  m.d_set_cntrl(my_details::has_null, conviniently_has_null);
  if (m.is_sso()) {
    return m.construct_sso_from_invalid(obj.m.begin + byte_offset, byte_count,
                                        true, std::move(alloc));
  }
  return m.construct_non_sso_from_invalid(
      obj.m.begin + byte_offset, byte_count, nullptr, 0,
      unsafe_assume_static_ || obj.m.is_s_view(), true, std::move(alloc));
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::as_substring(
    uintlen_t byte_offset, uintlen_t byte_count,
    bool try_to_add_null) noexcept {
  bool conviniently_has_null =
      make_right_then_give_has_null(byte_offset, byte_count);
  try_to_add_null |= m.is_sso();
  try_to_add_null &= !conviniently_has_null;
  MJZ_RELEASE {
    m.length = byte_count;
    if (!(try_to_add_null && m.add_null(true))) {
      m.d_set_cntrl(my_details::has_null, conviniently_has_null);
    }
  };
  if (!m.is_sso()) {
    m.begin += byte_offset;
    return true;
  }
  memomve_overlap(m.sso_buffer_location_ptr(),
                  m.sso_buffer_location_ptr() + byte_offset, byte_count);
  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>
basic_str_t<version_v, has_alloc_v_>::operator()(uintlen_t begin_i,
                                                 uintlen_t end_i) noexcept {
  return make_substring(begin_i, end_i - std::min(end_i, begin_i));
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::to_substring(
    uintlen_t byte_offset, uintlen_t byte_count,
    bool try_to_add_null) noexcept {
  asserts(asserts.assume_rn,
          as_substring(byte_offset, byte_count, try_to_add_null));
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN void basic_str_t<version_v, has_alloc_v_>::set_encoding(
    const dont_mess_up_t &, encodings_e encoding) noexcept {
  m.d_set_cntrl(my_details::encodings_bits, encoding);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::remove_suffix(
    uintlen_t byte_count) noexcept {
  uintlen_t u_{0};
  std::ignore = make_right_then_give_has_null(byte_count, u_);
  return as_substring(0, m.length - byte_count, false);
} /*
   * removes min(byte_count,length) from begin
   */

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::remove_prefix(
    uintlen_t byte_count) noexcept {
  return as_substring(byte_count, nops, false);
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t
basic_str_t<version_v, has_alloc_v_>::capacity(bool must_owner) const noexcept {
  return !must_owner || is_owner() ? m.get_capacity() : 0;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::reserve(
    uintlen_t mincap, uintlen_t prefer_cap, const alloc_ref &alloc,
    replace_flags flags) noexcept {
  if (mincap <= m.get_capacity() &&
      (flags.force_ownership ? m.is_owner() : true)) {
    return true;
  }
  if (prefer_cap < mincap) {
    prefer_cap = mincap;
  }
  auto delta =
      uintlen_t(std::max<intlen_t>(intlen_t(prefer_cap - length()), 0));
  return replace_data_with_char_il(nops, 0, delta, std::nullopt, alloc,
                                   flags) &&
         remove_suffix(delta);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::consider_stack(
    const dont_mess_up_t &, owned_stack_buffer &&stack_buffer) noexcept {
  MJZ_RELEASE { stack_buffer = owned_stack_buffer{}; };
  if (stack_buffer.buffer_size < length()) return true;
  memcpy(stack_buffer.buffer, data(), size());
  auto len_ = size();
  asserts(
      asserts.assume_rn,
      total_reset(true) && m.construct_non_sso_from_invalid(
                               stack_buffer.buffer, len_, stack_buffer.buffer,
                               stack_buffer.buffer_size, false, true));
  m.add_null(true);
  return true;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::shrink_to_fit(
    bool force_ownership) noexcept {
  if (!force_ownership || m.is_owner()) {
    return true;
  }
  return replace_data_with_char_il(0, 0, 0, std::nullopt, m.get_alloc(),
                                   replace_flags{
                                       .prefer_new_cap = m.length,
                                       .no_allocation = false,
                                       .dont_add_null = false,
                                       .allocate_exact = true,
                                       .force_another_buffer = true,

                                   });
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::clear(bool force_ownership) noexcept {
  if (!force_ownership || m.is_owner()) {
    m.length = 0;
    return true;
  }
  return replace_data_with_char_il(0, nops, 0, std::nullopt, m.get_alloc(),
                                   replace_flags{.no_allocation = true});
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::add_null() noexcept {
  if (has_null() || m.add_null(true)) return true;
  if (!replace_data_with_char_il(nops, 0, 1, '\0', m.get_alloc(),
                                 replace_flags{})) {
    return false;
  }
  m.length--;
  m.d_set_cntrl(my_details::has_null, true);
  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const char *basic_str_t<version_v, has_alloc_v_>::as_c_str()
    & noexcept {
  return add_null() ? data() : nullptr;
}
/*
 *calculates the hash
 */
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN success_t
basic_str_t<version_v, has_alloc_v_>::replace_data_with_char_impl_(
    uintlen_t &offset, uintlen_t &byte_count, uintlen_t &length_of_val,
    std::optional<char> &val, const alloc_ref &val_alloc,
    replace_flags &rep_flags) noexcept {
  uintlen_t null_overhead{uintlen_t(!rep_flags.dont_add_null)};
  std::ignore = make_right_then_give_has_null(offset, byte_count);
  if (max_size() < length_of_val) return false;
  const intlen_t delta{intlen_t(length_of_val) - intlen_t(byte_count)};
  // overflow is corret behaviour.
  const uintlen_t new_len = m.length + uintlen_t(delta);
  if (max_size() < new_len) return false;
  const uintlen_t index_rest{offset + byte_count};
  const uintlen_t length_rest{m.length - index_rest};
  const uintlen_t rindex_repl{offset + length_of_val};
  bool choose_other_alloc{};
  bool choose_other_threaded{};
  if (m.get_alloc_ptr() && val_alloc != m.get_alloc() &&
      rep_flags.would_change_alloc(m.has_c_alloc(), false)) {
    choose_other_alloc = true;
  }
  if (rep_flags.to_is_threaded_v !=
          !m.template d_get_cntrl<bool>(my_details::as_not_threaded_bit) &&
      rep_flags.would_change_threaded(m.has_c_alloc(), false)) {
    choose_other_threaded = true;
  }
  bool choose_alloc_route = rep_flags.force_another_buffer;
  bool can_choose_alloc_route =
      (new_len + null_overhead <= m.mut_data.sso_cap) ||
      !rep_flags.no_allocation;
  if (!m.is_heap()) {
    if (choose_other_alloc) {
      auto *p = m.get_alloc_ptr();
      if (p) *p = val_alloc;
    }
    if (choose_other_threaded) {
      m.d_set_cntrl(my_details::as_not_threaded_bit,
                    !rep_flags.to_is_threaded_v);
    }
  } else {
    choose_alloc_route |= choose_other_threaded || choose_other_alloc;
  }
  choose_alloc_route |=
      choose_alloc_route || m.get_capacity() < null_overhead + new_len;
  if (!can_choose_alloc_route && choose_alloc_route) return false;
  if (!rep_flags.force_ownership && !choose_alloc_route && length_of_val == 0 &&
      byte_count == 0)
    return true;
  /* note:
    that is_owner funtion acsesses the heap, if the string is not being used,
        this is a cash miss, so by only running it if needed,
        we reduce its use(thats why the same condition is checked)*/
  choose_alloc_route |= choose_alloc_route || !m.is_owner();
  if (!can_choose_alloc_route && choose_alloc_route) return false;

  if (choose_alloc_route) {
    str_heap_manager hm = str_heap_manager{
        choose_other_alloc ? val_alloc : get_alloc(),
        choose_other_threaded
            ? rep_flags.to_is_threaded_v
            : !m.template d_get_cntrl<bool>(my_details::as_not_threaded_bit),
        rep_flags.new_always_ownerize(
            m.template d_get_cntrl<bool>(my_details::is_ownerized))};
    bool choose_failed{};
    MJZ_RELEASE {
      if (choose_failed) return;
      if (choose_other_alloc) {
        auto *p = m.get_alloc_ptr();
        if (p) *p = val_alloc;
      }
      if (choose_other_threaded) {
        m.d_set_cntrl(my_details::as_not_threaded_bit,
                      !rep_flags.to_is_threaded_v);
      }
    };
    alias_t<char[m_t::mut_data_t::sso_cap]> temp_buf{};
    char *new_buf{temp_buf};
    char *new_begin{temp_buf};
    uintlen_t new_cap{sizeof(temp_buf)};
    if (new_cap < new_len) {
      if (rep_flags.no_allocation ||
          !hm.malloc(rep_flags.new_cap_calc(new_len, hm.get_is_threaded()),
                     !rep_flags.allocate_exact) ||
          !hm) {
        choose_failed = true;
        return false;
      }
      new_buf = hm.get_heap_begin();
      new_cap = hm.get_heap_cap();
      new_begin = new_buf + rep_flags.buffer_offset(new_cap, new_len);
    }
    memcpy(new_begin, m.begin, offset);
    memcpy(new_begin + rindex_repl, m.begin + index_rest, length_rest);
    if (val) {
      memset(new_begin + offset, length_of_val, *val);
    }
    if (!total_reset(true)) {
      choose_failed = true;
      return false;
    }
    if (!hm) {
      return m.construct_sso_from_invalid(new_buf, new_len, true);
    }
    asserts(asserts.assume_rn,
            m.construct_non_sso_from_invalid(new_begin, new_len, new_buf,
                                             new_cap, true, true));
    std::ignore = hm.steal_heap_begin(false);
    asserts(asserts.assume_rn, m.add_null(false) || rep_flags.dont_add_null);
    return true;
  }
  uintlen_t front_delta = uintlen_t(m.begin - m.buffer_location_ptr());
  uintlen_t back_delta = m.get_capacity() - m.length - front_delta;
  bool null_has_room = !null_overhead || back_delta;
  if (back_delta) back_delta -= null_overhead;
  bool choose_front = null_has_room && rep_flags.can_choose_front &&
                      !m.is_sso() && delta <= intlen_t(front_delta);
  bool choose_back = null_has_room && rep_flags.can_choose_back &&
                     delta <= intlen_t(back_delta);
  char *mut_begin = m.mut_begin();
  MJZ_RELEASE {
    m.begin = mut_begin;
    m.length = new_len;
    if (!rep_flags.dont_add_null) {
      asserts(asserts.assume_rn, m.add_null(false));
    } else if (!choose_front) {
      m.add_null(false);
    } else {
      m.d_set_cntrl(my_details::has_null, false);
    }
    if (val) {
      memset(mut_begin + offset, length_of_val, *val);
    }
  };
  if (choose_front && choose_back) {
    if (rep_flags.chose_more_cap_side) {
      choose_front = back_delta < front_delta;
    } else {
      choose_front = offset + index_rest < m.length;
    }
  }
  if (choose_front) {
    mut_begin = memomve_overlap(mut_begin - delta, mut_begin, offset);
    return true;
  }
  if (choose_back) {
    memomve_overlap(mut_begin + rindex_repl, m.begin + index_rest, length_rest);
    return true;
  }
  uintlen_t delta_count = rep_flags.buffer_offset(m.get_capacity(), new_len);
  if (m.is_sso()) delta_count = 0;
  mut_begin = &m.buffer_location_ptr()[delta_count];
  bool shift_begin_first{mut_begin <= m.begin};
  if (shift_begin_first) {
    memomve_overlap(mut_begin, m.begin, offset);
  }
  memomve_overlap(mut_begin + rindex_repl, m.begin + index_rest, length_rest);
  if (shift_begin_first) {
    return true;
  }
  memomve_overlap(mut_begin, m.begin, offset);
  return true;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::replace_data_with_char_il(
    uintlen_t offset, uintlen_t byte_count, uintlen_t length_of_val,
    std::optional<char> val, const alloc_ref &val_alloc,
    replace_flags rep_flags) noexcept {
  if (rep_flags.always_ownerize(true)) {
    rep_flags.force_ownership = true;
  }

  success_t ret = replace_data_with_char_impl_(
      offset, byte_count, length_of_val, val, val_alloc, rep_flags);
  if (!ret) return false;
  if (rep_flags.always_ownerize(false)) {
    m.d_set_cntrl(my_details::is_ownerized, false);
    return true;
  }
  if (rep_flags.always_ownerize(true)) {
    m.d_set_cntrl(my_details::is_ownerized, true);
    return true;
  }
  return true;
}

template <version_t version_v, bool has_alloc_v_>
template <class R_t>
  requires std::ranges::sized_range<R_t> && std::ranges::forward_range<R_t>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::replace_data_with_range(
    uintlen_t offset, uintlen_t byte_count, R_t &&range,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept

{
  std::ignore = make_right_then_give_has_null(offset, byte_count);
  auto range_len{uintlen_t(std::ranges::size(range))};
  if (max_size() < range_len) return false;
  if (!replace_data_with_char_il(offset, byte_count, range_len, std::nullopt,
                                 val_alloc, rep_flags))
    return false;
  char *range_ptr = m.mut_begin() + offset;
  auto &&begin_iter = std::ranges::begin(range);
  for (uintlen_t i{}; i < range_len; i++, begin_iter++) {
    auto ch = get_as_option<char>(*begin_iter);
    if (!ch) return false;
    range_ptr[i] = *ch;
  }
  return true;
}

template <version_t version_v, bool has_alloc_v_>
template <std::ranges::forward_range R_t>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::replace_data_with_range(
    uintlen_t offset, uintlen_t byte_count, R_t &&range,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  auto &&begin_iter = std::ranges::begin(range);
  auto &&end_iter = std::ranges::end(range);
  std::ignore = make_right_then_give_has_null(offset, byte_count);
  auto back_holder_temp = make_substrview(dont_mess_up, 0, 0, true, false);
  uintlen_t back_of_range_len = length() - offset - byte_count;
  if (!back_holder_temp.replace_data_with_char_il(
          0, 0, back_of_range_len, std::nullopt, val_alloc,
          [rep_flags_v = rep_flags]() mutable noexcept -> auto {
            rep_flags_v.allocate_exact = true;
            return rep_flags_v;
          }()))
    return false;
  memcpy(back_holder_temp.m.mut_begin(),
         m.begin + (m.length - back_of_range_len), back_of_range_len);
  if (!remove_suffix(length() - offset) ||
      !replace_data_with_char_il(0, 0, 0, std::nullopt, val_alloc, rep_flags))
    return false;
  auto front_holder_temp{std::move(*this)};
  for (; begin_iter != end_iter; ++begin_iter) {
    auto ch = get_as_option<char>(*begin_iter);
    if (!ch) return false;
    if (!front_holder_temp.replace_data_with_char_il(nops, 0, 1, *ch, val_alloc,
                                                     rep_flags))
      return false;
  }
  if (!front_holder_temp.replace_data_with_char_il(
          nops, 0, back_of_range_len, std::nullopt, val_alloc, rep_flags))
    return false;
  memcpy(front_holder_temp.m.mut_begin() +
             (front_holder_temp.length() - back_of_range_len),
         back_holder_temp.data(), back_of_range_len);
  *this = std::move(front_holder_temp);
  return true;
}

template <version_t version_v, bool has_alloc_v_>
template <std::ranges::forward_range R_t>
MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::insert_data_with_range(
    uintlen_t offset, R_t &&r, const alloc_ref &val_alloc,
    replace_flags rep_flags) noexcept {
  return replace_data_with_range(offset, 0, std::forward<R_t>(r), val_alloc,
                                 rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
template <std::ranges::forward_range R_t>

MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::append_data_with_range(
    R_t &&r, const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  return replace_data_with_range(nops, 0, std::forward<R_t>(r), val_alloc,
                                 rep_flags);
}
template <version_t version_v, bool has_alloc_v_>
template <std::ranges::forward_range R_t>

MJZ_CX_FN success_t
basic_str_t<version_v, has_alloc_v_>::assign_data_with_range(
    R_t &&r, const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  return replace_data_with_range(0, nops, std::forward<R_t>(r), val_alloc,
                                 rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::replace_data(
    uintlen_t offset, uintlen_t byte_count,
    const basic_str_t<version_v, has_alloc_v_> &&other,
    replace_flags rep_flags) noexcept {
  if (!rep_flags.no_encoding_check && other.get_encoding() != get_encoding())
    return false;

  std::ignore = make_right_then_give_has_null(offset, byte_count);
  if (!replace_data_with_char_il(offset, byte_count, other.m.length,
                                 std::nullopt, other.get_alloc(), rep_flags))
    return false;

  if (m.length) {
    memmove(m.mut_begin() + offset, other.m.begin, other.m.length);
  }

  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::replace_data(
    uintlen_t offset, uintlen_t byte_count,
    const basic_str_t<version_v, has_alloc_v_> &other,
    replace_flags rep_flags) noexcept {
  if (!rep_flags.no_encoding_check && other.get_encoding() != get_encoding())
    return false;

  std::ignore = make_right_then_give_has_null(offset, byte_count);

  if (this != &other &&
      !memory_has_overlap(m.cbuffer_location_ptr(), m.get_capacity(),
                          other.m.begin, other.m.length)) {
    return replace_data(offset, byte_count, other.uniqe_in_v(), rep_flags);
  }
  basic_str_t<version_v, has_alloc_v_> temp{*this};
  return temp.replace_data(offset, byte_count, other.uniqe_in_v(), rep_flags) &&
         move_init(std::move(temp));
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::erase_data(
    uintlen_t offset, uintlen_t byte_count, const alloc_ref &val_alloc,
    replace_flags rep_flags) noexcept {
  return replace_data_with_char_il(offset, byte_count, 0, std::nullopt,
                                   val_alloc, rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::insert_data(
    uintlen_t offset, const basic_str_t<version_v, has_alloc_v_> &other,
    replace_flags rep_flags) noexcept {
  return replace_data(offset, 0, other, rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::insert_data_with_char(
    uintlen_t offset, uintlen_t length_of_val, std::optional<char> val,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  return replace_data_with_char_il(offset, 0, length_of_val, val, val_alloc,
                                   rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::append_data(
    const basic_str_t<version_v, has_alloc_v_> &other,
    replace_flags rep_flags) noexcept {
  return replace_data(nops, 0, other, rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::append_data_with_char(
    uintlen_t length_of_val, std::optional<char> val,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  return replace_data_with_char_il(nops, 0, length_of_val, val, val_alloc,
                                   rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::push_back(
    std::optional<char> c, replace_flags rep_flags) noexcept {
  rep_flags.better_back();
  return append_data_with_char(uintlen_t(c ? 1 : 0), c, get_alloc(), rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char>
basic_str_t<version_v, has_alloc_v_>::pop_back() noexcept {
  if (auto v = back()) {
    remove_suffix(1);
    return v;
  }
  return std::nullopt;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::push_front(
    std::optional<char> c, replace_flags rep_flags) noexcept {
  rep_flags.better_front();
  return insert_data_with_char(0, uintlen_t(c ? 1 : 0), c, get_alloc(),
                               rep_flags);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char>
basic_str_t<version_v, has_alloc_v_>::pop_front() noexcept {
  if (auto v = front()) {
    remove_prefix(1);
    return v;
  }
  return std::nullopt;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::assign_own(
    const basic_str_t<version_v, has_alloc_v_> &other,
    bool no_allocate) noexcept {
  return copy_assign_data(other, no_allocate);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::assign(
    const basic_str_t<version_v, has_alloc_v_> &other,
    bool no_allocate) noexcept {
  return copy_assign(other, no_allocate);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::assign_share(
    const basic_str_t<version_v, has_alloc_v_> &other,
    bool no_allocate) noexcept {
  return share_init(other, no_allocate);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::assign_move(
    basic_str_t<version_v, has_alloc_v_> &&other) noexcept {
  return move_init(std::move(other));
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::assign(
    char c, bool no_allocator) noexcept {
  if (no_allocator && !m.is_owner()) return false;
  return replace_data_with_char_il(0, nops, 1, c, m.get_alloc());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::get_substr(
    uintlen_t src_byte_offset, uintlen_t src_byte_count,
    basic_str_t<version_v, has_alloc_v_> &&dest,
    uintlen_t dest_byte_replace_offset, uintlen_t dest_byte_replace_count,
    replace_flags dest_replace_flags) const & noexcept {
  return dest.replace_data(
      dest_byte_replace_offset, dest_byte_replace_count,
      make_substrview(dont_mess_up, src_byte_offset, src_byte_count),
      dest_replace_flags);
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::get_substr(
    uintlen_t src_byte_offset, uintlen_t src_byte_count,
    basic_str_t<version_v, has_alloc_v_> &&dest,
    uintlen_t dest_byte_replace_offset, uintlen_t dest_byte_replace_count,
    replace_flags dest_replace_flags) && noexcept {
  make_right_then_give_has_null(src_byte_offset, src_byte_count);
  dest.make_right_then_give_has_null(dest_byte_replace_offset,
                                     dest_byte_replace_count);
  asserts(asserts.assume_rn, as_substring(src_byte_offset, src_byte_count));
  auto normal_way = [&]() noexcept {
    return dest.replace_data(dest_byte_replace_offset, dest_byte_replace_count,
                             *this, dest_replace_flags);
  };
  if (!dest_replace_flags.could_change_one(dest.m.has_c_alloc())) {
    return normal_way();
  }
  replace_flags old_flags = dest_replace_flags;
  dest_replace_flags.force_another_buffer = false;
  dest_replace_flags.no_allocation = true;
  if (normal_way()) return true;
  uintlen_t new_dest_len{dest.m.length + src_byte_count -
                         dest_byte_replace_count};
  if (capacity() < new_dest_len || !is_owner()) {
    dest_replace_flags = old_flags;
    return normal_way();
  }
  dest_replace_flags.change_alloc_v =
      replace_flags::change_e::dont_change_at_all;
  dest_replace_flags.can_choose_back = false;
  dest_replace_flags.can_choose_front = false;
  success_t success_before = replace_data(
      0, 0, dest.make_substrview(dont_mess_up, 0, dest_byte_replace_offset),
      dest_replace_flags);
  success_t success_after = replace_data(
      nops, 0,
      dest.make_substrview(dont_mess_up,
                           dest_byte_replace_offset + dest_byte_replace_count,
                           nops),
      dest_replace_flags);
  if (success_before && success_after) {
    dest = std::move(*this);
    return true;
  }
  dest_replace_flags = old_flags;
  return normal_way();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::work_with_data(
    callable_c<success_t(const basic_str_t<version_v, has_alloc_v_>
                             &) noexcept> auto &&reader_fn) {
  return reader_fn(*this);
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::work_with_mut_data(
    callable_c<success_t(const mut_ref_t &&temp_mut_ref) noexcept> auto
        &&read_writer_fn,
    replace_flags flags) noexcept {
  flags.force_ownership = true;
  if (!reserve(m.length + uintlen_t(!flags.dont_add_null), 0, m.get_alloc(),
               flags)) {
    return false;
  }
  // so that the callback wont share it.
  basic_str_t<version_v, has_alloc_v_> temp{std::move(*this)};
  MJZ_RELEASE { *this = std::move(temp); };

  mut_ref_t ref{temp.m.length, temp.m.mut_begin(), get_encoding()};
  return read_writer_fn((const mut_ref_t &&)ref);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::resize_and_overwrite(
    uintlen_t new_len,
    callable_c<success_t(const mut_ref_t &&range) noexcept> auto
        &&read_writer_fn,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  return resize(new_len, std::nullopt, true, val_alloc, rep_flags) &&
         work_with_mut_data(read_writer_fn);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN void basic_str_t<version_v, has_alloc_v_>::swap(
    basic_str_t<version_v, has_alloc_v_> &o) noexcept {
  basic_str_t<version_v, has_alloc_v_> temp{o};
  o = *this;
  *this = std::move(temp);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::append_data_temp(
    basic_str_t<version_v, has_alloc_v_> &&str) noexcept {
  return std::move(str).get_substr(0, nops, uniqe_inout_v(), nops, 0);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::operator+=(
    basic_str_t<version_v, has_alloc_v_> &&obj) noexcept {
  reset_to_error_on_fail(append_data_temp(std::move(obj)),
                         "[Error]basic_str_t&operator+=( basic_str_t&& "
                         "obj):couldn't append string!");
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_> &
basic_str_t<version_v, has_alloc_v_>::operator+=(
    const basic_str_t<version_v, has_alloc_v_> &obj) noexcept {
  reset_to_error_on_fail(append_data(obj),
                         "[Error]basic_str_t&operator+=(const basic_str_t& "
                         "obj):couldn't append string!");
  return *this;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>
basic_str_t<version_v, has_alloc_v_>::operator_add(
    basic_str_t<version_v, has_alloc_v_> &rhs,
    basic_str_t<version_v, has_alloc_v_> &lhs) noexcept {
  rhs.reset_to_error_on_fail(
      rhs.append_data_temp(std::move(lhs)),
      "[Error]basic_str_t<version_v, "
      "has_alloc_v_>operator_add(basic_str_t<version_v, has_alloc_v_>rhs, "
      "basic_str_t<version_v, has_alloc_v_>lhs):couldn't concatinate string!");
  return rhs;
}
template <class T>
concept basic_str_t_has_indentity_c_ =
    requires() { typename std::remove_cvref_t<T>::basic_str_t_indentity_t_; };

template <class T>
using basic_str_t_indentity_t_helper_ =
    typename std::remove_cvref_t<T>::basic_str_t_indentity_t_;

template <basic_str_t_has_indentity_c_ RHS_t,
          basic_str_t_has_indentity_c_ LHS_t>
  requires std::same_as<basic_str_t_indentity_t_helper_<RHS_t>,
                        basic_str_t_indentity_t_helper_<LHS_t>>
MJZ_CX_FN auto operator+(RHS_t &&rhs_, LHS_t &&lhs_) noexcept {
  using self_t = basic_str_t_indentity_t_helper_<RHS_t>;
  self_t rhs{std::forward<RHS_t>(rhs_)}, lhs{std::forward<LHS_t>(lhs_)};
  return self_t::operator_add(rhs, lhs);
}

template <version_t version_v, bool has_alloc_v_>
template <std::integral T>
  requires(!std::same_as<T, bool>)

MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::as_integral(
    T val, const uint8_t raidex, bool upper_case) noexcept {
  uintlen_t new_len = 0;
  auto fn = [&](const mut_ref_t &range) noexcept -> success_t {
    auto ret = traits_type{}.template from_integral_fill<T>(
        range.data(), range.length(), val, upper_case, raidex);
    if (!ret) return false;
    new_len = *ret;
    return true;
  };
  bool success = resize_and_overwrite(m.mut_data.sso_cap, fn, get_alloc(),
                                      replace_flags{.no_allocation = true}) ||
                 resize_and_overwrite(1 + sizeof(T) * 8, fn);
  m.length = new_len;
  if (!success) return false;
  set_encoding(dont_mess_up, encodings_e::ascii);
  return true;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN success_t basic_str_t<version_v, has_alloc_v_>::resize(
    uintlen_t new_len, std::optional<char> val, bool force_ownership,
    const alloc_ref &val_alloc, replace_flags rep_flags) noexcept {
  if (max_size() < new_len) return false;
  if (m.length < new_len) {
    return append_data_with_char(new_len - m.length, val, val_alloc, rep_flags);
  }
  if (force_ownership) {
    return erase_data(new_len, nops, val_alloc, rep_flags);
  }
  m.length = new_len;
  return true;
}
#if MJZ_WITH_iostream

template <version_t version_v, bool has_alloc_v_>
MJZ_NCX_FN std::ostream &operator<<(
    std::ostream &cout_v, const basic_str_t<version_v, has_alloc_v_> &obj) {
  return cout_v << obj.make_subview(unsafe_ns::unsafe_v, 0,
                                    basic_str_t<version_v, has_alloc_v_>::nops);
} /*
   *CREDIT TO MSVC's IMPLEMENTATION , IDK WHAT IT MEANS std::istream IS NOT
   *NICE!
   */

template <version_t version_v, bool has_alloc_v_>
MJZ_NCX_FN std::istream &operator>>(std::istream &cin_v,
                                    basic_str_t<version_v, has_alloc_v_> &obj) {
  using traits = std::char_traits<char>;
  typename std::istream::iostate state = std::istream::goodbit;
  auto &cin_base{
      static_cast<std::basic_ios<char, std::char_traits<char>> &>(cin_v)};
  const typename std::istream::sentry is_ok(cin_v);
  if (!is_ok) {
    return cin_v;
  }  // state okay, extract characters

  bool no_throw{false};
  bool has_changed = false;
  bool success{true};
  success &= obj.clear();
  auto perivous_exp_state = cin_base.exceptions();
  {
    MJZ_RELEASE {
      cin_base.exceptions(std::istream::goodbit);
      if (no_throw) {
        cin_base.width(0);
        if (!has_changed || !success) {
          state |= std::istream::failbit;
        }
        cin_base.setstate(state);
      } else {
        cin_base.setstate(std::istream::badbit);
      }
    };
    uintlen_t len{};
    if (0 < cin_base.width() &&
        static_cast<uintlen_t>(cin_base.width()) < obj.max_size()) {
      len = static_cast<uintlen_t>(cin_base.width());
    } else {
      len = obj.max_size();
    }

    typename traits::int_type meta = cin_base.rdbuf()->sgetc();
    for (; 0 < len; --len, meta = cin_base.rdbuf()->snextc()) {
      if (traits::eq_int_type(traits::eof(), meta)) {  // end of file, quit
        state |= std::istream::eofbit;
        break;
      }
      if (typename basic_str_t<version_v, has_alloc_v_>::traits_type{}.is_space(
              traits::to_char_type(meta))) {
        break;  // whitespace, quit
      }

      // reserve ,got a character, add it to string
      success &=
          obj.reserve(obj.length() + 1, uintlen_t(cin_base.width()),
                      obj.get_alloc(),
                      replace_flags_t<version_v>{.exponential_resize = true}) &&
          obj.push_back(traits::to_char_type(meta));
      if (!success) break;
      has_changed = true;
    }
    no_throw = true;
    ;
  };  //~MJZ_RELEASE;
  cin_base.exceptions(perivous_exp_state);
  return cin_v;
}

/*
 *CREDIT TO MSVC's IMPLEMENTATION , IDK WHAT IT MEANS std::istream IS NOT
 *NICE!
 */
template <version_t version_v, bool has_alloc_v_>
MJZ_NCX_FN std::istream &getline(std::istream &cin_v,
                                 basic_str_t<version_v, has_alloc_v_> &obj,
                                 char delim = '\n') {
  typename std::istream::iostate state = std::istream::goodbit;
  const typename std::istream::sentry is_ok(cin_v, true);
  auto &cin_base{
      static_cast<std::basic_ios<char, std::char_traits<char>> &>(cin_v)};
  if (!is_ok) {  // state okay, extract characters
    return cin_v;
  }
  bool state_changed{};
  bool success{true};
  bool no_throw{false};
  auto perivous_exp_state = cin_base.exceptions();
  {
    MJZ_RELEASE {
      cin_base.exceptions(std::istream::goodbit);
      if (no_throw) {
        if (!state_changed || !success) {
          state |= std::istream::failbit;
        }
        cin_base.setstate(state);
      } else {
        cin_base.setstate(std::istream::badbit);
      }
    };
    using traits = std::char_traits<char>;
    success &= obj.clear();
    const typename traits::int_type metadelim = traits::to_int_type(delim);
    typename traits::int_type meta = cin_base.rdbuf()->sgetc();
    for (;; meta = cin_base.rdbuf()->snextc()) {
      if (traits::eq_int_type(traits::eof(), meta)) {  // end of file, quit
        state |= std::istream::eofbit;
        break;
      }
      if (traits::eq_int_type(
              meta,
              metadelim)) {  // got a delimiter, discard it and quit
        state_changed = true;
        cin_base.rdbuf()->sbumpc();
        break;
      }
      if (obj.max_size() <= obj.size()) {  // string too large, quit
        state |= std::istream::failbit;
        break;
      }
      // reserve ,got a character, add it to string
      success &=
          obj.reserve(obj.length() + 1, uintlen_t(cin_base.width()),
                      obj.get_alloc(),
                      replace_flags_t<version_v>{.exponential_resize = true}) &&
          obj.push_back(traits::to_char_type(meta));

      if (!success) break;
      state_changed = true;
    }
    no_throw = true;
  }  //~MJZ_RELEASE;
  cin_base.exceptions(perivous_exp_state);
  return cin_v;
}

#endif  // MJZ_WITH_iostream

};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_mut_ABI_LIB_HPP_FILE_