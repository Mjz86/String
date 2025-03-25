
#include "heaps.hpp"
#include "views.hpp"
#ifndef MJZ_BYTE_STRING_string_abi_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_abi_LIB_HPP_FILE_
namespace mjz ::bstr_ns {
struct wrapped_props_t {
  uintlen_t sso_min_cap{};
  bool has_alloc{};
  bool has_null{};
  bool is_ownerized{};
  MJZ_CX_FN uintlen_t cap() const noexcept {
    if (sso_min_cap % sizeof(uintlen_t)) {
      return (sso_min_cap | (sizeof(uintlen_t) - 1)) + 1;
    }
    return sso_min_cap;
  }
};
template <version_t version_v>
struct MJZ_DEPRECATED_R(" unneccesery alloc_ref creation ") base_str_info_t {
  allocs_ns::alloc_base_ref_t<version_v> alloc{};
  uintlen_t reserve_capacity{0};
  bool is_threaded{true};
  // mostly ignored for now
  bool add_null{true};
  // mostly ignored for now
  encodings_e encoding{encodings_e::ascii};
};
template <version_t version_v>
struct cheap_base_str_info_t {
  const allocs_ns::alloc_base_ref_t<version_v> *alloc_ptr{};
  uintlen_t reserve_capacity{0};
  bool is_threaded{true};
  // mostly ignored for now
  bool add_null{true};
  // mostly ignored for now
  encodings_e encoding{encodings_e::ascii};
};
namespace basic_str_abi_ns_ {
template <version_t version_v, bool has_alloc_v_>
struct alloc_t {};
template <version_t version_v>
struct alloc_t<version_v, true> {
  allocs_ns::alloc_base_ref_t<version_v> alloc_;
};
template <version_t version_v>
union nsso_u;
template <version_t version_v>
  requires(version_v.is_LE())
union nsso_u<version_v> {
  struct non_sso_t {
    char *buffer_begin;
    uintlen_t buffer_capacity_and_cntrl;
  };
  char dummy_{};
  non_sso_t non_sso;
  char sso_raw_buffer_and_cntrl[sizeof(non_sso_t)];
  MJZ_CONSTANT(uintlen_t) sso_cap = sizeof(non_sso_t) - 1;
  MJZ_CONSTANT(uintlen_t) buffer_cap_mask_ { uintlen_t(-1) >> 8 };
  MJZ_CONSTANT(uintlen_t) buffer_cap_max_ { buffer_cap_mask_ };
  MJZ_CONSTANT(uintlen_t) buffer_cntrl_mask_ { ~buffer_cap_mask_ };
  MJZ_CONSTANT(uintlen_t) buffer_cntrl_index_ { sizeof(uintlen_t) - 1 };
  MJZ_CONSTANT(uintlen_t) sso_cntrl_index_ { sizeof(non_sso_t) - 1 };
  MJZ_CONSTANT(uintlen_t) sso_buf_index_ { 0 };
  MJZ_DISABLE_ALL_WANINGS_START_;
};
MJZ_DISABLE_ALL_WANINGS_END_;
template <version_t version_v>
  requires(version_v.is_BE())
union nsso_u<version_v> {
  struct non_sso_t {
    uintlen_t buffer_capacity_and_cntrl;
    char *buffer_begin;
  };

  char dummy_{};
  non_sso_t non_sso;
  char sso_raw_buffer_and_cntrl[sizeof(non_sso_t)];
  MJZ_CONSTANT(uintlen_t) sso_cap = sizeof(non_sso_t) - 1;
  MJZ_CONSTANT(uintlen_t) buffer_cap_mask_ { uintlen_t(-1) >> 8 };
  MJZ_CONSTANT(uintlen_t) buffer_cntrl_mask_ { ~buffer_cap_mask_ };
  MJZ_CONSTANT(uintlen_t) buffer_cap_max_ { buffer_cap_mask_ };
  MJZ_CONSTANT(uintlen_t) sso_cntrl_index_ { 0 };
  MJZ_CONSTANT(uintlen_t) sso_buf_index_ { 1 };
  MJZ_CONSTANT(uintlen_t)
  buffer_cntrl_index_{0};

