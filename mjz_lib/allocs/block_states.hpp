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

#ifndef MJZ_ALLOCS_block_states_FILE_HPP_
#define MJZ_ALLOCS_block_states_FILE_HPP_
namespace mjz ::allocs_ns {
template <version_t version_v>
struct blk_state_t {
  char *bits_of_block_aliveness_metadata_ptr{};
  uintlen_t num_blocks{};
  struct block_range_t {
    uintlen_t begin_index{};
    uintlen_t len{};
  };
  struct blocks_ncx_info {
    uintlen_t align_mask{};
    const void *blocks_ptr{};
    uintlen_t block_size{};
    MJZ_CX_AL_FN bool is_aligned(uintlen_t i) const noexcept {
      MJZ_IF_CONSTEVAL { return true; }
      else {
        uintlen_t ptr =
            reinterpret_cast<uintptr_t>(blocks_ptr) + block_size * i;
        return (~align_mask & ptr) == ptr;
      }
    }
  };
  using cache_as_t = char;
  struct cache_t {
    alignas(sizeof(cache_as_t)) char buffer[sizeof(cache_as_t)];
  };
  struct allocation_cache_t {
    cache_t cache{};
    uintlen_t num_blocks{};
    uintlen_t num_bytes{};
    uintlen_t num_lines{};
    uintlen_t index_of_line{};
    char *bits_of_block_aliveness_metadata_ptr{};
    bool release{};

    MJZ_NO_MV_NO_CPY(allocation_cache_t);
    MJZ_CX_AL_FN allocation_cache_t(blk_state_t *This) noexcept
        : allocation_cache_t(&std::as_const(*This)) {
      release = true;
    }
    MJZ_CX_AL_FN allocation_cache_t(const blk_state_t *This) noexcept {
      bits_of_block_aliveness_metadata_ptr =
          This->bits_of_block_aliveness_metadata_ptr;
      num_blocks = This->num_blocks;
      num_bytes = (num_blocks / 8) + uintlen_t(!!(num_blocks % 8));
      num_lines = (num_bytes / sizeof(cache)) +
                  uintlen_t(!!(num_bytes % sizeof(cache)));
      refresh(0);
    }
    MJZ_CX_AL_FN ~allocation_cache_t() noexcept { flush(); }
    MJZ_CX_AL_FN void flush() noexcept {
      if (!release) return;
      uintlen_t real_i = index_of_line * sizeof(cache);
      memcpy(&this->bits_of_block_aliveness_metadata_ptr[real_i], cache.buffer,
             std::min(num_bytes - real_i, sizeof(cache)));
    }
    MJZ_CX_AL_FN void refresh(uintlen_t line_n) noexcept {
      flush();
      uintlen_t real_i = line_n * sizeof(cache);

      memcpy(cache.buffer, &this->bits_of_block_aliveness_metadata_ptr[real_i],
             std::min(num_bytes - real_i, sizeof(cache)));
      index_of_line = line_n;
    }
    // no bounds checks!
    MJZ_CX_AL_FN char &operator[](uintlen_t i) noexcept {
      uintlen_t line_n = i / sizeof(cache);
      if (line_n != index_of_line) MJZ_IS_UNLIKELY {
          refresh(line_n);
        }
      return cache.buffer[i % sizeof(cache)];
    }  // unsafe - no refresh
    MJZ_CX_AL_FN char &operator()(uintlen_t i) noexcept {
      return cache.buffer[i];
    }
  };
  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_AL_FN
  block_range_t get_best_avalible_block_range(
      bool is_best_fit, blocks_ncx_info info,
      uintlen_t min_number_of_blocks) const noexcept {
    block_range_t last_good_range{get_first_avalible_block_range(
        is_best_fit, info, min_number_of_blocks, 0)};
    block_range_t best_fit_range{last_good_range};
    if (!is_best_fit) {
      return best_fit_range;
    }
    for (;;) {
      last_good_range = get_first_avalible_block_range(
          is_best_fit, info, min_number_of_blocks,
          last_good_range.begin_index + last_good_range.len);
      if (!last_good_range.len) {
        return best_fit_range;
      }
      if (last_good_range.len < best_fit_range.len) {
        best_fit_range = last_good_range;
      }
    }
  }
  /* can probably be more optimized by simd */ /* can probably be more optimized
                                                  by simd */
  MJZ_CX_AL_FN
  block_range_t cx_get_first_avalible_block_range(
      blocks_ncx_info info, bool is_best_fit, uintlen_t min_number_of_blocks,
      const uintlen_t search_begin_index) const noexcept {
    uintlen_t index = search_begin_index;
    allocation_cache_t bits_begin = this;
    auto align_the_block_at_index = [&]() noexcept {
      for (; index < num_blocks && !info.is_aligned(index); index++);
      ;
      return index < num_blocks;
    };
    if (!align_the_block_at_index()) {
      return {};
    }
    uintlen_t begin_index{index};
    if (is_best_fit) {
      while (index < num_blocks) {
        if (bool(bits_begin[index / 8] & char(1 << (index % 8)))) {
          index++;
          continue;
        }
        if (min_number_of_blocks <= index - begin_index) {
          return {begin_index, index - begin_index};
        }
        index++;
        if (!align_the_block_at_index()) {
          return {};
        }
        begin_index = index;
      }
      if (min_number_of_blocks <= index - begin_index) {
        return {begin_index, index - begin_index};
      }
      return {};
    }
    /*
     * FAST MODE , UNREADABLE CODE, MORE FRAGMENTATION!!
     */
    if (!min_number_of_blocks) {
      return {};
    }
    min_number_of_blocks--;
    uintlen_t num_blocks_til_now{0};
    while (index < num_blocks) {
      bool hasnt_block = !bool(bits_begin[index / 8] & char(1 << (index % 8)));
      index++;
      if (hasnt_block) {
        for (; index < num_blocks && !info.is_aligned(index); index++);
        ;
        num_blocks_til_now = 0;
        begin_index = index;
        continue;
      }
      num_blocks_til_now++;
      if (min_number_of_blocks < num_blocks_til_now) {
        break;
      }
    }
    if (min_number_of_blocks < num_blocks_til_now) {
      return {begin_index, num_blocks_til_now};
    }
    return {};
  }

