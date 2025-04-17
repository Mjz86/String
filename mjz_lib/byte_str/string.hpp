
#include <compare>
#include <utility>

#include "../restricted_arguments.hpp"
#include "string_abi.hpp"

#ifndef MJZ_BYTE_STRING_string_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_LIB_HPP_FILE_
namespace mjz::bstr_ns {

template <class T, class self_str_t_>
concept str_c_ = requires() {
  { version_t{std::remove_cvref_t<T>::Version_v_} } noexcept;
  requires(std::remove_cvref_t<T>::Version_v_ ==
           std::remove_cvref_t<self_str_t_>::Version_v_);
  typename std::remove_cvref_t<T>::str_t_indentity_t_;
};
struct props_t {
  uintlen_t sso_min_cap{};
  bool has_alloc{};
  bool has_null{};
  bool is_ownerized{};
  may_bool_t is_threaded{may_bool_t::idk};
  align_direction_e align{};
};
template <version_t version_v_, props_t props_v_ = props_t{}>
struct basic_str_t : void_struct_t {
  MJZ_CONSTANT(version_t)
  version_v = version_v_;
  MJZ_CONSTANT(props_t)
  props_v = props_v_;
  MJZ_CONSTANT(version_t)
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
  MJZ_CONSTANT(uintlen_t) sso_cap = m_t::sso_cap;
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

  MJZ_CONSTANT(uintlen_t)
  npos{traits_type::npos};
  MJZ_CONSTANT(uintlen_t)
  nops{traits_type::npos};

 private:
  using replace_flags = replace_flags_t<version_v>;
  using static_string_view = static_string_view_t<version_v>;

  using dynamic_string_view = dynamic_string_view_t<version_v>;
  using generic_string_view = basic_string_view_t<version_v>;