  MJZ_DISABLE_ALL_WANINGS_START_;
};
MJZ_DISABLE_ALL_WANINGS_END_;
template <version_t version_v>
struct details_t {
  MJZ_CONSTANT(uint8_t) as_not_threaded_bit { 0b10000000 };
  MJZ_CONSTANT(uint8_t)
  is_sharable{0b01000000};
  /*this flag is not propagated by copy or share, only by move */
  MJZ_CONSTANT(uint8_t)
  is_ownerized{0b00100000};
  MJZ_CONSTANT(uint8_t)
  has_null{0b00010000};
  MJZ_CONSTANT(uint8_t)
  unused_for_now{0b00001000};
  MJZ_CONSTANT(uint8_t)
  encodings_bits{0b00000111};
  MJZ_CONSTANT(uint8_t)
  full_byte_{is_ownerized | is_sharable | has_null | unused_for_now |
             as_not_threaded_bit | encodings_bits};
  MJZ_SASSERT(full_byte_ == uint8_t(-1));
  MJZ_CONSTANT(uint8_t)
  encoding_max = encodings_bits;
};

template <version_t version_v, bool has_alloc_v_>
struct m_t : public basic_str_abi_ns_::alloc_t<version_v, has_alloc_v_> {
  const char *begin;
  uintlen_t length;
  using mut_data_t = basic_str_abi_ns_::nsso_u<version_v>;
  mut_data_t mut_data;

 public:
  MJZ_DISABLE_ALL_WANINGS_START_;
  MJZ_CX_FN m_t(nullptr_t) noexcept {};
  MJZ_CX_FN ~m_t() noexcept = default;
  MJZ_DISABLE_ALL_WANINGS_END_;
  MJZ_CX_FN m_t() noexcept = delete;
  MJZ_CX_FN m_t(m_t &&obj) noexcept = delete;
  MJZ_CX_FN m_t(const m_t &) noexcept = delete;
  MJZ_CX_FN m_t &operator=(m_t &&) noexcept = delete;
  MJZ_CX_FN m_t &operator=(const m_t &) noexcept = delete;

 private:
  template <class>
  friend class mjz_private_accessed_t;

  using str_heap_manager = str_heap_manager_t<version_v>;
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  using my_details = basic_str_abi_ns_::details_t<version_v>;

 public:  // MJZ_offsetof
  static_assert(
      []() {
        MJZ_FCONSTANT(auto)
        cntrl_offset_non_sso =
            MJZ_offsetof(mut_data_t, non_sso.buffer_capacity_and_cntrl) +
            mut_data_t::buffer_cntrl_index_;
        MJZ_FCONSTANT(auto)
        cntrl_offset_sso = MJZ_offsetof(mut_data_t, sso_raw_buffer_and_cntrl) +
                           mut_data_t::sso_cntrl_index_;
        return cntrl_offset_non_sso == cntrl_offset_sso;
      }(),
      "The ABI must have the control byte in the same location to be "
      "branchless in runtime");

 public:
  MJZ_CONSTANT(alloc_ref)
  empty_alloc{};
  template <class = void>
    requires has_alloc_v_
  MJZ_CX_FN const alloc_ref *get_alloc_ptr() const noexcept