  MJZ_CX_FN
  block_range_t get_first_avalible_block_range(
      bool is_best_fit, blocks_ncx_info info, uintlen_t min_number_of_blocks,
      const uintlen_t search_begin_index = 0) const noexcept {
#if 0
        MJZ_IFN_CONSTEVAL {
      bool is_already_aligned =
          (info.block_size & info.align_mask) == info.align_mask;
      is_already_aligned &= info.is_aligned(0);
      if (!is_already_aligned) {
        return ncx_get_first_avalible_block_range(info, min_number_of_blocks,
                                                  search_begin_index);
      }
      return ncx_get_first_avalible_block_range(
          blocks_ncx_info{}, min_number_of_blocks, search_begin_index);
    }
#endif
    return cx_get_first_avalible_block_range(
        info, is_best_fit, min_number_of_blocks, search_begin_index);
  }
  MJZ_CX_AL_FN
  block_range_t set_block_range_bits(const block_range_t range,
                                     bool val) noexcept {
    if (!range.len) return {};
    allocation_cache_t bits_begin = this;
    uintlen_t index{range.begin_index};
    uintlen_t end_index{range.begin_index + range.len};

    for (; index < end_index; index++) {
      auto &byte = bits_begin[index / 8];
      const auto mask = char(1 << (index % 8));
      byte &= ~mask;
      byte |= val ? 0 : mask;
    }
    return range;
  }
  MJZ_CX_FN
  block_range_t alloc_block_range(block_range_t range) noexcept {
    return set_block_range_bits(range, true);
  }
  MJZ_CX_FN block_range_t
  dealloc_block_range_val(block_range_t range) noexcept {
    return set_block_range_bits(range, false);
  }

