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
#ifndef MJZ_ALLOCS_page_alloc_FILE_HPP_
#define MJZ_ALLOCS_page_alloc_FILE_HPP_

MJZ_EXPORT namespace mjz ::allocs_ns {
  namespace stack_alloc_ns {
  template <version_t version_v, uintlen_t align_v_ = 16> struct stack_alloc_t {
    using alloc_t = stack_alloc_t;
    using alloc_base = alloc_base_t<version_v>;
    using block_info = block_info_t<version_v>;
    using alloc_info = alloc_info_t<version_v>;
    using ref_count = ref_count_t<version_v>;
    using alloc_speed = alloc_speed_t<version_v>;
    using alloc_ref = alloc_base_ref_t<version_v>;
    using heap_block_t = block_info;
    using strategy_t = alloc_info;
    using stack_ref_t =
        optional_ref_t<stack_allocator_meta_t<version_v, align_v_>>;
    stack_ref_t stack_refrence{};

    struct obj_t {
      stack_ref_t stack_refrence{};
      char *stack_begin_ptr{};
      uintlen_t stack_length{};
      template <class> friend class mjz_private_accessed_t;

    private:
      MJZ_CX_ND_FN friend bool operator==(const obj_t &a,
                                          const obj_t &b) noexcept = delete;

    public:
      MJZ_CX_ND_FN bool is_owner(const heap_block_t &blk,
                                 strategy_t) const & noexcept {
        return mjz::memory_is_inside(stack_begin_ptr, stack_length, blk.ptr,
                                     uintlen_t(!!blk.length));
      }
      MJZ_CX_ND_FN success_t deallocate(heap_block_t &&,
                                        strategy_t) & noexcept {
        return true;
      }
      MJZ_CX_ND_FN heap_block_t allocate(uintlen_t minsize,
                                         strategy_t strategy) & noexcept {
        bool bad = strategy.is_thread_safe;
        bad |= !minsize;
        bad |= !stack_refrence;
        bad |= align_v_ < strategy.cant_bother_with_good_size().get_alignof_z();
        heap_block_t ret{};
        if (bad) {
          return ret;
        }
        std::span<char> ret_ = stack_refrence->fn_alloca(minsize);
        ret.ptr = ret_.data();
        minsize = branchless_teranary(!strategy.allocate_exactly_minsize,
                                      ret_.size(), minsize);
        ret.length = std::min(minsize, ret_.size());
        return ret;
      }

    public:
      MJZ_CX_FN obj_t(alloc_t a, alloc_base &) noexcept {
        if (!a.stack_refrence)
          return;
        this->stack_refrence = a.stack_refrence;
        this->stack_begin_ptr = a.stack_refrence->sptr;
        this->stack_length =
            uintlen_t(a.stack_refrence->send - this->stack_begin_ptr);
      }
    };
  };
  }; // namespace stack_alloc_ns

  namespace page_alloc_ns {
  template <version_t version_v> struct page_alloc_t {
    using alloc_t = page_alloc_t;
    using alloc_base = alloc_base_t<version_v>;
    using block_info = block_info_t<version_v>;
    using alloc_info = alloc_info_t<version_v>;
    using ref_count = ref_count_t<version_v>;
    using alloc_speed = alloc_speed_t<version_v>;
    using alloc_ref = alloc_base_ref_t<version_v>;
    using heap_block_t = block_info;
    using strategy_t = alloc_info;
    using meta_ref_t = optional_ref_t<simple_page_alloc_t<version_v>>;
    meta_ref_t meta_refrence{};
    struct obj_t {
      meta_ref_t meta_refrence{};
      char *data_begin_ptr{};
      uintlen_t data_length{};
      template <class> friend class mjz_private_accessed_t;

    private:
      MJZ_CX_ND_FN friend bool operator==(const obj_t &a,
                                          const obj_t &b) noexcept = delete;

    public:
      MJZ_CX_ND_FN bool is_owner(const heap_block_t &blk,
                                 strategy_t) const & noexcept {
        return mjz::memory_is_inside(data_begin_ptr, data_length, blk.ptr,
                                     uintlen_t(!!blk.length));
      }
      MJZ_CX_ND_FN success_t deallocate(heap_block_t &&blk,
                                        strategy_t strategy) & noexcept {
        bool bad = strategy.cant_bother_with_good_size().is_thread_safe;
        bad |= !blk.length;
        bad |= !meta_refrence;
        heap_block_t ret{};
        if (bad) {
          return true;
        }
        meta_refrence->deallocate(std::span<char>{blk.ptr, blk.length},
                                  strategy.get_alignof_z());
        return true;
      }
      MJZ_CX_ND_FN heap_block_t allocate(uintlen_t minsize,
                                         strategy_t strategy) & noexcept {
        bool bad = strategy.cant_bother_with_good_size().is_thread_safe;
        bad |= !minsize;
        bad |= !meta_refrence;
        heap_block_t ret{};
        if (bad) {
          return ret;
        }
        std::span<char> ret_ =
            meta_refrence->allocate(minsize, strategy.get_alignof_z());
        ret.ptr = ret_.data();
        minsize = branchless_teranary(
            !strategy.allocate_exactly_minsize ,ret_.size(), minsize);
        ret.length = std::min(minsize, ret_.size());
        return ret;
      }

    public:
      MJZ_CX_FN obj_t(alloc_t a, alloc_base &) noexcept {
        if (!a.meta_refrence)
          return;
        this->meta_refrence = a.meta_refrence;
        this->data_begin_ptr = a.meta_refrence->data_ptr.pages.data();
        this->data_length = a.meta_refrence->data_ptr.pages.size_bytes();
      }
    };
  };
  }; // namespace page_alloc_ns

} // namespace mjz::allocs_ns
#endif // MJZ_ALLOCS_bump_alloc_FILE_HPP_