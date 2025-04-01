#include "blocks_alloc.hpp"
#ifndef MJZ_ALLOCS_pool_alloc_FILE_HPP_
#define MJZ_ALLOCS_pool_alloc_FILE_HPP_
namespace mjz ::allocs_ns {
template <version_t version_v, uintlen_t N, bool has_lock = true>
  requires requires() { requires(!!N && N <= ((sizeof(uintlen_t) * 8) - 3)); }
struct pool_alloc_info_t {
  using alloc_t = pool_alloc_info_t;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;
  using blk_state = blk_state_t<version_v>;
  using bucket_info_t = bucket_alloc_info_t<version_v, false>;
  using bucket_t = typename bucket_info_t::obj_t;
  MJZ_CONSTANT(auto) pow_of_2 = blk_state::pow_of_2;
  struct this_align_val_t {
    bool right_val{};
    MJZ_CX_ND_FN this_align_val_t(auto &&var) noexcept {
      right_val = (pow_of_2(N - 1) <= uintlen_t(var));
    }
    MJZ_CX_FN operator bool() const noexcept { return right_val; }
  };
  struct m_t {
    blk_state my_block_manager{};
    char *data_buffer{};
  };
  struct init_temp_blk_state {
    MJZ_NO_MV_NO_CPY(init_temp_blk_state);
    MJZ_CX_ND_FN init_temp_blk_state(bucket_t &bucket_,
                                     std::array<m_t, N> array,
                                     uintlen_t i) noexcept
        : bucket(bucket_) {
      auto &data = bucket.unsafe_get_handle();
      data.blk_size = pow_of_2(i);
      data.data_buffer = array[i].data_buffer;
      data.log2_align = i;
      data.blk_states = array[i].my_block_manager;
    }
    bucket_t &bucket;
    MJZ_CX_FN ~init_temp_blk_state() noexcept {
      bucket.unsafe_get_handle() =
          std::remove_cvref_t<decltype(bucket.unsafe_get_handle())>{};
    }
  };
  char *data_ptr{};
  uintlen_t byte_count{};
  this_align_val_t this_aligns{};
  struct obj_t {
    template <class>
    friend class mjz_private_accessed_t;

   private:
    template <uintlen_t i>
    MJZ_CX_FN static auto assume_aligned(char *ptr) noexcept -> char * {
      if constexpr (i >= N || i == 0) {
        return ptr;
      } else {
        return std::assume_aligned<pow_of_2(i)>(ptr);
      }
    };
    template <uintlen_t I>
    MJZ_CX_FN static auto assume_aligned_helper(char *ptr, uintlen_t i) noexcept
        -> char * {
      if (i == I || I == 0) {
        return assume_aligned<I>(ptr);
      }
      return assume_aligned_helper<std::max<uintlen_t>(I, 1) - 1>(ptr, i);
    };

    MJZ_CX_FN static auto assume_aligned(char *ptr, uintlen_t i) noexcept
        -> char * {
      return assume_aligned_helper<N - 1>(ptr, i);
    };

    std::array<m_t, N> bucket_data_s{};

    MJZ_NO_MV_NO_CPY(obj_t);

    template <class>
    friend class mjz_private_accessed_t;

   private:
    MJZ_CX_FN auto lock_gaurd(bool is_threaded, bool mut_op) const noexcept {
      return lock_details_ns::lock_gaurd_maker<version_v>(
          is_threaded, mutex_byte(), has_lock, !mut_op);
    }

