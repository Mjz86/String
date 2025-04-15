
#include "../allocs/alloc_ref.hpp"
#include "../byte_str/traits.hpp"
#include "../threads/atomic_ref.hpp"
#ifndef MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
namespace mjz ::bstr_ns {
template <version_t version_v>
class str_heap_manager_t {
  using alloc_base_ref = const allocs_ns::alloc_base_ref_t<version_v>;

  using block_info = allocs_ns::block_info_t<version_v>;

  template <class>
  friend class mjz_private_accessed_t;

 private:
  struct m_t {
    single_object_pointer_t<const alloc_base_ref> alloc_ref;
    bool is_threaded{};
    bool reduce_rc_on_manager_destruction{};
    bool is_owenrized{};
    char *heap_data_ptr{};
    uintlen_t heap_data_size{};
  } m;

  MJZ_CX_AL_FN allocs_ns::alloc_info_t<version_v> alloc_info_v(
      const alloc_base_ref &ref)
      const noexcept {
    allocs_ns::alloc_info_t<version_v> info=ref.get_vtbl().default_info;
    if (m.is_threaded) {
      info.make_threaded();
      info.size_multiplier = threaded_rf_block;
    } else {
      info.size_multiplier = non_threaded_rf_block;
      info.is_thread_safe = 0;
    } 
    info.set_alignof_z(non_threaded_rf_block);
    MJZ_IFN_CONSTEVAL {
      if (m.is_threaded) {
        info.set_alignof_z(threaded_rf_block);
      }
    }
    return info;
  }
  using refcr_t = threads_ns::atomic_ref_t<uintlen_t>;
  struct temp_layout_t {
    MJZ_NO_MV_NO_CPY(temp_layout_t);
    char_storage_as_temp_t<uintlen_t> var;
    refcr_t refcr;
    bool is_threaded;
    bool is_owenrized;
    MJZ_CX_AL_FN ~temp_layout_t() noexcept = default;
    MJZ_CX_AL_FN temp_layout_t(m_t &m) noexcept
        : var{m.heap_data_ptr, non_threaded_rf_block, non_threaded_rf_block},
          refcr{*var},
          is_threaded{m.is_threaded},
          is_owenrized{m.is_owenrized} {}
    MJZ_CX_AL_FN auto perform_ref(auto &&Lmabda_th,
                                  auto &&Lmabda_nth) noexcept {
      if (is_threaded && !is_owenrized) {
        return Lmabda_th(refcr);
      } else {
        return Lmabda_nth(*var);
      }
    }
  };
  MJZ_CX_FN void init_heap() noexcept {
    if (!can_add_shareholder()) return;
    char *rc_ptr = std::assume_aligned<non_threaded_rf_block>(m.heap_data_ptr);
    MJZ_IF_CONSTEVAL {
      mjz::memset(rc_ptr, non_threaded_rf_block, 0);
      char_storage_as_temp_t<uintlen_t> var{rc_ptr, non_threaded_rf_block,
                                            non_threaded_rf_block};
      *var = 1;
      return;
    }
    else {
      *reinterpret_cast<uintlen_t *>(rc_ptr) = 1;
      // ref.store(1, std::memory_order_relaxed);
      return;
    }
  }
  MJZ_CX_AL_FN success_t deinit_heap_and_ask() noexcept {
    asserts(asserts.assume_rn, !!*this);
    return remove_shareholder_then_check_has_no_owner();
  }
  MJZ_CONSTANT(uintlen_t)
  non_threaded_rf_block = sizeof(uintlen_t);
  MJZ_CONSTANT(uintlen_t)
  threaded_rf_block =
      std::max(hardware_destructive_interference_size, non_threaded_rf_block);

 public:
  MJZ_NO_CPY(str_heap_manager_t);

  MJZ_CX_AL_FN auto alloc_ptr() const noexcept { return m.alloc_ref; }

