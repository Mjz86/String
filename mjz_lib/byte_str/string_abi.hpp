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
#ifndef MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_

namespace mjz::bstr_ns {
enum class align_direction_e : char {
  relaxed = 0b00,
  center = 0b01,
  front = 0b10,
  back = 0b11
};

template <version_t version_v>
struct when_t_impl_ {
  MJZ_DEFAULTED_CLASS(when_t_impl_);
  enum val_e : char { relax, own_relax, no_heap, as_sso };
  val_e val{};
  MJZ_CX_FN when_t_impl_(val_e v) : val{v} {};
  MJZ_CX_FN when_t_impl_(auto) = delete;
  MJZ_CX_FN
  bool is_sso() const noexcept { return val == as_sso; }
  MJZ_CX_FN
  bool no_destroy() const noexcept { return val != relax && val != own_relax; }
  MJZ_CX_FN
  bool do_destroy() const noexcept { return !no_destroy(); }

  MJZ_CX_FN explicit operator bool() const noexcept { return do_destroy(); }
};

template <version_t version_v, bool has_alloc_v_, bool has_null_v_,
          bool is_ownerized_v_, may_bool_t is_threaded_v_,
          uintlen_t user_sso_selected_cap, align_direction_e align_direction_v_>
struct str_abi_t_ {
  static_assert(is_threaded_v_ != may_bool_t::err);
  /// <summary>
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  MJZ_STATIC_CLASS(str_abi_t_);
  /// </summary>
  MJZ_MCONSTANT(uintlen_t) min_sso_sicap = 4 * sizeof(uintlen_t) - 2;
  MJZ_MCONSTANT(uintlen_t)
  old_sso_cap_helper_ = std::max(user_sso_selected_cap, min_sso_sicap);
  MJZ_MCONSTANT(uintlen_t)
  old_sso_cap =
      std::same_as<
          uint_with_max_of_t<(sizeof(uintlen_t) + old_sso_cap_helper_ +
                              sizeof(uint_with_max_of_t<old_sso_cap_helper_>))>,
          uint_with_max_of_t<old_sso_cap_helper_>>
          ? old_sso_cap_helper_
          : old_sso_cap_helper_ +
                sizeof(uint_with_max_of_t<old_sso_cap_helper_>) +
                sizeof(uintlen_t);
  MJZ_MCONSTANT(uintlen_t) cntrl_size = sizeof(uint8_t);
  using cap_mins_length_t = uint_with_max_of_t<old_sso_cap>;
  MJZ_MCONSTANT(uintlen_t)
  min_buffer_size = old_sso_cap + sizeof(cap_mins_length_t);
  MJZ_MCONSTANT(uintlen_t)
  buffer_size =
      sizeof(uintlen_t) *
          (((min_buffer_size + cntrl_size) / sizeof(uintlen_t)) +
           uintlen_t(!!((min_buffer_size + cntrl_size) % sizeof(uintlen_t)))) -
      cntrl_size;
  struct nsso_mdata_t {
    // same as is_heap ()
    uint8_t no_destroy : 1 {1};
    // !(should_destroy&&!heap.might_share())||is_sharable
    uint8_t is_sharable : 1 {};
    uint8_t has_mut : 1 {};
    // !has_null|| data()[len]==0
    uint8_t has_null : 1 {};
    uint8_t str_align_direction : 2 {uint8_t(align_direction_v_)};
    uint8_t reserved_0_ : 2 {};

    // shallow bit cpy , share , deep cpy, deep cpy to sso

    // curruntly  this abi&api combo is not yet implemented correctly and stil
    // uses the old ways of variable use .

    MJZ_CX_FN bool operator==(const nsso_mdata_t&) const noexcept = default;
  };
  struct control_byte_t {
    uint8_t is_sso : 1 {};
    uint8_t is_ownerized : 1 {is_ownerized_v_};
    uint8_t as_not_threaded_bit : 1 {!bool(char(is_threaded_v_))};
    uint8_t encodings_bits : 5 {};
    MJZ_CX_FN bool operator==(const control_byte_t&) const noexcept = default;
  };