 public:
  MJZ_CX_ND_FN
  block_range_t alloc_block_range(uintlen_t min_num_blocks,
                                  uintlen_t max_mum_blocks, bool is_best_fit,
                                  blocks_ncx_info info) noexcept {
    if (min_num_blocks > max_mum_blocks) {
      return {};
    }
    auto ret = get_best_avalible_block_range(is_best_fit, info, min_num_blocks);
    ret.len = std::min(ret.len, max_mum_blocks);
    if (!ret.len) {
      ret.begin_index = 0;
    }
    return alloc_block_range(ret);
  }
  MJZ_CX_ND_FN
  block_range_t alloc_block_range(uintlen_t exact_num_blocks, bool is_best_fit,
                                  blocks_ncx_info info) noexcept {
    return alloc_block_range(exact_num_blocks, exact_num_blocks, is_best_fit,
                             info);
  }

  MJZ_CX_FN
  void dealloc_block_range(block_range_t range) noexcept {
    dealloc_block_range_val(range);
  }
  MJZ_CX_FN
  void init() noexcept {
    memset(bits_of_block_aliveness_metadata_ptr,
           num_blocks / 8 + uintlen_t(bool(num_blocks % 8)), 0);
    // the reason is that the last block's byte must be 1 only for its blocks
    dealloc_block_range(block_range_t{.len = num_blocks});
  }
  template <bool do_mem_restart = false>
  MJZ_CX_FN success_t deinit() noexcept {
    MJZ_RELEASE {
      if constexpr (do_mem_restart) {
        init();
      }
    };
    if constexpr (!MJZ_IN_DEBUG_MODE) {
      MJZ_IFN_CONSTEVAL { return true; }
    }
    allocation_cache_t ptr = this;
    auto len(num_blocks / 8 + uintlen_t(bool(num_blocks % 8)));
    for (uintlen_t i{}; i < len; i++) {
      if (!ptr[i]) return false;
    }
    return true;
  }
  MJZ_CX_FN static uintlen_t container_metadata_size(uintlen_t count) noexcept {
    return ((count / 8) + !!(count % 8));
  }
  MJZ_CX_FN static uintlen_t container_data_size(uintlen_t count,
                                                 uintlen_t index) noexcept {
    return (count * (uintlen_t(1) << index));
  }
  MJZ_CX_FN static uintlen_t container_total_size(uintlen_t count,
                                                  uintlen_t index) noexcept {
    return container_metadata_size(count) + container_data_size(count, index);
  }
  template <uintlen_t Number_of_containers>
  MJZ_CX_FN static std::array<uintlen_t, Number_of_containers>
  calculate_block_count_for_each_container(
      uintlen_t avaible_mem_byte_count) noexcept {
    constexpr uintlen_t one{1};
    constexpr uintlen_t bits_in_bytes{8};
    constexpr uintlen_t log_of_bits_in_bytes{3};
    std::array<uintlen_t, Number_of_containers> ret{};
    static_assert(Number_of_containers <=
                  ((sizeof(uintlen_t) * bits_in_bytes) - log_of_bits_in_bytes));

    constexpr uintlen_t pow2_sq_sum{pow_of_2(Number_of_containers) - 1};
    uintlen_t base_value{uintlen_t((bits_in_bytes * avaible_mem_byte_count) /
                                   (Number_of_containers + (8 * pow2_sq_sum)))};
    uintlen_t remainder{avaible_mem_byte_count - (pow2_sq_sum * base_value)};
    remainder -= (base_value / bits_in_bytes + !!(base_value % bits_in_bytes)) *
                 Number_of_containers;
    auto do_with_bad_base_vals = [&]() noexcept {
      for (uintlen_t i{}; i < Number_of_containers; i++) {
        uintlen_t real_index{Number_of_containers - i - one};
        auto &obj = ret[real_index];
        obj = base_value;
        uintlen_t extract_val_of_remainder{};
        do {
          extract_val_of_remainder =
              !(obj % bits_in_bytes) + pow_of_2(real_index);
          if (extract_val_of_remainder > remainder) {
            break;
          }
          obj++;
          remainder -= extract_val_of_remainder;
        } while (true);
      }
    };
    if (!(base_value % bits_in_bytes)) {
      do_with_bad_base_vals();
      return ret;
    }
    for (uintlen_t i{}; i < Number_of_containers; i++) {
      ret[i] = base_value;
      uintlen_t pow2_i{pow_of_2(i)};
      if (remainder & pow2_i) {
        ret[i]++;
        remainder -= pow2_i;
      }
    }
    if (remainder) MJZ_MOSTLY_UNLIKELY {
        do_with_bad_base_vals();
        return ret;
      }

    return ret;
  }