  {
    return &this->alloc_;
  }
  template <class = void>
    requires(!has_alloc_v_)
  MJZ_CX_FN const alloc_ref *get_alloc_ptr() const noexcept {
    return {};
  }
  MJZ_CX_FN const alloc_ref &get_alloc() const noexcept {
    return get_alloc_ptr() ? *get_alloc_ptr() : empty_alloc;
  }
  template <class = void>
    requires has_alloc_v_
  MJZ_CX_FN alloc_ref *get_alloc_ptr() noexcept {
    return &this->alloc_;
  }
  template <class = void>
    requires(!has_alloc_v_)
  MJZ_CX_FN alloc_ref *get_alloc_ptr() noexcept {
    return {};
  }
  MJZ_CX_FN bool has_c_alloc() const noexcept {
    return get_alloc_ptr() && *get_alloc_ptr();
  }
  MJZ_CX_FN char *non_sso_buffer_location_ptr() const noexcept {
    return std::launder(&mut_data.non_sso)->buffer_begin;
  }
  MJZ_CX_FN char *&non_sso_buffer_location_ptr() noexcept {
    return std::launder(&mut_data.non_sso)->buffer_begin;
  }
  MJZ_CX_FN const char *raw_sso_buffer_location_ptr() const noexcept {
    return mut_data.sso_raw_buffer_and_cntrl;
  }
  MJZ_CX_FN char *raw_sso_buffer_location_ptr() noexcept {
    return mut_data.sso_raw_buffer_and_cntrl;
  }
  MJZ_CX_FN char *sso_buffer_location_ptr() noexcept {
    return std::launder(raw_sso_buffer_location_ptr() +
                        mut_data.sso_buf_index_);
  }
  MJZ_CX_FN const char *sso_buffer_location_ptr() const noexcept {
    return std::launder(raw_sso_buffer_location_ptr() +
                        mut_data.sso_buf_index_);
  }
  MJZ_CX_FN const char *dead_sso_buffer_location_ptr() const noexcept {
    return raw_sso_buffer_location_ptr() + mut_data.sso_buf_index_;
  }
  MJZ_CX_FN const char *sso_cntrl_ptr() const noexcept {
    return raw_sso_buffer_location_ptr() + mut_data.sso_cntrl_index_;
  }
  MJZ_CX_FN char *sso_cntrl_ptr() noexcept {
    return raw_sso_buffer_location_ptr() + mut_data.sso_cntrl_index_;
  }
  MJZ_CONSTANT(uintlen_t)
  cntrl_in_cap_begin = mjz::get_begin_bit_index(mut_data_t::buffer_cntrl_mask_);
  MJZ_CX_FN uint8_t get_cntrl(bool is_sso_) const noexcept {
    MJZ_IFN_CONSTEVAL {
      return *reinterpret_cast<const uint8_t *>(sso_cntrl_ptr());
    }
    if (is_sso_) {
      return std::bit_cast<uint8_t>(*sso_cntrl_ptr());
    } else {
      auto &ref{mut_data.non_sso.buffer_capacity_and_cntrl};
      uintlen_t same_val{ref};
      same_val >>= cntrl_in_cap_begin;
      asserts(asserts.assume_rn, [&]() noexcept {
        uint8_t val = std::bit_cast<uint8_t>(
            std::bit_cast<std::array<char, sizeof(uintlen_t)>>(
                ref)[mut_data.buffer_cntrl_index_]);
        return same_val == uintlen_t(val);
      });
      return uint8_t(same_val);
    }
  }
  MJZ_CX_FN void set_cntrl(uint8_t val, bool is_sso_) noexcept {
    MJZ_IFN_CONSTEVAL {
      *reinterpret_cast<uint8_t *>(sso_cntrl_ptr()) = val;
      return;
    }
    if (is_sso_) {
      *sso_cntrl_ptr() = std::bit_cast<char>(val);
      return;
    }
    auto &ref{mut_data.non_sso.buffer_capacity_and_cntrl};
    uintlen_t same_val{ref};
    same_val &= mut_data.buffer_cap_mask_;
    same_val |= uintlen_t(val) << cntrl_in_cap_begin;
    asserts(asserts.assume_rn, [&]() noexcept {
      auto a = std::bit_cast<std::array<char, sizeof(uintlen_t)>>(ref);
      a[mut_data.buffer_cntrl_index_] = std::bit_cast<char>(val);
      return same_val == std::bit_cast<uintlen_t>(a);
    });
    ref = same_val;
    return;
  }
  MJZ_CX_FN void d_set_cntrl(bool is_sso_, const uint8_t x,
                             auto value) noexcept {
    uint8_t cntrl = get_cntrl(is_sso_);
    cntrl &= ~x;

    cntrl |= (static_cast<uint8_t>(value) << mjz::get_begin_bit_index(x));
    return set_cntrl(cntrl, is_sso_);
  }
  template <typename T>
  MJZ_CX_FN std::remove_cvref_t<T> d_get_cntrl(bool is_sso_,
                                               const uint8_t x) const noexcept {
    return static_cast<std::remove_cvref_t<T>>((x & get_cntrl(is_sso_)) >>
                                               mjz::get_begin_bit_index(x));
  }
  MJZ_CX_FN bool is_sso() const noexcept {
    bool B = begin == dead_sso_buffer_location_ptr();
    MJZ_IFN_CONSTEVAL {
      uint8_t cntrl{*reinterpret_cast<const uint8_t *>(sso_cntrl_ptr())};
      asserts(asserts.assume_rn, !(B && (cntrl & my_details::is_sharable)));
    }
    else {
      asserts(asserts.assume_rn,
              !(B && d_get_cntrl<bool>(B, my_details::is_sharable)));
    }

    return B;
  }
  MJZ_CX_FN void d_set_cntrl(const uint8_t x, auto value) noexcept {
    return d_set_cntrl(is_sso(), x, value);
  }
  template <typename T>
  MJZ_CX_FN std::remove_cvref_t<T> d_get_cntrl(const uint8_t x) const noexcept {
    return d_get_cntrl<T>(is_sso(), x);
  }

