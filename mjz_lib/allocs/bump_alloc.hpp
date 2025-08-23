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
  struct obj_t {
    MJZ_NO_MV_NO_CPY(obj_t);
    fast_alloc_chache_t<version_v> &m;

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
      return m.monotonic_ptr ? m.monotonic_ptr + m.monotonic_left : nullptr;
    }

    MJZ_CX_ND_FN friend bool operator==(const obj_t &a,
                                        const obj_t &b) noexcept = delete;

   public:
    MJZ_CX_ND_FN bool is_owner(const heap_block_t &blk,
                               strategy_t) const & noexcept {
      return memory_is_inside(m.monotonic_begin,
                              uintlen_t(m.monotonic_ptr - m.monotonic_begin),
                              blk.ptr, blk.length);
    }
    MJZ_CX_ND_FN success_t deallocate(heap_block_t &&, strategy_t) & noexcept {
      return true;
    }
    MJZ_CX_ND_FN heap_block_t allocate(uintlen_t minsize,
                                       strategy_t strategy) & noexcept {
      auto l = lock_gaurd(strategy.is_thread_safe);
      if (!minsize || !l) {
        return {};
      }
      m.can_use_monotonic = true;
      std::span<char> ret_ =
          m.monotonic_allocate(minsize, strategy.get_alignof_z());
      m.can_use_monotonic = false;
      return {ret_.data(), std::min(minsize, ret_.size())};
    }

   public:
    MJZ_CX_FN obj_t(alloc_t a, alloc_base &self) noexcept
        : m{[&self, &a]() noexcept -> auto & {
            if (!(a.buffer && a.size)) return self.alloc_chache;
            alloc_vtable_t<version_v> table = self.vtable;
            table.default_info.allocation_mode_val = uint16_t(
                alias_t<
                    typename alloc_info::allocation_mode_e>::monotonic_mode);
            table.default_info.is_thread_safe = 0;
            fast_alloc_chache_t<version_v> cache_{};

            cache_.can_use_stack = true;
            cache_.can_use_monotonic = true;
            cache_.monotonic_begin = a.buffer;
            cache_.monotonic_ptr = a.buffer;
            cache_.monotonic_left = a.size;
            cache_.monotonic_log2_align = log2_of_val_create(a.align);
            cache_.stack_log2_align = cache_.monotonic_log2_align;

            if constexpr (has_lock) {
              cache_.monotonic_ptr[--cache_.monotonic_left] = 0;
              cache_.can_use_stack = false;
              cache_.can_use_monotonic = false;
            } else {
              std::span<char> stack_ = cache_.monotonic_allocate(
                  cache_.monotonic_left >> 1, uintlen_t(1)
                                                  << cache_.stack_log2_align);
              cache_.stack_left = stack_.size();
              cache_.stack_ptr = stack_.data();
              cache_.stack_begin = stack_.data();
            }
            std::destroy_at(&self);
            std::construct_at(&self, table)->alloc_chache = cache_;
            return self.alloc_chache;
          }()} {}
  };
};
};  // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_bump_alloc_FILE_HPP_