  MJZ_CX_FN static uintlen_t calculate_block_count_for_single_blk(
      uintlen_t avaible_mem_byte_count, uintlen_t blksize) noexcept {
    return avaible_mem_byte_count * 8 / (1 + blksize * 8);
  };
  MJZ_CX_FN static uintlen_t calculate_memsize_for_single_blk(
      uintlen_t blk_count, uintlen_t blksize) noexcept {
    return (blk_count / 8) + uintlen_t(!!(blk_count % 8)) + blk_count * blksize;
  };

  MJZ_MCONSTANT(auto)
  pow_of_2 = [](std::integral auto x) noexcept {
    return uint_sizeof_t<std::max(sizeof(x), sizeof(uintlen_t))>(1) << x;
  };
  template <uint64_t percition>
  MJZ_CX_ND_FN static auto score_of_allocation(uint64_t size,
                                               uint64_t sizeof_block) noexcept
      -> uint64_t {
    uint64_t added_score{};
    if ((sizeof_block << 2) < size) {
      added_score = (size - (sizeof_block << 2)) << 1;
    }
    uint64_t num_unused_bytes = size % sizeof_block;
    num_unused_bytes = num_unused_bytes ? sizeof_block - num_unused_bytes : 0;
    return added_score + (size / sizeof_block) + 8 * (num_unused_bytes);
  };
  template <uintlen_t Number_of_containers>
  struct scored_index_t {
    uintlen_t index{};
    uint64_t score{};

    MJZ_CX_FN scored_index_t() noexcept {}
    MJZ_CX_FN scored_index_t(uintlen_t size_, uintlen_t index_) noexcept
        : index(index_),
          score(score_of_allocation<pow_of_2(Number_of_containers)>(
              size_, pow_of_2(index))) {}
    MJZ_CX_FN auto operator<=>(const scored_index_t &b) const noexcept {
      return score <=> b.score;
    }
  };
  template <uintlen_t Number_of_containers>
  MJZ_CX_FN static std::array<scored_index_t<Number_of_containers>,
                              Number_of_containers>
  get_scored_index_es(uintlen_t size) noexcept {
    std::array<scored_index_t<Number_of_containers>, Number_of_containers>
        array{};
    for (uintlen_t i{0}; i < Number_of_containers; i++) {
      array[i] = scored_index_t<Number_of_containers>{size, i};
    }
    for (uintlen_t i{0}; i < Number_of_containers; i++) {
      for (uintlen_t j{0}; j < i; j++) {
        if (array[i] < array[j]) std::swap(array[i], array[j]);
      }
    }
    return array;
  }
  template <uintlen_t Number_of_containers>
  MJZ_CX_FN static std::array<uintlen_t, Number_of_containers>
  get_best_index_es(uintlen_t size) noexcept {
    std::array<uintlen_t, Number_of_containers> array{};
    auto temp = get_scored_index_es<Number_of_containers>(size);
    for (uintlen_t j{0}; j < Number_of_containers; j++) {
      array[j] = temp[j].index;
    }
    return array;
  }
  struct byte_bits {
    bool bit0 : 1;
    bool bit1 : 1;
    bool bit2 : 1;
    bool bit3 : 1;
    bool bit4 : 1;
    bool bit5 : 1;
    bool bit6 : 1;
    bool bit7 : 1;
  };
  static_assert(sizeof(byte_bits) == 1);
  //// unsafe
  struct alingment_cache_t {
    cache_t cache{};
    uintlen_t num_blocks{};
    uintlen_t num_bytes{};
    uintlen_t num_lines{};
    uintlen_t index_of_line{};
    blocks_ncx_info info{};

