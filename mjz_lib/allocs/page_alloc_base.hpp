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
#include <span>

#include "../optional_ref.hpp"
#include "../versions.hpp"
#ifndef MJZ_ALLOCS_page_base_alloc_FILE_HPP_
#define MJZ_ALLOCS_page_base_alloc_FILE_HPP_
namespace mjz ::allocs_ns {

namespace page_alloc_ns {
MJZ_CONSTANT(uintlen_t)
val_page = 8 * sizeof(uintlen_t);
MJZ_CONSTANT(uintlen_t) log2_page = log2_of_val_create(val_page);
template <version_t>
struct block_range_t {
  uintlen_t page_index : sizeof(uintlen_t) * 8 - 2 * log2_page{};
  uintlen_t blk_index : log2_page{};
  uintlen_t blk_count : log2_page{};
};
template <version_t version_v>
struct pages_meta_t {
  using block_range = block_range_t<version_v>;
  std::span<uintlen_t> pages{};
  MJZ_CX_FN void release() const noexcept {
    MJZ_IF_CONSTEVAL {
      for (auto& x : pages) {
        x = 0;
      }
      return;
    }
    memset(reinterpret_cast<char*>(pages.data()), pages.size_bytes(), 0);
  }
  MJZ_CX_FN void deallocate(block_range range) const noexcept {
    uintlen_t& page = pages[range.page_index];
    uintlen_t page_mask = 1;
    page_mask <<= range.blk_count;
    page_mask--;
    page_mask <<= range.blk_index;
    page &= page_mask;
  }
  MJZ_CX_FN block_range allocate(uintlen_t blk_count) const noexcept {
    if (val_page < blk_count) {
      return {};
    }
    uintlen_t page_mask = 1;
    page_mask <<= blk_count;
    page_mask--;
    const uintlen_t page_room = val_page - blk_count;
    for (uintlen_t& page : pages) {
      for (uintlen_t i{}; i < val_page; i++) {
        bool bad{page_room <= i};
        uintlen_t page_mask_shift = alias_t<uintlen_t[2]>{page_mask, 0}[bad]
                                    << i;
        bad |= uintlen_t(page_mask_shift & page) != page_mask_shift;
        if (bad) {
          continue;
        }
        page |= page_mask_shift;
        return block_range{.page_index = uintlen_t(&page - pages.data()),
                           .blk_index = i,
                           .blk_count = blk_count};
      }
    }
    return {};
  }
};
template <version_t version_v>
struct data_meta_t {
  using block_range = block_range_t<version_v>;
  std::span<char> pages{};
  uintlen_t blk_align_log2{};

  MJZ_CX_FN data_meta_t(std::span<char> data = std::span<char>{},
                        uintlen_t align = 1) noexcept {
    blk_align_log2 = log2_of_val_create(uintlen_t(align));
    uintlen_t blk_count = data.size() >> blk_align_log2;
    pages = data.subspan(0, blk_count << blk_align_log2);
  }
  MJZ_CX_FN std::span<char> to_real(block_range range) const noexcept {
    if (!range.blk_count) return {};
    return pages.subspan((range.page_index << (blk_align_log2 + log2_page)) +
                             (range.blk_index << blk_align_log2),
                         range.blk_count << blk_align_log2);
  }
  MJZ_CX_FN block_range to_meta(std::span<char> real_meta) const noexcept {
    if (!real_meta.size()) return {};
    uintlen_t log2_align = blk_align_log2 + log2_page;
    uintlen_t byte_offset = uintlen_t(real_meta.data() - pages.data());
    uintlen_t size = real_meta.size();
    uintlen_t log2_blk = (log2_align - log2_page);
    size >>= log2_blk;
    uintlen_t blk_offset = size << log2_blk;
    size += (real_meta.size() != blk_offset);
    block_range ret{.page_index = uintlen_t{byte_offset >> log2_align},
                    .blk_count = size};
    ret.blk_index = (byte_offset - (ret.page_index << log2_align)) >> log2_blk;
    return ret;
  }

