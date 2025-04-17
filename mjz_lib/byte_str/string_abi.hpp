
#include "base_abi.hpp"
#ifndef MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_
namespace mjz::bstr_ns {
enum class align_direction_e : char {
  relaxed = 0,
  center = 1,
  front = 2,
  back = 3
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
  MJZ_CONSTANT(uintlen_t) min_sso_sicap = 4 * sizeof(uintlen_t) - 2;
  MJZ_CONSTANT(uintlen_t)
  old_sso_cap_helper_ = std::max(user_sso_selected_cap, min_sso_sicap);
  MJZ_CONSTANT(uintlen_t)
  old_sso_cap =
      std::same_as<
          uint_with_max_of_t<(sizeof(uintlen_t) + old_sso_cap_helper_ +
                              sizeof(uint_with_max_of_t<old_sso_cap_helper_>))>,
          uint_with_max_of_t<old_sso_cap_helper_>>
          ? old_sso_cap_helper_
          : old_sso_cap_helper_ +
                sizeof(uint_with_max_of_t<old_sso_cap_helper_>) +
                sizeof(uintlen_t);
  MJZ_CONSTANT(uintlen_t) cntrl_size = sizeof(uint8_t);
  using cap_mins_length_t = uint_with_max_of_t<old_sso_cap>;
  MJZ_CONSTANT(uintlen_t)
  min_buffer_size = old_sso_cap + sizeof(cap_mins_length_t);
  MJZ_CONSTANT(uintlen_t)
  buffer_size =
      sizeof(uintlen_t) *
          (((min_buffer_size + cntrl_size) / sizeof(uintlen_t)) +
           uintlen_t(!!((min_buffer_size + cntrl_size) % sizeof(uintlen_t)))) -
      cntrl_size;

  MJZ_PACKING_START_;
  struct non_sso_t {
    char raw_capacity[sizeof(uintlen_t) - 1]{};
    uintlen_t length{};
    const char* begin_ptr{};
    char* data_block{};
  };
  union alignas(1) raw_data_u {
    non_sso_t non_sso{};
    char sso_buffer[buffer_size];
  };
  template <bool is_prop_v_>
  using uint8p_t = std::conditional_t<is_prop_v_, const uint8_t, uint8_t>;
  struct control_byte_t {
    uint8_t is_sso : 1 {};
    uint8p_t<is_threaded_v_ != may_bool_t::idk> as_not_threaded_bit : 1 {
        !bool(char(is_threaded_v_))};
    uint8_t is_sharable : 1 {};
    uint8p_t<is_ownerized_v_> is_ownerized : 1 {is_ownerized_v_};
    uint8_t has_null : 1 {};
    uint8_t encodings_bits : 3 {};

    MJZ_CX_FN control_byte_t() noexcept = default;
    MJZ_CX_FN ~control_byte_t() noexcept = default;
    MJZ_CX_FN control_byte_t(const control_byte_t&) noexcept = default;
    MJZ_CX_FN control_byte_t& operator=(const control_byte_t& src) noexcept {
      std::destroy_at(this);
      return *std::construct_at(this, src);
    }
  };
  struct raw_data_t {
    static_assert(sizeof(control_byte_t) == sizeof(uint8_t));
    control_byte_t control_byte{};
    raw_data_u raw_data{};
  };
  MJZ_PACKING_END_;
  static_assert(sizeof(raw_data_t) == sizeof(raw_data_u) + sizeof(uint8_t));
  struct data_t : abi_ns_::alloc_t<version_v, has_alloc_v_> {
   public:
    MJZ_CONSTANT(uintlen_t) sso_cap = buffer_size - sizeof(cap_mins_length_t);
    constexpr static const alloc_ref& empty_alloc{
        allocs_ns::empty_alloc<version_v>};

    struct when_t {
      MJZ_DEFAULTED_CLASS(when_t);
      enum val_e : char { relax, own_relax, no_heap, as_sso };
      val_e val{};
      MJZ_CX_FN when_t(val_e v) : val{v} {};
      MJZ_CX_FN when_t(auto) = delete;
      MJZ_CX_FN
      bool is_sso() const noexcept { return val == as_sso; }
      MJZ_CX_FN
      bool no_destroy() const noexcept {
        return val != relax && val != own_relax;
      }
      MJZ_CX_FN
      bool do_destroy() const noexcept { return !no_destroy(); }