  MJZ_PACKING_START_;
  static_assert(sizeof(nsso_mdata_t) == sizeof(uint8_t));
  constexpr static uintlen_t raw_lencap_size_v =
      buffer_size - (sizeof(char*) * 2 + sizeof(nsso_mdata_t));
  struct non_sso_t {
    const char* begin_ptr{};
    char* data_block{};
    char raw_lencap[raw_lencap_size_v]{};
    nsso_mdata_t nsso_mdata{};
  };
  // sorry 16 bit ptrs
  static_assert((sizeof(char*) * 2) % sizeof(uintlen_t) == 0);
  union alignas(1) raw_data_u {
    non_sso_t non_sso{};
    char sso_buffer[buffer_size];
  };
  static_assert(sizeof(raw_data_u) == sizeof(non_sso_t));

  static constexpr size_t raw_length_offset = 0;
  static constexpr size_t raw_capacity_offset =
      raw_lencap_size_v < sizeof(uintlen_t) * 2 ? sizeof(uintlen_t) - 1
                                                : sizeof(uintlen_t);
  static constexpr bool is_raw_capacity_aliged =
      !(raw_capacity_offset % sizeof(uintlen_t));
  static constexpr bool are_raw_feilds_aliged = is_raw_capacity_aliged;

  struct raw_data_t {
    static_assert(sizeof(control_byte_t) == sizeof(uint8_t));
    raw_data_u raw_data{};
    control_byte_t control_byte{};
  };
  MJZ_PACKING_END_;
  static_assert(sizeof(raw_data_t) == sizeof(raw_data_u) + sizeof(uint8_t) &&
                sizeof(raw_data_u) == buffer_size);
  static_assert((sizeof(raw_data_t) % alignof(uintlen_t)) == 0);

  struct alignas(2) str_data0_t {
    nsso_mdata_t first{};
    control_byte_t second{};
  };
  static_assert(sizeof(str_data0_t) == 2);
  struct str_data_t : nsso_mdata_t, control_byte_t {
    MJZ_CX_FN str_data_t(str_data0_t d) noexcept
        : nsso_mdata_t{d.first}, control_byte_t{d.second} {}
    MJZ_CX_FN bool operator==(const str_data_t&) const noexcept = default;
  };

  struct data_t : abi_ns_::alloc_t<version_v, has_alloc_v_> {
   public:
    MJZ_MCONSTANT(uintlen_t) sso_cap = buffer_size - sizeof(cap_mins_length_t);
    constexpr static const alloc_ref& empty_alloc{
        allocs_ns::empty_alloc<version_v>};

    using when_t = when_t_impl_<version_v>;

   public:
    alignas(alignof(uintlen_t)) raw_data_t m{};

   public:
    template <class = void>
      requires has_alloc_v_
    MJZ_CX_AL_FN const alloc_ref* get_alloc_ptr() const noexcept

    {
      return &this->alloc_;
    }
    template <class = void>
      requires(!has_alloc_v_)
    MJZ_CX_AL_FN const alloc_ref* get_alloc_ptr() const noexcept {
      return {};
    }
    MJZ_CX_AL_FN const alloc_ref& get_alloc() const noexcept {
      if constexpr (has_alloc_v_) {
        return *get_alloc_ptr();
      } else {
        return empty_alloc;
      }
    }
    template <class = void>
      requires has_alloc_v_
    MJZ_CX_AL_FN alloc_ref* get_alloc_ptr() noexcept {
      return &this->alloc_;
    }
    template <class = void>
      requires(!has_alloc_v_)
    MJZ_CX_AL_FN alloc_ref* get_alloc_ptr() noexcept {
      return {};
    }
    MJZ_CX_AL_FN static bool has_alloc_v() noexcept { return has_alloc_v_; }
    MJZ_CX_AL_FN const raw_data_t& m_v() const noexcept {
      return *mjz::assume_aligned<alignof(uintlen_t)>(&m);
    }
    MJZ_CX_AL_FN raw_data_t& m_v() noexcept {
      return *mjz::assume_aligned<alignof(uintlen_t)>(&m);
    }
    MJZ_CX_AL_FN non_sso_t& non_sso() noexcept {
      return m_v().raw_data.non_sso;
    }
    MJZ_CX_AL_FN const non_sso_t& non_sso() const noexcept {
      return m_v().raw_data.non_sso;
    }

