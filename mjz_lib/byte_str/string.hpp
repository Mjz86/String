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

#include "../restricted_arguments.hpp"
#include "string_abi.hpp"

#ifndef MJZ_BYTE_STRING_string_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_LIB_HPP_FILE_
namespace mjz::bstr_ns {

template <auto = 0>
struct useless_tag_t_ {};

template <class T, class self_str_t_>
concept str_c_ = requires() {
  { version_t{std::remove_cvref_t<T>::Version_v_} } noexcept;
  requires(std::remove_cvref_t<T>::Version_v_ ==
           std::remove_cvref_t<self_str_t_>::Version_v_);
  typename std::remove_cvref_t<T>::str_t_indentity_t_;
  requires std::same_as<std::remove_cvref_t<T>,
                        typename std::remove_cvref_t<T>::str_t_indentity_t_>;
};

template <class T, class self_str_t_>
concept str_forward_c_ =
    str_c_<T, self_str_t_> && forward_like_c<T, self_str_t_>;

template <version_t version_v_>
struct basic_str_props_t {
  uintlen_t sso_min_cap{};
  bool has_alloc{};
  bool has_null{};
  bool is_ownerized{};
  may_bool_t is_threaded{may_bool_t::idk};
  align_direction_e align{};
};
template <version_t version_v_, basic_str_props_t<version_v_> props_v_ =
                                    basic_str_props_t<version_v_>{}>
struct MJZ_trivially_relocatable basic_str_t : void_struct_t {
  template <version_t version_v_0_, basic_str_props_t<version_v_0_>>
  friend struct basic_str_t;
  MJZ_MCONSTANT(version_t)
  version_v = version_v_;
  MJZ_MCONSTANT(basic_str_props_t<version_v_>)
  props_v = props_v_;
  MJZ_MCONSTANT(version_t)
  Version_v_{version_v};

 private:
  using abi = str_abi_t_<version_v, props_v.has_alloc, props_v.has_null,
                         props_v.is_ownerized, props_v.is_threaded,
                         props_v.sso_min_cap, props_v.align>;
  using m_t = typename abi::data_t;
  template <class>
  friend class mjz_private_accessed_t;

 private:
  m_t m{};

 public:
  MJZ_CX_FN const m_t &unsafe_handle_pv_(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return m;
  }
  MJZ_CX_FN m_t &unsafe_handle_pv_(unsafe_ns::i_know_what_im_doing_t) noexcept {
    return m;
  }
  using unsafe_handle_pv_t_ = m_t;

 public:
  MJZ_MCONSTANT(uintlen_t) sso_cap = m_t::sso_cap;
  using self_t = basic_str_t;
  using traits_type = byte_traits_t<version_v>;
  using value_type = char;
  using mut_type = std::conditional_t<props_v.is_ownerized, char, const char>;
  using pointer = mut_type *;
  using const_pointer = const char *;
  using reference = mut_type &;
  using const_reference = const char &;
  using const_iterator = iterator_t<const self_t>;
  using iterator = iterator_t<self_t>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using size_type = uintlen_t;
  using difference_type = intlen_t;
  using back_insert_iterator_t = base_out_it_t<version_v>;
  using str_t_indentity_t_ = self_t;

  MJZ_MCONSTANT(uintlen_t)
  npos{traits_type::npos};
  MJZ_MCONSTANT(uintlen_t)
  nops{traits_type::npos};

 private:
  using static_string_view = static_string_view_t<version_v>;

  using dynamic_string_view = dynamic_string_view_t<version_v>;
  using generic_string_view = basic_string_view_t<version_v>;

  using str_heap_manager =
      str_heap_manager_t<version_v, props_v.is_threaded, props_v.is_ownerized,
                         props_v.has_alloc>;
  using str_t = self_t;
  // using    base_str_info = base_str_info_t<version_v>;
  using cheap_str_info = cheap_base_str_info_t<version_v>;
  using EM_t = encodings_e;
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  using owned_stack_buffer = owned_stack_buffer_t<version_v>;
  using hash_t = hash_bytes_t<version_v>;
  using dont_mess_up_t = unsafe_ns::i_know_what_im_doing_t;

 private:
  static constexpr const dont_mess_up_t &dont_mess_up = unsafe_ns::unsafe_v;
  using when_t = typename m_t::when_t;

  template <when_t when_v, bool checked_, bool no_allocate_,
            bool the_room_is_infront>
  MJZ_CX_AL_FN success_t
  add_null_impl_(uintlen_t reserve_if_allocate_ = 0) noexcept {
    if constexpr (no_allocate_) {
      if (!m.template has_room_for<when_v>(reserve_if_allocate_,
                                           props_v.has_null)) {
        str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
        if (!hm.u_malloc(uintlen_t(props_v.has_null) + reserve_if_allocate_,
                         false))
          MJZ_IS_UNLIKELY {
            hm.unsafe_clear();
            return false;
          }
        m.template memcpy_to_non_sso<when_v>(m.get_begin(), m.get_length(),
                                             hm.get_heap_begin(),
                                             hm.get_heap_cap(), true);
        hm.unsafe_clear();
        return true;
      }
    } else if constexpr (checked_) {
      if (!m.template has_room_for<when_v>(reserve_if_allocate_,
                                           props_v.has_null))
        return false;
    }
    return m.template add_null<when_v, the_room_is_infront>();
  }

  template <when_t when_v>
  MJZ_CX_FN void reset_to_error_fail_(static_string_view view) noexcept {
    asserts(asserts.assume_rn, !!view.unsafe_handle().is_static);
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    if constexpr (!props_v.is_ownerized) {
      m.set_invalid_to_non_sso_begin(view.data(), view.size(), nullptr, 0, true,
                                     view.has_null());
    } else {
      view = "[Err]";
      m.set_invalid_to_sso(view.data(), view.size());
    }
    m.cntrl().encodings_bits = uint8_t(encodings_e::err_ascii);
  }
  template <when_t when_v>
  MJZ_CX_NL_FN void reset_to_error_fail_() noexcept {
    return reset_to_error_fail_<when_v>("[Err]");
  }
  template <when_t when_v>
  MJZ_CX_AL_FN void reset_to_error_on_fail_(success_t op,
                                            static_string_view view) noexcept {
    if (op) MJZ_IS_LIKELY {
        return;
      }
    if constexpr (!MJZ_IN_DEBUG_MODE) {
      reset_to_error_fail_<when_v>();
    } else {
      reset_to_error_fail_<when_v>(view);
    }
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t memcpy_to_me_nonsso_alloc(const char *ptr,
                                                   uintlen_t len) noexcept {
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.u_malloc(uintlen_t(props_v.has_null) + len)) MJZ_IS_UNLIKELY {
        m.invalid_to_empty();
        hm.unsafe_clear();
        return false;
      }
    m.template memcpy_to_non_sso<when_t::no_heap>(ptr, len, hm.get_heap_begin(),
                                                  hm.get_heap_cap(), true);
    hm.unsafe_clear();
    return true;
  }