  MJZ_CX_FN uint8_t get_cntrl() const noexcept { return get_cntrl(is_sso()); }
  MJZ_CX_FN void set_cntrl(uint8_t val) noexcept {
    return set_cntrl(val, is_sso());
  }
  MJZ_CX_FN void keep_flags_construct_sso_from_invalid_fast() noexcept {
    char cntrl_old = char(get_cntrl());
    MJZ_IF_CONSTEVAL {
      mut_data.sso_raw_buffer_and_cntrl[0] = 0;
      memset(raw_sso_buffer_location_ptr(),
             sizeof(mut_data.sso_raw_buffer_and_cntrl), '\0');
    }
    std::construct_at(sso_cntrl_ptr(),
                      char((cntrl_old & (my_details::as_not_threaded_bit |
                                         my_details::encodings_bits))));
    begin = sso_buffer_location_ptr();
    length = 0;
  }
  MJZ_CX_FN void construct_sso_from_invalid_fast(bool keep_flags) noexcept {
    if (keep_flags) {
      keep_flags_construct_sso_from_invalid_fast();
    } else {
      fast_uninitilized_constructor_to_empty_sso();
    }
  }
  MJZ_CX_FN void construct_sso_from_invalid(bool keep_flags) noexcept {
    char cntrl_old = char(get_cntrl());
    MJZ_IF_CONSTEVAL {
      mut_data.sso_raw_buffer_and_cntrl[0] = 0;
      memset(raw_sso_buffer_location_ptr(),
             sizeof(mut_data.sso_raw_buffer_and_cntrl), '\0');
    }

    if (!keep_flags) {
      std::construct_at(sso_cntrl_ptr(), char(0));
      d_set_cntrl(true, my_details::as_not_threaded_bit, false);
      d_set_cntrl(true, my_details::encodings_bits, encodings_e::ascii);
    } else {
      std::construct_at(sso_cntrl_ptr(),
                        char(cntrl_old & (my_details::as_not_threaded_bit |
                                          my_details::encodings_bits)));
    }
    begin = sso_buffer_location_ptr();
    length = 0;
    d_set_cntrl(true, my_details::is_sharable, false);
    *std::launder(sso_buffer_location_ptr()) = '\0';
    d_set_cntrl(true, my_details::has_null, true);
  }
  MJZ_CX_FN success_t construct_sso_from_invalid(const char *begin_,
                                                 uintlen_t length_,
                                                 bool keep_flags) noexcept {
    construct_sso_from_invalid(keep_flags);
    if (mut_data.sso_cap < length_) return false;
    memmove(std::launder(sso_buffer_location_ptr()), begin_, length_);
    length = length_;
    if (length_ < mut_data.sso_cap) {
      sso_buffer_location_ptr()[length_] = '\0';
      d_set_cntrl(true, my_details::has_null, true);
    } else {
      d_set_cntrl(true, my_details::has_null, false);
    }
    return true;
  }
  MJZ_CX_FN success_t construct_sso_from_invalid(const char *begin_,
                                                 uintlen_t length_,
                                                 bool keep_flags,
                                                 alloc_ref &&Alloc_) noexcept {
    if (!construct_sso_from_invalid(begin_, length_, keep_flags)) return false;
    if (alloc_ref *aptr = get_alloc_ptr()) {
      *aptr = std::move(Alloc_);
    }
    return true;
  }
  MJZ_CX_FN void construct_non_sso_from_invalid_fast_no_keep() noexcept {
    std::construct_at(&mut_data.non_sso);
    begin = "";
    length = 0;
    set_cntrl(my_details::has_null | my_details::is_sharable, false);
  }
  MJZ_CX_FN void construct_non_sso_from_invalid_fast_keep() noexcept {
    auto oldcntrl = get_cntrl();
    std::construct_at(&mut_data.non_sso);
    begin = "";
    length = 0;
    set_cntrl(
        uint8_t(my_details::has_null | my_details::is_sharable |
                uint8_t(uint8_t(oldcntrl) & (my_details::as_not_threaded_bit |
                                             my_details::encodings_bits))),
        false);
  }
  MJZ_CX_FN void construct_non_sso_from_invalid_fast(bool keep_flags) noexcept {
    if (keep_flags) {
      construct_non_sso_from_invalid_fast_keep();
    } else {
      construct_non_sso_from_invalid_fast_no_keep();
    }
  }
  MJZ_CX_FN void construct_non_sso_from_invalid(bool keep_flags) noexcept {
    MJZ_RELEASE { asserts(asserts.assume_rn, !is_sso()); };
    auto cntrl = keep_flags ? get_cntrl() : uint8_t(0);
    std::construct_at(&mut_data.non_sso);
    begin = "";
    d_set_cntrl(false, uint8_t(-1), keep_flags ? cntrl : 0);
    if (!keep_flags) {
      d_set_cntrl(false, my_details::as_not_threaded_bit, false);
      d_set_cntrl(false, my_details::encodings_bits, encodings_e::ascii);
    }
    d_set_cntrl(false, my_details::is_sharable, true);
    d_set_cntrl(false, my_details::has_null, true);
  }
  MJZ_CX_FN uintlen_t get_non_sso_capacity() const noexcept {
    return mut_data.non_sso.buffer_capacity_and_cntrl &
           mut_data.buffer_cap_mask_;
  }
  MJZ_CX_FN uintlen_t get_capacity() const noexcept {
    return is_sso() ? mut_data.sso_cap : get_non_sso_capacity();
  }
  MJZ_CX_FN const m_t &as_const() const noexcept { return *this; }
  MJZ_CX_FN const char *buffer_location_ptr() const noexcept {
    return (is_sso() ? sso_buffer_location_ptr()
                     : non_sso_buffer_location_ptr());
  }
  MJZ_CX_FN const char *cbuffer_location_ptr() const noexcept {
    return (is_sso() ? sso_buffer_location_ptr()
                     : non_sso_buffer_location_ptr());
  }
  MJZ_CX_FN char *buffer_location_ptr() noexcept {
    asserts(asserts.assume_rn, [&]() noexcept { return is_owner(); });
    return is_sso() ? sso_buffer_location_ptr() : non_sso_buffer_location_ptr();
  }
  MJZ_CX_FN char *mut_begin() noexcept {
    asserts(asserts.assume_rn, !!as_const().buffer_location_ptr());
    asserts(asserts.assume_rn, [&]() noexcept { return is_owner(); });
    return buffer_location_ptr() + (begin - buffer_location_ptr());
  }
  MJZ_CX_FN bool can_add_null(bool check_ownership = true) const noexcept {
    return buffer_location_ptr() &&
           (begin + length < buffer_location_ptr() + get_capacity()) &&
           (!check_ownership || is_owner());
  }
  MJZ_CX_FN success_t add_null(bool check_ownership = true) noexcept {
    if (can_add_null(check_ownership)) {
      mut_begin()[length] = '\0';
      d_set_cntrl(my_details::has_null, true);
      return true;
    }
    d_set_cntrl(my_details::has_null, false);
    return false;
  }
  MJZ_CX_FN bool is_heap() const noexcept {
    return d_get_cntrl<bool>(my_details::is_sharable) &&
           non_sso_buffer_location_ptr();
  }
  MJZ_CX_FN bool is_s_view() const noexcept {
    return d_get_cntrl<bool>(my_details::is_sharable) &&
           !non_sso_buffer_location_ptr();
  }
  MJZ_CX_FN void set_non_sso_capacity(uintlen_t value) noexcept {
    auto &ref{mut_data.non_sso.buffer_capacity_and_cntrl};
    asserts(asserts.assume_rn, value == (value & mut_data.buffer_cap_mask_));
    ref &= mut_data.buffer_cntrl_mask_;
    ref |= value;
  }
  MJZ_CX_FN static success_t check_buffer_correct_ness_(
      const char *begin, uintlen_t length, char *buffer_begin,
      uintlen_t buffer_capacity) noexcept {
    return (buffer_capacity < mut_data_t::buffer_cap_max_) &&
           (length < mut_data_t::buffer_cap_max_) &&
           (buffer_begin || !buffer_capacity) && (begin || !length) &&
           (!begin || !buffer_begin ||
            (buffer_begin <= begin &&
             begin + length <= buffer_begin + buffer_capacity &&
             length <= buffer_capacity));
  }
  MJZ_CX_FN success_t construct_non_sso_from_invalid(
      const char *begin_, uintlen_t length_, char *buffer_begin_,
      uintlen_t capacity_, bool is_shared_, bool keep_flags,
      alloc_ref &&Alloc_) noexcept {
    if ((Alloc_ && !has_alloc_v_) ||
        !construct_non_sso_from_invalid(begin_, length_, buffer_begin_,
                                        capacity_, is_shared_, keep_flags))
      return false;
    if (alloc_ref *aptr = get_alloc_ptr()) {
      *aptr = std::move(Alloc_);
    }
    return true;
  }
  MJZ_CX_FN success_t construct_non_sso_from_invalid(
      const char *begin_, uintlen_t length_, char *buffer_begin_,
      uintlen_t capacity_, bool is_shared_, bool keep_flags) noexcept {
    if (!check_buffer_correct_ness_(begin_, length_, buffer_begin_, capacity_))
      return false;
    construct_non_sso_from_invalid(keep_flags);

    begin = begin_;
    length = length_;
    mut_data.non_sso.buffer_begin = buffer_begin_;
    set_non_sso_capacity(capacity_);
    d_set_cntrl(false, my_details::is_sharable, is_shared_);
    return true;
  }
  MJZ_CX_FN void fast_uninitilized_constructor_to_empty_sso() noexcept {
    MJZ_IF_CONSTEVAL {
      mut_data.sso_raw_buffer_and_cntrl[0] = 0;
      memset(raw_sso_buffer_location_ptr(),
             sizeof(mut_data.sso_raw_buffer_and_cntrl), '\0');
    }
    std::construct_at(sso_cntrl_ptr(), char());
    begin = sso_buffer_location_ptr();
    length = 0;
  }