 private:
  template <bool has_alloc_v = true>
  MJZ_CX_FN success_t malloc_p(MJZ_WILL_USE uintlen_t min_size) noexcept {
    block_info blk{};
    if constexpr (has_alloc_v) {
      const alloc_base_ref &ref = *m.alloc_ref;
      blk = ref.allocate_bytes(min_size, alloc_info_v(ref));
    } else {
      const alloc_base_ref &ref = allocs_ns::empty_alloc<version_v>;
      blk = ref.allocate_bytes(min_size, alloc_info_v(ref));
    }
    bool succuss = !!blk.ptr;
    m.reduce_rc_on_manager_destruction = succuss;
    // as far as i know nullptr is aligned
    m.heap_data_ptr = std::assume_aligned<non_threaded_rf_block>(blk.ptr);
    m.heap_data_size = blk.length;
    bool is_valid_len = !(byte_traits_t<version_v>::npos - 1 < blk.length);
    if constexpr (MJZ_IN_DEBUG_MODE) {
      if (!is_valid_len) {
        asserts(asserts.assume_rn, is_valid_len,
                "the allocate_bytes shall not give more than "
                "view_t::max_size(), and shall return nullptr");
      }
    } else {
      asserts(asserts.assume_rn, is_valid_len);
    }
    init_heap();
    asserts(asserts.assume_rn,
            !succuss || std::cmp_less_equal(min_size, blk.length));
    return succuss;
  }
  MJZ_CX_FN uintlen_t minsize_calc(uintlen_t min_size, bool round_up) noexcept {
    uintlen_t min_size_tr{min_size};
    min_size_tr = (min_size_tr / threaded_rf_block) +
                  uintlen_t(!!(min_size_tr % threaded_rf_block));
    min_size_tr *= threaded_rf_block;
    min_size_tr += alias_t<uintlen_t[2]>{
        0, threaded_rf_block}[cow_threaded_threshold() < min_size_tr];
    uintlen_t min_size_ntr{min_size};
    min_size_ntr = (min_size_ntr / non_threaded_rf_block) +
                   uintlen_t(!!(min_size_ntr % non_threaded_rf_block));
    min_size_ntr *= non_threaded_rf_block;
    min_size_ntr += non_threaded_rf_block;
    min_size = alias_t<uintlen_t[2]>{min_size_ntr, min_size_tr}[m.is_threaded];
    min_size = alias_t<uintlen_t[2]>{
        min_size, uintlen_t(1) << log2_ceil_of_val_create(min_size)}[round_up];
    return min_size;
  }

 public:
  template <bool has_alloc_v = true>
  MJZ_CX_FN success_t malloc(MJZ_WILL_USE uintlen_t min_size,
                             bool round_up = true) noexcept {
    bool bad_ret = !min_size;
    bad_ret |= !m.alloc_ref;
    bad_ret |= !!*this;
    if (bad_ret) MJZ_IS_UNLIKELY {
        return m.reduce_rc_on_manager_destruction && free();
      }
    return u_malloc<has_alloc_v>(min_size, round_up);
  }
  template <bool has_alloc_v>
  MJZ_CX_FN success_t u_malloc(MJZ_WILL_USE uintlen_t min_size,
                               bool round_up = true) noexcept {
    return malloc_p<has_alloc_v>(minsize_calc(min_size, round_up));
  }

 private:
  template <bool has_alloc_v = true>
  MJZ_CX_FN success_t free_p() noexcept {
    asserts(asserts.assume_rn, !!*this);
    if (!deinit_heap_and_ask()) {
      return true;
    }
    return u_free_p();
  }

  template <bool has_alloc_v = true>
  MJZ_CX_FN success_t u_free_p() noexcept {
    if constexpr (has_alloc_v) {
      const alloc_base_ref &ref = *m.alloc_ref;
      return ref.deallocate_bytes(block_info{m.heap_data_ptr, m.heap_data_size},
                                  alloc_info_v(ref));
    } else {
      const alloc_base_ref &ref = allocs_ns::empty_alloc<version_v>;
      return ref.deallocate_bytes(block_info{m.heap_data_ptr, m.heap_data_size},
                                  alloc_info_v(ref));
    }
  }

 public:
  template <bool is_ownerized_v, bool has_alloc_v>
  MJZ_CX_AL_FN void u_must_free() noexcept {
    MJZ_RELEASE { unsafe_clear(); };
    if constexpr (!is_ownerized_v) {
      bool do_rf_free = m.is_owenrized;
      do_rf_free |= !can_add_shareholder();
      if (!(do_rf_free || remove_shareholder_then_check_has_no_owner_heap())) {
        return;
      }
    }
    asserts(asserts.assume_rn, u_free_p<has_alloc_v>());
  }
  MJZ_CX_AL_FN success_t free() noexcept {
    if (!!*this) {
      asserts(asserts.assume_rn, free_p());
    }
    unsafe_clear();
    return true;
  }
  MJZ_CX_AL_FN void must_free() noexcept { asserts(asserts.assume_rn, free()); }
  MJZ_CX_AL_FN uintlen_t cow_threaded_threshold() const noexcept {
     return m.alloc_ref->get_vtbl()
        .cow_threashold;
  }
  MJZ_CX_AL_FN uintlen_t buffer_overhead() const noexcept {
    bool is_nfull = !*this;
    bool is_cow = cow_threaded_threshold() < m.heap_data_size;
    bool is_threaded = m.is_threaded;
    uintlen_t awnser[2][2][2]{{{non_threaded_rf_block, non_threaded_rf_block},
                               {0, threaded_rf_block}},
                              {}};
    // branch-less
    return awnser[is_nfull][is_threaded][is_cow];
  }
  MJZ_CX_AL_FN char *get_heap_begin() const noexcept {
    return std::assume_aligned<non_threaded_rf_block>(m.heap_data_ptr) +
           buffer_overhead();
  }
  MJZ_CX_AL_FN char *steal_heap_begin(bool change_rc) noexcept {
    if (!to_non_owner(change_rc)) return nullptr;
    auto ret = get_heap_begin();
    unsafe_clear();
    return ret;
  }
  MJZ_CX_AL_FN success_t to_non_owner(bool change_rc = true) noexcept {
    if (!*this) return false;
    if (m.reduce_rc_on_manager_destruction == false) return true;
    if (change_rc) {
      if (!free()) {
        return false;
      }
    }
    m.reduce_rc_on_manager_destruction = false;
    return true;
  }
  MJZ_CX_AL_FN success_t to_owner(bool change_rc = true) noexcept {
    if (!*this) return false;
    if (m.reduce_rc_on_manager_destruction == true) return true;
    if (!change_rc || !add_shareholder()) return false;
    m.reduce_rc_on_manager_destruction = true;
    return true;
  }
  MJZ_CX_AL_FN uintlen_t get_heap_cap() const noexcept {
    return m.heap_data_size - buffer_overhead();
  }
  MJZ_CX_AL_FN bool get_is_threaded() const noexcept { return m.is_threaded; }