  template <when_t when_v>
  MJZ_CX_AL_FN success_t memcpy_to_me_nonsso(const char *ptr, uintlen_t len,
                                             bool no_allocate) noexcept {
    if constexpr (!when_v.is_sso()) {
      if (!m.is_sso() &&
          m.template has_room_for<when_v>(len, props_v.has_null)) {
        m.template memcpy_to_non_sso<when_t::no_heap>(
            ptr, len, m.get_buffer_ptr(), m.get_capacity(), m.is_sharable());
        return true;
      }
    }
    if (no_allocate) return false;
    return memcpy_to_me_nonsso_alloc<when_v>(ptr, len);
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t memcpy_to_me_sso(const char *ptr,
                                          uintlen_t len) noexcept {
    asserts(asserts.assume_rn, len <= sso_cap);
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    m.set_invalid_to_sso(ptr, len);
    return true;
  }

  template <when_t when_v>
  MJZ_CX_AL_FN success_t memcpy_to_me_(const char *ptr, uintlen_t len,
                                       bool no_allocate) noexcept {
    if (len <= sso_cap) {
      return memcpy_to_me_sso<when_v>(ptr, len);
    }
    return memcpy_to_me_nonsso<when_v>(ptr, len, no_allocate);
  }

  template <when_t when_v>
  MJZ_CX_AL_FN success_t init_view_cpy_(const generic_string_view &view,
                                        bool no_allocate) noexcept {
    if (!memcpy_to_me_<when_v>(view.data(), view.size(), no_allocate)) {
      return false;
    }
    m.cntrl().encodings_bits = uint8_t(view.get_encoding());
    return true;
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t init_view_(const generic_string_view &view,
                                    bool no_allocate) noexcept {
    if constexpr (props_v.is_ownerized) {
      return init_view_cpy_<when_v>(view, no_allocate);
    } else {
      if (init_view_cpy_<when_v>(view, true)) {
        return true;
      }
      if constexpr (when_v) {
        m.destruct_to_invalid();
      }
      m.set_invalid_to_non_sso_begin(view.data(), view.size(), nullptr, 0,
                                     view.unsafe_handle().is_static,
                                     view.has_null());
      return true;
    }
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t memcopy_assign_(str_c_<self_t> auto const &obj,
                                         bool no_allocate, uintlen_t offset,
                                         uintlen_t count) noexcept {
    if (!memcpy_to_me_<when_v>(obj.m.get_begin() + offset, count,
                               no_allocate)) {
      return false;
    }
    m.cntrl().encodings_bits = obj.m.cntrl().encodings_bits;
    return true;
  }

  MJZ_CX_AL_FN bool make_right_then_give_has_null(
      uintlen_t &byte_offset, uintlen_t &byte_count) const noexcept {
    auto len = m.get_length();
    byte_count = std::min(byte_count, len);
    byte_offset = std::min(byte_offset, len);
    byte_count = std::min(byte_offset + byte_count, len) - byte_offset;
    bool ret = byte_offset + byte_count == len;
    ret &= m.has_null();
    return ret;
  }

 private:
  template <when_t when_v>
  MJZ_CX_AL_FN success_t as_substring_impl_(uintlen_t byte_offset,
                                            uintlen_t byte_count) noexcept {
    bool has_null = make_right_then_give_has_null(byte_offset, byte_count);
    if constexpr (!when_v.is_sso()) {
      if (!m.is_sso()) {
        m.non_sso().begin_ptr += byte_offset;
        m.non_sso().str_data.length = byte_count;
        m.non_sso().str_data.has_null = has_null;
        return true;
      }
    }
    char *buf = m.m_sso_buffer_();
    memomve_overlap(buf, buf + byte_offset, byte_count);
    // buf[byte_count] = '\0';
    m.set_sso_length(byte_count);
    return true;
  }

 public:
  template <when_t when_v>
  MJZ_CX_AL_FN success_t as_substring_(uintlen_t byte_offset,
                                       uintlen_t byte_count) noexcept {
    asserts(asserts.assume_rn,
            as_substring_impl_<when_v>(byte_offset, byte_count));
    if constexpr (!props_v.has_null || when_v.is_sso()) {
      return true;
    }
    if (m.is_sso()) return true;
    return add_null_impl_<when_v, true, false, false>();
  }
  template <when_t when_v, str_c_<self_t> T>
  MJZ_CX_AL_FN success_t move_init_impl_(alias_t<T &&> str) noexcept {
    constexpr bool is_same_type = partial_same_as<decltype(str), self_t>;
    auto destruct_fn = [this]() noexcept {
      if constexpr (when_v) {
        m.destruct_to_invalid();
      }
    std::ignore=  this;
    };
    if constexpr (is_same_type && when_v.is_sso()) {
      destruct_fn();
      m = std::exchange(str.m, {});
      return true;
    }
    auto cpy_mv_assign = [&str,this]() noexcept {
      MJZ_RELEASE {
        std::destroy_at(&str);
        std::construct_at(&str);
      };
      if constexpr (!props_v.is_ownerized) {
        if (str.m.is_ownerized() != m.is_ownerized()) {
          m.destruct_to_invalid();
          m.set_ownerized(str.m.is_ownerized());
          return copy_assign_<when_t::no_heap>(str);
        }
      }
      return copy_assign_<when_v>(str);
    };
    if (m.template has_room_for<when_v>(str.m.get_length(), props_v.has_null)) {
      return cpy_mv_assign();
    }
    if constexpr (is_same_type) {
      destruct_fn();
      m = std::exchange(str.m, {});
      return true;
    }
    constexpr basic_str_props_t<version_v> other_prop =
        std::remove_cvref_t<T>::props_v;
    if constexpr ((other_prop.is_ownerized || props_v.is_ownerized) ||
                  (props_v.is_threaded != may_bool_t::idk &&
                   props_v.is_threaded != other_prop.is_threaded)) {
      return cpy_mv_assign();
    }
    if constexpr (other_prop.has_alloc && !props_v.has_alloc) {
      if (str.m.get_alloc() != alloc_ref{}) {
        return cpy_mv_assign();
      }
    }
    if (!str.m.is_sharable()) {
      return cpy_mv_assign();
    }
    destruct_fn();
    m.cntrl().encodings_bits = str.m.cntrl().encodings_bits;
    m.set_threaded(str.m.is_threaded());
    m.set_ownerized(str.m.is_ownerized());
    if constexpr (props_v.has_alloc) {
      *m.get_alloc_ptr() = std::move(str.m.get_alloc());
    }
    m.set_invalid_to_non_sso_begin(str.m.get_begin(), str.length(),
                                   str.m.get_buffer_ptr(), str.m.get_capacity(),
                                   true, str.m.has_null());
    std::ignore = std::exchange(str.m, {});
    return true;
  }

  template <when_t when_v, str_c_<self_t> T>
  MJZ_CX_AL_FN success_t move_init_(alias_t<T &&> str) noexcept {
    if (void_struct_cast_t::up_cast(this) ==
        &void_struct_cast_t::up_cast(str)) {
      return true;
    }
    return move_init_impl_<when_v>(std::move(str));
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t share_init_(str_c_<self_t> auto const &obj,
                                     bool no_allocate = false,
                                     uintlen_t offset = 0,
                                     uintlen_t count = nops) noexcept {
    if (void_struct_cast_t::up_cast(this) ==
        &void_struct_cast_t::up_cast(obj)) {
      return as_substring_<when_v>(offset, count);
    }
    return share_init_impl_<when_v>(obj, no_allocate, offset, count);
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t share_init_impl_(str_c_<self_t> auto const &obj,
                                          bool no_allocate = false,
                                          uintlen_t offset = 0,
                                          uintlen_t count = nops) noexcept {
    set_prpos_to_<when_v>(obj);
    bool has_null_{obj.make_right_then_give_has_null(offset, count)};
    constexpr bool need_to_check_alloc_equality =
        std::remove_cvref_t<decltype(m)>::has_alloc_v() ||
        std::remove_cvref_t<decltype(obj.m)>::has_alloc_v();
    if constexpr (props_v.is_ownerized) {
      return memcopy_assign_<when_v>(obj, no_allocate, offset, count);
    }
    bool cant = !obj.m.is_sharable();
    cant |= props_v.has_null && !has_null_;
    cant |= m.is_ownerized();
    cant |= obj.m.is_ownerized();
    bool unmached_threaded = m.is_threaded() != m.is_threaded();
    unmached_threaded &= obj.m.has_mut();
    cant |= unmached_threaded;
    if constexpr (need_to_check_alloc_equality) {
      if (!cant) cant |= m.get_alloc() != obj.m.get_alloc();
    }
    if (cant) {
      return memcopy_assign_<when_v>(obj, no_allocate, offset, count);
    }
    if (!obj.m.has_mut()) {
      if constexpr (when_v) {
        m.destruct_to_invalid();
      }
      m.set_invalid_to_non_sso_begin(obj.m.get_begin() + offset, count, nullptr,
                                     0, true, has_null_);
      return true;
    }

    auto hm = obj.m.non_sso_my_heap_manager_no_own();
    if (!hm.can_add_shareholder()) {
      return memcopy_assign_<when_v>(obj, no_allocate, offset, count);
    }
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    asserts(asserts.assume_rn, hm.add_shareholder());
    m.set_invalid_to_non_sso_begin(obj.m.get_begin() + offset, count,
                                   hm.get_heap_begin(), hm.get_heap_cap(), true,
                                   has_null_);
    return true;
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t copy_assign_(str_c_<self_t> auto const &obj,
                                      bool no_allocate = false,
                                      uintlen_t offset = 0,
                                      uintlen_t count = nops) noexcept {
    if (void_struct_cast_t::up_cast(this) ==
        &void_struct_cast_t::up_cast(obj)) {
      return as_substring_<when_v>(offset, count);
    }
    return copy_assign_impl_<when_v>(obj, no_allocate, offset, count);
  }
  template <when_t when_v>
  MJZ_CX_AL_FN success_t copy_assign_impl_(str_c_<self_t> auto const &obj,
                                           bool no_allocate = false,
                                           uintlen_t offset = 0,
                                           uintlen_t count = nops) noexcept {
    set_prpos_to_<when_v>(obj);
    obj.make_right_then_give_has_null(offset, count);
    if constexpr (props_v.is_ownerized) {
      return memcopy_assign_<when_v>(obj, no_allocate, offset, count);
    }
    if (memcopy_assign_<when_v>(obj, true, offset, count)) {
      return true;
    }
    return share_init_<when_v>(obj, no_allocate, offset, count);
  }

  template <when_t when_v>
  MJZ_CX_AL_FN success_t reset_(cheap_str_info &info) noexcept {
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    if constexpr (props_v.has_alloc) {
      if (info.alloc_ptr) *m.get_alloc_ptr() = *info.alloc_ptr;
    }
    m.cntrl().encodings_bits = uint8_t(info.encoding);
    m.set_threaded(info.is_threaded);
    if (info.reserve_capacity <= sso_cap) {
      m.invalid_to_empty();
      return true;
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.u_malloc(info.reserve_capacity +
                         uintlen_t(props_v.has_null || info.add_null),
                     false))
      MJZ_IS_UNLIKELY {
        m.invalid_to_empty();
        hm.unsafe_clear();
        return false;
      }
    uintlen_t cap = hm.get_heap_cap();
    uintlen_t offset = m.s_buffer_offset(cap, 0);
    char *buf = hm.get_heap_begin();
    char *beg = buf + offset;
    beg[0] = '\0';
    m.set_invalid_to_non_sso_begin(beg, 0, buf, cap, true, true);
    hm.unsafe_clear();
    return true;
  }
  template <when_t when_v>
  MJZ_CX_AL_FN void assign_ch_(char c) noexcept {
    auto fn_ = [this, c]() noexcept {
      char *p = m.u_get_mut_begin();
      p[0] = c;
      as_substring(0, 1);
      return;
    };
    if (m.template has_room_for<when_v>(1, true)) return fn_();
    m.destruct_all();
    return fn_();
  }

  template <when_t when_v>
  MJZ_CX_AL_FN void set_prpos_to_(str_c_<self_t> auto const &src) noexcept {
    if constexpr (!when_v) {
      if constexpr (props_v.has_null) {
        *m.get_alloc_ptr() = src.m.get_alloc();
      }
      m.set_threaded(src.m.is_threaded());
      return;
    }
    bool destroy_v{};
    if constexpr (props_v.is_threaded == may_bool_t::idk) {
      destroy_v |= m.is_threaded() != src.m.is_threaded();
    }
    if constexpr (props_v.has_null) {
      if (!destroy_v) {
        destroy_v |= *m.get_alloc_ptr() != src.m.get_alloc();
      }
    }
    if (!destroy_v) return;
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    if constexpr (props_v.has_null) {
      *m.get_alloc_ptr() = src.m.get_alloc();
    }
    m.set_threaded(src.m.is_threaded());
  }

 private:
  template <bool no_null_>
  MJZ_CX_ND_FN basic_str_t(const dont_mess_up_t &,
                           owned_stack_buffer &stack_buffer,
                           uintlen_t byte_offset, uintlen_t byte_count,
                           std::bool_constant<no_null_>) noexcept
  /* not needed : basic_str_t()*/ {
    m.set_invalid_to_non_sso_begin(
        stack_buffer.buffer, stack_buffer.buffer_size, stack_buffer.buffer,
        stack_buffer.buffer_size, false, false);
    make_right_then_give_has_null(byte_offset, byte_count);
    m.non_sso().begin_ptr += byte_offset;
    m.non_sso().str_data.length = byte_count;
    if constexpr (!props_v.has_null) {
      return;
    }
    if constexpr (no_null_) {
      return;
    }
    reset_to_error_on_fail_<when_t::no_heap>(
        add_null_impl_<when_t::no_heap, true, false, false>(),
        "[Error] basic_str_t(const dont_mess_up_t &, owned_stack_buffer "
        "&,uintlen_t , uintlen_t,bool) : cannot add null , init fail ");
  }

 private:
  MJZ_CX_FN basic_str_t(str_forward_c_<self_t> auto &&src,
                        useless_tag_t_<>) noexcept
      : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        move_init_<when_t::as_sso>(std::move(src)),
        "[Error]basic_str_t(basic_str_t&&):couldn't move string!");
  }
  MJZ_CX_FN basic_str_t(str_forward_c_<self_t const &> auto &&src,
                        useless_tag_t_<>) noexcept
      : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        copy_assign_<when_t::as_sso>(src),
        "[Error]basic_str_t(const basic_str_t&):couldn't copy string!");
  }
  MJZ_CX_FN basic_str_t(str_forward_c_<self_t const> auto &&src,
                        useless_tag_t_<>) noexcept
      : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        share_init_<when_t::as_sso>(src),
        "[Error]basic_str_t(const basic_str_t&&):couldn't share string!");
  }
  MJZ_CX_FN basic_str_t &operator_assign_(str_forward_c_<self_t> auto &&src,
                                          useless_tag_t_<>) noexcept {
    if (void_struct_cast_t::up_cast(this) == &void_struct_cast_t::up_cast(src))
      return *this;
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          move_init_<when_t::no_heap>(std::move(src)),
          "[Error]basic_str_t&operator=( basic_str_t&& "
          "obj):couldn't move string!");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          move_init_<when_t::relax>(std::move(src)),
          "[Error]basic_str_t&operator=( basic_str_t&& "
          "obj):couldn't move string!");
    }
    return *this;
  }
  MJZ_CX_FN basic_str_t &operator_assign_(
      str_forward_c_<self_t const &> auto &&src, useless_tag_t_<>) noexcept {
    if (void_struct_cast_t::up_cast(this) == &void_struct_cast_t::up_cast(src))
      return *this;
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          copy_assign_<when_t::no_heap>(src),
          "[Error]basic_str_t&operator=(const basic_str_t& "
          "obj):couldn't copy string!");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          copy_assign_<when_t::relax>(src),
          "[Error]basic_str_t&operator=(const basic_str_t& "
          "obj):couldn't copy string!");
    }
    return *this;
  }
  MJZ_CX_FN basic_str_t &operator_assign_(
      str_forward_c_<const self_t> auto &&src, useless_tag_t_<>) noexcept {
    if (void_struct_cast_t::up_cast(this) == &void_struct_cast_t::up_cast(src))
      return *this;
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          share_init_<when_t::no_heap>(src),
          "[Error]basic_str_t&operator=(const basic_str_t&&):couldn't share "
          "string!");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          share_init_<when_t::relax>(src),
          "[Error]basic_str_t&operator=(const basic_str_t&&):couldn't share "
          "string!");
    }
    return *this;
  };
  MJZ_CX_ND_FN basic_str_t(str_forward_c_<self_t const &> auto &&obj,
                           cheap_str_info info, useless_tag_t_<>) noexcept
      : basic_str_t(&info) {
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          copy_assign_<when_t::no_heap>(obj),
          "[Error]basic_str_t(const basic_str_t& "
          "obj,cheap_str_info):couldn't copy string!");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          copy_assign_<when_t::relax>(obj),
          "[Error]basic_str_t(const basic_str_t& "
          "obj,cheap_str_info):couldn't copy string!");
    }
  }

 public:
  MJZ_CX_FN ~basic_str_t() noexcept { m.destruct_to_invalid(); }
  MJZ_CX_FN basic_str_t() noexcept { m.invalid_to_empty(); }
  MJZ_CX_FN basic_str_t(basic_str_t &&val) noexcept
      : basic_str_t(std::move(val), useless_tag_t_<>{}) {}

  MJZ_CX_ND_FN basic_str_t(self_t const &obj, cheap_str_info info) noexcept
      : basic_str_t(obj, info, useless_tag_t_<>{}) {}

  MJZ_CX_FN basic_str_t(const basic_str_t &&val) noexcept
      : basic_str_t((const basic_str_t &&)(val), useless_tag_t_<>{}) {}
  MJZ_CX_FN basic_str_t(const basic_str_t &val) noexcept
      : basic_str_t(val, useless_tag_t_<>{}) {}
  MJZ_CX_FN basic_str_t &operator=(basic_str_t &&val) noexcept {
    return operator_assign_(std::forward<basic_str_t>(val), useless_tag_t_<>{});
  }
  MJZ_CX_FN basic_str_t &operator=(const basic_str_t &&val) noexcept {
    return operator_assign_(std::forward<const basic_str_t>(val),
                            useless_tag_t_<>{});
  }
  MJZ_CX_FN basic_str_t &operator=(const basic_str_t &val) noexcept {
    return operator_assign_(std::forward<const basic_str_t &>(val),
                            useless_tag_t_<>{});
  }

  template <str_c_<self_t> T>
    requires(!partial_same_as<T, self_t>)
  MJZ_CX_ND_FN basic_str_t(T &&obj, cheap_str_info info) noexcept
      : basic_str_t(std::forward<T>(obj), info, useless_tag_t_<>{}) {}
  template <str_c_<self_t> T>
    requires(!partial_same_as<T, self_t>)
  MJZ_CX_ND_FN basic_str_t(T &&obj) noexcept
      : basic_str_t(std::forward<T>(obj), useless_tag_t_<>{}) {}

  template <class T>
    requires requires(basic_str_t &str) {
      str.operator_assign_(just_some_invalid_obj<T &&>(), useless_tag_t_<>{});
    }
  MJZ_CX_FN basic_str_t &operator=(T &&val) noexcept {
    return operator_assign_(std::forward<T>(val), useless_tag_t_<>{});
  }

 public:
  MJZ_CX_ND_FN basic_str_t(cheap_str_info *info) noexcept : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        reset_<when_t::as_sso>(*info), "[Error]basic_str_t(cheap_str_info)");
  }

 public:
  MJZ_CX_ND_FN explicit basic_str_t(nullptr_t) noexcept : basic_str_t() {}

  MJZ_CX_ND_FN basic_str_t(cheap_str_info info) noexcept : basic_str_t(&info) {}
  /*
   *
   */
  MJZ_CX_ND_FN basic_str_t(const dont_mess_up_t &,
                           owned_stack_buffer &&stack_buffer,
                           cheap_str_info &&info, uintlen_t byte_offset = 0,
                           uintlen_t byte_count = 0) noexcept
      : basic_str_t(dont_mess_up, stack_buffer, byte_offset, byte_count,
                    std::true_type{}) {
    m.cntrl().encodings_bits = uint8_t(info.encoding);
    m.set_threaded(info.is_threaded);
    m.cntrl().encodings_bits = uint8_t(info.encoding);
    if constexpr (props_v.has_alloc) {
      if (info.alloc_ptr) {
        auto *p = m.get_alloc_ptr();
        *p = *info.alloc_ptr;
      }
    }
    bool good{};
    if (info.add_null || props_v.has_null) {
      good = add_null_impl_<when_t::no_heap, true, false, false>(
          info.reserve_capacity);
    } else {
      good = reserve_<when_t::no_heap>(info.reserve_capacity);
    }
    reset_to_error_on_fail_<
        when_t::no_heap /* if heap was used then it wouldnt have failed*/>(
        good,
        "[Error]:basic_str_t(const dont_mess_up_t &, owned_stack_buffer && "
        ",cheap_str_info && , uintlen_t  , uintlen_t ): coulnt initilize");
    return;
  }

 public:
  MJZ_CX_ND_FN basic_str_t(const dont_mess_up_t &ok,
                           owned_stack_buffer &&stack_buffer,
                           uintlen_t byte_offset = 0,
                           uintlen_t byte_count = 0) noexcept
      : basic_str_t(ok, stack_buffer, byte_offset, byte_count,
                    std::false_type{}) {}
  /*
   * initilizes the string as a view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &, generic_string_view view) noexcept
      : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        init_view_<when_t::as_sso>(view, false),
        "[Error]basic_str_t(generic_string_view)");
  } /*
     * reserves space if appropreate then copies,
     * if not initilizes the string as a view
     */
  MJZ_CX_ND_FN basic_str_t(static_string_view view,
                           cheap_str_info info) noexcept
      : basic_str_t(dont_mess_up, generic_string_view(view), info) {}
  /*
   * reserves space if appropreate then copies, if not
   * initilizes the string as a   static  view (string litteral view)
   *
   */
  MJZ_CX_ND_FN basic_str_t(const dont_mess_up_t &, generic_string_view view,
                           cheap_str_info info) noexcept
      : basic_str_t([&info, &view]() noexcept {
          bool do_alloc_ = props_v.is_ownerized;
          do_alloc_ |= !!(info.reserve_capacity);
          info.reserve_capacity = branchless_teranary<uintlen_t>(
              !do_alloc_, 0, std::max(view.size(), info.reserve_capacity));
          return info;
        }()) {
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          init_view_<when_t::no_heap>(view, false),
          "[Error]basic_str_t(static_string_view,cheap_str_info)");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          init_view_<when_t::relax>(view, false),
          "[Error]basic_str_t(static_string_view,cheap_str_info)");
    }
  }
  /*
   * initilizes the string as a   static  view (string litteral view)
   */
  MJZ_CX_ND_FN basic_str_t(static_string_view view) noexcept : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        init_view_<when_t::as_sso>(view, false),
        "[Error]basic_str_t(static_string_view)");
  }
  /*
   * reserves space if appropreate then copies, if not
   *initilizes the string as a non sharable dynamic view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &, dynamic_string_view view,
              cheap_str_info info) noexcept
      : basic_str_t(dont_mess_up, generic_string_view(view), info) {}

  /*
   * initilizes the string as a non sharable dynamic view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &, dynamic_string_view view) noexcept
      : basic_str_t(dont_mess_up, generic_string_view(view)) {}
  /*
   * moves the content of obj in
   * then makes the obj empty
   */
  /*
   * reserves info.reserve_capacity if appropriate
   *safely copies the data without allocation of the obj if possible, if not
   *,shares it if possible, if not copies it with an allocation.
   */

  MJZ_DEPRECATED_R("the cheap_str_info is used but discarded afterwards")
  MJZ_CX_FN basic_str_t(basic_str_t &&obj,
                        cheap_str_info &&info) noexcept = delete;

  MJZ_DEPRECATED_R("the cheap_str_info is used but discarded afterwards")
  MJZ_CX_FN
  basic_str_t(const basic_str_t &&obj, cheap_str_info &&info) noexcept = delete;
  /*
   * reserves space if appropreate then copies, if not
   * initilizes the string as "c" in sso
   */
  MJZ_CX_ND_FN basic_str_t(char c, cheap_str_info &&info) noexcept
      : basic_str_t(&info) {
    assign_ch_<when_t::own_relax>(c);
  }
  /*
   * initilizes the string as "c" in sso
   */
  MJZ_CX_ND_FN basic_str_t(char c) noexcept : basic_str_t() {
    assign_ch_<when_t::as_sso>(c);
  }

 public:
  MJZ_CX_FN basic_str_t &operator=(char c) noexcept {
    assign_ch_<when_t::relax>(c);
    return *this;
  }

 public:
  MJZ_CX_ND_FN const alloc_ref &get_alloc() const noexcept {
    return m.get_alloc();
  }
  MJZ_CX_FN str_heap_manager make_empty_heap() noexcept {
    return str_heap_manager{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
  }

  MJZ_CX_FN str_heap_manager
  drop_heap(unsafe_ns::i_know_what_im_doing_t) noexcept {
    MJZ_RELEASE { m.invalid_to_empty(); };
    if (m.no_destroy()) {
      return make_empty_heap();
    }
    return m.non_sso_my_heap_manager_no_own(true);
  }
  MJZ_CX_FN success_t set_heap(unsafe_ns::i_know_what_im_doing_t,
                               str_heap_manager &&hm) noexcept {
    if constexpr (!props_v.has_alloc) {
      if (!!*hm.alloc_ptr()) {
        return false;
      }
    }
    if constexpr (props_v.is_threaded != may_bool_t::idk) {
      if (bool(props_v.is_threaded) != hm.get_is_threaded()) {
        return false;
      }
    }
    uintlen_t cap = hm.get_heap_cap();
    m.destruct_to_invalid();
    m.set_threaded(hm.get_is_threaded());
    if constexpr (props_v.has_alloc) {
      *m.get_alloc_ptr() = *hm.alloc_ptr();
    }
    if (!cap) return true;
    char *buf = hm.get_heap_begin();
    uintlen_t offset_of_beg = m.s_buffer_offset(cap, 0);
    char *beg = buf + offset_of_beg;
    m.set_invalid_to_non_sso_begin(beg, 0, buf, cap, true, false);
    hm.unsafe_clear();
    return true;
  }

  MJZ_CX_FN success_t
  set_alloc(const alloc_ref &a_, uintlen_t reserve_may = 0,
            may_bool_t threaded_ = may_bool_t::idk) noexcept {
    const bool good_alloc_{a_ == m.get_alloc()};
    if (may_bool_t::yes < threaded_) threaded_ = may_bool_t(get_threaded());
    const bool good_threaded_{bool(threaded_) == get_threaded()};

    if (good_threaded_ && good_alloc_) {
      return true;
    }
    if constexpr (!props_v.has_alloc) {
      if (!good_alloc_) {
        return false;
      }
    }
    if constexpr (props_v.is_threaded != may_bool_t::idk) {
      if (!good_threaded_) {
        return false;
      }
    }

    const alloc_ref &a{good_alloc_ ? m.get_alloc() : a_};

    if (m.no_destroy()) {
      if constexpr (props_v.has_alloc) {
        *m.get_alloc_ptr() = a;
      }
      m.set_threaded(bool(threaded_));
      return true;
    }
    uintlen_t new_len = length();
    str_heap_manager hm{a, bool(threaded_), m.is_ownerized()};
    if (!hm.u_malloc(uintlen_t(props_v.has_null) +
                     std::max(reserve_may, new_len)))
      MJZ_IS_UNLIKELY {
        hm.unsafe_clear();
        return false;
      }
    uintlen_t cap = hm.get_heap_cap();
    char *buf = hm.get_heap_begin();
    const char *old_ptr = m.get_begin();
    uintlen_t offset_of_beg = m.s_buffer_offset(cap, new_len);
    char *beg = buf + offset_of_beg;
    memcpy(beg, old_ptr, new_len);
    m.destruct_heap();
    m.set_invalid_to_non_sso_begin(beg, new_len, buf, cap, true, false);
    if constexpr (props_v.has_alloc) {
      *m.get_alloc_ptr() = a;
    }
    m.set_threaded(bool(threaded_));
    hm.unsafe_clear();
    if constexpr (!props_v.has_null) {
      return true;
    }
    asserts(asserts.assume_rn,
            add_null_impl_<when_t::own_relax, false, true, true>());
    return true;
  }

  /* similar to as_substring_*/
  MJZ_CX_ND_FN self_t make_substring(uintlen_t byte_offset,
                                     uintlen_t byte_count) const noexcept {
    self_t ret{};
    ret.reset_to_error_on_fail_<when_t::as_sso>(
        ret.copy_assign_<when_t::as_sso>(*this, false, byte_offset, byte_count),
        "[Error]basic_str_t::make_substring");
    return ret;
  }
  /*similar to make_substring but gives range from [begin_i,end_i) */
  MJZ_CX_ND_FN self_t operator()(uintlen_t begin_i = 0,
                                 uintlen_t end_i = nops) const noexcept {
    return make_substring(begin_i, end_i - begin_i);
  }
  MJZ_CX_FN success_t as_substring(uintlen_t byte_offset,
                                   uintlen_t byte_count) noexcept {
    if (m.no_destroy()) {
      return as_substring_<when_t::no_heap>(byte_offset, byte_count);
    } else {
      return as_substring_<when_t::relax>(byte_offset, byte_count);
    }
  }
  MJZ_CX_FN self_t &to_substring(uintlen_t byte_offset,
                                 uintlen_t byte_count) noexcept {
    if (m.no_destroy()) {
      reset_to_error_on_fail_<when_t::no_heap>(
          as_substring_<when_t::no_heap>(byte_offset, byte_count),
          "[Error]basic_str_t::to_substring");
    } else {
      reset_to_error_on_fail_<when_t::relax>(
          as_substring_<when_t::relax>(byte_offset, byte_count),
          "[Error]basic_str_t::to_substring");
    }
    return *this;
  }
  MJZ_CX_FN bool is_stable() const noexcept {
    bool good = m.is_sharable();
    good |= m.is_sso();
    return good;
  }
  MJZ_CX_FN bool is_stable_or_owner() const noexcept {
    bool good = is_stable();
    good |= m.has_mut();
    return good;
  }

 private:
  MJZ_CX_FN success_t make_stable_impl_() noexcept {
    uintlen_t len = length();
    m.set_invalid_to_non_sso_begin(data(), len, nullptr, 0, false, false);
    return reserve_<when_t::no_heap>(len, false);
  }

 public:
  /* a string would manage its own lifetime */
  MJZ_CX_FN success_t as_stable() noexcept {
    if (is_stable()) {
      return true;
    }
    return make_stable_impl_();
  }
  /* a string would manage its own lifetime,  stack buffer is also assumed as
   * managed
   */
  MJZ_CX_FN success_t as_stable_or_owner() noexcept {
    if (is_stable_or_owner()) {
      return true;
    }
    return make_stable_impl_();
  }

  void data() && = delete;
  void data() const && = delete;
  /* gives a pointer to the data */
  MJZ_CX_ND_FN const char *data() const & noexcept { return m.get_begin(); }
  template <int = 0>
    requires(!props_v.is_ownerized)
  MJZ_CX_ND_FN const char *data() & noexcept {
    return m.get_begin();
  }
  template <int = 0>
    requires(props_v.is_ownerized)
  MJZ_CX_ND_FN char *data() & noexcept {
    return m.u_get_mut_begin();
  }
  /*
   *is the data a "valid" C string?
   * (does it have a null terminator(='\0') at the end?
   */
  MJZ_CX_ND_FN bool has_null() const noexcept { return !!m.has_null(); }
  /* gives  length of the data */
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return m.get_length(); }
  /* gives  encoding of the data */
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept {
    return encodings_e(m.cntrl().encodings_bits);
  }
  MJZ_CX_FN bool is_error() const noexcept {
    return get_encoding() == encodings_e::err_ascii;
  }
  /* sets  encoding of the data */
  MJZ_CX_FN void set_encoding(const dont_mess_up_t &,
                              encodings_e encoding) noexcept {
    m.cntrl().encodings_bits = uint8_t(encoding);
    return;
  }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    static_assert(2 < sso_cap && sso_cap < nops - 2);
    return nops - 1;
  }
  MJZ_CX_ND_FN optional_ref_t<const char> at(const uintlen_t i) const noexcept {
    bool bad = (i < length());
    auto ptr = data();
    ptr = branchless_teranary<const char *>(!bad, ptr, nullptr);
    return ptr + branchless_teranary<uintlen_t>(!bad, i, 0);
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> at(const uintlen_t i) noexcept {
    bool bad = (i < length());
    auto ptr = data();
    ptr = branchless_teranary<mut_type *>(!bad, ptr, nullptr);
    return ptr + branchless_teranary<uintlen_t>(!bad, i, 0);
  }

  MJZ_CX_ND_FN optional_ref_t<const char> operator[](
      const uintlen_t i) const noexcept {
    return at(i);
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> operator[](const uintlen_t i) noexcept {
    return at(i);
  }

  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, length());
  }
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept { return begin(); }
  MJZ_CX_ND_FN const_iterator cend() const noexcept { return end(); }

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }
  MJZ_CX_ND_FN reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  MJZ_CX_ND_FN reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  MJZ_CX_ND_FN iterator begin() noexcept { return iterator(*this, 0); }
  MJZ_CX_ND_FN iterator end() noexcept { return iterator(*this, length()); }

  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept { return rend(); }

  MJZ_CX_ND_FN size_type size() const noexcept { return length(); }
  MJZ_CX_ND_FN bool empty() const noexcept { return length() == 0; }
  MJZ_CX_ND_FN optional_ref_t<const char> front() const noexcept {
    return at(0);
  }
  MJZ_CX_ND_FN optional_ref_t<const char> back() const noexcept {
    return at(length() - 1);
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> front() noexcept { return at(0); }
  MJZ_CX_ND_FN optional_ref_t<mut_type> back() noexcept {
    return at(length() - 1);
  }

  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) const && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) && = delete;

  /* similar to make_substrview, exept this even views the SSO */
  MJZ_CX_ND_FN generic_string_view
  make_subview(const dont_mess_up_t &, uintlen_t byte_offset = 0,
               uintlen_t byte_count = nops,
               bool unsafe_assume_static_ = false) const & noexcept {
    base_string_view_t<version_v> view = m.get_view().unsafe_handle();
    view.is_static |= unsafe_assume_static_;
    view.has_null_v = make_right_then_give_has_null(byte_offset, byte_count);
    view.len = byte_count;
    view.ptr += byte_offset;
    return view;
  }
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) const && = delete;
  /*
   * makes a view.
   * the view lifetime must be a subset of the string lifetime.
   * if unsafe_assume_static_ is true, the created view will not get a memcpy if
   * it was shared/copied by value. and the return type is treated as if it was
   * from a staticstring litteral. if propgate_alloc is false , the return will
   * not have any custom allocators(this is useless if the string has only
   * global allocators).
   */
  MJZ_CX_ND_FN self_t
  make_substrview(const dont_mess_up_t &, uintlen_t byte_offset,
                  uintlen_t byte_count, bool propgate_alloc = true,
                  bool unsafe_assume_static_ = false) const & noexcept {
    auto view = make_subview(dont_mess_up, byte_offset, byte_count,
                             unsafe_assume_static_);
    self_t ret{};
    if (propgate_alloc) {
      ret.set_prpos_to_<when_t::as_sso>(*this);
    }
    ret.reset_to_error_on_fail_<when_t::as_sso>(
        ret.init_view_<when_t::as_sso>(view, false),
        "[Error]basic_str_t::make_substrview");
    return ret;
  }
  /*
   * removes min(byte_count,length) from end
   */
  MJZ_CX_FN success_t remove_suffix(uintlen_t byte_count) noexcept {
    auto len = m.get_length() - byte_count;
    if (m.no_destroy()) {
      return as_substring_<when_t::no_heap>(0, len);
    }
    return as_substring_<when_t::relax>(0, len);
  } /*
     * removes min(byte_count,length) from begin
     */
  MJZ_CX_FN success_t remove_prefix(uintlen_t byte_count) noexcept {
    if (m.no_destroy()) {
      return as_substring_<when_t::no_heap>(byte_count, nops);
    }
    return as_substring_<when_t::relax>(byte_count, nops);
  }
  /*
   *copies the substring in buf.
   * buf must at least be of  min(byte_count,length)+ uintlen_t(add_null) in
   *size. if add_null then we add a null terminatior.
   */
  MJZ_CX_FN std::optional<uintlen_t> copy_bytes(
      uintlen_t byte_offset, uintlen_t byte_count, char *buf,
      bool add_null = false) const noexcept {
    generic_string_view view =
        make_subview(dont_mess_up, byte_count, byte_offset);
    memcpy(buf, view.data(), view.length());
    if (add_null) buf[view.length()] = '\0';
    return view.length();
  }
  /*
   * returns the number of bytes the string can modify in its buffer.
   */
  MJZ_CX_ND_FN uintlen_t capacity(bool must_owner = true) const noexcept {
    if (must_owner || m.is_owner()) return m.get_capacity();
    return 0;
  }
  /*
   * if cap <= capacity(force_ownership) then returns.
   * else if allocate_exact
   *allocates minimum cap bytes of capacity.
   * else
   *allocates a "convinient round" number bigger than cap bytes of capacity.
   */
 private:
  template <when_t when_v>
  MJZ_CX_AL_FN success_t reserve_(uintlen_t mincap,
                                  bool round_up = false) noexcept {
    if (m.template has_room_for<when_v>(mincap, props_v.has_null)) {
      return true;
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.u_malloc(uintlen_t(props_v.has_null) + mincap, round_up))
      MJZ_IS_UNLIKELY {
        hm.unsafe_clear();
        return false;
      }
    m.template memcpy_to_non_sso<when_v>(m.get_begin(), m.get_length(),
                                         hm.get_heap_begin(), hm.get_heap_cap(),
                                         true);
    hm.unsafe_clear();
    return true;
  }

 public:
  MJZ_CX_FN success_t reserve(uintlen_t mincap,
                              bool round_up = false) noexcept {
    return m.no_destroy() ? reserve_<when_t::no_heap>(mincap, round_up)
                          : reserve_<when_t::relax>(mincap, round_up);
  }

  MJZ_CX_FN success_t consider_stack(
      const dont_mess_up_t &, owned_stack_buffer &&stack_buffer) noexcept {
    if (stack_buffer.buffer_size < length() + uintlen_t(props_v.has_null))
      return true;
    return u_consider_stack(stack_buffer);
  }

  MJZ_CX_FN success_t u_consider_stack_pv_(
      const dont_mess_up_t &, owned_stack_buffer &stack_buffer) noexcept {
    return u_consider_stack(stack_buffer);
  }

 private:
  MJZ_CX_FN success_t
  u_consider_stack(owned_stack_buffer &stack_buffer) noexcept {
    MJZ_RELEASE { stack_buffer = owned_stack_buffer{}; };
    auto len_ = size();
    char *buf =
        stack_buffer.buffer + m.s_buffer_offset(stack_buffer.buffer_size, len_);
    memcpy(buf, data(), len_);
    m.destruct_to_invalid();
    m.set_invalid_to_non_sso_begin(buf, len_, stack_buffer.buffer,
                                   stack_buffer.buffer_size, false, false);
    if constexpr (!props_v.has_null) {
      return true;
    }
    asserts(asserts.assume_rn,
            add_null_impl_<when_t::no_heap, false, true, true>());
    return true;
  }

 public:
  MJZ_CX_FN success_t may_reconsider_stack(const dont_mess_up_t &,
                                           owned_stack_buffer &where) noexcept {
    bool cannot_fit =
        where.buffer_size < length() + uintlen_t(props_v.has_null);
    bool is_this_already = m.get_buffer_ptr() == where.buffer;
    asserts(asserts.assume_rn, !is_this_already || is_stacked());
    if (bool(int(cannot_fit) | int(is_this_already))) {
      return true;
    }
    if (m.template has_room_for<when_t::relax>(where.buffer_size, false)) {
      return true;
    }
    return u_consider_stack(where);
  }
  MJZ_CX_FN success_t shrink_to_fit(bool force_ownership = false) noexcept {
    if (!force_ownership && !m.is_owner()) return true;
    if (!m.is_heap()) return true;  // we werent on heap to begin with
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.u_malloc(uintlen_t(props_v.has_null) + m.get_length(), false))
      MJZ_IS_UNLIKELY {
        hm.unsafe_clear();
        return false;
      }
    m.template memcpy_to_non_sso<when_t::relax>(m.get_begin(), m.get_length(),
                                                hm.get_heap_begin(),
                                                hm.get_heap_cap(), true);
    hm.unsafe_clear();
    return true;
  }

  MJZ_CX_FN success_t clear(bool force_ownership = false) noexcept {
    if (!as_substring(0, 0)) return false;
    if (force_ownership) {
      return as_ownerized();
    }
    return true;
  }

  MJZ_CX_FN success_t add_null() noexcept {
    return has_null() || add_null_impl_<when_t::relax, true, false, false>();
  }

  MJZ_CX_FN const char *as_c_str() & noexcept {
    bool good = add_null();
    return branchless_teranary<const char *>(!good, nullptr, data());
  }
  /*
   *calculates the hash
   */
  MJZ_CX_ND_FN hash_t hash() const noexcept { return m.get_view().hash(); }

  MJZ_CX_ND_FN concatabe_hash_t<version_v> concatable_hash() const noexcept {
    return m.get_view().concatable_hash();
  }

  /*
   *checks to see if modification of the data is allowed
   */
  MJZ_CX_ND_FN bool is_owner() const noexcept { return m.is_owner(); }
  /*
   *gets the falg bits
   */
  MJZ_CX_ND_FN auto get_states() const noexcept { return m.cntrl(); }
  MJZ_CX_FN bool get_threaded() const noexcept { return m.is_threaded(); }
  MJZ_CX_FN bool is_stacked() const noexcept {
    bool ret{true};
    ret |= m.has_mut();
    ret &= !m.is_heap();
    return ret;
  }

 private:
  template <when_t when_v>
  MJZ_CX_FN success_t replace_data_with_none_impl_(
      uintlen_t offset, uintlen_t byte_count, uintlen_t fill_len,
      bool choose_front = true, bool choose_back = true,
      align_direction_e align = props_v.align) noexcept {
    if (max_size() < fill_len) {
      return false;
    }
    make_right_then_give_has_null(offset, byte_count);
    uintlen_t new_len{fill_len + m.get_length() - byte_count};
    if (max_size() < new_len) {
      return false;
    }
    bool looped_once{};
    do {
      bool has_room{};

      if (!looped_once) {
        has_room |= m.template has_room_for<when_v>(new_len, props_v.has_null);
      }
      has_room |= looped_once;

      if (has_room) {
        char *beg = m.u_get_mut_begin();
        char *buf = m.get_buffer_ptr();
        uintlen_t cap = m.get_capacity();
        uintlen_t len = m.get_length();
        uintlen_t front_del = uintlen_t(beg - buf);
        uintlen_t back_del = cap - front_del - len;
        intlen_t del = intlen_t(new_len) - intlen_t(len);

        choose_back &= std::cmp_less_equal(del, back_del);
        choose_front &= std::cmp_less_equal(del, front_del);
        choose_front &= !m.is_sso();
        MJZ_RELEASE {
          if (!m.is_sso()) {
            m.set_invalid_to_non_sso_begin(beg, new_len, buf, cap,
                                           !!m.is_sharable(), false);
          } else {
            m.set_sso_length(new_len);
          }
          asserts(asserts.assume_rn,
                  !props_v.has_null ||
                      add_null_impl_<when_t::own_relax, false, true, true>());
        };
        bool choose_both = choose_back;
        choose_both &= choose_front;
        bool temp_choose_front = offset + byte_count < len - offset;
        choose_front =
            branchless_teranary(!choose_both, choose_front, temp_choose_front);
        if (choose_front) {
          beg = memomve_overlap(beg - del, beg, offset);
          return true;
        }
        if (choose_back) {
          memomve_overlap(beg + offset + fill_len, beg + offset + byte_count,
                          len - (offset + byte_count));
          return true;
        }
        uintlen_t delta_count =
            m.s_buffer_offset(m.get_capacity(), new_len, align);
        delta_count =
            branchless_teranary<uintlen_t>(!m.is_sso(), delta_count, 0);
        char *old_beg{beg};
        beg = buf + delta_count;
        bool shift_begin_first{beg <= old_beg};
        if (shift_begin_first) {
          memomve_overlap(beg, old_beg, offset);
        }
        memomve_overlap(beg + offset + fill_len, old_beg + offset + byte_count,
                        len - (offset + byte_count));
        if (shift_begin_first) {
          return true;
        }
        memomve_overlap(beg, old_beg, offset);
        return true;
      }

      MJZ_RELEASE { looped_once = true; };
      if (new_len <= sso_cap) {
        m_t temp{};
        asserts(asserts.assume_rn, !m.is_sso());
        temp.cntrl() = m.cntrl();
        temp.non_sso() = std::exchange(m.non_sso(), {});
        if constexpr (when_v) {
          if constexpr (props_v.has_alloc) {
            *temp.get_alloc_ptr() = std::move(*m.get_alloc_ptr());
          }
        }
        m.invalid_to_empty();
        char *buf = m.get_buffer_ptr();
        const char *old_ptr = temp.get_begin();
        uintlen_t old_len = temp.get_length();
        memcpy(buf, old_ptr, offset);
        memcpy(buf + offset + fill_len, old_ptr + offset + byte_count,
               old_len - (offset + byte_count));
        byte_count = fill_len;
        m.set_sso_length(new_len);
        if constexpr (when_v) {
          temp.destruct_to_invalid();
          if constexpr (props_v.has_alloc) {
            *m.get_alloc_ptr() = std::move(*temp.get_alloc_ptr());
          }
        }
        continue;
      }
      str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
      if (!hm.u_malloc(uintlen_t(props_v.has_null) + new_len)) MJZ_IS_UNLIKELY {
          hm.unsafe_clear();
          return false;
        }
      uintlen_t cap = hm.get_heap_cap();
      char *buf = hm.get_heap_begin();
      const char *old_ptr = m.get_begin();
      uintlen_t old_len = m.get_length();
      uintlen_t offset_of_beg = m.s_buffer_offset(cap, new_len, align);
      char *beg = buf + offset_of_beg;
      memcpy(beg, old_ptr, offset);
      memcpy(beg + offset + fill_len, old_ptr + offset + byte_count,
             old_len - (offset + byte_count));
      byte_count = fill_len;
      if constexpr (when_v) {
        m.destruct_to_invalid();
      }
      m.set_invalid_to_non_sso_begin(beg, new_len, buf, cap, true, false);
      hm.unsafe_clear();
    } while (true);
    return false;
  }
  MJZ_CX_FN success_t replace_data_with_none(
      uintlen_t offset, uintlen_t byte_count, uintlen_t fill_len,
      bool choose_front = true, bool choose_back = true,
      align_direction_e align_direction = align_direction_e{}) noexcept {
    return m.no_destroy() ? replace_data_with_none_impl_<when_t::no_heap>(
                                offset, byte_count, fill_len, choose_front,
                                choose_back, align_direction)
                          : replace_data_with_none_impl_<when_t::relax>(
                                offset, byte_count, fill_len, choose_front,
                                choose_back, align_direction);
  }

 public:
  /*
   * replacas the data in range [offset,offset+byte_count) with other.
   *failes if the string object doesnt satisfy the criteria of rep_flags
   * or
   * if allocation fails
   */

  MJZ_CX_ND_FN success_t replace_data_with_char(
      uintlen_t offset, uintlen_t byte_count, uintlen_t length_of_val,
      std::optional<char> val) noexcept {
    make_right_then_give_has_null(offset, byte_count);
    if (!replace_data_with_none(offset, byte_count, length_of_val)) {
      return false;
    }
    if (!val) return true;
    memset(&m.u_get_mut_begin()[offset], length_of_val, *val);
    return true;
  }
  MJZ_CX_ND_FN success_t as_ownerized() noexcept {
    if (m.template has_room_for<when_t::relax>(m.get_length(),
                                               props_v.has_null))
      return true;
    return reserve(length());
  }

  MJZ_CX_ND_FN success_t as_always_ownerized(bool flag_state_) noexcept {
    if constexpr (!props_v.is_ownerized) {
      if (flag_state_ == false) {
        m.set_ownerized(false);
        return true;
      }
      if (!as_ownerized()) {
        return false;
      }
      m.set_ownerized(true);
      return true;
    }
    return flag_state_ == true;
  }
  template <int = 0>
    requires(props_v.is_ownerized)
  MJZ_CX_ND_FN success_t as_always_ownerized(bool flag_state_) const noexcept {
    return flag_state_ == true;
  }
  template <class R_t>
    requires std::ranges::sized_range<R_t> && std::ranges::input_range<R_t>
  MJZ_CX_ND_FN success_t replace_data_with_range(uintlen_t offset,
                                                 uintlen_t byte_count,
                                                 R_t &&range) noexcept {
    return ::mjz::noexcept_er_helper_t<>{} *
               [&]() /*noexcept(...) todo!*/ -> success_t {
      std::ignore = make_right_then_give_has_null(offset, byte_count);
      auto range_len{uintlen_t(std::ranges::size(range))};
      if (max_size() < range_len) return false;
      if (!replace_data_with_none(offset, byte_count, range_len)) return false;
      char *range_ptr = m.u_get_mut_begin() + offset;
      auto &&begin_iter = [&]() {
        if constexpr (requires() {
                        {
                          std::ranges::data(range)
                        } -> std::convertible_to<const char *>;
                      }) {
          return static_cast<const char *>(std::ranges::data(range));
        } else {
          return std::ranges::begin(range);
        }
      }();
      for (uintlen_t i{}; i < range_len; i++, ++begin_iter) {
        auto &&itr = *begin_iter;
        auto ch = optional_ref_t<const char>(itr);
        if (!ch) return false;
        range_ptr[i] = *ch;
      }
      return true;
    };
  }
  template <std::ranges::input_range R_t>
  MJZ_CX_ND_FN success_t replace_data_with_range(uintlen_t offset,
                                                 uintlen_t byte_count,
                                                 R_t &&range) noexcept {
    return ::mjz::noexcept_er_helper_t<>{} *
               [&]() /*noexcept(...) todo!*/ -> success_t {
      auto &&begin_iter = std::ranges::begin(range);
      auto &&end_iter = std::ranges::end(range);
      std::ignore = make_right_then_give_has_null(offset, byte_count);
      auto back_holder_temp = make_substrview(dont_mess_up, 0, 0, true, false);
      uintlen_t back_of_range_len = length() - offset - byte_count;
      if (!back_holder_temp.replace_data_with_none(0, 0, back_of_range_len))
        return false;
      memcpy(back_holder_temp.m.u_get_mut_begin(),
             m.get_begin() + (m.get_length() - back_of_range_len),
             back_of_range_len);
      if (!remove_suffix(length() - offset) || !as_ownerized()) return false;
      auto front_holder_temp{std::move(*this)};
      for (; begin_iter != end_iter; ++begin_iter) {
        auto &&itr = *begin_iter;
        auto ch = optional_ref_t<const char>(itr);
        if (!ch) return false;
        if (!front_holder_temp.push_back(*ch)) return false;
      }
      if (!front_holder_temp.replace_data_with_none(nops, 0, back_of_range_len))
        return false;
      memcpy(front_holder_temp.m.u_get_mut_begin() +
                 (front_holder_temp.length() - back_of_range_len),
             back_holder_temp.data(), back_of_range_len);
      *this = std::move(front_holder_temp);
      return true;
    };
  }

  template <std::ranges::input_range R_t>
  MJZ_CX_ND_FN success_t insert_data_with_range(uintlen_t offset,
                                                R_t &&r) noexcept {
    return replace_data_with_range(offset, 0, std::forward<R_t>(r));
  }
  template <std::ranges::input_range R_t>
  MJZ_CX_ND_FN success_t append_data_with_range(R_t &&r) noexcept {
    return insert_data_with_range(nops, std::forward<R_t>(r));
  }
  template <std::ranges::input_range R_t>
  MJZ_CX_ND_FN success_t assign_data_with_range(R_t &&r) noexcept {
    return replace_data_with_range(0, nops, std::forward<R_t>(r));
  }

  MJZ_CX_ND_FN success_t erase_data(uintlen_t offset,
                                    uintlen_t byte_count) noexcept {
    make_right_then_give_has_null(offset, byte_count);
    return replace_data_with_none(offset, byte_count, 0);
  }

  MJZ_CX_ND_FN MJZ_FORCED_INLINE success_t
  push_back(const std::optional<char> c) noexcept {
    const auto len = size();
    if (!replace_data_with_none(len, 0, 1, false, true,
                                align_direction_e::front)) {
      return false;
    }
    if (!c) {
      remove_prefix(1);
      return true;
    }
    m.u_get_mut_begin()[len - 1] = *c;
    return true;
  }
  MJZ_CX_ND_FN MJZ_FORCED_INLINE success_t
  push_front(const std::optional<char> c) noexcept {
    if (!replace_data_with_none(0, 0, 1, true, false,
                                align_direction_e::back)) {
      return false;
    }
    if (!c) {
      remove_suffix(1);
      return true;
    }
    m.u_get_mut_begin()[0] = *c;
    return true;
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> pop_back() noexcept {
    optional_ref_t<mut_type> ch{back()};
    if (remove_suffix(1)) return ch;
    return {};
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> pop_front() noexcept {
    optional_ref_t<mut_type> ch{front()};
    if (remove_prefix(1)) return ch;
    return {};
  }
  /*
   * gives a mutable buffer (begin and length) with the sames values as the
   * currunnt buffer that will be in the buffer after the callback exit.
   */
  MJZ_CX_ND_FN success_t
  resize(uintlen_t new_len, std::optional<char> val = std::nullopt) noexcept {
    uintlen_t old_len = length();
    intlen_t del = intlen_t(new_len) - intlen_t(old_len);
    intlen_t pdel = branchless_teranary<intlen_t>(!(del < 0), del, 0);
    intlen_t ndel = branchless_teranary<intlen_t>(!(del > 0), -del, 0);
    return replace_data_with_char(new_len, uintlen_t(ndel), uintlen_t(pdel),
                                  val);
  }
  MJZ_CX_ND_FN std::optional<intlen_t> compare(
      const self_t &rhs) const noexcept {
    return m.get_view().compare(rhs.m.get_view());
  }

  MJZ_CX_ND_FN bool starts_with(const self_t &rhs) const noexcept {
    return m.get_view().starts_with(rhs.m.get_view());
  }

  MJZ_CX_ND_FN bool starts_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().starts_with(rhs, encoding);
  }

  MJZ_CX_ND_FN bool ends_with(const self_t &rhs) const noexcept {
    return m.get_view().ends_with(rhs.m.get_view());
  }
  MJZ_CX_ND_FN bool ends_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().ends_with(rhs, encoding);
  }

  MJZ_CX_ND_FN bool contains(const self_t &rhs) const noexcept {
    return m.get_view().contains(rhs.m.get_view());
  }
  MJZ_CX_ND_FN bool contains(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().contains(rhs, encoding);
  }
  MJZ_CX_ND_FN uintlen_t find(const self_t &rhs,
                              const uintlen_t offset = 0) const noexcept {
    return m.get_view().find(rhs.m.get_view(), offset);
  }
  MJZ_CX_ND_FN uintlen_t rfind(const self_t &rhs,
                               const uintlen_t offset = nops) const noexcept {
    return m.get_view().rfind(rhs.m.get_view(), offset);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(const self_t &rhs, const uintlen_t offset = 0) const noexcept {
    return m.get_view().find_first_of(rhs.m.get_view(), offset);
  }

  MJZ_CX_ND_FN uintlen_t find_last_of(
      const self_t &rhs, const uintlen_t offset = nops) const noexcept {
    return m.get_view().find_last_of(rhs.m.get_view(), offset);
  }
  MJZ_CX_ND_FN uintlen_t find_first_not_of(
      const self_t &rhs, const uintlen_t offset = 0) const noexcept {
    return m.get_view().find_first_not_of(rhs.m.get_view(), offset);
  }
  MJZ_CX_ND_FN uintlen_t find_last_not_of(
      const self_t &rhs, const uintlen_t offset = nops) const noexcept {
    return m.get_view().find_last_not_of(rhs.m.get_view(), offset);
  }

  MJZ_CX_ND_FN uintlen_t
  find(char rhs, const uintlen_t offset = 0,
       encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().find(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  rfind(char rhs, const uintlen_t offset = nops,
        encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().rfind(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(char rhs, const uintlen_t offset = 0,
                encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().find_first_of(rhs, offset, encoding);
  }

  MJZ_CX_ND_FN uintlen_t
  find_last_of(char rhs, const uintlen_t offset = nops,
               encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().find_last_of(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_not_of(char rhs, const uintlen_t offset = 0,
                    encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().find_first_not_of(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_last_not_of(char rhs, const uintlen_t offset = nops,
                   encodings_e encoding = encodings_e{}) const noexcept {
    return m.get_view().find_last_not_of(rhs, offset, encoding);
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral(uint8_t raidex = 0) const noexcept {
    return m.get_view().template to_integral<T>(raidex);
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_real_floating() const noexcept {
    return m.get_view().template to_real_floating<T>();
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_floating() const noexcept {
    return m.get_view().template to_floating<T>();
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_FN success_t as_integral(T val, const uint8_t raidex = 10,
                                  bool upper_case = false) noexcept {
    uintlen_t new_len = 0;
    auto fn = [&]() noexcept -> success_t {
      auto ret = traits_type{}.template from_integral_fill<T>(
          m.u_get_mut_begin(), m.get_capacity() - uintlen_t(props_v.has_null),
          val, upper_case, raidex);
      if (!ret) return false;
      new_len = *ret;
      return true;
    };
    if (!clear(true)) {
      return false;
    }
    while (!fn()) {
      if (!reserve(m.get_capacity() + 1, true)) {
        return false;
      }
    };
    set_encoding(dont_mess_up, encodings_e::ascii);
    return as_substring_<when_t::own_relax>(0, new_len);
  }

 private:
 public:
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_FN static self_t s_make_str(T val, const uint8_t raidex,
                                     bool upper_case = false) noexcept {
    self_t ret{};
    if constexpr (int_to_dec_unchekced_size_v<T> <= sso_cap) {
      if (9 < raidex) {
        ret.m
            .set_sso_length(
                *traits_type{}
                     .template from_integral_fill<T, sso_cap, alignof(self_t) /*sso buffer is aligned at beginning of the object*/>(
                         ret.m.m_sso_buffer_(), sso_cap, val, upper_case,
                         raidex));
        asserts(asserts.assume_rn,
                ret.m.is_sso() && ret.m.no_destroy() && !ret.get_alloc());

        return ret;
      }
    }
    ret.as_integral(val, raidex, upper_case);
    return ret;
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_FN static self_t s_make_str(T val) noexcept {
    self_t ret{};
    if constexpr (int_to_dec_unchekced_size_v<T> <= sso_cap) {
      ret.m
          .set_sso_length(traits_type{}
                              .template dec_from_int<sso_cap, alignof(self_t) /*sso buffer is aligned at beginning of the object*/>(
                                  ret.m.m_sso_buffer_(), sso_cap, val));
      asserts(asserts.assume_rn,
              ret.m.is_sso() && ret.m.no_destroy() && !ret.get_alloc());
    } else {
      ret.as_integral(val, 10, false);
    }
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_FN static self_t s_make_str(T val,
                                     uint8_t accuracacy = sizeof(T)) noexcept {
    self_t ret{};
    static_assert(15 <= sso_cap);
    constexpr uint8_t max_accuracy{
        uint8_t(std::min((sso_cap - 1) / 2, sso_cap - 8))};
    accuracacy = uint8_t(std::min<uintlen_t>(max_accuracy, accuracacy));
    ret.m.set_sso_length(traits_type{}.template from_float_format_fill<T>(
        ret.m.m_sso_buffer_(), sso_cap, val, accuracacy, false,
        floating_format_e::general));
    asserts(asserts.assume_rn,
            ret.m.is_sso() && ret.m.no_destroy() && !ret.get_alloc());
    return ret;
  }

  template <std::floating_point T>
  MJZ_CX_FN success_t as_floating(
      T val, uintlen_t accuracacy = sizeof(uintlen_t), bool upper_case = false,
      floating_format_e floating_format = floating_format_e::general) noexcept {
    uintlen_t new_len = 0;
    auto fn = [&]() noexcept -> success_t {
      auto ret = traits_type{}.template from_float_format_fill<T>(
          m.u_get_mut_begin(), m.get_capacity() - uintlen_t(props_v.has_null),
          val, accuracacy, upper_case, floating_format);
      if (!ret) return false;
      new_len = *ret;
      return true;
    };
    if (!clear(true)) {
      return false;
    }
    while (!fn()) {
      if (!reserve((m.get_capacity() * 3) >> 1, true)) {
        return false;
      }
    };
    set_encoding(dont_mess_up, encodings_e::ascii);
    return as_substring_<when_t::own_relax>(0, new_len);
  }

  using bview = base_string_view_t<version_v>;

  MJZ_CX_FN bview
  to_base_view_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return m.get_view().to_base_view_pv_fn_(dont_mess_up);
  }
  using blazy_t = base_lazy_view_t<version_v>;
  MJZ_CX_FN blazy_t
  to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return m.get_view().to_base_lazy_pv_fn_(dont_mess_up);
  }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, blazy_t v) noexcept {
    bool good{};
    MJZ_RELEASE {
      reset_to_error_on_fail_<when_t::relax>(
          good, "[Error]format_back_insert_append_pv_fn_:invalid input");
    };
    if (v.is_invalid() || v.get_encoding() != get_encoding()) {
      return false;
    }
    uintlen_t offset = m.get_length();
    if (!replace_data_with_none_impl_<when_t::relax>(
            nops, 0, v.len, false, true, align_direction_e::front)) {
      return false;
    }
    if (v.is_resurve()) {
      return good = as_substring(0, offset);
    }
    char *ptr = &m.u_get_mut_begin()[offset];
    return good = !!v.get_value(ptr);
  }

  // not implemented yet
  MJZ_CX_ND_FN success_t assign_own(const self_t &other,
                                    bool no_allocate = false) noexcept {
    return memcopy_assign_<when_t::relax>(other, no_allocate, 0, nops);
  }
  MJZ_CX_ND_FN success_t assign(const self_t &other,
                                bool no_allocate = false) noexcept {
    return copy_assign_<when_t::relax>(other, no_allocate, 0, nops);
  }
  MJZ_CX_ND_FN success_t assign_share(const self_t &other,
                                      bool no_allocate = false) noexcept {
    return share_init_<when_t::relax>(other, no_allocate, 0, nops);
  }
  MJZ_CX_ND_FN success_t assign_move(self_t &&other) noexcept {
    return move_init_<when_t::relax>(std::move(other));
  }
  MJZ_CX_ND_FN success_t assign(char c) noexcept {
    assign_ch_<when_t::relax>(c);
    return true;
  }

  MJZ_CX_ND_FN success_t replace_data(uintlen_t offset, uintlen_t byte_count,
                                      const self_t &other) noexcept {
    if (other.get_encoding() != get_encoding()) return false;
    if (&other == this) {
      return false;
    }
    make_right_then_give_has_null(offset, byte_count);
    if (!replace_data_with_none(offset, byte_count, other.size())) {
      return true;
    }
    memcpy(m.u_get_mut_begin() + offset, other.data(), other.size());
    return true;
  }

  MJZ_CX_ND_FN success_t insert_data(uintlen_t offset,
                                     const self_t &other) noexcept {
    return replace_data(offset, 0, other);
  }
  MJZ_CX_ND_FN success_t append_data(const self_t &other) noexcept {
    return insert_data(nops, other);
  }

  MJZ_CX_ND_FN success_t
  insert_data_with_char(uintlen_t offset, uintlen_t length_of_val,
                        std::optional<char> val) noexcept {
    return replace_data_with_char(offset, 0, length_of_val, val);
  }
  MJZ_CX_ND_FN success_t append_data_with_char(
      uintlen_t length_of_val, std::optional<char> val) noexcept {
    return insert_data_with_char(nops, length_of_val, val);
  }

  MJZ_CX_FN success_t append_data_temp(self_t &&str) noexcept {
    uintlen_t new_len = str.length() + length();
    bool can_fit_lhs =
        m.template has_room_for<when_t::relax>(new_len, props_v.has_null);
    bool can_fit_rhs = str.m.template has_room_for<when_t::relax>(
        new_len, str.props_v.has_null);
    bool both_can_fit = can_fit_lhs;
    both_can_fit |= can_fit_rhs;
    bool rhs_is_better = m.get_capacity() < str.m.get_capacity();
    can_fit_rhs = branchless_teranary(!both_can_fit, can_fit_rhs, rhs_is_better

    );
    if (can_fit_rhs) {
      if (!str.insert_data(0, *this)) {
        return false;
      }
      *this = std::move(str);
      return true;
    }
    if (!str.append_data(*this)) {
      return false;
    }
    return true;
  }
  MJZ_CX_FN self_t &operator+=(self_t &&obj) noexcept {
    reset_to_error_on_fail_<when_t::relax>(
        append_data_temp(std::move(obj)),
        "[Error]basic_str_t::operator+=(self_t &&obj)");
    return *this;
  }
  MJZ_CX_FN self_t &operator+=(const self_t &obj) noexcept {
    reset_to_error_on_fail_<when_t::relax>(
        append_data(obj), "[Error]basic_str_t::operator+=(const self_t &obj)");
    return *this;
  }
  MJZ_CX_FN static self_t operator_add(self_t &&rhs, self_t &&lhs) noexcept {
    rhs += std::move(lhs);
    return std::move(rhs);
  }

  MJZ_CX_FN static self_t operator_add(self_t &&rhs,
                                       const self_t &lhs) noexcept {
    rhs += lhs;
    return std::move(rhs);
  }

  MJZ_CX_FN static self_t operator_add(const self_t &rhs,
                                       self_t &&lhs) noexcept {
    lhs.reset_to_error_on_fail_<when_t::relax>(
        lhs.insert_data(0, rhs),
        "[Error]basic_str_t::operator_add(const self_t &,self_t&&)");
    return std::move(lhs);
  }
  MJZ_CX_FN static self_t operator_add(const self_t &rhs,
                                       const self_t &lhs) noexcept {
    self_t ret{rhs};
    ret += lhs;
    return ret;
  }
  template <partial_same_as<self_t> R_t, partial_same_as<self_t> L_t>
  MJZ_CX_FN friend self_t operator+(R_t &&rhs, L_t &&lhs) noexcept {
    return operator_add(std::forward<R_t>(rhs), std::forward<L_t>(lhs));
  }

  MJZ_CX_FN void reset_to_error_on_fail(success_t op,
                                        static_string_view view) noexcept {
    return reset_to_error_on_fail_<when_t::relax>(op, view);
  }

#if MJZ_WITH_iostream

  MJZ_NCX_FN friend std::ostream &operator<<(std::ostream &cout_v,
                                             const self_t &obj) {
    return cout_v << obj.m.get_view();
  }
  /*
   *CREDIT TO MSVC's IMPLEMENTATION , IDK WHAT IT MEANS std::istream IS NOT
   *NICE!
   */
  MJZ_NCX_FN friend std::istream &getline(std::istream &cin_v, self_t &obj,
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
        success &= obj.push_back(traits::to_char_type(meta));

        if (!success) break;
        state_changed = true;
      }
      no_throw = true;
    }  //~MJZ_RELEASE;
    cin_base.exceptions(perivous_exp_state);
    return cin_v;
  }
  /*
   *CREDIT TO MSVC's IMPLEMENTATION , IDK WHAT IT MEANS std::istream IS NOT
   *NICE!
   */

  MJZ_NCX_FN friend std::istream &operator>>(std::istream &cin_v, self_t &obj) {
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
        if (traits_type{}.is_space(traits::to_char_type(meta))) {
          break;  // whitespace, quit
        }

        // reserve ,got a character, add it to string
        success &= obj.push_back(traits::to_char_type(meta));
        if (!success) break;
        has_changed = true;
      }
      no_throw = true;
      ;
    };  //~MJZ_RELEASE;
    cin_base.exceptions(perivous_exp_state);
    return cin_v;
  }

#endif  // MJZ_WITH_iostream

  MJZ_CX_FN friend bool operator==(const self_t &rhs,
                                   const self_t &lhs) noexcept {
    if (rhs.length() != lhs.length()) return false;
    auto v = rhs.compare(lhs);
    return v && *v == 0;
  }

  MJZ_CX_FN friend std::partial_ordering operator<=>(
      const self_t &rhs, const self_t &lhs) noexcept {
    if (auto r = rhs.compare(lhs)) {
      if (*r == 0) return std::partial_ordering::equivalent;
      if (*r < 0) return std::partial_ordering::less;
      if (0 < *r) return std::partial_ordering::greater;
    }
    return std::partial_ordering::unordered;
  }
};
template <version_t version_v>
template <typename T>
MJZ_CX_FN auto base_out_it_t<version_v>::append_obj_impl_(
    const T &view) noexcept {
  using big_str_t_ =
      basic_str_t<version_v, basic_str_props_t<version_v>{.sso_min_cap = 30}>;
  if constexpr (requires() { view.to_base_view_pv_fn_(unsafe_ns::unsafe_v); }) {
    return append(view.to_base_view_pv_fn_(unsafe_ns::unsafe_v));
  } else if constexpr (requires() { big_str_t_::s_make_str(view); }) {
    return append(
        big_str_t_::s_make_str(view).to_base_view_pv_fn_(unsafe_ns::unsafe_v));
  } else {
    return;
  }
}

};  // namespace mjz::bstr_ns

namespace mjz ::bstr_ns {
namespace litteral_ns {
MJZ_FCONSTANT(version_t) version_V_var1_{};
/*
 *makes a gengeric basic_str_t  that views the string
 */
template <str_litteral_t L, version_t vr = version_t{},
          basic_str_props_t<vr> props_v = basic_str_props_t<vr>{}>
MJZ_CE_FN decltype(auto) operator_str() noexcept
  requires(!std::is_empty_v<basic_str_t<vr, props_v>>)
{
  if constexpr (props_v.is_ownerized && props_v.sso_min_cap < L.size()) {
    return basic_str_t<vr, props_v>{operator_view<L, vr>()};
  } else {
    return make_static_data([]() noexcept {
      return basic_str_t<vr, props_v>(operator_view<L, vr>());
    });
  }
};
template <int = 0>
using default_str_t_ =
    basic_str_t<version_V_var1_, basic_str_props_t<version_V_var1_>{}>;
/*
 *specialized operator for newest version
 */
/*
 *makes a basic_str_t(with custom allocator feature) that views the string
 */
template <str_litteral_t L>
MJZ_CX_FN const default_str_t_<> &operator_cxstr_() noexcept
  requires(!std::is_empty_v<default_str_t_<>>)
{
  return make_static_data([]() noexcept {
    return default_str_t_<>(operator_view<L, version_V_var1_>());
  });
}
template <str_litteral_t L>
MJZ_CX_FN auto operator""_cxstr() noexcept -> const default_str_t_<> & {
  return operator_cxstr_<L>();
}
template <str_litteral_t L>
MJZ_CX_FN auto operator""_str() noexcept -> default_str_t_<> {
  using ret_t = default_str_t_<>;
  constexpr const auto &m =
      operator_cxstr_<L>().unsafe_handle_pv_(unsafe_ns::unsafe_v);
  ret_t ret{};
  /* assert that we strat with sso*/
  /* assert that string is not self referential , and its address is not part
   * of its identity*/
  auto &m_ret = ret.unsafe_handle_pv_(unsafe_ns::unsafe_v);
  MJZ_IF_CONSTEVAL { m_ret = m; }
  else {
    memcpy(reinterpret_cast<char *>(&m_ret), reinterpret_cast<const char *>(&m),
           sizeof(ret_t));
  }
  return ret;
}
};  // namespace litteral_ns
};  // namespace mjz::bstr_ns
template <mjz::version_t version_v,
          mjz::bstr_ns::basic_str_props_t<version_v> props_v>
struct std::hash<mjz::bstr_ns::basic_str_t<version_v, props_v>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};

#endif  // MJZ_BYTE_STRING_string_LIB_HPP_FILE_