  MJZ_CX_FN success_t deconstruct_sso_to_invalid() noexcept { return true; }

  MJZ_CX_FN success_t deallocate_non_sso() noexcept {
    asserts(asserts.assume_rn, !is_sso());
    if (!mut_data.non_sso.buffer_begin ||
        !d_get_cntrl<bool>(my_details::is_sharable)) {
      construct_non_sso_from_invalid_fast_keep();
      return true;
    }
    MJZ_RELEASE { construct_non_sso_from_invalid_fast_keep(); };

    return non_sso_my_heap_manager_no_own().free();
  }
  MJZ_CX_FN str_heap_manager non_sso_my_heap_manager_no_own() const noexcept {
    return str_heap_manager(
        get_alloc(), !d_get_cntrl<bool>(my_details::as_not_threaded_bit), false,
        false, mut_data.non_sso.buffer_begin, get_non_sso_capacity());
  }
  MJZ_CX_FN str_heap_manager new_heap_manager() const noexcept {
    return str_heap_manager(
        get_alloc(), !d_get_cntrl<bool>(my_details::as_not_threaded_bit));
  }

  MJZ_CX_FN success_t deconstruct_non_sso_to_invalid() noexcept {
    if (!deallocate_non_sso()) return false;
    return true;
  }

  MJZ_CX_FN success_t deconstruct_to_invalid() noexcept {
    return is_sso() ? deconstruct_sso_to_invalid()
                    : deconstruct_non_sso_to_invalid();
  }
  MJZ_CX_FN bool is_owner() const noexcept {
    if (d_get_cntrl<bool>(my_details::is_ownerized)) {
      std::ignore = is_sso();  // debug assersions in it
      return true;
    }
    if (is_sso()) return true;
    if (!mut_data.non_sso.buffer_begin) return false;
    if (!d_get_cntrl<bool>(my_details::is_sharable)) {
      return true;
    }
    return non_sso_my_heap_manager_no_own().is_owner();
  };

