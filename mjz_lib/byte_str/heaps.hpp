
#include "../allocs/alloc_ref.hpp"
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

  MJZ_CX_AL_FN allocs_ns::alloc_info_t<version_v> alloc_info_v() const noexcept {
    allocs_ns::alloc_info_t<version_v> info{};
    if (m.is_threaded) {
      info.make_threaded();
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
    MJZ_CX_AL_FN temp_layout_t(m_t &m) noexcept
        : var{m.heap_data_ptr, non_threaded_rf_block, non_threaded_rf_block},
          refcr{*var},
          is_threaded{m.is_threaded},
          is_owenrized{m.is_owenrized} {}
    MJZ_CX_AL_FN auto perform_ref(auto &&Lmabda_th, auto &&Lmabda_nth) noexcept {
      if (is_threaded && !is_owenrized) {
        return Lmabda_th(refcr);
      } else {
        return Lmabda_nth(*var);
      }
    }
  };
  MJZ_CX_AL_FN void init_heap() noexcept {
    asserts(asserts.assume_rn, !!*this);
    if (!can_add_shareholder()) return;
    mjz::memset(m.heap_data_ptr, non_threaded_rf_block, 0);
    temp_layout_t layout{m};
    layout.perform_ref(
        [](auto &ref) noexcept { ref.store(1, std::memory_order_relaxed); },
        [](auto &ref) noexcept { ref = 1; });
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
  MJZ_CONSTANT(uintlen_t)
  cow_threaded_threshold = threaded_rf_block * 5;

 public:
  MJZ_NO_CPY(str_heap_manager_t);

  MJZ_CX_AL_FN auto alloc_ptr() const noexcept { return m.alloc_ref; }

  MJZ_CX_AL_FN success_t malloc(MJZ_WILL_USE uintlen_t min_size,
                             bool round_up = true) noexcept {
    if (!m.alloc_ref) return false;
    if (!!*this) {
      if (m.reduce_rc_on_manager_destruction && !free()) {
        return false;
      }
    }
    if (!min_size) return true;
    if (m.is_threaded) {
      if (cow_threaded_threshold < min_size + threaded_rf_block) {
        min_size = std::max(min_size, cow_threaded_threshold);
        min_size += threaded_rf_block;
      }
    } else {
      min_size += non_threaded_rf_block;
    }
    if (round_up) {
      constexpr uint8_t round_up_min_ratio_pow{2};
      constexpr uint8_t round_up_min_pow{5};
      uint8_t round_pow(std::max<uint8_t>(
          uint8_t(std::max<uint8_t>(
                      uint8_t(round_up_min_pow + round_up_min_ratio_pow),
                      log2_ceil_of_val_create(min_size)) -
                  round_up_min_ratio_pow),
          uint8_t(alloc_info_v().log2_of_align_val)));
      if (min_size & ~((uintlen_t(-1) >> round_pow) << round_pow)) {
        min_size >>= round_pow;
        min_size++;
        min_size <<= round_pow;
      }
    }
    const alloc_base_ref &ref = *m.alloc_ref;
    block_info blk = ref.allocate_bytes(min_size, alloc_info_v());
    if (!blk.ptr) {
      m.reduce_rc_on_manager_destruction = false;
      return false;
    } else {
      m.reduce_rc_on_manager_destruction = true;
    }
    m.heap_data_ptr = std::assume_aligned<non_threaded_rf_block>(blk.ptr);
    m.heap_data_size = blk.length;
    init_heap();
    return true;
  }
  MJZ_CX_AL_FN success_t free() noexcept {
    if (!*this) {
      m.heap_data_size = 0;
      return true;
    }
    MJZ_RELEASE {
      m.heap_data_ptr = nullptr;
      m.heap_data_size = 0;
      m.reduce_rc_on_manager_destruction = false;
    };
    if (!deinit_heap_and_ask()) {
      return true;
    }
    const alloc_base_ref &ref = *m.alloc_ref;
    return ref.deallocate_bytes(block_info{m.heap_data_ptr, m.heap_data_size},
                                alloc_info_v());
  }
  MJZ_CX_AL_FN void must_free() noexcept { asserts(asserts.assume_rn, free()); }
  MJZ_CX_AL_FN uintlen_t buffer_overhead() const noexcept {
    if (!*this) return 0;
    if (!m.is_threaded) {
      return non_threaded_rf_block;
    }
    if (cow_threaded_threshold < m.heap_data_size) {
      return threaded_rf_block;
    }
    return 0;
  }
  MJZ_CX_AL_FN char *get_heap_begin() const noexcept {
    if (!m.heap_data_ptr) return nullptr;
    return std::assume_aligned<non_threaded_rf_block>(m.heap_data_ptr) +
           buffer_overhead();
  }
  MJZ_CX_AL_FN char *steal_heap_begin(bool change_rc = true) noexcept {
    if (!to_non_owner(change_rc)) return nullptr;

    MJZ_RELEASE {
      m.heap_data_ptr = nullptr;
      m.heap_data_size = 0;
      m.reduce_rc_on_manager_destruction = false;
    };
    return get_heap_begin();
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
    if (!m.heap_data_size) return 0;
    return m.heap_data_size - buffer_overhead();
  }
  MJZ_CX_AL_FN bool get_is_threaded() const noexcept { return m.is_threaded; }

  MJZ_CX_AL_FN operator bool() const noexcept {
    return m.alloc_ref && m.heap_data_ptr;
  }

  MJZ_CX_AL_FN success_t can_add_shareholder() const noexcept {
    return !!buffer_overhead();
  }

  MJZ_CX_AL_FN success_t add_shareholder() noexcept {
    if (!can_add_shareholder()) return !!*this;
    asserts(asserts.assume_rn, !m.is_owenrized);
    temp_layout_t{m}.perform_ref(
        [](auto &ref) noexcept { ref.fetch_add(1, std::memory_order_acquire); },
        [](auto &ref) noexcept { ++ref; });
    return true;
  }
  MJZ_CX_AL_FN bool is_owner() const noexcept {
    if (!can_add_shareholder()) return !!*this;
    char_storage_as_temp_t<uintlen_t> var{
        m.heap_data_ptr, non_threaded_rf_block, non_threaded_rf_block};
    if (!var) return false;
    if (m.is_owenrized) return true;
    if (!m.is_threaded) {
      return *var < 2;
    }
    refcr_t ref_count{*var};
    return (ref_count).load(std::memory_order_acquire) < 2;
  }
  MJZ_CX_AL_FN bool remove_shareholder_then_check_has_no_owner() noexcept {
    if (!*this) return false;
    if (m.is_owenrized) return true;
    if (!can_add_shareholder()) return true;
    /*
     * if this is zero, then we know we where the last person.
     */
    return temp_layout_t{m}.perform_ref(
               [](auto &ref) noexcept -> uintlen_t {
                 return ref.fetch_sub(1, std::memory_order_release) - 1;
               },
               [](auto &ref) noexcept -> uintlen_t { return --ref; }) < 1;
  }

  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc, bool is_threaded_,
                               bool is_owenrized_) noexcept
      : m{&alloc, is_threaded_, true, is_owenrized_} {}
  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc, bool is_threaded_,
                               bool is_owenrized_, uintlen_t min_size) noexcept
      : str_heap_manager_t{alloc, is_threaded_, is_owenrized_} {
    malloc(min_size);
  }
  MJZ_CX_AL_FN str_heap_manager_t(const alloc_base_ref &alloc, bool is_threaded_,
                               bool is_owenrized_,
                               bool add_rc_on_manager_construction,
                               bool reduce_rc_on_manager_destruction,
                               char *heap_begin,
                               uintlen_t capacity = 0) noexcept
      : str_heap_manager_t{alloc, is_threaded_, is_owenrized_} {
    if (!heap_begin || !capacity) return;
    m.heap_data_ptr = std::assume_aligned<non_threaded_rf_block>(heap_begin);
    m.heap_data_size = capacity;
    m.heap_data_size += buffer_overhead();
    m.reduce_rc_on_manager_destruction = reduce_rc_on_manager_destruction;
    if (add_rc_on_manager_construction) {
      add_shareholder();
    }
  }

  MJZ_CX_AL_FN str_heap_manager_t(str_heap_manager_t &&obj) noexcept
      : m{std::exchange(obj.m, m_t{obj.m.alloc_ref, obj.m.is_threaded})} {}
  MJZ_CX_AL_FN str_heap_manager_t &operator=(str_heap_manager_t &&obj) noexcept {
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