    MJZ_CX_AL_FN bool is_sso() const noexcept { return cntrl_u8().is_sso; }

    MJZ_CX_AL_FN bool is_ownerized() const noexcept {
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, !!cntrl_u8().is_ownerized);
        return true;
      }
      return !!cntrl_u8().is_ownerized;
    }
    MJZ_CX_AL_FN bool is_threaded() const noexcept {
      if constexpr (is_threaded_v_ != may_bool_t::idk) {
        asserts(asserts.assume_rn,
                bool(char(is_threaded_v_)) == !cntrl_u8().as_not_threaded_bit);
        return bool(char(is_threaded_v_));
      }
      return !cntrl_u8().as_not_threaded_bit;
    }
    MJZ_CX_AL_FN void set_threaded(bool var) noexcept {
      if constexpr (is_threaded_v_ != may_bool_t::idk) {
        return;
      } else {
        cntrl_u8().as_not_threaded_bit = !var;
      }
    }
    MJZ_CX_AL_FN void set_ownerized(bool var) noexcept {
      if constexpr (is_ownerized_v_) {
        return;
      } else {
        cntrl_u8().is_ownerized = var;
      }
    }
    MJZ_CX_AL_FN static auto m_sso_buffer_(
        partial_same_as<data_t> auto&& self_) noexcept {
      const auto This_ =
          std::launder(mjz::assume_aligned<alignof(uintlen_t)>(&self_));
      const auto ptr_ = This_->m_v().raw_data.sso_buffer;
      return mjz::assume_aligned<alignof(uintlen_t)>(ptr_);
    }

    MJZ_CX_AL_FN char* m_sso_buffer_() noexcept { return m_sso_buffer_(*this); }
    MJZ_CX_AL_FN const char* m_sso_buffer_() const noexcept {
      return m_sso_buffer_(*this);
    }
    MJZ_CX_AL_FN uintlen_t get_cap_minus_sso_length() const noexcept {
      return cpy_bitcast<cap_mins_length_t>(m_sso_buffer_() + sso_cap);
    }
    MJZ_CX_AL_FN const nsso_mdata_t& nsso_mdata_() const noexcept {
      return m_v().raw_data.non_sso.nsso_mdata;
    }
    MJZ_CX_AL_FN nsso_mdata_t& nsso_mdata_() noexcept {
      return m_v().raw_data.non_sso.nsso_mdata;
    }

    MJZ_CX_AL_FN const control_byte_t& cntrl_u8() const noexcept {
      return m_v().control_byte;
    }
    MJZ_CX_AL_FN control_byte_t& cntrl_u8() noexcept {
      return m_v().control_byte;
    }
    MJZ_DISABLE_ALL_WANINGS_START_;
    MJZ_CX_AL_FN const str_data_t cntrl() const noexcept {
      MJZ_DISABLE_ALL_WANINGS_END_;
      MJZ_IF_CONSTEVAL {
        return str_data0_t{is_sso() ? nsso_mdata_t{} : nsso_mdata_(),
                           cntrl_u8()};
      }
      return std::bit_cast<str_data0_t>(cpy_aligned_bitcast<uint16_t>(
          reinterpret_cast<const char*>(&m) + (sizeof(m) - 2)));
    }

    MJZ_CX_AL_FN void set_cap_minus_sso_length(uintlen_t new_val_) noexcept {
      const cap_mins_length_t new_val{cap_mins_length_t(new_val_)};
      asserts(asserts.assume_rn, new_val == new_val_);
      cpy_bitcast(m_sso_buffer_() + sso_cap, new_val);
      return;
    }

    MJZ_CX_AL_FN uintlen_t get_sso_length() const noexcept {
      return sso_cap - get_cap_minus_sso_length();
    }
    MJZ_CX_AL_FN void set_sso_length(uintlen_t new_len_) noexcept {
      set_cap_minus_sso_length(sso_cap - new_len_);
      asserts(asserts.assume_rn, get_sso_length() == new_len_);
      m_sso_buffer_()[new_len_] = '\0';
    }