  struct cntrl_state_t {
    bool is_ownerized;
    bool is_sharable;
    bool has_null;
    bool as_not_threaded_bit;
    encodings_e encodings_bits;
  };
  MJZ_CX_FN cntrl_state_t cntrl_states() const noexcept {
    return cntrl_state_t{
        d_get_cntrl<bool>(my_details::is_ownerized),
        d_get_cntrl<bool>(my_details::is_sharable),
        d_get_cntrl<bool>(my_details::has_null),
        d_get_cntrl<bool>(my_details::as_not_threaded_bit),
        d_get_cntrl<encodings_e>(my_details::encodings_bits),
    };
  }
  /*
   * says m_t doesnt initilize the values at constrution
   */
  MJZ_DISABLE_ALL_WANINGS_START_;
};
MJZ_DISABLE_ALL_WANINGS_END_;

};  // namespace basic_str_abi_ns_

template <typename T>
MJZ_CX_ND_FN static std::optional<T> get_as_option(T val) noexcept {
  return val;
}
template <typename T>
MJZ_CX_ND_FN static std::optional<T> get_as_option(
    std::optional<T> val) noexcept {
  return val;
}

template <class begin_iter_t, class end_iter_t>
concept char_iter_pair_c =
    requires(begin_iter_t &&begin_iter, end_iter_t &&end_iter) {
      {
        get_as_option<char>(*begin_iter)
      } noexcept -> std::same_as<std::optional<char>>;
      { ++begin_iter } noexcept;
      { begin_iter != end_iter } noexcept -> std::same_as<bool>;
    };

template <class begin_iter_t, class end_iter_t>
concept sizeed_char_iter_pair_c = requires(begin_iter_t &&begin_iter,
                                           end_iter_t &&end_iter) {
  requires char_iter_pair_c<begin_iter_t, end_iter_t>;

  { end_iter - begin_iter } noexcept;
  requires requires(
      std::remove_reference_t<decltype(end_iter - begin_iter)> v) {
    { get_as_option(v) } noexcept;
    requires requires(std::remove_reference_t<decltype(get_as_option(v))> v2) {
      { *v2 } noexcept;
      requires requires(std::remove_cvref_t<decltype(*v2)> v3) {
        requires std::integral<std::remove_cvref_t<decltype(v3)>>;
      };
    };
  };
};
};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_abi_LIB_HPP_FILE_