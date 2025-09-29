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
#include "block_states.hpp"

#ifndef MJZ_ALLOCS_blocks_alloc_FILE_HPP_
#define MJZ_ALLOCS_blocks_alloc_FILE_HPP_
MJZ_EXPORT namespace mjz ::allocs_ns {
  template <version_t version_v, bool has_lock = true>
  struct bucket_alloc_info_t {
    using alloc_t = bucket_alloc_info_t;

    using alloc_base = alloc_base_t<version_v>;
    using block_info = block_info_t<version_v>;
    using alloc_info = alloc_info_t<version_v>;
    using ref_count = ref_count_t<version_v>;
    using alloc_speed = alloc_speed_t<version_v>;
    using alloc_ref = alloc_base_ref_t<version_v>;
    using blk_state = blk_state_t<version_v>;
    char *ptr{};
    uintlen_t length{};
    uintlen_t align{};
    uintlen_t blk_size{};
    struct obj_t {
      struct m_t : void_struct_t {
        blk_state blk_states{};
        uintlen_t blk_size : (sizeof(uintlen_t) * 8 - 6){};
        uintlen_t log2_align : 6 {};
        char *data_buffer{};
      };
      m_t m{};
      MJZ_NO_MV_NO_CPY(obj_t);
      MJZ_CX_FN obj_t(alloc_t &&o, alloc_base &) noexcept {
        if (!o.ptr) {
          return;
        }
        m.log2_align = log2_of_val_create(o.align);
        m.blk_size = log2_of_val_to_val(log2_ceil_of_val_create(o.blk_size));
        uintlen_t num_blocks =
            m.blk_states.calculate_block_count_for_single_blk(
                o.length - uintlen_t(has_lock), o.blk_size);
        uintlen_t num_state_bytes =
            num_blocks / 8 + uintlen_t(!!(num_blocks % 8));
        m.blk_states.bits_of_block_aliveness_metadata_ptr =
            &o.ptr[o.length - num_state_bytes];
        if constexpr (has_lock) {
          MJZ_UNUSED auto &&mutex_byte =
              *(m.blk_states.bits_of_block_aliveness_metadata_ptr - 1);
          mutex_byte = 0;
        }
        m.blk_states.num_blocks = num_blocks;
        m.data_buffer = o.ptr;
        m.blk_states.init();
      }
      MJZ_CX_FN auto &&unsafe_get_handle() noexcept { return m; }
      MJZ_CX_FN auto &&unsafe_get_handle() const noexcept { return m; }

    private:
      MJZ_CX_FN bool operator==(const obj_t &other) const noexcept = delete;

    public:
      MJZ_CX_FN bool is_owner(MJZ_MAYBE_UNUSED const block_info &blk,
                              MJZ_MAYBE_UNUSED alloc_info ai) const noexcept {
        auto l = lock_gaurd(ai.is_thread_safe);
        if (!l)
          return {};
        return memory_is_inside(this->m.data_buffer,
                                this->m.blk_size *
                                    this->m.blk_states.num_blocks,
                                blk.ptr, blk.length);
      }

    private:
      using block_range_t = blk_state::block_range_t;
      MJZ_CX_FN auto lock_gaurd(bool is_threaded) const noexcept {
        auto p = m.blk_states.bits_of_block_aliveness_metadata_ptr;
        return lock_details_ns::lock_gaurd_maker<version_v>(
            is_threaded, p ? p - 1 : nullptr, has_lock);
      }
      MJZ_CX_ND_FN block_info just_allocate(uintlen_t size,
                                            size_t alloc_alignment,
                                            bool is_best_fit) noexcept {
        size_t align_val = log2_of_val_to_val(m.log2_align);
        if (!(m.blk_size && alloc_alignment && size && align_val))
          return {};
        bool dosnt_need_realign{alloc_alignment < align_val};
        MJZ_IF_CONSTEVAL {
          if (!dosnt_need_realign)
            return {};
        }
        block_range_t range = m.blk_states.alloc_block_range(
            size / m.blk_size + uintlen_t(!!(size % m.blk_size)), is_best_fit,
            alias_t<typename blk_state::blocks_ncx_info>{
                .align_mask{alloc_alignment - 1},
                .blocks_ptr{m.data_buffer},
                .block_size{m.blk_size},
            });
        if (!range.len) {
          return {};
        }
        return block_info{.ptr{&m.data_buffer[range.begin_index * m.blk_size]},
                          .length{range.len * m.blk_size}};
      }
      MJZ_CX_ND_FN success_t just_deallocate(block_info &&blk,
                                             bool len_is_exact,
                                             bool needs_realignment) noexcept {
        uintptr_t deltaptr = uintptr_t(blk.ptr - m.data_buffer);
        uintptr_t lenptr(blk.length);
        block_range_t range{.begin_index = deltaptr / m.blk_size,
                            .len = lenptr / m.blk_size};
        range.len += uintlen_t(!!(lenptr % m.blk_size));
        range.len += uintlen_t(len_is_exact) & uintlen_t(needs_realignment);
        m.blk_states.dealloc_block_range(std::move(range));
        return true;
      }

    public:
      MJZ_CX_FN block_info allocate(MJZ_MAYBE_UNUSED uintlen_t minsize,
                                    MJZ_MAYBE_UNUSED alloc_info ai) noexcept {
        auto l = lock_gaurd(ai.cant_bother_with_good_size().is_thread_safe);
        if (!l)
          return {};
        block_info ret{};
        bool needs_realignment{m.log2_align < ai.log2_of_align_val};
        if (!ai.uses_munual_alignment && needs_realignment) {
          return ret;
        }
        // ptr  shall point in the first block for a funcioning destructor.
        auto align_v = ai.get_alignof_z();
        ret = just_allocate(minsize + (needs_realignment ? m.blk_size : 0),
                            align_v, ai.uses_best_fit);
        if (!ret.ptr) {
          return ret;
        }
        uintptr_t remainder{};
        MJZ_IFN_CONSTEVAL {
          remainder = reinterpret_cast<uintptr_t>(ret.ptr) % align_v;
          asserts(asserts.assume_rn, remainder < m.blk_size);
          ret.ptr = &ret.ptr[remainder];
          ret.length -= remainder;
        }
        return ret;
      }
      MJZ_CX_FN
      success_t deallocate(MJZ_MAYBE_UNUSED block_info &&blk,
                           MJZ_MAYBE_UNUSED alloc_info ai) noexcept {
        auto l = lock_gaurd(ai.cant_bother_with_good_size().is_thread_safe);
        if (!l)
          return {};
        if (!blk.ptr)
          return false;
        bool needs_realignment{m.log2_align < ai.log2_of_align_val};
        if (!ai.uses_munual_alignment && needs_realignment) {
          return false;
        }
        bool len_is_exact{!!ai.allocate_exactly_minsize};

        return just_deallocate(std::move(blk), len_is_exact, needs_realignment);
      }
    };
  };
}; // namespace mjz::allocs_ns
#endif // MJZ_ALLOCS_blocks_alloc_FILE_HPP_