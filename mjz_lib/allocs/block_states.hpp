#include "generic_alloc.hpp"

#ifndef MJZ_ALLOCS_block_states_FILE_HPP_
#define MJZ_ALLOCS_block_states_FILE_HPP_
namespace mjz ::allocs_ns {
template <version_t version_v>
struct blk_state_t {
  char* bits_of_block_aliveness_metadata_ptr{};
  uintlen_t num_blocks{};
  struct block_range_t {
    uintlen_t begin_index{};
    uintlen_t len{};
  };

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_FN
  block_range_t get_best_avalible_block_range(
      bool is_best_fit,
      callable_c<bool(uintlen_t begin_blocks_index) noexcept> auto&&
          is_a_good_alignment,
      uintlen_t min_number_of_blocks) const noexcept {
    block_range_t last_good_range{get_first_avalible_block_range(
        is_best_fit, is_a_good_alignment, min_number_of_blocks, 0)};
    block_range_t best_fit_range{last_good_range};
    if (!is_best_fit) {
      return best_fit_range;
    }
    for (;;) {
      last_good_range = get_first_avalible_block_range(
          is_best_fit, is_a_good_alignment, min_number_of_blocks,
          last_good_range.begin_index + last_good_range.len);
      if (!last_good_range.len) {
        return best_fit_range;
      }
      if (last_good_range.len < best_fit_range.len) {
        best_fit_range = last_good_range;
      }
    }
  }

  MJZ_CX_FN
  block_range_t get_first_avalible_block_range(
      bool is_best_fit,
      callable_c<bool(uintlen_t begin_blocks_index) noexcept> auto&&
          is_a_good_alignment,
      uintlen_t min_number_of_blocks,
      const uintlen_t search_begin_index = 0) const noexcept {
    uintlen_t index = search_begin_index;
    const char* bits_begin = bits_of_block_aliveness_metadata_ptr;
    auto align_the_block_at_index = [&]() noexcept {
      for (; index < num_blocks && !is_a_good_alignment(index); index++)
        ;
      ;
      return index < num_blocks;
    };
    if (!align_the_block_at_index()) {
      return {};
    }
    uintlen_t begin_index{index};
    if (is_best_fit) {
      while (index < num_blocks) {
        if (!bool(bits_begin[index / 8] & char(1 << (index % 8)))) {
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
      bool hasnt_block = bool(bits_begin[index / 8] & char(1 << (index % 8)));
      index++;
      if (hasnt_block) {
        MJZ_IF_CONSTEVAL {
          for (; index < num_blocks && !is_a_good_alignment(index); index++)
            ;
          ;
        }
        else {
          /*
           * invalid pointers(indexes) are passed , but they will never get
           * derefrenced!!. also , this is increadably unsafe , beacuse if the
           * is_a_good_alignment never returns true, this will loop forever.
           * but this will probably just be an easy optimization considering
           * valid uses.
           */
          for (; !is_a_good_alignment(index); index++)
            ;
          ;
        }

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
  block_range_t set_block_range_bits(const block_range_t range,
                                     bool val) noexcept {
    char* bits_begin = bits_of_block_aliveness_metadata_ptr;
    uintlen_t index{range.begin_index};
    uintlen_t end_index{range.begin_index + range.len};

    for (; index < end_index; index++) {
      auto& byte = bits_begin[index / 8];
      const auto mask = char(1 << (index % 8));
      byte &= ~mask;
      byte |= val ? mask : 0;
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
  /*
   * the is_a_good_alignment function must return true on the first byte of the
   * returned block
   */
  MJZ_CX_ND_FN
  block_range_t alloc_block_range(
      uintlen_t min_num_blocks, uintlen_t max_mum_blocks, bool is_best_fit,
      callable_c<bool(uintlen_t begin_blocks_index) noexcept> auto&&
          is_a_good_alignment) noexcept {
    if (min_num_blocks > max_mum_blocks) {
      return {};
    }
    auto ret = get_best_avalible_block_range(is_best_fit, is_a_good_alignment,
                                             min_num_blocks);
    ret.len = std::min(ret.len, max_mum_blocks);
    if (!ret.len) {
      ret.begin_index = 0;
    }
    return alloc_block_range(ret);
  }
  MJZ_CX_ND_FN
  block_range_t alloc_block_range(
      uintlen_t exact_num_blocks, bool is_best_fit,
      callable_c<bool(uintlen_t begin_blocks_index) noexcept> auto&&
          is_a_good_alignment) noexcept {
    return alloc_block_range(exact_num_blocks, exact_num_blocks, is_best_fit,
                             is_a_good_alignment);
  }

  MJZ_CX_FN
  void dealloc_block_range(block_range_t range) noexcept {
    dealloc_block_range_val(range);
  }
  MJZ_CX_FN
  void init() noexcept {
    memset(bits_of_block_aliveness_metadata_ptr,
           num_blocks / 8 + uintlen_t(bool(num_blocks % 8)), 0);
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
    auto ptr(bits_of_block_aliveness_metadata_ptr);
    auto len(num_blocks / 8 + uintlen_t(bool(num_blocks % 8)));
    for (uintlen_t i{}; i < len; i++) {
      if (ptr[i]) return false;
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
        auto& obj = ret[real_index];
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

  MJZ_CONSTANT(auto)
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
    MJZ_CX_FN auto operator<=>(const scored_index_t& b) const noexcept {
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
};
};  // namespace mjz::allocs_ns
#endif//MJZ_ALLOCS_block_states_FILE_HPP_