      MJZ_CX_FN explicit operator bool() const noexcept { return do_destroy(); }
    };

   public:
    alignas(alignof(uintlen_t)) raw_data_t m{};

   public:
    template <class = void>
      requires has_alloc_v_
    MJZ_CX_FN const alloc_ref* get_alloc_ptr() const noexcept

    {
      return &this->alloc_;
    }
    template <class = void>
      requires(!has_alloc_v_)
    MJZ_CX_FN const alloc_ref* get_alloc_ptr() const noexcept {
      return {};
    }
    MJZ_CX_FN const alloc_ref& get_alloc() const noexcept {
      if constexpr (has_alloc_v_) {
        return *get_alloc_ptr();
      } else {
        return empty_alloc;
      }
    }
    template <class = void>
      requires has_alloc_v_
    MJZ_CX_FN alloc_ref* get_alloc_ptr() noexcept {
      return &this->alloc_;
    }
    template <class = void>
      requires(!has_alloc_v_)
    MJZ_CX_FN alloc_ref* get_alloc_ptr() noexcept {
      return {};
    }
    MJZ_CX_FN static bool has_alloc_v() noexcept { return has_alloc_v_; }
    MJZ_CX_FN const raw_data_t& m_v() const noexcept {
      return *std::assume_aligned<alignof(uintlen_t)>(&m);
    }
    MJZ_CX_FN raw_data_t& m_v() noexcept {
      return *std::assume_aligned<alignof(uintlen_t)>(&m);
    }
    MJZ_CX_FN non_sso_t& non_sso() noexcept { return m_v().raw_data.non_sso; }

    MJZ_CX_FN const control_byte_t& cntrl() const noexcept {
      return m_v().control_byte;
    }
    MJZ_CX_FN control_byte_t& cntrl() noexcept { return m_v().control_byte; }
    MJZ_CX_FN bool is_sso() const noexcept { return cntrl().is_sso; }

    MJZ_CX_FN bool is_ownerized() const noexcept {
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, !!cntrl().is_ownerized);
        return true;
      }
      return !!cntrl().is_ownerized;
    }
    MJZ_CX_FN bool is_threaded() const noexcept {
      if constexpr (is_threaded_v_ != may_bool_t::idk) {
        asserts(asserts.assume_rn,
                bool(char(is_threaded_v_)) == !cntrl().as_not_threaded_bit);
        return bool(char(is_threaded_v_));
      }
      return !cntrl().as_not_threaded_bit;
    }
    MJZ_CX_FN void set_threaded(bool var) noexcept {
      if constexpr (is_threaded_v_ != may_bool_t::idk) {
        return;
      } else {
        cntrl().as_not_threaded_bit = var;
      }
    }
    MJZ_CX_FN void set_ownerized(bool var) noexcept {
      if constexpr (is_ownerized_v_) {
        return;
      } else {
        cntrl().is_ownerized = var;
      }
    }

    MJZ_CX_FN uintlen_t get_cap_minus_sso_length() const noexcept {
      const char* len_ptr = m_v().raw_data.sso_buffer + sso_cap;
      len_ptr = std::assume_aligned<alignof(cap_mins_length_t)>(len_ptr);
      MJZ_IFN_CONSTEVAL {
        return *std::launder(
            reinterpret_cast<const cap_mins_length_t*>(len_ptr));
      }
      else {
        std::array<char, sizeof(cap_mins_length_t)> buf{};
        for (uintlen_t i{}; i < buf.size(); i++) buf[i] = len_ptr[i];
        return std::bit_cast<cap_mins_length_t>(buf);
      }
    }
    MJZ_CX_FN void set_cap_minus_sso_length(uintlen_t new_val_) noexcept {
      cap_mins_length_t new_val{cap_mins_length_t(new_val_)};
      asserts(asserts.assume_rn, new_val == new_val_);
      char* len_ptr = m_v().raw_data.sso_buffer + sso_cap;
      len_ptr = std::assume_aligned<alignof(cap_mins_length_t)>(len_ptr);
      MJZ_IFN_CONSTEVAL {
        *std::launder(reinterpret_cast<cap_mins_length_t*>(len_ptr)) = new_val;
        return;
      }
      else {
        auto buf =
            std::bit_cast<std::array<char, sizeof(cap_mins_length_t)>>(new_val);
        for (uintlen_t i{}; i < buf.size(); i++) len_ptr[i] = buf[i];
      }
    }