    MJZ_CX_AL_FN uintlen_t get_length() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (!is_sso()) {
          return u_nsso_len();
        }
        return get_sso_length();
      } /* UB? na , just branchless */
      return branchless_teranary(!is_sso(), uu_nsso_len(), get_sso_length());
    }

    MJZ_CX_AL_FN encodings_e get_encoding() const noexcept {
      return encodings_e(cntrl_u8().encodings_bits);
    }
    MJZ_CX_AL_FN const char* get_begin() const noexcept {
      MJZ_IF_CONSTEVAL_ {
        if (!is_sso()) {
          return m_v().raw_data.non_sso.begin_ptr;
        }
        return m_sso_buffer_();
      }
      /* UB? na , just branchless */
      return branchless_teranary(
          !is_sso(),
          cpy_aligned_bitcast<const char*>(
              reinterpret_cast<const char*>(&m_v().raw_data.non_sso) +
              offsetof(non_sso_t, begin_ptr)),
          m_sso_buffer_());
    }
    MJZ_CX_AL_FN void check_buffer_correct_ness_(
        const char* begin, uintlen_t length, char* buffer_begin,
        uintlen_t buffer_capacity) const noexcept {
      asserts(asserts.assume_rn,
              buffer_capacity <= byte_traits_t<version_v>::npos - 1 &&
                  length <= byte_traits_t<version_v>::npos - 1 &&
                  (buffer_begin || !buffer_capacity) && (begin || !length) &&
                  (!begin || !buffer_begin ||
                   (buffer_begin <= begin &&
                    begin + length <= buffer_begin + buffer_capacity &&
                    length <= buffer_capacity)));
      MJZ_IF_CONSTEVAL return;
      asserts(asserts.assume_rn,
              !memory_has_overlap_ncx(this, sizeof(*this), buffer_begin,
                                      buffer_capacity));
      asserts(asserts.assume_rn,
              !memory_has_overlap_ncx(this, sizeof(*this), begin, length));
    }
    template <bool is_aligned>
    MJZ_CX_AL_FN static void u_headless_uintlen_set(char* ptr_0_,
                                                    uintlen_t val) noexcept {
      asserts(asserts.assume_rn, val <= byte_traits_t<version_v>::npos - 1);
      if constexpr (are_raw_feilds_aliged) {
        return cpy_aligned_bitcast(ptr_0_, val);
      }
      char* ptr_ = ptr_0_;
      alignas(alignof(uintlen_t)) char buf_[sizeof(uintlen_t)]{};
      if constexpr (!is_aligned) {
        memcpy(buf_, ptr_, sizeof(uintlen_t) - 1);
        ptr_ = buf_;
      }
      uintlen_t impure_val = cpy_aligned_bitcast<uintlen_t>(ptr_);
      if constexpr (version_v.is_BE()) {
        impure_val &= ~(uintlen_t(-1) << 8);
        impure_val |= val << 8;
      } else {
        impure_val &= ~(uintlen_t(-1) >> 8);
        impure_val |= val;
      }

      cpy_aligned_bitcast(ptr_, impure_val);
      if constexpr (!is_aligned) {
        memcpy(ptr_0_, buf_, sizeof(uintlen_t) - 1);
      }
    }
    template <bool is_aligned>
    MJZ_CX_AL_FN static uintlen_t u_headless_uintlen_get(
        const char* ptr_) noexcept {
      uintlen_t ret{};
      if constexpr (!are_raw_feilds_aliged) {
        alignas(alignof(uintlen_t)) char buf_[sizeof(uintlen_t)]{};
        if constexpr (!is_aligned) {
          memcpy(buf_, ptr_, sizeof(uintlen_t) - 1);
          ptr_ = buf_;
        }
        ret = cpy_aligned_bitcast<uintlen_t>(ptr_);
        if constexpr (version_v.is_BE()) {
          ret >>= 8;
        } else {
          ret &= uintlen_t(-1) >> 8;
        }
      } else {
        ret = cpy_aligned_bitcast<uintlen_t>(ptr_);
      }
      asserts(asserts.assume_rn, ret <= byte_traits_t<version_v>::npos - 1);
      return ret;
    }

    MJZ_CX_AL_FN void set_invalid_to_non_sso_begin(
        const char* begin_, uintlen_t length_, char* buffer_begin_,
        uintlen_t capacity_, bool is_shared_, bool has_null_,
        bool should_destroy_) noexcept {
      asserts(asserts.assume_rn, !should_destroy_ || buffer_begin_);

      asserts(asserts.assume_rn, !has_null_ || begin_);

      asserts(asserts.assume_rn,
              !is_shared_ || should_destroy_ || !buffer_begin_);

      check_buffer_correct_ness_(begin_, length_, buffer_begin_, capacity_);

      asserts(asserts.assume_rn, !should_destroy_ || buffer_begin_);

      non_sso_t& data = *std::construct_at(&m_v().raw_data.non_sso);
      cntrl_u8().is_sso = false;
      data.begin_ptr = begin_;
      data.data_block = buffer_begin_;
      u_nsso_lenas(length_);
      u_nsso_nullas(has_null_);
      data.nsso_mdata.is_sharable = is_shared_;
      data.nsso_mdata.no_destroy = !should_destroy_;
      data.nsso_mdata.has_mut = !!buffer_begin_;

      u_headless_uintlen_set<is_raw_capacity_aliged>(
          const_cast<char*>(uu_nsso_capp()), capacity_);
    }
    template <when_t when_v>
    MJZ_CX_AL_FN void memcpy_to_non_sso(const char* ptr, uintlen_t len,
                                        char* buffer_begin_,
                                        uintlen_t capacity_, bool is_shared_,
                                        bool should_destroy_) noexcept {
      if constexpr (when_v.no_destroy()) {
        asserts(asserts.assume_rn, no_destroy());
      }

      check_buffer_correct_ness_(ptr, len, nullptr, 0);

      check_buffer_correct_ness_(buffer_begin_, capacity_, nullptr, 0);

      uintlen_t cap = capacity_;
      uintlen_t offset = s_buffer_offset(cap, len);
      char* buf = buffer_begin_;
      if (!ptr) {
        asserts(asserts.assume_rn, !len);
      }
      if (!buf) {
        asserts(asserts.assume_rn, !!buf);
      }
      char* beg = buf + offset;
      memcpy(beg, ptr, len);
      bool has_null{beg + len < cap + buf};

      asserts(asserts.assume_rn, has_null || !has_null_v_);

      if (has_null) {
        beg[len] = '\0';
      }
      if constexpr (when_v) {
        destruct_to_invalid();
      }

      set_invalid_to_non_sso_begin(beg, len, buf, cap, is_shared_, has_null,
                                   should_destroy_);
    }

    MJZ_CX_AL_FN uintlen_t get_non_sso_capacity() const noexcept {
      return u_headless_uintlen_get<is_raw_capacity_aliged>(uu_nsso_capp());
    }
    MJZ_CX_AL_FN uintlen_t get_capacity() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (is_sso()) {
          return sso_cap;
        }
        return get_non_sso_capacity();
      }

      return branchless_teranary(!is_sso(), get_non_sso_capacity(), sso_cap);
    }

    MJZ_CX_AL_FN void set_invalid_to_sso(const char* non_overlapping_ptr,
                                         uintlen_t len) noexcept {
      asserts(asserts.assume_rn, len <= sso_cap);
      m_v().raw_data.sso_buffer[0] = 0;  // make the buffer alive
      char* buf = m_sso_buffer_();
      MJZ_IF_CONSTEVAL { memset(buf, sso_cap, 0); }
      MJZ_DISABLE_ALL_WANINGS_START_;
      memcpy(buf, non_overlapping_ptr, len)[len] = '\0';
      MJZ_DISABLE_ALL_WANINGS_END_;

      cntrl_u8().is_sso = true;
      set_sso_length(len);
    }

    MJZ_CX_AL_FN void set_length(uintlen_t new_len) noexcept {
      if (!is_sso()) {
        u_nsso_lenas(new_len);
        return;
      }
      set_sso_length(new_len);
    }

    MJZ_CX_AL_FN const char* get_buffer_ptr() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (!is_sso()) {
          return m_v().raw_data.non_sso.data_block;
        }
        return m_sso_buffer_();
      }
      /* UB? na , just branchless */ return branchless_teranary(
          !is_sso(),
          cpy_aligned_bitcast<const char*>(
              reinterpret_cast<const char*>(&m_v().raw_data.non_sso) +
              offsetof(non_sso_t, data_block)),
          m_sso_buffer_());

      ;
    }

    MJZ_CX_AL_FN char* get_buffer_ptr() noexcept {
      // no need to write same stuff
      return const_cast<char*>(std::as_const(*this).get_buffer_ptr());
    }
    MJZ_CX_AL_FN char* get_mut_begin() noexcept {
      return branchless_teranary(!has_mut(), nullptr, u_get_mut_begin());
    }
    MJZ_CX_AL_FN bool has_mut() const noexcept { return !!cntrl().has_mut; }
    MJZ_CX_AL_FN bool has_null() const noexcept {
      str_data_t cnt = cntrl();
      return (cnt.is_sso | cnt.has_null) & 1;
    }
    MJZ_CX_AL_FN bool is_sharable() const noexcept {
      str_data_t cnt = cntrl();
      return (~cnt.is_sso & cnt.is_sharable) & 1;
    }
    MJZ_CX_FN basic_string_view_t<version_v> get_view() const noexcept {
      return base_string_view_t<version_v>::make(
          get_begin(), get_length(), get_encoding(), has_null(), is_s_view());
    }

    using str_heap_manager = str_heap_manager_t<version_v, is_threaded_v_,
                                                is_ownerized_v_, has_alloc_v_>;
    MJZ_CX_AL_FN str_heap_manager non_sso_my_heap_manager_no_own(
        bool destroy_on_exit_ = false) const noexcept {
      asserts(asserts.assume_rn, !no_destroy());
      return str_heap_manager(get_alloc(), cntrl().is_sharable, is_threaded(),
                              is_ownerized(), false, destroy_on_exit_,
                              m_v().raw_data.non_sso.data_block,
                              get_non_sso_capacity());
    }

   private:
    MJZ_MCONSTANT(uintlen_t)
    string_size = sizeof(raw_data_t) +
                  (std::is_empty_v<abi_ns_::alloc_t<version_v, has_alloc_v_>>
                       ? 0
                       : sizeof(uintlen_t));

    MJZ_CX_AL_FN void destruct_to_invalid_impl_big_() noexcept {}
    MJZ_CX_FN void destruct_to_invalid_impl_big() noexcept {
      return destruct_to_invalid_impl_big_();
    }
    MJZ_CX_AL_FN void desturct_heap_impl_al_(str_data_t cnt) noexcept {
      asserts(asserts.assume_rn, cnt == cntrl());
      non_sso_my_heap_manager_no_own().u_must_free();
    }

    MJZ_CX_NL_FN void desturct_heap_impl_nl_(str_data_t cnt) noexcept {
      return desturct_heap_impl_al_(cnt);
    }
    MJZ_CX_FN void desturct_heap_impl_(str_data_t cnt) noexcept {
      return desturct_heap_impl_al_(cnt);
    }

   public:
    MJZ_CX_FN void destruct_heap_nl_() noexcept {
      desturct_heap_impl_(cntrl());
    }
    MJZ_CX_AL_FN void destruct_heap() noexcept { desturct_heap_impl_(cntrl()); }
    MJZ_CX_AL_FN bool no_destroy() const noexcept { return !is_heap(); }
    MJZ_CX_FN void destruct_all() noexcept {
      if (!is_heap()) {
        return invalid_to_empty();
      }
      destruct_heap();
      invalid_to_empty();
    }
    MJZ_CX_AL_FN void destruct_to_invalid() noexcept {
      if (is_heap()) {
        return destruct_heap();
      }
      return;
    }
    MJZ_CX_FN void invalid_to_empty() noexcept { set_invalid_to_sso("", 0); }

    MJZ_CX_AL_FN static uintlen_t s_buffer_offset(
        uintlen_t cap, uintlen_t len,
        align_direction_e align = align_direction_v_,
        bool has_null_ = has_null_v_) noexcept {
      if constexpr (align_direction_v_ != align_direction_e::relaxed) {
        align = align_direction_v_;
      }
      uintlen_t delta = cap - len;
      uintlen_t center = delta >> 1;
      uintlen_t back = std::max(uintlen_t(has_null_), delta) - has_null_;
      uintlen_t front = 0;
      const uintlen_t ret = branchless_teranary(
          !(uint8_t(char(align)) & 2), center,
          branchless_teranary(!(uint8_t(char(align)) & 1), front, back));

      return ret & ~uintlen_t(alignof(uintlen_t) - 1);
    }

   private:
    template <when_t when_v>
    MJZ_CX_AL_FN bool is_heap_owner() const noexcept {
      if constexpr (when_v.no_destroy()) {
        asserts(asserts.assume_rn, false);
      }

      asserts(asserts.assume_rn, !cntrl_u8().is_ownerized);
      asserts(asserts.assume_rn, !no_destroy());
      if constexpr (when_v.val == when_t::own_relax) {
        return bool(char(may_bool_t::yes));
      }
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, false);
        return false;
      }

      str_heap_manager hm = non_sso_my_heap_manager_no_own();
      bool is_owner = hm.is_owner();
      // compiler did non optimize free away if this wasnt present
      hm.unsafe_clear();
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, is_owner == true);
      }
      return is_owner;
    }
    MJZ_CX_AL_FN may_bool_t is_owner_no_heap() const noexcept {
      str_data_t cnt = cntrl();
      bool certin_yes =
          (cnt.is_sso | cnt.is_ownerized | (cnt.is_sharable ^ cnt.no_destroy)) &
          1;
      may_bool_t ret = branchless_teranary(
          certin_yes, may_bool_t::yes,
          branchless_teranary(!!cnt.no_destroy, may_bool_t::no,
                              may_bool_t::idk));

      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, ret == may_bool_t::yes);
        return may_bool_t::yes;
      }
      return ret;
    }

   private:
    MJZ_CX_AL_FN const char* uu_nsso_lenp() const noexcept {
      const char* ptr{};
      auto optr = &m_v().raw_data.non_sso;
      MJZ_IF_CONSTEVAL {
        // the feild values should not be relevant in sso mode.
        asserts(!is_sso());
        ptr = optr->raw_lencap;
      }
      else {
        ptr = reinterpret_cast<const char*>(optr) +
              offsetof(non_sso_t, raw_lencap);
      }
      static_assert(raw_length_offset == 0);
      return std::assume_aligned<alignof(uintlen_t)>(ptr + raw_length_offset);
    }
    MJZ_CX_AL_FN const char* uu_nsso_capp() const noexcept {
      const char* ptr{};
      auto optr = &m_v().raw_data.non_sso;
      MJZ_IF_CONSTEVAL {
        // the feild values should not be relevant in sso mode.
        asserts(!is_sso());
        ptr = optr->raw_lencap;
      }
      else {
        ptr = reinterpret_cast<const char*>(optr) +
              offsetof(non_sso_t, raw_lencap);
      }
      return (ptr + raw_capacity_offset);
    }
    MJZ_CX_AL_FN uintlen_t uu_nsso_len() const noexcept {
      return u_headless_uintlen_get<true>(uu_nsso_lenp());
    }
    MJZ_CX_AL_FN void uu_nsso_lenas(uintlen_t new_len) noexcept {
      u_headless_uintlen_set<true>(const_cast<char*>(uu_nsso_lenp()), new_len);
    }

   public:
    MJZ_CX_AL_FN bool is_owner() const noexcept {
      may_bool_t ret = is_owner_no_heap();
      if (may_bool_t::idk == ret) {
        return is_heap_owner<when_t::relax>();
      }
      return bool(char(ret));
    };
    template <when_t when_v>
    MJZ_CX_AL_FN bool has_room_for(uintlen_t new_size,
                                   bool with_null = false) const noexcept {
      if constexpr (when_v.is_sso()) {
        asserts(asserts.assume_rn, is_sso());
        return uintlen_t(with_null) + new_size <= sso_cap;
      }
      may_bool_t ret = is_owner_no_heap();
      bool has_room = uintlen_t(with_null) + new_size <= get_capacity();
      ret = branchless_teranary(!has_room, may_bool_t::no, ret);
      if constexpr (when_v) {
        if (may_bool_t::idk == ret) {
          return is_heap_owner<when_v>();
        }
      }
      return bool(char(ret));
    }
    template <when_t when_v>
    MJZ_CX_AL_FN bool is_the_owner() const noexcept {
      if constexpr (when_v.is_sso()) {
        asserts(asserts.assume_rn, is_sso());
        return true;
      }
      may_bool_t ret = is_owner_no_heap();
      if constexpr (when_v) {
        if (may_bool_t::idk == ret) {
          return is_heap_owner<when_v>();
        }
      }
      return bool(char(ret));
    }

    MJZ_CX_AL_FN char* u_get_mut_begin() noexcept {
      return const_cast<char*>(get_begin());
    }
    template <when_t when_v>
    MJZ_CX_AL_FN bool can_add_null() const noexcept {
      if constexpr (when_v.is_sso()) {
        asserts(asserts.assume_rn, is_sso());
        return true;
      }
      bool is_mut = has_mut();
      bool has_room =
          get_begin() + get_length() < get_buffer_ptr() + get_capacity();

      bool ret = has_room;
      ret &= is_mut;
      may_bool_t is_own = is_owner_no_heap();
      if constexpr (when_v) {
        if (may_bool_t::idk == is_own) {
          is_own = may_bool_t(char(is_heap_owner<when_v>()));
        }
      }
      ret &= bool(char(is_own));
      return ret;
    }

    MJZ_CX_AL_FN void u_nsso_nullas(bool b) noexcept {
      asserts(asserts.assume_rn, !is_sso());
      non_sso().nsso_mdata.has_null = b;
    }
    MJZ_CX_AL_FN void u_nsso_lenas(uintlen_t new_len) noexcept {
      asserts(asserts.assume_rn, !is_sso());
      uu_nsso_lenas(new_len);
    }
    MJZ_CX_AL_FN uintlen_t u_nsso_len() const noexcept {
      asserts(asserts.assume_rn, !is_sso());
      return uu_nsso_len();
    }

    MJZ_CX_AL_FN void u_nsso_subas(uintlen_t byte_offset,
                                   uintlen_t byte_count) noexcept {
      u_nsso_lenas(byte_count);
      non_sso().begin_ptr += byte_offset;
    }
    MJZ_CX_AL_FN void u_nsso_subas(uintlen_t byte_offset, uintlen_t byte_count,
                                   bool nullas) noexcept {
      u_nsso_nullas(nullas);
      u_nsso_subas(byte_offset, byte_count);
    }

    template <when_t when_v, bool the_room_is_infront, bool check_ = false>
    MJZ_CX_AL_FN success_t add_null() noexcept {
      if (is_sso() || when_v.is_sso()) {
        m_sso_buffer_()[get_length()] = '\0';
        return true;
      }

      uintlen_t len = get_length();
      uintlen_t cap = get_capacity();
      if constexpr (check_)
        if (!has_room_for<when_v>(len, true)) {
          u_nsso_nullas(false);
          return false;
        }

      bool can_add_ = true;
      if constexpr (!the_room_is_infront) {
        can_add_ = can_add_null<when_t::own_relax>();
      }
      if (can_add_) {
        u_get_mut_begin()[len] = '\0';
        u_nsso_nullas(true);
        return true;
      }
      asserts(asserts.assume_rn, !is_sso());
      uintlen_t offset = s_buffer_offset(cap, len, align_direction_v_, true);
      char* buf = memcpy(get_buffer_ptr() + offset, get_begin(), len);
      buf[len] = '\0';
      set_invalid_to_non_sso_begin(buf, len, get_buffer_ptr(), cap,
                                   is_sharable(), true, is_heap());
      return true;
    }
    MJZ_CX_AL_FN bool is_heap() const noexcept {
      str_data_t cnt = cntrl();
      return !((cnt.is_sso | cnt.no_destroy) & 1);
    }
    MJZ_CX_AL_FN bool is_s_view() const noexcept {
      str_data_t cnt = cntrl();
      return (~cnt.is_sso & cnt.is_sharable & cnt.no_destroy) & 1;
    }
    MJZ_CX_AL_FN bool is_stable() const noexcept {
      str_data_t cnt = cntrl();
      // sso
      // heap
      // static view
      return (cnt.is_sso | ~cnt.no_destroy | cnt.is_sharable) & 1;
    }
    MJZ_CX_AL_FN bool is_stable_or_owner() const noexcept {
      str_data_t cnt = cntrl();
      return (cnt.is_sso | ~cnt.no_destroy | cnt.is_sharable | cnt.has_mut) & 1;
    }
  };
};
};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_