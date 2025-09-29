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

#include "../allocs/alloc_ref.hpp"
#include "../byte_str/traits.hpp"
#include "../threads/atomic_ref.hpp"
#ifndef MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz ::bstr_ns {
  template <version_t version_v, may_bool_t is_threaded_v_,
            bool is_ownerized_v_, bool has_alloc_v_>
  class str_heap_manager_t {
    using alloc_base_ref = const allocs_ns::alloc_base_ref_t<version_v>;

    using block_info = allocs_ns::block_info_t<version_v>;

    template <class> friend class mjz_private_accessed_t;

  private:
    struct m_t {
      single_object_pointer_t<const alloc_base_ref> alloc_ref{
          &allocs_ns::empty_alloc<version_v>};
      bool is_threaded{};
      bool reduce_rc_on_manager_destruction{};
      bool is_owenrized{};
      // curruntly  this abi&api combo is not yet implemented correctly and stil
      // uses the old ways of variable use . !can_add_shareholder()  ||
      // heap_rc_ptr==heap_data_ptr+buffer_overhead_impl_()-sizeof(uintlen_t)
      char *heap_rc_ptr{};
      char *heap_data_ptr{};
      uintlen_t heap_data_size{};
      uintlen_t cow_threaded_threshold_temp_{};
    } m;

    MJZ_CX_AL_FN uintlen_t buffer_overhead_impl_() noexcept {
      return branchless_teranary<uintlen_t>(
          !!*this,
          branchless_teranary<uintlen_t>(
              get_is_threaded(),
              branchless_teranary<uintlen_t>(
                  !(cow_threaded_threshold() < m.heap_data_size), 0,
                  threaded_rf_block),
              non_threaded_rf_block),
          0);
    }
    MJZ_CX_AL_FN allocs_ns::alloc_info_t<version_v>
    alloc_info_v(const alloc_base_ref &ref) const noexcept {
      allocs_ns::alloc_info_t<version_v> info = ref.get_vtbl().default_info;
      if (get_is_threaded()) {
        info.make_threaded();
        info.size_multiplier = threaded_rf_block;
      } else {
        info.size_multiplier = non_threaded_rf_block;
        info.is_thread_safe = 0;
      }
      info.set_alignof_z(non_threaded_rf_block);
      MJZ_IFN_CONSTEVAL {
        if (get_is_threaded()) {
          info.set_alignof_z(threaded_rf_block);
        }
      }
      return info;
    }
    using refcr_t =
        std::conditional_t<is_ownerized_v_ || is_threaded_v_ == may_bool_t::no,
                           threads_ns::cx_atomic_ref_t<uintlen_t>,
                           threads_ns::atomic_ref_t<uintlen_t>>;
    struct temp_layout_t {
      MJZ_NO_MV_NO_CPY(temp_layout_t);
      char_storage_as_temp_t<uintlen_t> var;
      refcr_t refcr;
      bool is_threaded;
      bool is_owenrized;
      MJZ_CX_AL_FN ~temp_layout_t() noexcept = default;
      MJZ_CX_AL_FN temp_layout_t(m_t &m) noexcept
          : var{m.heap_rc_ptr, non_threaded_rf_block, non_threaded_rf_block},
            refcr{*var}, is_threaded{m.is_threaded},
            is_owenrized{m.is_owenrized} {}
      MJZ_CX_AL_FN auto perform_ref(MJZ_MAYBE_UNUSED auto &&Lmabda_th,
                                    auto &&Lmabda_nth) noexcept {
        if constexpr (is_ownerized_v_ || is_threaded_v_ == may_bool_t::no) {
          return Lmabda_nth(*var);
        } else if (is_threaded && !is_owenrized) {
          return Lmabda_th(refcr);
        } else {
          return Lmabda_nth(*var);
        }
      }
    };
    MJZ_CX_AL_FN void init_heap() noexcept {
      alignas(
          non_threaded_rf_block) char dummy[sizeof(non_threaded_rf_block)]{};
      char *rc_ptr =
          mjz::assume_aligned<non_threaded_rf_block>(m.heap_data_ptr) +
          buffer_overhead_impl_();

      rc_ptr = (m.heap_rc_ptr = m.heap_data_ptr == rc_ptr
                                    ? nullptr
                                    : rc_ptr - sizeof(uintlen_t));
      rc_ptr = rc_ptr ? rc_ptr : &dummy[0];
      MJZ_IF_CONSTEVAL {
        mjz::memset(rc_ptr, non_threaded_rf_block, 0);
        char_storage_as_temp_t<uintlen_t> var{rc_ptr, non_threaded_rf_block,
                                              non_threaded_rf_block};
        *var = 1;
        return;
      }
      else {
        new (rc_ptr) uintlen_t{1};
        return;
      }
    }
    MJZ_CX_AL_FN success_t deinit_heap_and_ask() noexcept {
      if (is_owner()) {
        return true;
      }
      if (remove_shareholder_then_check_has_no_owner())
        MJZ_IS_UNLIKELY { return true; }
      return false;
    }
    MJZ_MCONSTANT(uintlen_t)
    non_threaded_rf_block = sizeof(uintlen_t);
    MJZ_MCONSTANT(uintlen_t)
    threaded_rf_block =
        std::max(hardware_destructive_interference_size, non_threaded_rf_block);

  public:
    MJZ_NO_CPY(str_heap_manager_t);

    MJZ_CX_AL_FN auto alloc_ptr() const noexcept {
      if constexpr (has_alloc_v_) {
        return m.alloc_ref;
      } else {
        return &allocs_ns::empty_alloc<version_v>;
      }
    }

  private:
    MJZ_CX_AL_FN success_t malloc_p(MJZ_WILL_USE uintlen_t min_size) noexcept {
      block_info blk{};
      const alloc_base_ref &ref = *alloc_ptr();
      blk = ref.allocate_bytes(min_size, alloc_info_v(ref));

      bool succuss = !!blk.ptr;
      m.reduce_rc_on_manager_destruction = succuss;
      // as far as i know nullptr is aligned
      m.heap_data_ptr = mjz::assume_aligned<non_threaded_rf_block>(blk.ptr);
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
              !succuss || std::cmp_less_equal(blk.length, min_size));
      return succuss;
    }
    MJZ_CX_AL_FN uintlen_t minsize_calc(uintlen_t min_size,
                                        bool round_up) noexcept {
      uintlen_t min_size_tr{min_size};
      min_size_tr = (min_size_tr / threaded_rf_block) +
                    uintlen_t(!!(min_size_tr % threaded_rf_block));
      min_size_tr *= threaded_rf_block;
      min_size_tr += branchless_teranary<uintlen_t>(
          !(cow_threaded_threshold() < min_size_tr), 0, threaded_rf_block);
      uintlen_t min_size_ntr{min_size};
      min_size_ntr = (min_size_ntr / non_threaded_rf_block) +
                     uintlen_t(!!(min_size_ntr % non_threaded_rf_block));
      min_size_ntr *= non_threaded_rf_block;
      min_size_ntr += non_threaded_rf_block;
      min_size =
          branchless_teranary(!get_is_threaded(), min_size_ntr, min_size_tr);
      min_size = branchless_teranary(!round_up, min_size,
                                     uintlen_t(1)
                                         << log2_ceil_of_val_create(min_size));
      return min_size;
    }

  public:
    MJZ_CX_AL_FN success_t malloc(MJZ_WILL_USE uintlen_t min_size,
                                  bool round_up = true) noexcept {
      bool bad_ret = !min_size;
      bad_ret |= !alloc_ptr();
      bad_ret |= !!*this;
      if (bad_ret)
        MJZ_IS_UNLIKELY { return m.reduce_rc_on_manager_destruction && free(); }
      return u_malloc(min_size, round_up);
    }
    MJZ_CX_AL_FN success_t u_malloc(MJZ_WILL_USE uintlen_t min_size,
                                    bool round_up = true) noexcept {
      return malloc_p(minsize_calc(min_size, round_up));
    }

  private:
    MJZ_CX_AL_FN success_t free_p() noexcept {
      asserts(asserts.assume_rn, !!*this);
      if constexpr (!is_ownerized_v_) {
        if (!deinit_heap_and_ask()) {
          return true;
        }
      }
      return u_free_p();
    }

    MJZ_CX_AL_FN success_t u_free_p() noexcept {
      const alloc_base_ref &ref = *alloc_ptr();
      return ref.deallocate_bytes(block_info{m.heap_data_ptr, m.heap_data_size},
                                  alloc_info_v(ref));
    }

  public:
    MJZ_CX_AL_FN void u_must_free() noexcept {
      asserts(asserts.assume_rn, free_p());
      unsafe_clear();
    }
    MJZ_CX_AL_FN success_t free() noexcept {
      if (!!*this) {
        asserts(asserts.assume_rn, free_p());
      }
      unsafe_clear();
      return true;
    }
    MJZ_CX_AL_FN void must_free() noexcept {
      asserts(asserts.assume_rn, free());
    }
    MJZ_CX_AL_FN uintlen_t cow_threaded_threshold() noexcept {
      if (!m.cow_threaded_threshold_temp_) {
        m.cow_threaded_threshold_temp_ = alloc_ptr()->get_vtbl().cow_threashold;
      }
      return m.cow_threaded_threshold_temp_;
    }
    MJZ_CX_AL_FN uintlen_t buffer_overhead() const noexcept {
      return uintlen_t(m.heap_rc_ptr
                           ? m.heap_rc_ptr + sizeof(uintlen_t) - m.heap_data_ptr
                           : 0);
    }
    MJZ_CX_AL_FN
    char *get_heap_begin() const noexcept {
      return mjz::assume_aligned<non_threaded_rf_block>(m.heap_data_ptr) +
             buffer_overhead();
    }
    MJZ_CX_AL_FN char *steal_heap_begin(bool change_rc) noexcept {
      if (!to_non_owner(change_rc))
        return nullptr;
      auto ret = get_heap_begin();
      unsafe_clear();
      return ret;
    }
    MJZ_CX_AL_FN success_t to_non_owner(bool change_rc = true) noexcept {
      if (!*this)
        return false;
      if (m.reduce_rc_on_manager_destruction == false)
        return true;
      if (change_rc) {
        if (!free()) {
          return false;
        }
      }
      m.reduce_rc_on_manager_destruction = false;
      return true;
    }
    MJZ_CX_AL_FN success_t to_owner(bool change_rc = true) noexcept {
      if (!*this)
        return false;
      if (m.reduce_rc_on_manager_destruction == true)
        return true;
      if (!change_rc || !add_shareholder())
        return false;
      m.reduce_rc_on_manager_destruction = true;
      return true;
    }
    MJZ_CX_AL_FN uintlen_t get_heap_cap() const noexcept {
      return m.heap_data_size - buffer_overhead();
    }
    MJZ_CX_AL_FN bool get_is_threaded() const noexcept {
      static_assert(may_bool_t::err != is_threaded_v_);
      if constexpr (may_bool_t::idk == is_threaded_v_) {
        return m.is_threaded;
      } else {
        return !!uint8_t(is_threaded_v_);
      }
    }

    MJZ_CX_AL_FN operator bool() const noexcept { return !!m.heap_data_ptr; }

    MJZ_CX_AL_FN bool can_add_shareholder() const noexcept {
      return m.heap_rc_ptr && !m.is_owenrized;
    }
    MJZ_CX_AL_FN bool might_share() const noexcept { return !!m.heap_rc_ptr; }

    MJZ_CX_AL_FN success_t add_shareholder() noexcept {
      if (!can_add_shareholder())
        return !!*this;
      asserts(asserts.assume_rn, !m.is_owenrized);
      temp_layout_t{m}.perform_ref(
          [](auto &ref) noexcept {
            ref.fetch_add(1, std::memory_order_acquire);
          },
          [](auto &ref) noexcept { ++ref; });
      return true;
    }
    MJZ_CX_AL_FN void unsafe_clear() noexcept {
      m.heap_data_ptr = nullptr;
      m.heap_rc_ptr = nullptr;
      m.heap_data_size = 0;
      m.reduce_rc_on_manager_destruction = false;
      m.cow_threaded_threshold_temp_ = false;
    }

  private:
    MJZ_CX_AL_FN bool is_owner_heap() const noexcept {
      char_storage_as_temp_t<uintlen_t> var{
          m.heap_data_ptr, non_threaded_rf_block, non_threaded_rf_block};
      if (!var)
        return false;

      if (!get_is_threaded()) {
        return *var < 2;
      }
      refcr_t ref_count{*var};
      return (ref_count).load(std::memory_order_acquire) < 2;
    }

  public:
    MJZ_CX_AL_FN bool is_owner() const noexcept {
      if (!can_add_shareholder())
        return !!*this;
      if (m.is_owenrized)
        return true;
      return is_owner_heap();
    }

  private:
    MJZ_CX_AL_FN bool
    remove_shareholder_then_check_has_no_owner_heap() noexcept {
      return temp_layout_t{m}.perform_ref(
                 [](auto &ref) noexcept -> uintlen_t {
                   return ref.fetch_sub(1, std::memory_order_acq_rel) - 1;
                 },
                 [](auto &ref) noexcept -> uintlen_t { return --ref; }) < 1;
    }

  public:
    MJZ_CX_AL_FN bool remove_shareholder_then_check_has_no_owner() noexcept {
      if (!*this)
        return false;
      if (m.is_owenrized)
        return true;
      if (!can_add_shareholder())
        return true;
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
    MJZ_CX_AL_FN
    str_heap_manager_t(const alloc_base_ref &alloc, bool can_share_,
                       bool is_threaded_, bool is_owenrized_,
                       bool add_rc_on_manager_construction,
                       bool reduce_rc_on_manager_destruction, char *heap_begin,
                       uintlen_t capacity) noexcept
        : str_heap_manager_t{alloc, is_threaded_, is_owenrized_} {
      asserts(asserts.assume_rn, !!heap_begin && !!capacity);
      m.heap_data_ptr = mjz::assume_aligned<non_threaded_rf_block>(heap_begin);
      m.heap_data_size = capacity;
      if (can_share_) {
        uintlen_t overhead{is_threaded_ ? threaded_rf_block
                                        : non_threaded_rf_block};
        m.heap_data_size += overhead;
        m.heap_rc_ptr = m.heap_data_ptr - sizeof(uintlen_t);
        m.heap_data_ptr -= overhead;
      }
      m.reduce_rc_on_manager_destruction = reduce_rc_on_manager_destruction;
      if (add_rc_on_manager_construction) {
        add_shareholder();
      }
    }

    MJZ_CX_AL_FN str_heap_manager_t(str_heap_manager_t &&obj) noexcept
        : m{std::exchange(obj.m,
                          m_t{obj.m.alloc_ptr(), obj.get_is_threaded()})} {}
    MJZ_CX_AL_FN str_heap_manager_t &
    operator=(str_heap_manager_t &&obj) noexcept {
      must_free();
      m = std::exchange(obj.m, m_t{obj.m.alloc_ptr(), obj.get_is_threaded()});
    }
    MJZ_CX_AL_FN ~str_heap_manager_t() noexcept {
      if (!m.reduce_rc_on_manager_destruction) {
        return;
      }
      must_free();
    }
  };

} // namespace mjz::bstr_ns

#endif // MJZ_BYTE_STRING_heap_LIB_HPP_FILE_