    MJZ_CX_FN uintlen_t get_sso_length() const noexcept {
      return sso_cap - get_cap_minus_sso_length();
    }
    MJZ_CX_FN void set_sso_length(uintlen_t new_len_) noexcept {
      set_cap_minus_sso_length(sso_cap - new_len_);
    }

    MJZ_CX_FN uintlen_t get_length() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (!is_sso()) {
          return m_v().raw_data.non_sso.length;
        }
        return get_sso_length();
      } /* UB? na , just branchless */
      uintlen_t lengths[2]{
          cpy_aligned_bitcast<uintlen_t>(
              reinterpret_cast<const char*>(&m_v().raw_data.non_sso) +
              offsetof(non_sso_t, length)),
          get_sso_length()};
      return lengths[is_sso()];
    }

    MJZ_CX_FN encodings_e get_encoding() const noexcept {
      return encodings_e(cntrl().encodings_bits);
    }
    MJZ_CX_FN const char* get_begin() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (!is_sso()) {
          return m_v().raw_data.non_sso.begin_ptr;
        }
        return m_v().raw_data.sso_buffer;
      }
      /* UB? na , just branchless */
      const char* begins[2]{
          cpy_aligned_bitcast<const char*>(
              reinterpret_cast<const char*>(&m_v().raw_data.non_sso) +
              offsetof(non_sso_t, begin_ptr)),
          m_v().raw_data.sso_buffer};
      return begins[is_sso()];
    }
    MJZ_CX_FN static void check_buffer_correct_ness_(
        const char* begin, uintlen_t length, char* buffer_begin,
        uintlen_t buffer_capacity) noexcept {
      asserts(asserts.assume_rn,
              buffer_capacity <= byte_traits_t<version_v>::npos - 1 &&
                  length <= byte_traits_t<version_v>::npos - 1 &&
                  (buffer_begin || !buffer_capacity) && (begin || !length) &&
                  (!begin || !buffer_begin ||
                   (buffer_begin <= begin &&
                    begin + length <= buffer_begin + buffer_capacity &&
                    length <= buffer_capacity)));
    }
    MJZ_CX_FN void set_invalid_to_non_sso_begin(
        const char* begin_, uintlen_t length_, char* buffer_begin_,
        uintlen_t capacity_, bool is_shared_, bool has_null_) noexcept {
      check_buffer_correct_ness_(begin_, length_, buffer_begin_, capacity_);
      non_sso_t& data = *std::construct_at(&m_v().raw_data.non_sso);
      data.begin_ptr = begin_;
      data.data_block = buffer_begin_;
      data.length = length_;
      uintlen_t cap_ask{uintlen_t(-1) >> 8};
      if constexpr (version_v.is_LE()) {
        capacity_ <<= 8;
        cap_ask <<= 8;
      }
      cntrl().is_sso = false;
      cntrl().has_null = has_null_;
      cntrl().is_sharable = is_shared_;
      MJZ_IFN_CONSTEVAL {
        auto ptr_ = reinterpret_cast<uint8_t*>(data.raw_capacity) - 1;
        uintlen_t cntrl_and_cap = cpy_aligned_bitcast<uintlen_t>(ptr_);
        cntrl_and_cap &= ~cap_ask;
        cntrl_and_cap |= capacity_;
        cpy_aligned_bitcast(ptr_, cntrl_and_cap);
        return;
      }
      auto buff = std::bit_cast<std::array<char, sizeof(uintlen_t)>>(capacity_);
      for (uintlen_t i{1}; i < buff.size(); i++) {
        data.raw_capacity[i - 1] = buff[i];
      }
      return;
    }
    template <when_t when_v>
    MJZ_CX_FN void memcpy_to_non_sso(const char* ptr, uintlen_t len,
                                     char* buffer_begin_, uintlen_t capacity_,
                                     bool is_shared_) noexcept {
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
      if (has_null) {
        beg[len] = '\0';
      }
      if constexpr (when_v) {
        destruct_to_invalid();
      }
      set_invalid_to_non_sso_begin(beg, len, buf, cap, is_shared_, has_null);
    }

    MJZ_CX_FN uintlen_t get_non_sso_capacity() const noexcept {
      uintlen_t ret{};
      MJZ_IFN_CONSTEVAL {
        ret = cpy_aligned_bitcast<uintlen_t>(
            reinterpret_cast<const uint8_t*>(
                m_v().raw_data.non_sso.raw_capacity) -
            1);
      }
      else {
        std::array<char, sizeof(uintlen_t)> buff{};
        for (uintlen_t i{1}; i < buff.size(); i++) {
          buff[i] = m_v().raw_data.non_sso.raw_capacity[i - 1];
        }
        ret = std::bit_cast<uintlen_t>(buff);
      }
      if constexpr (version_v.is_LE()) {
        ret >>= 8;
      } else {
        ret &= uintlen_t(-1) >> 8;
      }
      return ret;
    }
    MJZ_CX_FN uintlen_t get_capacity() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (is_sso()) {
          return sso_cap;
        }
        return get_non_sso_capacity();
      }
      return alias_t<uintlen_t[2]>{get_non_sso_capacity(), sso_cap}[is_sso()];
    }

    MJZ_CX_FN void set_invalid_to_sso(const char* non_overlapping_ptr,
                                      uintlen_t len) noexcept {
      asserts(asserts.assume_rn, len <= sso_cap);
      m_v().raw_data.sso_buffer[0] = 0;  // make the buffer alive
      char* buf = m_v().raw_data.sso_buffer;
      MJZ_IF_CONSTEVAL { memset(buf, sso_cap, 0); }
      MJZ_DISABLE_ALL_WANINGS_START_;
      memcpy(buf, non_overlapping_ptr, len)[len] = '\0';
      MJZ_DISABLE_ALL_WANINGS_END_;

      cntrl().is_sso = true;
      cntrl().has_null = true;
      cntrl().is_sharable = false;
      set_sso_length(len);
    }
    MJZ_CX_FN void set_length(uintlen_t new_len) noexcept {
      if (!is_sso()) {
        m_v().raw_data.non_sso.length = new_len;
        return;
      }
      set_sso_length(new_len);
    }

    MJZ_CX_FN const char* get_buffer_ptr() const noexcept {
      MJZ_IF_CONSTEVAL {
        if (!is_sso()) {
          return m_v().raw_data.non_sso.data_block;
        }
        return m_v().raw_data.sso_buffer;
      }
      /* UB? na , just branchless */
      const char* buffs[2]{
          cpy_aligned_bitcast<const char*>(
              reinterpret_cast<const char*>(&m_v().raw_data.non_sso) +
              offsetof(non_sso_t, data_block)),
          m_v().raw_data.sso_buffer};
      return buffs[is_sso()];
      ;
    }

    MJZ_CX_FN char* get_buffer_ptr() noexcept {
      // no need to write same stuff
      return const_cast<char*>(std::as_const(*this).get_buffer_ptr());
    }
    MJZ_CX_FN char* get_mut_begin() noexcept {
      return alias_t<alias_t<char*>[2]>{nullptr, u_get_mut_begin()}[has_mut()];
    }
    MJZ_CX_FN bool has_mut() const noexcept { return !!get_buffer_ptr(); }
    MJZ_CX_FN basic_string_view_t<version_v> get_view() const noexcept {
      bool is_s_view_ = cntrl().is_sharable;
      is_s_view_ &= !has_mut();
      return base_string_view_t<version_v>::make(get_begin(), get_length(),
                                                 get_encoding(),
                                                 cntrl().has_null, is_s_view_);
    }

    using str_heap_manager = str_heap_manager_t<version_v>;
    MJZ_CX_FN str_heap_manager non_sso_my_heap_manager_no_own() const noexcept {
      asserts(asserts.assume_rn, !is_sso());
      return str_heap_manager(get_alloc(), is_threaded(), is_ownerized(), false,
                              false, m_v().raw_data.non_sso.data_block,
                              get_non_sso_capacity());
    }

   private:
    MJZ_CX_FN void destruct_heap() noexcept {
      str_heap_manager hm = non_sso_my_heap_manager_no_own();
      hm.template u_must_free<is_ownerized_v_, has_alloc_v_>();
      hm.unsafe_clear();
    }

   public:
    MJZ_CX_FN bool no_destroy() const noexcept {
      alias_t<uint8_t[3]> b{!!cntrl().is_sso, !cntrl().is_sharable, !has_mut()};
      return !!(b[0] + b[1] + b[2]);
    }
    MJZ_CX_FN void destruct_all() noexcept {
      if (no_destroy()) {
        return invalid_to_empty();
      }
      destruct_heap();
      invalid_to_empty();
    }
    MJZ_CX_FN void destruct_to_invalid() noexcept {
      if (no_destroy()) {
        return;
      }
      destruct_heap();
    }
    MJZ_CX_FN void invalid_to_empty() noexcept { set_invalid_to_sso("", 0); }

    MJZ_CX_FN static uintlen_t s_buffer_offset(
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
      return alias_t<uintlen_t[4]>{center, center, front,
                                   back}[uint8_t(char(align))];
    }

   private:
    template <when_t when_v>
    MJZ_CX_FN bool is_heap_owner() const noexcept {
      if constexpr (when_v.val == when_t::own_relax) {
        return bool(char(may_bool_t::yes));
      }
      if constexpr (is_ownerized_v_ && !MJZ_IN_DEBUG_MODE) {
        asserts(asserts.assume_rn, false);
        return false;
      }

      asserts(asserts.assume_rn,
              !cntrl().is_ownerized && cntrl().is_sharable && has_mut());
      str_heap_manager hm = non_sso_my_heap_manager_no_own();
      bool is_owner = hm.is_owner();
      // compiler did non optimize free away if this wasnt present
      hm.unsafe_clear();
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, is_owner == true);
      }
      return is_owner;
    }
    MJZ_CX_FN may_bool_t is_owner_no_heap() const noexcept {
      if constexpr (is_ownerized_v_ && !MJZ_IN_DEBUG_MODE) {
        return may_bool_t::yes;
      }
      bool is_ownerized = cntrl().is_ownerized;
      bool is_sharable = cntrl().is_sharable;
      bool has_mut_ = has_mut();
      asserts(asserts.assume_rn, !is_ownerized || has_mut_);
      bool is_owner = !is_sharable;
      is_owner &= has_mut_;
      is_owner |= is_ownerized;
      bool no_check = is_owner;
      no_check |= !has_mut_;
      auto ret = alias_t<may_bool_t[2]>{may_bool_t::idk,
                                        may_bool_t(char(is_owner))}[no_check];
      if constexpr (is_ownerized_v_) {
        asserts(asserts.assume_rn, ret == may_bool_t::yes);
      }
      return ret;
    }

   public:
    MJZ_CX_FN bool is_owner() const noexcept {
      may_bool_t ret = is_owner_no_heap();
      if (may_bool_t::idk == ret) {
        return is_heap_owner<when_t::relax>();
      }
      return bool(char(ret));
    };
    template <when_t when_v>
    MJZ_CX_FN bool has_room_for(uintlen_t new_size,
                                bool with_null = false) const noexcept {
      if constexpr (when_v.is_sso()) {
        asserts(asserts.assume_rn, is_sso());
        return uintlen_t(with_null) + new_size <= sso_cap;
      }
      may_bool_t ret = is_owner_no_heap();
      bool has_room = uintlen_t(with_null) + new_size <= get_capacity();
      ret = alias_t<may_bool_t[2]>{may_bool_t::no, ret}[has_room];
      if constexpr (when_v) {
        if (may_bool_t::idk == ret) {
          return is_heap_owner<when_v>();
        }
      }
      return bool(char(ret));
    }

    MJZ_CX_FN char* u_get_mut_begin() noexcept {
      return const_cast<char*>(get_begin());
    }
    template <when_t when_v>
    MJZ_CX_FN bool can_add_null() const noexcept {
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
    template <when_t when_v>
    MJZ_CX_FN success_t add_null() noexcept {
      uintlen_t len = get_length();
      uintlen_t cap = get_capacity();
      if (!has_room_for<when_v>(len, true)) {
        cntrl().has_null = false;
        return false;
      }
      if (can_add_null<when_t::own_relax>()) {
        u_get_mut_begin()[len] = '\0';
        cntrl().has_null = true;
        return true;
      }
      asserts(asserts.assume_rn, !is_sso());
      uintlen_t offset = s_buffer_offset(cap, len, align_direction_v_,true);
      char* buf = memcpy(get_buffer_ptr() + offset, get_begin(), len);
      buf[len] = '\0';
      set_invalid_to_non_sso_begin(buf, len, get_buffer_ptr(), cap,
                                   !!cntrl().is_sharable, true);
      return true;
    }
    MJZ_CX_FN bool is_heap() const noexcept {
      return has_mut() && cntrl().is_sharable;
    }
    MJZ_CX_FN bool is_s_view() const noexcept {
      return !has_mut() && cntrl().is_sharable;
    }
  };
};

};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_