  MJZ_CX_AL_FN operator bool() const noexcept {
    return !!m.heap_data_ptr;
  }

  MJZ_CX_AL_FN bool can_add_shareholder() const noexcept {
    bool ret = !!buffer_overhead();
    ret &= !m.is_owenrized;
    return ret;
  }

  MJZ_CX_FN success_t add_shareholder() noexcept {
    if (!can_add_shareholder()) return !!*this;
    asserts(asserts.assume_rn, !m.is_owenrized);
    temp_layout_t{m}.perform_ref(
        [](auto &ref) noexcept { ref.fetch_add(1, std::memory_order_acquire); },
        [](auto &ref) noexcept { ++ref; });
    return true;
  }
  MJZ_CX_AL_FN void unsafe_clear() noexcept {
    m.heap_data_ptr = nullptr;
    m.heap_data_size = 0;
    m.reduce_rc_on_manager_destruction = false;
  }

 private:
  MJZ_CX_FN bool is_owner_heap() const noexcept {
    char_storage_as_temp_t<uintlen_t> var{
        m.heap_data_ptr, non_threaded_rf_block, non_threaded_rf_block};
    if (!var) return false;
    if (!m.is_threaded) {
      return *var < 2;
    }
    refcr_t ref_count{*var};
    return (ref_count).load(std::memory_order_acquire) < 2;
  }

 public:
  MJZ_CX_AL_FN bool is_owner() const noexcept {
    if (!can_add_shareholder()) return !!*this;
    if (m.is_owenrized) return true;
    return is_owner_heap();
  }

 private:
  MJZ_CX_FN bool remove_shareholder_then_check_has_no_owner_heap() noexcept {
    return temp_layout_t{m}.perform_ref(
               [](auto &ref) noexcept -> uintlen_t {
                 return ref.fetch_sub(1, std::memory_order_acq_rel) - 1;
               },
               [](auto &ref) noexcept -> uintlen_t { return --ref; }) < 1;
  }

 public:
  MJZ_CX_AL_FN bool remove_shareholder_then_check_has_no_owner() noexcept {
    if (!*this) return false;
    if (m.is_owenrized) return true;
    if (!can_add_shareholder()) return true;
    /*
     * if this is zero, then we know we where the last person.
     */
    return remove_shareholder_then_check_has_no_owner_heap();
  }

  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc,
                                  bool is_threaded_,
                                  bool is_owenrized_) noexcept
      : m{&alloc, is_threaded_, false, is_owenrized_} {}
  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc,
                                  bool is_threaded_, bool is_owenrized_,
                                  uintlen_t min_size) noexcept
      : str_heap_manager_t{alloc, is_threaded_, is_owenrized_} {
    malloc(min_size);
  }
  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc,
                                  bool is_threaded_, bool is_owenrized_,
                                  bool add_rc_on_manager_construction,
                                  bool reduce_rc_on_manager_destruction,
                                  char *heap_begin,
                                  uintlen_t capacity = 0) noexcept
      : str_heap_manager_t{alloc, is_threaded_, is_owenrized_} {
    m.heap_data_ptr = std::assume_aligned<non_threaded_rf_block>(heap_begin);
    m.heap_data_size = capacity;
    uintlen_t overhead = buffer_overhead();
    m.heap_data_size += overhead;
    m.heap_data_ptr -= overhead;
    m.reduce_rc_on_manager_destruction = reduce_rc_on_manager_destruction;
    if (add_rc_on_manager_construction) {
      add_shareholder();
    }
    asserts(asserts.assume_rn, !heap_begin == !capacity);
  }

  MJZ_CX_AL_FN str_heap_manager_t(str_heap_manager_t &&obj) noexcept
      : m{std::exchange(obj.m, m_t{obj.m.alloc_ref, obj.m.is_threaded})} {}
  MJZ_CX_AL_FN str_heap_manager_t &operator=(
      str_heap_manager_t &&obj) noexcept {
    must_free();
    m = std::exchange(obj.m, m_t{obj.m.alloc_ref, obj.m.is_threaded});
  }
  MJZ_CX_AL_FN ~str_heap_manager_t() noexcept {
    if (!m.reduce_rc_on_manager_destruction) {
      return;
    }
    must_free();
  }
};

}  // namespace mjz::bstr_ns

#endif  // MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
