#include "generic_alloc.hpp"
#ifndef MJZ_ALLOCS_bump_alloc_FILE_HPP_
#define MJZ_ALLOCS_bump_alloc_FILE_HPP_
namespace mjz ::allocs_ns {
template <version_t version_v, bool has_lock = true>
struct stack_alloc_t {
  using alloc_t = stack_alloc_t;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;
  using heap_block_t = block_info;
  using strategy_t = alloc_info;
  char *buffer;
  uintlen_t size;
  uintlen_t align;
  struct m_t {
    char *stack_buffer_ptr{};
    uintlen_t stack_buffer_align{};
    uintlen_t stack_buffer_len{};
    uintlen_t index_of_last_end{};
  };
  struct obj_t {
    m_t m{};

    template <class>
    friend class mjz_private_accessed_t;

   private:
    MJZ_CX_FN auto lock_gaurd(bool is_threaded) const noexcept {
      return lock_details_ns::lock_gaurd_maker<version_v>(
          is_threaded, mutex_byte(), has_lock);
    }
    MJZ_CX_FN char *mutex_byte() const noexcept
      requires(has_lock)
    {
      return m.stack_buffer_ptr ? m.stack_buffer_ptr + m.stack_buffer_len : 0;
    }

    MJZ_CX_ND_FN friend bool operator==(const obj_t &a,
                                        const obj_t &b) noexcept = delete;

   public:
    MJZ_CX_ND_FN bool is_owner(const heap_block_t &blk,
                               strategy_t) const & noexcept {
      return mjz::memory_has_overlap(blk.ptr, blk.length, m.stack_buffer_ptr,
                                     m.stack_buffer_len);
    }
    MJZ_CX_ND_FN success_t deallocate(heap_block_t &&, strategy_t) & noexcept {
      return true;
    }
    MJZ_CX_ND_FN heap_block_t allocate(uintlen_t minsize,
                                       strategy_t strategy) & noexcept {
      auto l = lock_gaurd(strategy.is_thread_safe);
      heap_block_t ret{};
      if (!minsize || !l) {
        return ret;
      }

      auto align_v = strategy.get_alignof_z();
      if (!strategy.uses_munual_alignment && align_v < 2) {
        return ret;
      }
      uintptr_t r{};
      MJZ_IFN_CONSTEVAL {
        r = (reinterpret_cast<uintptr_t>(m.stack_buffer_ptr) +
             m.index_of_last_end) %
            align_v;
      }
      else {
        if (align_v > m.stack_buffer_align) return ret;
        r = m.index_of_last_end % align_v;
      }
      m.index_of_last_end += r ? align_v - r : 0;
      if (m.index_of_last_end > m.stack_buffer_len) {
        m.index_of_last_end = m.stack_buffer_len;
      }
      if (m.index_of_last_end + minsize > m.stack_buffer_len) {
        return ret;
      }
      ret.length = minsize;
      ret.ptr = &m.stack_buffer_ptr[m.index_of_last_end];
      m.index_of_last_end += minsize;
      return ret;
    }

   public:
    MJZ_CX_FN obj_t(alloc_t a) noexcept {
      if (!(a.buffer && a.size)) return;
      m.stack_buffer_len = a.size;
      m.stack_buffer_ptr = a.buffer;
      m.stack_buffer_align = log2_of_val_to_val(log2_of_val_create(a.align));
      if constexpr (has_lock) {
        m.stack_buffer_len--;
        *mutex_byte() = 0;
      }
    }
  };
};
};  // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_bump_alloc_FILE_HPP_