  using str_heap_manager = str_heap_manager_t<version_v>;
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
  MJZ_CX_ND_FN success_t memcpy_to_me_nonsso_alloc(const char *ptr,
                                                   uintlen_t len) noexcept {
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.template u_malloc<props_v.has_alloc>(uintlen_t(props_v.has_null) +
                                                 len))
      MJZ_IS_UNLIKELY {
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
  MJZ_CX_ND_FN success_t memcpy_to_me_nonsso(const char *ptr, uintlen_t len,
                                             bool no_allocate) noexcept {
    if constexpr (!when_v.is_sso()) {
      if (!m.is_sso() &&
          m.template has_room_for<when_v>(len, props_v.has_null)) {
        m.template memcpy_to_non_sso<when_t::no_heap>(
            ptr, len, m.get_buffer_ptr(), m.get_capacity(),
            m.cntrl().is_sharable);
        return true;
      }
    }
    if (no_allocate) return false;
    return memcpy_to_me_nonsso_alloc<when_v>(ptr, len);
  }
  template <when_t when_v>
  MJZ_CX_ND_FN success_t memcpy_to_me_sso(const char *ptr,
                                          uintlen_t len) noexcept {
    asserts(asserts.assume_rn, len <= sso_cap);
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    m.set_invalid_to_sso(ptr, len);
    return true;
  }

  template <when_t when_v>
  MJZ_CX_ND_FN success_t memcpy_to_me_(const char *ptr, uintlen_t len,
                                       bool no_allocate) noexcept {
    if (len <= sso_cap) {
      return memcpy_to_me_sso<when_v>(ptr, len);
    }
    return memcpy_to_me_nonsso<when_v>(ptr, len, no_allocate);
  }

  template <when_t when_v>
  MJZ_CX_ND_FN success_t init_view_cpy_(const generic_string_view &view,
                                        bool no_allocate) noexcept {
    if (!memcpy_to_me_<when_v>(view.data(), view.size(), no_allocate)) {
      return false;
    }
    m.cntrl().encodings_bits = uint8_t(view.get_encoding());
    return true;
  }
  template <when_t when_v>
  MJZ_CX_ND_FN success_t init_view_(const generic_string_view &view,
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
  MJZ_CX_ND_FN success_t memcopy_assign_(str_c_<self_t> auto const &obj,
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
    byte_offset = std::min(byte_offset, len);
    byte_count = std::min(byte_offset + byte_count, len) - byte_offset;
    bool ret = byte_offset + byte_count == len;
    ret &= m.cntrl().has_null;
    return ret;
  }

  template <when_t when_v>
  MJZ_CX_FN success_t as_substring_(uintlen_t byte_offset,
                                    uintlen_t byte_count) noexcept {
    bool has_null = make_right_then_give_has_null(byte_offset, byte_count);
    if (m.is_sso()) {
      char *buf = m.m_v().raw_data.sso_buffer;
      memomve_overlap(buf, buf + byte_offset, byte_count);
      buf[byte_count] = '\0';
      m.set_sso_length(byte_count);
      return true;
    }
    m.non_sso().begin_ptr += byte_offset;
    m.non_sso().length = byte_count;
    m.cntrl().has_null = has_null;
    if constexpr (!props_v.has_null) {
      return true;
    }
    if (m.template add_null<when_v>()) {
      return true;
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.template u_malloc<props_v.has_alloc>(1 + byte_count, false))
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
  template <when_t when_v, str_c_<self_t> T>
  MJZ_CX_ND_FN success_t move_init_(alias_t<T &&> str) noexcept {
    if constexpr (when_v) {
      m.destruct_to_invalid();
    }
    if constexpr (partial_same_as<decltype(str), self_t>) {
      m = std::exchange(str.m, {});
      return true;
    }
    if constexpr (std::remove_cvref_t<T>::sso_cap <= sso_cap) {
      m.cntrl().encodings_bits = str.m.cntrl().encodings_bits;
      m.set_threaded(str.m.is_threaded());
      if constexpr (!props_v.is_ownerized) {
        m.cntrl().is_ownerized = str.m.cntrl().is_ownerized;
      }
      m.set_invalid_to_sso(str.m.get_begin(), str.m.get_length());
      return true;
    }
    return copy_assign_<when_v>(str);
  }
  template <when_t when_v>
  MJZ_CX_ND_FN success_t share_init_(str_c_<self_t> auto const &obj,
                                     bool no_allocate = false,
                                     uintlen_t offset = 0,
                                     uintlen_t count = nops) noexcept {
    if (void_struct_cast_t::up_cast(this) ==
        &void_struct_cast_t::up_cast(obj)) {
      return as_substring_<when_v>(offset, count);
    }
    set_prpos_to_<when_v>(obj);
    bool has_null_{obj.make_right_then_give_has_null(offset, count)};
    constexpr bool need_to_check_alloc_equality =
        std::remove_cvref_t<decltype(m)>::has_alloc_v() ||
        std::remove_cvref_t<decltype(obj.m)>::has_alloc_v();
    if constexpr (props_v.is_ownerized) {
      return memcopy_assign_<when_v>(obj, no_allocate, offset, count);
    }
    bool cant = !obj.m.cntrl().is_sharable;
    cant |= props_v.has_null && !has_null_;
    cant |= m.is_ownerized();
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

    str_heap_manager hm = obj.m.non_sso_my_heap_manager_no_own();
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
  MJZ_CX_ND_FN success_t copy_assign_(str_c_<self_t> auto const &obj,
                                      bool no_allocate = false,
                                      uintlen_t offset = 0,
                                      uintlen_t count = nops) noexcept {
    if (void_struct_cast_t::up_cast(this) ==
        &void_struct_cast_t::up_cast(obj)) {
      return as_substring_<when_v>(offset, count);
    }
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
  MJZ_CX_ND_FN success_t reset_(cheap_str_info &info) noexcept {
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
    if (!hm.template u_malloc<props_v.has_alloc>(
            info.reserve_capacity +
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
  MJZ_CX_FN void assign_ch_(char c) noexcept {
    auto fn_ = [this, c]() noexcept {
      char *p = m.u_get_mut_begin();
      p[0] = c;
      if constexpr (props_v.has_null) {
        p[1] = 0;
        m.cntrl().has_null = true;
      } else {
        m.cntrl().has_null = false;
      }
      m.set_length(1);
      return;
    };
    if (m.template has_room_for<when_v>(1, props_v.has_null)) return fn_();
    m.destruct_all();
    return fn_();
  }

  template <when_t when_v>
  MJZ_CX_FN void set_prpos_to_(str_c_<self_t> auto const &src) noexcept {
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
    m.non_sso().length = byte_count;
    if constexpr (!props_v.has_null) {
      return;
    }
    if constexpr (no_null_) {
      return;
    }
    if (m.template add_null<when_t::no_heap>()) {
      return;
    }

    reset_to_error_on_fail_<when_t::no_heap>(
        as_ownerized(true),
        "[Error] basic_str_t(const dont_mess_up_t &, owned_stack_buffer "
        "&,uintlen_t , uintlen_t,bool) : cannot add null , init fail ");
  }

 public:
  MJZ_CX_FN ~basic_str_t() noexcept { m.destruct_to_invalid(); }
  MJZ_CX_FN basic_str_t() noexcept { m.invalid_to_empty(); }
  MJZ_CX_FN basic_str_t(self_t &&src) noexcept : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        move_init_<when_t::as_sso>(std::move(src)),
        "[Error]basic_str_t(basic_str_t&&):couldn't move string!");
  }
  MJZ_CX_FN basic_str_t(self_t const &src) noexcept : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        copy_assign_<when_t::as_sso>(src),
        "[Error]basic_str_t(const basic_str_t&):couldn't copy string!");
  }
  MJZ_CX_FN basic_str_t(self_t const &&src) noexcept : basic_str_t() {
    reset_to_error_on_fail_<when_t::as_sso>(
        share_init_<when_t::as_sso>(src),
        "[Error]basic_str_t(const basic_str_t&&):couldn't share string!");
  }
  MJZ_CX_FN basic_str_t &operator=(self_t &&src) noexcept {
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
  MJZ_CX_FN basic_str_t &operator=(self_t const &src) noexcept {
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
  MJZ_CX_FN basic_str_t &operator=(self_t const &&src) noexcept {
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
    info.reserve_capacity = std::max(info.reserve_capacity, length());
    bool add_null_ = !!(info.add_null + props_v.has_null);
    if (m.template has_room_for<when_t::no_heap>(info.reserve_capacity,
                                                 add_null_)) {
      if (add_null_) {
        asserts(asserts.assume_rn, m.template add_null<when_t::no_heap>());
      }
      return;
    }
    reset_to_error_on_fail_<when_t::no_heap>(
        reserve_<when_t::no_heap>(info.reserve_capacity + uintlen_t(add_null_)),
        "[Error]:basic_str_t(const dont_mess_up_t &, owned_stack_buffer && "
        ",cheap_str_info && , uintlen_t  , uintlen_t ): coulnt initilize");
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
          info.reserve_capacity = alias_t<uintlen_t[2]>{
              0, std::max(view.size(), info.reserve_capacity)}[do_alloc_];
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
  MJZ_CX_ND_FN basic_str_t(const basic_str_t &obj, cheap_str_info info) noexcept
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
                                 uintlen_t end_i = nops) noexcept {
    return make_substring(begin_i, end_i - begin_i);
  }

  /* similar to as_substring_*/
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
  MJZ_CX_ND_FN bool has_null() const noexcept { return !!m.cntrl().has_null; }
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
    ptr = alias_t<const_pointer[2]>{ptr, nullptr}[bad];
    return ptr + alias_t<uintlen_t[2]>{i, 0}[bad];
  }
  MJZ_CX_ND_FN optional_ref_t<mut_type> at(const uintlen_t i) noexcept {
    bool bad = (i < length());
    auto ptr = data();
    ptr = alias_t<pointer[2]>{ptr, nullptr}[bad];
    return ptr + alias_t<uintlen_t[2]>{i, 0}[bad];
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
  MJZ_CX_FN success_t reserve_(uintlen_t mincap,
                               bool round_up = false) noexcept {
    if (m.template has_room_for<when_v>(mincap, props_v.has_null)) {
      return true;
    }
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.template u_malloc<props_v.has_alloc>(
            uintlen_t(props_v.has_null) + mincap, round_up))
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
    asserts(asserts.assume_rn, m.template add_null<when_t::no_heap>());
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
    return u_consider_stack( where);
  }
  MJZ_CX_FN success_t shrink_to_fit(bool force_ownership = false) noexcept {
    if (!force_ownership && !m.is_owner()) return true;
    if (!m.is_heap()) return true;  // we werent on heap to begin with
    str_heap_manager hm{m.get_alloc(), m.is_threaded(), m.is_ownerized()};
    if (!hm.template u_malloc<props_v.has_alloc>(
            uintlen_t(props_v.has_null) + m.get_length(), false))
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
    m.set_length(0);
    force_ownership |= props_v.has_null;
    if (force_ownership) {
      return as_ownerized(props_v.has_null);
    }
    return true;
  }

  MJZ_CX_FN success_t add_null() noexcept {
    return has_null() || m.template add_null<when_t::relax>() ||
           as_ownerized(true);
  }

  MJZ_CX_FN const char *as_c_str() & noexcept {
    bool good = add_null();
    return alias_t<alias_t<const char *>[2]>{nullptr, data()}[good];
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
                                           !!m.cntrl().is_sharable, false);
          } else {
            m.set_sso_length(new_len);
          }
          asserts(asserts.assume_rn,
                  !props_v.has_null || m.template add_null<when_t::no_heap>());
        };
        bool choose_both = choose_back;
        choose_both &= choose_front;
        bool temp_choose_front = offset + byte_count < len - offset;
        choose_front =
            alias_t<bool[2]>{choose_front, temp_choose_front}[choose_both];
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
        delta_count = alias_t<uintlen_t[2]>{delta_count, 0}[m.is_sso()];
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
      if (!hm.template u_malloc<props_v.has_alloc>(uintlen_t(props_v.has_null) +
                                                   new_len))
        MJZ_IS_UNLIKELY {
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
  MJZ_CX_FN success_t replace_data_with_none(uintlen_t offset,
                                             uintlen_t byte_count,
                                             uintlen_t fill_len,
                                             bool choose_front = true,
                                             bool choose_back = true) noexcept {
    return m.no_destroy()
               ? replace_data_with_none_impl_<when_t::no_heap>(
                     offset, byte_count, fill_len, choose_front, choose_back)
               : replace_data_with_none_impl_<when_t::relax>(
                     offset, byte_count, fill_len, choose_front, choose_back);
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

  MJZ_CX_ND_FN success_t
  as_ownerized(bool add_null = props_v.has_null) noexcept {
    if (m.template has_room_for<when_t::relax>(m.get_length(), add_null))
      return true;
    return reserve(uintlen_t(add_null) + length()) &&
           m.template add_null<when_t::own_relax>();
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
    requires std::ranges::sized_range<R_t> && std::ranges::forward_range<R_t>
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
        auto ch = get_as_option<char>(*begin_iter);
        if (!ch) return false;
        range_ptr[i] = *ch;
      }
      return true;
    };
  }
  template <std::ranges::forward_range R_t>
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
        auto ch = get_as_option<char>(*begin_iter);
        if (!ch) return false;
        if (!front_holder_temp.replace_data_with_char(nops, 0, 1, *ch))
          return false;
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

  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t insert_data_with_range(uintlen_t offset,
                                                R_t &&r) noexcept {
    return replace_data_with_range(offset, 0, std::forward<R_t>(r));
  }
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t append_data_with_range(R_t &&r) noexcept {
    return insert_data_with_range(nops, std::forward<R_t>(r));
  }
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t assign_data_with_range(R_t &&r) noexcept {
    return replace_data_with_range(0, nops, std::forward<R_t>(r));
  }

  MJZ_CX_ND_FN success_t erase_data(uintlen_t offset,
                                    uintlen_t byte_count) noexcept {
    return replace_data_with_none(offset, byte_count, 0);
  }

  MJZ_CX_ND_FN success_t push_back(std::optional<char> c) noexcept {
    return replace_data_with_char(nops, 0, 1, c);
  }
  MJZ_CX_ND_FN success_t push_front(std::optional<char> c) noexcept {
    return replace_data_with_char(0, 0, 1, c);
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
    intlen_t pdel = alias_t<intlen_t[2]>{del, 0}[del < 0];
    intlen_t ndel = alias_t<intlen_t[2]>{-del, 0}[del > 0];
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
    m.set_length(new_len);
    asserts(asserts.assume_rn, m.template add_null<when_t::own_relax>());
    set_encoding(dont_mess_up, encodings_e::ascii);
    return true;
  }
  template <std::floating_point T>
  MJZ_CX_FN success_t as_floating(
      T val, uintlen_t accuracacy = sizeof(uintlen_t), bool upper_case = false,
      floating_format_e floating_format = floating_format_e::general,
      char point_ch = '.') noexcept {
    uintlen_t new_len = 0;
    auto fn = [&]() noexcept -> success_t {
      auto ret = traits_type{}.template from_float_format_fill<T>(
          m.u_get_mut_begin(), m.get_capacity() - uintlen_t(props_v.has_null),
          val, accuracacy, upper_case, floating_format, point_ch);
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
    m.set_length(new_len);
    asserts(asserts.assume_rn, m.template add_null<when_t::own_relax>());
    set_encoding(dont_mess_up, encodings_e::ascii);

    return true;
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
      m.set_length(offset);
    } else {
      char *ptr = &m.u_get_mut_begin()[offset];
      good = true;
      uintlen_t total{};
      good &= v.get_value(
          [&](base_string_view_t<version_v> read_slice) noexcept -> success_t {
            if constexpr (MJZ_IN_DEBUG_MODE) {
              total += read_slice.len;
              asserts(asserts.assume_rn, total <= v.len);
            }
            memcpy(ptr, read_slice.ptr, read_slice.len);
            ptr += read_slice.len;
            return true;
          });
      if constexpr (MJZ_IN_DEBUG_MODE) {
        asserts(asserts.assume_rn, total == v.len);
      }
    }
    if constexpr (props_v.has_null) {
      if (good)
        asserts(asserts.assume_rn, m.template add_null<when_t::own_relax>());
    }
    return good = true;
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
    can_fit_rhs = alias_t<bool[2]>{can_fit_rhs, rhs_is_better}[both_can_fit];
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
  MJZ_CX_FN friend self_t operator+(R_t &&rhs, L_t &lhs) noexcept {
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
};  // namespace mjz::bstr_ns

namespace mjz ::bstr_ns {
namespace litteral_ns {
MJZ_CONSTANT(version_t) version_V_var1_{};
/*
 *makes a gengeric basic_str_t  that views the string
 */
template <str_litteral_t L, version_t vr = version_t{},
          props_t props_v = props_t{}>
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
/*
 *specialized operator for newest version
 */
/*
 *makes a basic_str_t(with custom allocator feature) that views the string
 */
template <str_litteral_t L>
MJZ_CX_FN decltype(auto) operator""_str() noexcept
  requires(!std::is_empty_v<basic_str_t<version_V_var1_, props_t{}>>)
{
  return make_static_data([]() noexcept {
    return basic_str_t<version_V_var1_, props_t{}>(
        operator_view<L, version_V_var1_>());
  });
}
};  // namespace litteral_ns
};  // namespace mjz::bstr_ns
template <mjz::version_t version_v, mjz::bstr_ns::props_t props_v>
struct std::hash<mjz::bstr_ns::basic_str_t<version_v, props_v>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};

#endif  // MJZ_BYTE_STRING_string_LIB_HPP_FILE_