   public:
    MJZ_CX_ND_FN obj_t(alloc_t a) noexcept {
      if (!(a.data_ptr && a.byte_count)) return;
      constexpr uintlen_t align_v{pow_of_2(N - 1)};
      a.byte_count -= uintlen_t(has_lock);
      if (!a.this_aligns) {
        asserts(false, "alignment is lower than (1<<(N-1)) ", true);
        auto start_intptr = reinterpret_cast<uintptr_t>(a.data_ptr);
        auto delta = start_intptr % align_v;
        delta = delta ? (align_v - delta) : delta;
        if (a.byte_count <= delta) return;
        a.byte_count -= delta;
        a.data_ptr += a.byte_count;
      }
      a.data_ptr = MJZ_ASSUME_ALIGNESV_GET(a.data_ptr, align_v);

      auto block_counts =
          blk_state::template calculate_block_count_for_each_container<N>(
              a.byte_count);
      char *usable_data_ptr{a.data_ptr};
      char *usable_data_ptr_from_end{a.data_ptr + a.byte_count +
                                     uintlen_t(has_lock)};
      for (uintlen_t i_{}; i_ < N; i_++) {
        uintlen_t i{N - i_ - 1};
        if (!block_counts[i]) continue;
        auto &data_obj = this->bucket_data_s[i];
        data_obj.data_buffer = assume_aligned(usable_data_ptr, i);
        blk_state &bm = data_obj.my_block_manager;
        bm.num_blocks = block_counts[i];
        usable_data_ptr_from_end -= bm.container_metadata_size(block_counts[i]);
        bm.bits_of_block_aliveness_metadata_ptr = usable_data_ptr_from_end;
        usable_data_ptr += bm.container_data_size(block_counts[i], i);
        bm.init();
        (void)*usable_data_ptr;
        (void)*usable_data_ptr_from_end;
      }
      if constexpr (has_lock) {
        *mutex_byte() = 0;
      }
    }

    template <class>
    friend class mjz_private_accessed_t;

   private:
    MJZ_CX_FN char *mutex_byte() const noexcept
      requires(has_lock)
    {
      return this->bucket_data_s[0].data_buffer
                 ? this->bucket_data_s[0].data_buffer +
                       this->bucket_data_s[0].my_block_manager.num_blocks *
                           pow_of_2(0)
                 : nullptr;
    }
    MJZ_CX_FN success_t
    run_loop(callable_c<success_t(uintlen_t i) noexcept> auto &&in_the_loop,
             alloc_info &s, uintlen_t size, bool in_allocate,
             bool mut_op) const noexcept {
      bool is_thread_safe = s.is_thread_safe;
      s.is_thread_safe = 0;
      auto Fn = [&](uintlen_t i) noexcept {
        auto l = lock_gaurd(is_thread_safe, mut_op);
        if (!l) return false;
        return in_the_loop(i);
      };

      if (!in_allocate) {
        for (uintlen_t i{0}; i < N; i++) {
          if (auto ret = Fn(i); !!ret) {
            return ret;
          }
        }
        return false;
      }
      auto array = blk_state::template get_best_index_es<N>(
          std::max(size, s.get_alignof_z()));
      for (uintlen_t i{0}; i < N; i++) {
        if (auto ret = Fn(array[i]); !!ret) {
          return ret;
        }
      }
      return false;
    }

    template <class>
    friend class mjz_private_accessed_t;

   private:
    MJZ_CX_ND_FN friend bool operator==(const obj_t &a,
                                        const obj_t &b) noexcept = delete;

   public:
    MJZ_CX_ND_FN bool is_owner(const block_info &blk,
                               alloc_info s) const noexcept {
      return run_loop(
          [&](uintlen_t i) noexcept {
            bucket_t bucket{bucket_info_t{}};
            init_temp_blk_state tt{bucket, this->bucket_data_s, i};
            if (bucket.is_owner(blk, s)) {
              return true;
            }
            return false;
          },
          s, blk.length, false, false);
    }

    MJZ_CX_ND_FN success_t deallocate(block_info &&blk, alloc_info s) noexcept {
      return run_loop(
          [&](uintlen_t i) noexcept {
            bucket_t bucket{bucket_info_t{}};
            init_temp_blk_state tt{bucket, this->bucket_data_s, i};
            if (bucket.is_owner(blk, s)) {
              return bucket.deallocate(std::move(blk), s);
            }
            return false;
          },
          s, blk.length, false, true);
    }
    MJZ_CX_ND_FN block_info allocate(uintlen_t size, alloc_info s) noexcept {
      block_info ret_val{};
      run_loop(
          [&](uintlen_t i) noexcept {
            bucket_t bucket{bucket_info_t{}};
            init_temp_blk_state tt{bucket, this->bucket_data_s, i};
            if (block_info ret = bucket.allocate(size, s); !!ret.ptr) {
              ret_val = ret;
              ret_val.ptr = assume_aligned(ret_val.ptr, i);
              return true;
            }
            return false;
          },
          s, size, true, true);
      return ret_val;
    }
  };
};
}  // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_pool_alloc_FILE_HPP_