    MJZ_NO_MV_NO_CPY(alingment_cache_t);
    MJZ_CX_AL_FN alingment_cache_t(const blk_state_t *This,
                                   blocks_ncx_info info_) noexcept
        : info(info_) {
      num_blocks = This->num_blocks;
      num_bytes = (num_blocks / 8) + uintlen_t(!!(num_blocks % 8));
      num_lines = (num_bytes / sizeof(cache)) +
                  uintlen_t(!!(num_bytes % sizeof(cache)));
      refresh(0);
    }

    MJZ_CX_AL_FN void refresh(uintlen_t line_n) noexcept {
      uintlen_t real_i = line_n * sizeof(cache) * 8;
      for (auto &c : cache.buffer) {
        byte_bits values{info.is_aligned(real_i++), info.is_aligned(real_i++),
                         info.is_aligned(real_i++), info.is_aligned(real_i++),
                         info.is_aligned(real_i++), info.is_aligned(real_i++),
                         info.is_aligned(real_i++), info.is_aligned(real_i++)};
        c = std::bit_cast<char>(values);
      }
      index_of_line = line_n;
    }
    // no bounds checks!
    MJZ_CX_AL_FN char &operator()(uintlen_t i) noexcept {
      return cache.buffer[i];
    }
  };
  // this is a more brancless version
  MJZ_CX_AL_FN block_range_t ncx_get_first_avalible_block_range(
      blocks_ncx_info info, uintlen_t min_number_of_blocks,
      const uintlen_t search_begin_index) const noexcept {
    allocation_cache_t bits_begin = this;
    alingment_cache_t good_alignment{this, info};
    const uintlen_t cached_blocks = 8 * sizeof(cache_t);
    const uintlen_t cached_bytes = sizeof(cache_t);
    uintlen_t cache_index{search_begin_index / cached_blocks};
    const uintlen_t cache_num{bits_begin.num_lines};
    uintlen_t begin_index{cache_index * cached_blocks};
    uintlen_t currant_index{begin_index};
    uintlen_t index_in_cache{};
    cache_t cache_result{};
    {
      uintlen_t begin_index_local{begin_index};
      auto fnv = [&]() noexcept {
        return search_begin_index <= begin_index_local++;
      };
      for (auto &c : cache_result.buffer) {
        byte_bits values{fnv(), fnv(), fnv(), fnv(),
                         fnv(), fnv(), fnv(), fnv()};
        c = std::bit_cast<char>(values);
      }
    }
    /* note that out of bounds are viewed like allocated blocks*/
    while (true) {
      for (auto &c : cache_result.buffer) {
        c &= bits_begin(index_in_cache) | ~good_alignment(index_in_cache);
        index_in_cache++;
      }
      index_in_cache = 0;
      while (index_in_cache < cached_bytes) {
        uint8_t free_blocks = uint8_t(cache_result.buffer[index_in_cache++]);
        for (uint8_t i{}; i < 8; i++) {
          /* branchless transformation go brrrrr*/
          uintlen_t local_currant_index{currant_index},
              local_begin_index{begin_index};
          bool is_free = (free_blocks & 1);
          free_blocks >>= 1;
          bool return_if_not_free =
              min_number_of_blocks <= currant_index - begin_index;
          local_currant_index++;
          local_begin_index = local_currant_index;
          local_currant_index = branchless_teranary(
              !return_if_not_free, local_currant_index, currant_index);
          local_begin_index = branchless_teranary(
              !return_if_not_free, local_begin_index, begin_index);

          currant_index = branchless_teranary(!is_free, local_currant_index,
                                              currant_index + 1);
          begin_index =
              branchless_teranary(!is_free, local_begin_index, begin_index);
        }
      }
      if (min_number_of_blocks <= currant_index - begin_index) {
        break;
      }
      cache_index++;
      if (cache_index < cache_num) break;
      bits_begin.refresh(cache_index);
      good_alignment.refresh(cache_index);
      for (auto &c : cache_result.buffer) {
        c = uint8_t(-1);
      }
    }
    if (min_number_of_blocks <= currant_index - begin_index) {
      return {begin_index, currant_index - begin_index};
    }
    return {};
  }
};
};  // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_block_states_FILE_HPP_