  MJZ_CX_FN bool is_owner(std::span<char> real_meta) const noexcept {
    return real_meta.size() &&
           memory_has_overlap(real_meta.data(), 1, pages.data(),
                              pages.size_bytes());
  }
};

template <version_t version_v>
struct simple_page_alloc_t {
  data_meta_t<version_v> data_ptr{};
  pages_meta_t<version_v> data_meta{};
  MJZ_NCX_FN std::span<char> allocate(uintlen_t count,
                                      uintlen_t align) const noexcept {
    if (data_ptr.blk_align_log2 < log2_of_val_create(align)) {
      return {};
    }
    uintlen_t blk_num = count >> data_ptr.blk_align_log2;
    blk_num += (blk_num << data_ptr.blk_align_log2) != count;
    return data_ptr.to_real(data_meta.allocate(blk_num));
  }
  MJZ_NCX_FN void deallocate(std::span<char> blk, uintlen_t) const noexcept {
    if (!blk.size()) return;
    data_meta.deallocate(data_ptr.to_meta(blk));
  }
  MJZ_CX_FN bool is_owner(std::span<char> blk, uintlen_t align) const noexcept {
    if (data_ptr.blk_align_log2 < log2_of_val_create(align)) {
      return false;
    }
    return data_ptr.is_owner(blk);
  }
};
};  // namespace page_alloc_ns
namespace stack_alloc_ns {

template <version_t version_v,
          uintlen_t align_v = 16 /*dont change 16 for defualt*/>
struct stack_allocator_meta_t {
  MJZ_CONSTANT(uintlen_t) align { 16 };
  using self_t = stack_allocator_meta_t;
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN std::span<char> fn_alloca(const uintlen_t min_size,
                                      const uintlen_t align_ = align) noexcept {
    uintlen_t size = min_size;
    const bool bad_align = !!(size & (align - 1));
    size &= ~(align - 1);
    size += alias_t<uintlen_t[2]>{0, align}[bad_align];
    const bool bad =
        bool(int(uintlen_t(send - sptr) < size) | int(align < align_));
    const std::span<char> dummy[2]{
        std::span<char>{std::assume_aligned<align>(sptr), size}};
    const auto ret = dummy[bad];
    sptr += ret.size();
    return ret;
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN void fn_dealloca(std::span<char>&& blk) noexcept {
    sptr = std::assume_aligned<align>(
        alias_t<alias_t<char*>[2]>{sptr, blk.data()}[!!blk.size()]);
  }

 private:
  struct private_tag_ {};

 public:
  struct stack_buffer_t {
    MJZ_NO_MV_NO_CPY_DC(stack_buffer_t);
    MJZ_CX_FN stack_buffer_t(self_t& This, uintlen_t size,
                             private_tag_) noexcept {
      blk = This.fn_alloca(size);
      This_ = This;
    }
    MJZ_CX_FN stack_buffer_t(auto&&...) noexcept = delete;
    MJZ_CX_FN ~stack_buffer_t() noexcept { This_->fn_dealloca(std::move(blk)); }

    MJZ_CX_FN const std::span<char>& operator*() & noexcept { return blk; }
    MJZ_CX_FN const std::span<char>* operator->() & noexcept { return &blk; }

   private:
    std::span<char> blk{};
    optional_ref_t<self_t> This_{};
  };

  MJZ_CX_FN stack_buffer_t make_alloc(uintlen_t size) noexcept {
    return stack_buffer_t{*this, size, private_tag_{}};
  }

  MJZ_CX_FN stack_allocator_meta_t(std::span<char> bytes = std::span<char>{},
                                   uintlen_t align_ = 0) noexcept {
    if (align_ < align) return;
    sptr = std::assume_aligned<align>(bytes.data());
    send = sptr + bytes.size();
  }

 public:
  char* send{};
  char* sptr{};
};

template <version_t version_v, uintlen_t buffer_size_v, uintlen_t align_v = 16>
struct areana_t : stack_allocator_meta_t<version_v, align_v> {
  MJZ_CX_FN areana_t() noexcept : stack_allocator_meta_t<version_v, align_v>{} {
    this->sptr = std::assume_aligned<align_v>(&(raw.raw_buffer_[0] = 0));
    this->send = this->sptr + buffer_size_v;
  }

 private:
  MJZ_NO_MV_NO_CPY(areana_t);
  union alignas(std::max<uintlen_t>(alignof(std::span<char>), align_v)) raw_t {
    char dummy{};
    char raw_buffer_[buffer_size_v];
  } raw{};
};

}  // namespace stack_alloc_ns
};  // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_bump_alloc_FILE_HPP_