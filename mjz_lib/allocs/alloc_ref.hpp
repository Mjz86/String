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
#include "page_alloc_base.hpp"
#if MJZ_LOG_ALLOCATIONS_
#include "../outputs.hpp"
#endif
#include <stdlib.h>

#include <cstdlib>
#include <memory_resource>
#include <new>
#include <numeric>

#include "../threads/big_mutext.hpp"
#include "../threads/lock_guard.hpp"
#ifndef MJZ_ALLOCS_alloc_refs_FILE_HPP_
#define MJZ_ALLOCS_alloc_refs_FILE_HPP_
namespace mjz::allocs_ns {

// configurable
template <version_t version_v>
constexpr static const uintlen_t cow_threashold_v{
    4 * hardware_destructive_interference_size};

// configurable
template <version_t version_v>
constexpr static const bool uses_pmr_sync_v{MJZ_PMR_GLOBAL_ALLOCATIONS_};

// configurable
template <version_t version_v>
constexpr static const bool uses_pmr_async_v{MJZ_PMR_GLOBAL_ALLOCATIONS_};

// configurable
template <version_t version_v>
constexpr static const uintlen_t page_alloc_page_count_v{0};
// configurable
template <version_t version_v>
constexpr static const uintlen_t page_alloc_blk_size_v{
    hardware_constructive_interference_size};
// configurable
template <version_t version_v>
constexpr static const bool check_the_alloc_info{MJZ_IN_DEBUG_MODE};

// configurable
template <version_t version_v>
constexpr static const bool force_deafult_alloc_v{false};

// configurable
template <version_t version_v>
MJZ_CONSTANT(uintlen_t)
thread_local_stack_size_v{
    0 /*force_deafult_alloc_v<version_v> ? 0 : 1024 * 16*/};

// configurable

template <version_t version_v>
MJZ_CONSTANT(std::align_val_t)
thread_local_stack_align_v{16};

template <version_t v>
struct alloc_base_t;
template <version_t version_v>
class alloc_base_ref_t;

template <version_t v>
struct alloc_info_t {
  template <class>
  friend class mjz_private_accessed_t;
  template <class>
  friend class mjz_private_accessed_t;
  enum class allocation_mode_e : uint16_t {
    // dont change the values!
    relaxed_mode = 0b00,
    monotonic_mode = 0b01,
    alloca_mode = 0b10,
    alloca_or_monotonic_mode = 0b11
  };
  MJZ_CX_FN bool operator==(const alloc_info_t &) const noexcept = default;

 private:
  MJZ_CX_ND_FN static uint8_t log2_of_align_val_create_z(size_t val) noexcept {
    return log2_ceil_of_val_create(val);
  }
  MJZ_CX_ND_FN static size_t log2_of_align_val_to_val_z(
      uint8_t log2_val) noexcept {
    return static_cast<size_t>(1ull << log2_val);
  }

 public:
  uintlen_t size_multiplier{1};
  uintlen_t log2_of_align_val : 6 {0};
  uintlen_t /*allocation_mode_e*/ allocation_mode_val : 2 {};
  uintlen_t uses_upstream_forwarding : 1 {1};
  uintlen_t uses_default_upstream_forwarding : 1 {1};
  uintlen_t allocate_exactly_minsize : 1 {0};
  uintlen_t uses_munual_alignment : 1 {1};
  uintlen_t is_thread_safe : 1 {1};
  uintlen_t uses_best_fit : 1 {1};
  uintlen_t is_one_of_many_nodes : 1 {0};
  // side-note: does not fail for the default constructed allocator
  uintlen_t cache_only_allocation : 1 {0};
  uintlen_t dummy_ : sizeof(uintlen_t) * 8 - 16;

 public:
  // bit cast in clang is broken ;-;
  MJZ_CX_ND_FN std::array<uintlen_t, 2> idk_bit_cast_() const noexcept {
    std::array<uintlen_t, 2> ret{};
    ret[0] = size_multiplier;
    ret[1] |= log2_of_align_val;
    ret[1] <<= 6;
    ret[1] |= allocation_mode_val;
    ret[1] <<= 2;
    ret[1] |= uses_upstream_forwarding;
    ret[1] <<= 1;
    ret[1] |= uses_default_upstream_forwarding;
    ret[1] <<= 1;
    ret[1] |= allocate_exactly_minsize;
    ret[1] <<= 1;
    ret[1] |= uses_munual_alignment;
    ret[1] <<= 1;
    ret[1] |= is_thread_safe;
    ret[1] <<= 1;
    ret[1] |= uses_best_fit;
    ret[1] <<= 1;
    ret[1] |= is_one_of_many_nodes;
    ret[1] <<= 1;
    ret[1] |= cache_only_allocation;
    ret[1] <<= 1;
    ret[1] |= dummy_;
    return ret;
  }

 public:
  template <class T>
  MJZ_CX_ND_FN alloc_info_t &consider_type(T *) noexcept {
    size_multiplier = std::lcm(size_multiplier, sizeof(T));
    set_alignof_z(std::max(get_alignof_z(), alignof(T)));
    return *this;
  }
  MJZ_CX_ND_FN alloc_info_t &cant_bother_with_good_size() noexcept {
    allocate_exactly_minsize |= log2_of_val_create(size_multiplier) !=
                                log2_ceil_of_val_create(size_multiplier);
    allocate_exactly_minsize |= get_alignof_z() < size_multiplier;
    return *this;
  }
  MJZ_CX_ND_FN
  size_t get_alignof_z() const noexcept {
    return log2_of_align_val_to_val_z(log2_of_align_val);
  }
  MJZ_CX_FN

  alloc_info_t &make_threaded() noexcept {
    is_thread_safe = true;
    return *this;
  }
  MJZ_CX_FN

  alloc_info_t &make_allocate_exactly_minsize() noexcept {
    allocate_exactly_minsize = true;
    return *this;
  }

  MJZ_CX_FN

  alloc_info_t &make_non_threaded() noexcept {
    is_thread_safe = false;
    return *this;
  }
  MJZ_CX_FN

  alloc_info_t &set_alignof_z(size_t aval) noexcept {
    log2_of_align_val = log2_of_align_val_create_z(aval);
    return *this;
  }
  MJZ_CX_FN

  alloc_info_t &set_alignof(std::align_val_t aval) noexcept {
    set_alignof_z(size_t(aval));
    return *this;
  }
  MJZ_CX_FN
  alloc_info_t &set_as_nodes() noexcept {
    is_one_of_many_nodes = true;
    return *this;
  }
  MJZ_CX_ND_FN
  std::align_val_t get_alignof() const noexcept {
    return std::align_val_t{get_alignof_z()};
  }

 public:
  MJZ_CE_FN static alloc_info_t this_s_alloc_info(
      std::align_val_t aval, bool is_threaded_ = true) noexcept {
    alloc_info_t ret{};
    ret.set_alignof(aval);
    ret.allocate_exactly_minsize = 1;
    ret.is_thread_safe = uint16_t(is_threaded_);
    return ret;
  }
};

template <version_t v>
struct ref_count_t {
  uintlen_t optional_count = uintlen_t(-1);
};
template <version_t v, typename T = char>
struct block_info_t {
  T *ptr;
  uintlen_t length;
  MJZ_CX_FN bool operator==(const block_info_t &) const noexcept = default;
};
enum class complex_alloc_relations_e : char {
  equal,
  idk,
  none,
  neighbor,
  superset,
  subset
};
enum class alloc_relations_e : char {
  equal,
  none,
};

template <version_t version_v>
struct alloc_speed_t {
  uintlen_t wat{};
};

template <version_t version_v>
struct alloc_vtable_t {
 public:
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;

 public:
  struct funcs_t {
    template <typename func_t>
    using F_t = like_funcptr_of_t<func_t, alloc_base>;

    using is_owner =
        F_t<may_bool_t(const block_info &, alloc_info) const noexcept>;
    using is_equal = F_t<alloc_relations_e(const alloc_ref &) const noexcept>;
    using ref_call = F_t<void(bool add_vs_destroy) noexcept>;
    using handle =
        F_t<const void_struct_t *(const void_struct_t *) const noexcept>;
    using refresh_call =
        F_t<success_t(uintlen_t monotonic_minsize,
                      uintlen_t monotonic_min_align, uintlen_t stack_minsize,
                      uintlen_t stack_min_align, bool release_all) noexcept>;
    using allocate = F_t<block_info(uintlen_t, alloc_info) noexcept>;
    using deallocate = F_t<void(block_info, alloc_info) noexcept>;
    template <class>
    friend class mjz_private_accessed_t;

   private:  // not useful rn.
    using add_ref = F_t<success_t(intlen_t) noexcept>;
    using alloc_call = F_t<void(block_info &blk, alloc_info) noexcept>;
    using num_ref = F_t<ref_count(std::memory_order) const noexcept>;
    using destroy_obj = F_t<success_t() noexcept>;
    using compare =
        F_t<complex_alloc_relations_e(const alloc_ref &) const noexcept>;
    using can_destroy_obj = F_t<bool() const noexcept>;
    using speed_of = F_t<alloc_speed(uintlen_t, alloc_info) const noexcept>;
    using releaser_all = F_t<success_t() noexcept>;
    using num_sub_alloc = F_t<ref_count() const noexcept>;
    using add_sub_alloc = F_t<success_t(const alloc_ref &) noexcept>;
    using has_sub_alloc = F_t<may_bool_t(const alloc_ref &) const noexcept>;
    using remove_sub_alloc = F_t<success_t(const alloc_ref &) noexcept>;
    using has_super_alloc = F_t<may_bool_t() const noexcept>;
    using add_super_alloc = F_t<success_t(const alloc_ref &) noexcept>;
    using is_super_alloc = F_t<may_bool_t(const alloc_ref &) const noexcept>;
    using get_super_alloc = F_t<alloc_ref() const noexcept>;
    using remove_super_alloc = F_t<success_t() noexcept>;
    using could_use_super_alloc = F_t<bool() const noexcept>;
    using could_use_sub_alloc = F_t<bool() const noexcept>;
  };

  template <class>
  friend class mjz_private_accessed_t;

 private:
  struct not_used_t_ {
    typename funcs_t::releaser_all releaser_all;
    typename funcs_t::can_destroy_obj can_destroy_obj;
    typename funcs_t::compare compare;
    typename funcs_t::speed_of speed_of;
    typename funcs_t::num_sub_alloc num_sub_alloc;
    typename funcs_t::add_sub_alloc add_sub_alloc;
    typename funcs_t::has_sub_alloc has_sub_alloc;
    typename funcs_t::remove_sub_alloc remove_sub_alloc;
    typename funcs_t::has_super_alloc has_super_alloc;
    typename funcs_t::add_super_alloc add_super_alloc;
    typename funcs_t::is_super_alloc is_super_alloc;
    typename funcs_t::get_super_alloc get_super_alloc;
    typename funcs_t::remove_super_alloc remove_super_alloc;
    typename funcs_t::could_use_super_alloc could_use_super_alloc;
    typename funcs_t::could_use_sub_alloc could_use_sub_alloc;
    typename funcs_t::add_ref add_ref;
    typename funcs_t::num_ref num_ref;
    typename funcs_t::destroy_obj destroy_obj;
    typename funcs_t::alloc_call alloc_call;
  };
  static_assert(std::gcd(cow_threashold_v<version_v>,
                         hardware_destructive_interference_size) ==
                hardware_destructive_interference_size);

 public:
  alloc_info default_info{};  // 2w
  //--end first 8words--//
  //--start second 8words--//
  size_t cow_threashold{cow_threashold_v<version_v>};
  typename funcs_t::allocate allocate{};
  typename funcs_t::deallocate deallocate{};
  typename funcs_t::ref_call ref_call{};
  typename funcs_t::is_equal is_equal{};
  typename funcs_t::is_owner is_owner{};
  typename funcs_t::handle handle{};
  typename funcs_t::refresh_call refresh_call{};
  //--end second 8words--//
  MJZ_CX_FN bool operator==(const alloc_vtable_t &rhs) const noexcept = default;
};

template <version_t version_v>
struct alignas(16) alloc_base_t : void_struct_t {
  //--start first 8words--//
  fast_alloc_chache_t<version_v> alloc_chache{};  // 6w
  const alloc_vtable_t<version_v> vtable{};
  MJZ_NO_MV_NO_CPY(alloc_base_t);
  MJZ_CX_FN alloc_base_t() noexcept = default;
  MJZ_CX_FN alloc_base_t(const alloc_vtable_t<version_v> &vtable_val) noexcept
      : alloc_base_t(
            [&vtable_val]() noexcept {
              alloc_vtable_t<version_v> vtable_{vtable_val};
              uintlen_t min = hardware_destructive_interference_size;
              uintlen_t align_log2 = log2_ceil_of_val_create(min);
              vtable_.cow_threashold >>= align_log2;
              vtable_.cow_threashold <<= align_log2;
              vtable_.cow_threashold += (size_t)branchless_teranary<uintlen_t>(
                  vtable_.cow_threashold != vtable_val.cow_threashold, min, 0);
              return vtable_;
            },
            void_struct_t{}) {}

 private:
  MJZ_CX_FN alloc_base_t(auto &&lam, void_struct_t) noexcept : vtable(lam()) {}
};
template <version_t version_v, int priority>
class global_allocator_class_t {};
template <version_t version_v>
struct global_allocator_class_t<version_v, 256> : void_struct_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <typename T>
  using block_info_ot = block_info_t<version_v, T>;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;
  using bptr_t = const alloc_base *;

  struct pages_data_t {
    MJZ_NO_MV_NO_CPY(pages_data_t);
    MJZ_DISABLE_ALL_WANINGS_START_;
    union byte_data_t {
      alignas(std::max(hardware_destructive_interference_size,
                       page_alloc_blk_size_v<version_v>)) char dummy_{};
      char raw[std::max<uintlen_t>(page_alloc_page_count_v<version_v> *
                                       page_alloc_blk_size_v<version_v> * 8 *
                                       sizeof(uintlen_t),
                                   1)];
    };
    MJZ_DISABLE_ALL_WANINGS_END_;
    threads_ns::big_mutex_t<> mutex{};
    page_alloc_ns::simple_page_alloc_t<version_v> m{};
    uintlen_t meta_data[page_alloc_page_count_v<version_v>]{};
    byte_data_t byte_data{};
    MJZ_CX_FN pages_data_t() noexcept {
      byte_data.raw[0] = 0;
      m.data_ptr.pages = byte_data.raw;
      m.data_ptr.blk_align_log2 =
          log2_of_val_to_val(page_alloc_blk_size_v<version_v>);
      m.data_meta.pages = meta_data;
      m.data_meta.release();
    }
  };

  MJZ_NCX_FN static void *nothrow_new_th(size_t size,
                                         std::align_val_t align) noexcept {
    void *ptr{};
    if constexpr (force_deafult_alloc_v<version_v> ||
                  !uses_pmr_sync_v<version_v>) {
      if (default_new_align_v < align) {
        ptr = ::operator new(size, align, std::nothrow);
      } else {
        ptr = ::operator new(size, std::nothrow);
      }
      return ptr;

    } else {
      MJZ_NOEXCEPT { ptr = th_pool().allocate(size, size_t(align)); };
      return ptr;
    }
  }
  MJZ_NCX_FN static void *nothrow_new(size_t size, std::align_val_t align,
                                      bool is_threaded) noexcept {
    if constexpr (force_deafult_alloc_v<version_v> ||
                  !uses_pmr_async_v<version_v>) {
      return nothrow_new_th(size, align);
    } else if (is_threaded) {
      return nothrow_new_th(size, align);
    } else {
      return nothrow_new_nth(size, align);
    }
  }

  MJZ_NCX_FN static void nothrow_delete(void *ptr, size_t size,
                                        std::align_val_t align,
                                        bool is_threaded) noexcept {
    if constexpr (force_deafult_alloc_v<version_v> ||
                  !uses_pmr_async_v<version_v>) {
      return nothrow_delete_th(ptr, size, align);
    } else if (is_threaded) {
      return nothrow_delete_th(ptr, size, align);
    } else {
      return nothrow_delete_nth(ptr, size, align);
    }
  }

  MJZ_NCX_FN static std::span<char> nothrow_page_new(
      uintlen_t count, std::align_val_t align, bool is_threaded) noexcept {
    if ((page_alloc_blk_size_v<version_v>) < uintlen_t(align)) return {};
    if (is_threaded) {
      static pages_data_t pages{};
      auto lock = threads_ns::lock_guard_t{pages.mutex};
      if (!lock) return {};
      return pages.m.allocate(count, uintlen_t(align));
    }
    thread_local pages_data_t pages{};
    return pages.m.allocate(count, uintlen_t(align));
  }
  MJZ_NCX_FN static success_t nothrow_page_try_delete(
      std::span<char> blk, std::align_val_t align, bool is_threaded) noexcept {
    if (blk.size() == 0) return true;
    if ((page_alloc_blk_size_v<version_v>) < uintlen_t(align)) return false;
    if (is_threaded) {
      static pages_data_t pages{};
      while (true) {
        auto lock = threads_ns::lock_guard_t{pages.mutex};
        if (!lock) continue;
        if (!pages.m.is_owner(blk, uintlen_t(align))) return false;
        pages.m.deallocate(blk, uintlen_t(align));
        return true;
      }
    }
    thread_local pages_data_t pages{};
    if (!pages.m.is_owner(blk, uintlen_t(align))) return false;
    pages.m.deallocate(blk, uintlen_t(align));
    return true;
  }

  MJZ_NCX_FN static void nothrow_delete_th(void *ptr, size_t size,
                                           std::align_val_t align) noexcept {
    if constexpr (force_deafult_alloc_v<version_v> ||
                  !uses_pmr_sync_v<version_v>) {
      if (default_new_align_v < align) {
        ::operator delete(ptr, size, align);
      } else {
        ::operator delete(ptr, size);
      }

      return;
    } else {
      MJZ_NOEXCEPT { th_pool().deallocate(ptr, size, size_t(align)); };
    }
  }
  template <class T>
  class std_pmr_resource_t final : public T {
   public:
    using T::T;
  };
  using sync_pmr_t = std_pmr_resource_t<std::pmr::synchronized_pool_resource>;
  using async_pmr_t =
      std_pmr_resource_t<std::pmr::unsynchronized_pool_resource>;
  MJZ_NCX_FN static sync_pmr_t &th_pool() {
    alignas(hardware_destructive_interference_size) static sync_pmr_t pool{
        std::pmr::new_delete_resource()};
    return pool;
  }
  MJZ_NCX_FN static async_pmr_t &nth_pool() {
    alignas(hardware_constructive_interference_size) thread_local async_pmr_t
        async_pool{std::pmr::new_delete_resource()};
    return async_pool;
  }
  MJZ_NCX_FN static void nothrow_delete_nth(void *ptr, size_t size,
                                            std::align_val_t align) noexcept {
    MJZ_NOEXCEPT { nth_pool().deallocate(ptr, size, size_t(align)); };
  }

  MJZ_NCX_FN static void *nothrow_new_nth(size_t size,
                                          std::align_val_t align) noexcept {
    void *ptr{};
    MJZ_NOEXCEPT { ptr = nth_pool().allocate(size, size_t(align)); };
    return ptr;
  }
  static_assert(log2_of_val_create(page_alloc_blk_size_v<version_v>) ==
                log2_ceil_of_val_create(page_alloc_blk_size_v<version_v>));

 public:
  MJZ_CX_FN static block_info global_alloc(uintlen_t minsize,
                                           alloc_info ai) noexcept {
    auto align_val = ai.get_alignof();
    uintlen_t size = minsize;
    MJZ_IFN_CONSTEVAL {
      if constexpr (!force_deafult_alloc_v<version_v> &&
                    page_alloc_page_count_v<version_v>) {
        if (std::span<char> data =
                nothrow_page_new(minsize, ai.get_alignof(), ai.is_thread_safe);
            !!data.size()) {
          if (!ai.allocate_exactly_minsize) {
            minsize = data.size();
          }
          return {data.data(), minsize};
        }
      }
      void *ptr{};
      ptr = nothrow_new((size_t)size, align_val, ai.is_thread_safe);
#if MJZ_LOG_NEW_ALLOCATIONS_
      MJZ_NOEXCEPT { mjz_debug_cout::print("[new:", size, "]"); };
#endif
      size = branchless_teranary<size_t>(!ptr, 0, size);
      block_info blk{};
      blk.length = size;
      blk.ptr = reinterpret_cast<char *>(std::exchange(ptr, nullptr));
      return blk;
    }
    if (static_cast<size_t>(align_val) > default_new_align_z) return {};
    return {new char[(size_t)size], size};
  }
  MJZ_CX_FN static void global_dealloc(block_info blk, alloc_info ai) noexcept {
    MJZ_IFN_CONSTEVAL {
#if MJZ_LOG_NEW_ALLOCATIONS_
      MJZ_NOEXCEPT { mjz_debug_cout::print("[delete:", blk.length, "]"); };
#endif
      if constexpr (!force_deafult_alloc_v<version_v> &&
                    page_alloc_page_count_v<version_v>) {
        if (nothrow_page_try_delete(std::span(blk.ptr, blk.length),
                                    ai.get_alignof(), ai.is_thread_safe)) {
          return;
        }
      }
      nothrow_delete(blk.ptr, blk.length, ai.get_alignof(), ai.is_thread_safe);
      return;
    }
    delete[] blk.ptr;
    return;
  }
};

template <version_t version_v, int priority>
struct global_allocator_selector_helper_t_ {
  using type = typename global_allocator_selector_helper_t_<version_v,
                                                            priority + 1>::type;
};
template <version_t version_v, int priority>
  requires std::derived_from<global_allocator_class_t<version_v, priority>,
                             void_struct_t>
struct global_allocator_selector_helper_t_<version_v, priority> {
  using type = global_allocator_class_t<version_v, priority>;
};

template <version_t version_v>
using global_allocator_t =
    typename global_allocator_selector_helper_t_<version_v, -1>::type;

template <version_t version_v>
class alloc_base_ref_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <typename T>
  using block_info_ot = block_info_t<version_v, T>;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;
  using alloc_cache_ref = optional_ref_t<fast_alloc_chache_t<version_v>>;
  using bptr_t = const alloc_base *;
  alignas(sizeof(uintlen_t)) alloc_base *ref{};
  template <typename... Ts>
  MJZ_CX_FN static auto run_grantee_table(alloc_base &ref, auto &&fn,
                                          Ts &&...args) noexcept {
    auto const old_vtable = ref.vtable;
    auto const old_ref = &ref;
    MJZ_RELEASE {
      asserts(asserts.assume_rn, old_vtable == ref.vtable && old_ref == &ref);
    };
    return fn(&ref, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN auto run(auto &&fn, Ts &&...args) const noexcept {
    return fn(ref, std::forward<Ts>(args)...);
  }

 private:
  MJZ_CX_AL_FN alloc_cache_ref
  get_cache_impl(alloc_base *dummy, bool good_ = true) const noexcept {
    good_ &= !!this->ref;
    alloc_base *vptr = branchless_teranary(good_, this->ref, dummy);
    return &vptr->alloc_chache;
  }

  constexpr static const alloc_base dummy_base_0_{};

 public:
  MJZ_CX_FN const alloc_vtable_t<version_v> &get_vtbl() const noexcept {
    bptr_t vptr =
        branchless_teranary<bptr_t>(!this->ref, &dummy_base_0_, this->ref);
    return vptr->vtable;
  }
  MJZ_CX_FN alloc_cache_ref get_cache() const noexcept {
    return const_cast<fast_alloc_chache_t<version_v> *>(
        branchless_teranary<bptr_t>(
            !!this->ref,
            &branchless_teranary<bptr_t>(!!this->ref, this->ref, &dummy_base_0_)
                 ->alloc_chache,
            nullptr));
  }
  MJZ_CX_FN alloc_base *get_ptr() const noexcept { return this->ref; }
  MJZ_CX_FN alloc_base &get_ref() const noexcept {
    asserts(asserts.assume_rn, !!this->ref);
    return *this->ref;
  }
  MJZ_CX_FN void reset() noexcept { destroy_obj(); }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!this->ref; };

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_FN void copy_init(const alloc_base_ref_t &other) noexcept {
    if (!other.add_ref()) return;
    this->ref = other.ref;
  }

 public:
  MJZ_CX_ND_FN alloc_base_ref_t() noexcept = default;
  MJZ_CX_ND_FN alloc_base_ref_t(nullptr_t) noexcept : ref{} {};
  MJZ_CX_ND_FN alloc_base_ref_t(const alloc_base_ref_t &other) noexcept
      : ref{} {
    copy_init(other);
  }
  MJZ_CX_FN alloc_base_ref_t(alloc_base_ref_t &&other) noexcept
      : ref{std::move(other).to_raw(false)} {}

  MJZ_CX_FN
  alloc_base_ref_t(single_object_pointer_t<alloc_base> raw_ptr,
                   bool add_ref_count) noexcept
      : ref{raw_ptr} {
    if (add_ref_count) add_ref();
  }
  MJZ_CX_FN alloc_base_ref_t &operator=(
      const alloc_base_ref_t &other) noexcept {
    if (ref == other.ref) return *this;
    reset();
    copy_init(other);
    return *this;
  }
  MJZ_CX_FN alloc_base_ref_t &operator=(alloc_base_ref_t &&other) noexcept {
    if (ref == other.ref) return *this;
    reset();
    this->ref = std::exchange(other.ref, nullptr);
    return *this;
  }
  MJZ_CX_FN ~alloc_base_ref_t() noexcept { reset(); }
  MJZ_CX_FN alloc_base_ref_t operator()() const noexcept { return *this; }
  MJZ_CX_FN alloc_base_ref_t operator+() const noexcept { return *this; }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_FN
  success_t destroy_obj() noexcept {
    const auto func_ = get_vtbl().ref_call;
    if (!func_) return true;
    MJZ_RELEASE { this->ref = nullptr; };
    run(func_, false);
    return true;
  }
  MJZ_CX_FN
  success_t add_ref() const noexcept {
    const auto func_ = get_vtbl().ref_call;
    if (!func_) return true;
    run_grantee_table(*ref, func_, true);
    return true;
  }
  MJZ_CX_FN
  may_bool_t is_owner_of_bytes_cache_(const block_info &blk_,
                                      uintlen_t align) const noexcept {
    alloc_base dummy_{};
    alloc_cache_ref cache = get_cache_impl(&dummy_);
    std::span<char> blk{blk_.ptr, blk_.length};
    bool good = cache->is_monotonic(blk, align);
    good |= cache->is_stack(blk, align);
    return branchless_teranary(!good, may_bool_t::idk, may_bool_t::yes);
  }

 public:
  MJZ_CX_ND_FN auto to_raw(bool add_ref_count) noexcept
      -> single_object_pointer_t<alloc_base> {
    single_object_pointer_t<alloc_base> ret{};
    if (!add_ref_count) {
      ret = std::exchange(this->ref, nullptr);
      return ret;
    }
    return (*this)().to_raw(false);
  }
  MJZ_CX_FN
  may_bool_t is_owner_of_bytes(const block_info &blk,
                               alloc_info ai) const noexcept {
    if (is_owner_of_bytes_cache_(blk, ai.get_alignof_z()) == may_bool_t::yes)
      return may_bool_t::yes;
    const auto func_ = get_vtbl().is_owner;
    if (!func_) return may_bool_t::idk;
    return run_grantee_table(*ref, func_, blk, ai);
  }
  MJZ_CX_FN
  alloc_relations_e is_equal(const alloc_ref &ar) const noexcept {
    bool no_call{};
    bool is_eq = this->ref == ar.ref;
    const auto func_ = get_vtbl().is_equal;
    no_call |= is_eq;
    no_call |= !func_;
    if (no_call) {
      return branchless_teranary(!is_eq, alloc_relations_e::none,
                                 alloc_relations_e::equal);
    }
    return run_grantee_table(*ref, func_, ar);
  }
  MJZ_CX_FN bool operator==(const alloc_ref &ar) const noexcept {
    return is_equal(ar) == alloc_relations_e::equal;
  }

 private:
  MJZ_CX_FN static block_info local_alloc(alloc_base &ref, uintlen_t minsize,
                                          alloc_info ai) noexcept {
    block_info blk{};
    if (!minsize) return blk;
    blk = run_grantee_table(ref, ref.vtable.allocate, minsize, ai);
#if MJZ_LOG_ALLOC_ALLOCATIONS_
    MJZ_NOEXCEPT { mjz_debug_cout::print("[alloc:", blk.length, "]"); };
#endif
    /* lets assume this isnt needed beacuse of c++20:
   new (ptr) char[(size_t)size]*/
    return blk;
  }
  MJZ_CX_FN static void local_dealloc(alloc_base &ref, block_info blk,
                                      alloc_info ai) noexcept {
    if (!blk.ptr) return;
#if MJZ_LOG_ALLOC_ALLOCATIONS_
    MJZ_NOEXCEPT { mjz_debug_cout::print("[dealloc:", blk.length, "]"); };
#endif
    run_grantee_table(ref, ref.vtable.deallocate, blk, ai);
    blk = block_info{};
    return;
  }
  using allocation_mode_e = typename alloc_info::allocation_mode_e;
  MJZ_CX_FN static block_info monotonic_allocate_bytes_cache_(
      alloc_base &ref, uintlen_t minsize, alloc_info ai, bool bad) noexcept {
    bad |= !!!(ai.allocation_mode_val &
               uint16_t(allocation_mode_e::monotonic_mode));
    std::span<char> ret =
        ref.alloc_chache.monotonic_allocate(minsize, ai.get_alignof_z(), bad);
    return block_info{ret.data(), ret.size()};
  }
  MJZ_CX_FN static success_t monotonic_deallocate_bytes_cache_(
      alloc_base &ref, block_info blk, alloc_info ai, bool bad) noexcept {
    bad |= !!!(ai.allocation_mode_val &
               uint16_t(allocation_mode_e::monotonic_mode));
    return ref.alloc_chache.is_monotonic(std::span<char>{blk.ptr, blk.length},
                                         ai.get_alignof_z(), bad);
  }
  MJZ_CX_FN static block_info alloca_bytes_cache_(alloc_base &ref,
                                                  uintlen_t minsize,
                                                  alloc_info ai,
                                                  bool bad) noexcept {
    bad |=
        !!!(ai.allocation_mode_val & uint16_t(allocation_mode_e::alloca_mode));
    const std::span<char> ret =
        ref.alloc_chache.fn_alloca(minsize, ai.get_alignof_z(), bad);
    return block_info{
        ret.data(),
        branchless_teranary(!!ai.allocate_exactly_minsize,
                            std::min(ret.size(), minsize), ret.size())};
  }
  MJZ_CX_FN static success_t dealloca_bytes_cache_(alloc_base &ref,
                                                   block_info blk,
                                                   alloc_info ai,
                                                   bool bad) noexcept {
    bad |=
        !!!(ai.allocation_mode_val & uint16_t(allocation_mode_e::alloca_mode));
    return ref.alloc_chache.fn_try_dealloca(
        std::span<char>{blk.ptr, blk.length}, ai.get_alignof_z(), bad);
  }
  MJZ_CX_FN
  static void good_cache_impl_(const alloc_base &ref) noexcept {
    const fast_alloc_chache_t<version_v> &cr = ref.alloc_chache;
    asserts(asserts.assume_rn,
            !memory_is_inside(
                cr.stack_begin,
                uintlen_t(cr.stack_left + cr.stack_ptr - cr.stack_begin),
                cr.monotonic_begin,
                uintlen_t(cr.monotonic_left + cr.monotonic_ptr -
                          cr.monotonic_begin)));
  }
  MJZ_CX_FN
  static block_info allocate_bytes_impl_(alloc_base *ref, uintlen_t minsize,
                                         alloc_info ai) noexcept {
    typename alloc_vtable_t<version_v>::funcs_t::allocate allocate_fn{};
    const bool can_cache_ = int(!!ref) & int(!ai.is_thread_safe);
    if (can_cache_) {
      bool success{};
      block_info blk{};
      good_cache_impl_(*ref);
      allocate_fn = ref->vtable.allocate;
      const block_info blk1 = alloca_bytes_cache_(*ref, minsize, ai, false);
      success = !!blk1.ptr;
      const block_info blk2 =
          monotonic_allocate_bytes_cache_(*ref, minsize, ai, success);
      blk = success ? blk1 : blk2;
      success = !!blk.ptr;
      success |= !!ai.cache_only_allocation;
      if (success) {
        return blk;
      }
    }
    if (!allocate_fn) {
      return global_allocator_t<version_v>::global_alloc(minsize, ai);
    }
    return local_alloc(*ref, minsize, ai);
  }
  MJZ_CX_FN static success_t deallocate_bytes_impl_(alloc_base *ref,
                                                    block_info blk,
                                                    alloc_info ai) noexcept {
    typename alloc_vtable_t<version_v>::funcs_t::allocate allocate_fn{};
    typename alloc_vtable_t<version_v>::funcs_t::deallocate deallocate_fn{};
    const bool can_cache_ = int(!!ref) & int(!ai.is_thread_safe);
    if (can_cache_) {
      bool success{};
      good_cache_impl_(*ref);
      allocate_fn = ref->vtable.allocate;
      deallocate_fn = ref->vtable.deallocate;
      success = dealloca_bytes_cache_(*ref, blk, ai, false);
      success |= monotonic_deallocate_bytes_cache_(*ref, blk, ai, success);
      success |= ai.cache_only_allocation;
      if (success) {
        return true;
      }
    }
    if (deallocate_fn) {
      local_dealloc(*ref, blk, ai);
      return true;
    }
    if (allocate_fn) {
      // monotonic
      return true;
    }
    global_allocator_t<version_v>::global_dealloc(blk, ai);
    return true;
  }

  struct alloc_log_info {
    version_t version{version_v};
    alloc_info ai{};
    block_info ret{};
    MJZ_CX_FN bool operator==(const alloc_log_info &) const noexcept = default;
  };
  struct cx_alloc_log_info {
    version_t version{version_v};
    std::array<uintlen_t, 2> ai_{};
    alignas(alignof(block_info)) char block_info_[sizeof(block_info)]{};
    MJZ_CX_FN bool operator==(const cx_alloc_log_info &) const noexcept =
        default;
  };
  static_assert(sizeof(cx_alloc_log_info) == sizeof(alloc_log_info) &&
                bitcastable_c<alloc_log_info> &&
                bitcastable_c<cx_alloc_log_info>);

  MJZ_CX_FN void allocate_bytes_log_check(
      MJZ_MAYBE_UNUSED uintlen_t &minsize,
      MJZ_MAYBE_UNUSED alloc_info &ai) const noexcept {
    if (!minsize) return;
    ai.allocate_exactly_minsize = true;
    minsize += sizeof(alloc_log_info);
  }
  MJZ_CX_FN void allocate_bytes_log_fix(MJZ_MAYBE_UNUSED block_info &ret,
                                        MJZ_MAYBE_UNUSED alloc_info
                                            old_ai) const noexcept {
    if (!ret.length) return;
    ret.length -= sizeof(alloc_log_info);
    char *alloc_log_info_ptr = ret.ptr + ret.length;
    MJZ_IF_CONSTEVAL {
      cx_alloc_log_info log{};
      log.ai_ = old_ai.idk_bit_cast_();
      cpy_bitcast(log.block_info_, ret.length);
      cpy_bitcast(alloc_log_info_ptr, log);
    }
    else {
      alloc_log_info log{};
      log.ai = old_ai;
      log.ret = ret;
      cpy_bitcast(alloc_log_info_ptr, log);
    }
  }

  MJZ_CX_FN void deallocate_bytes_log_check_fix(
      MJZ_MAYBE_UNUSED block_info &blk,
      MJZ_MAYBE_UNUSED alloc_info &ai) const noexcept {
    if (!blk.length) return;
    MJZ_RELEASE {
      ai.allocate_exactly_minsize = true;
      blk.length += sizeof(alloc_log_info);
    };
    char *alloc_log_info_ptr = blk.ptr + blk.length;
    MJZ_IF_CONSTEVAL {
      cx_alloc_log_info log{};
      log.ai_ = ai.idk_bit_cast_();
      cpy_bitcast(log.block_info_, blk.length);
      cx_alloc_log_info val =
          cpy_bitcast<cx_alloc_log_info>(alloc_log_info_ptr);
      asserts(asserts.condition_rn, val == log);
    }
    else {
      alloc_log_info log{};
      log.ai = ai;
      log.ret = blk;
      alloc_log_info val = cpy_bitcast<alloc_log_info>(alloc_log_info_ptr);
      asserts(asserts.condition_rn, val == log);
    }
  }
  MJZ_CX_FN alloc_info get_default_th_info() const noexcept {
    return get_default_info().make_threaded();
  }
  MJZ_CX_FN alloc_info get_default_nth_ex_info() const noexcept {
    alloc_info ret = get_default_info();
    ret.is_thread_safe = false;
    ret.make_allocate_exactly_minsize();
    return ret;
  }
  MJZ_CX_FN alloc_info get_default_th_ex_info() const noexcept {
    alloc_info ret = get_default_th_info();
    ret.make_allocate_exactly_minsize();
    return ret;
  }

 public:
  MJZ_CX_FN
  block_info allocate_bytes(uintlen_t minsize, alloc_info ai) const noexcept {
    if constexpr (check_the_alloc_info<version_v>) {
      alloc_info old_ai{ai};
      allocate_bytes_log_check(minsize, ai);
      block_info ret = allocate_bytes_impl_(this->ref, minsize, ai);
      allocate_bytes_log_fix(ret, old_ai);
      return ret;
    } else {
      return allocate_bytes_impl_(this->ref, minsize, ai);
    }
  }
  MJZ_CX_FN alloc_info get_default_info() const noexcept {
    return get_vtbl().default_info;
  }

  MJZ_CX_FN
  success_t deallocate_bytes(block_info &&blk, alloc_info ai) const noexcept {
    if constexpr (check_the_alloc_info<version_v>) {
      deallocate_bytes_log_check_fix(blk, ai);
      auto ret = deallocate_bytes_impl_(this->ref, blk, ai);
      blk = block_info{};
      return ret;
    } else {
      return deallocate_bytes_impl_(this->ref, blk, ai);
    }
  }
  MJZ_CX_FN
  block_info allocate_bytes(uintlen_t minsize) const noexcept {
    return allocate_bytes(minsize, get_default_th_info());
  }
  MJZ_CX_FN success_t deallocate_bytes(block_info &&blk) const noexcept {
    return deallocate_bytes(std::move(blk), get_default_th_info());
  }
  template <typename T>
  MJZ_CX_ND_ALLOC_FN block_info_ot<T> allocate(uintlen_t count) const noexcept {
    return allocate<T>(count, get_default_th_info());
  }

  template <typename T>
  MJZ_CX_FN success_t deallocate(block_info_ot<T> &&blk) const noexcept {
    return deallocate<T>(std::move(blk), get_default_th_info());
  }
  template <typename T>
  MJZ_CX_ND_ALLOC_FN T *allocate_exact(uintlen_t count) const noexcept {
    return allocate<T>(count, get_default_th_ex_info()).ptr;
  }

  template <typename T>
  MJZ_CX_FN void deallocate_exact(T *ptr, uintlen_t count) const noexcept {
    block_info_ot<T> val{};
    val.ptr = ptr;
    val.length = count;
    asserts(asserts.assume_rn,
            deallocate(std::move(val), get_default_th_ex_info()));
  }

  template <typename T>
  MJZ_CX_ND_ALLOC_FN T *allocate_exact_no_threads(
      uintlen_t count) const noexcept {
    return allocate<T>(count, get_default_nth_ex_info()).ptr;
  }

  template <typename T>
  MJZ_CX_FN void deallocate_exact_no_threads(T *ptr,
                                             uintlen_t count) const noexcept {
    block_info_ot<T> val{};
    val.ptr = ptr;
    val.length = count;
    asserts(asserts.assume_rn,
            deallocate(std::move(val), get_default_nth_ex_info()));
  }
  MJZ_CX_FN
  const void_struct_t *handle(const void_struct_t *input) const noexcept {
    const auto func_ = get_vtbl().handle;
    if (!func_) return nullptr;
    return run(func_, input);
  }
  template <typename T>
  MJZ_CX_ND_ALLOC_FN block_info_ot<T> allocate(
      uintlen_t count, alloc_info strategy) const noexcept {
    strategy = preapare_strategy<T>(strategy);
    MJZ_IF_CONSTEVAL {
      return {std::allocator<T>().allocate((size_t)count), count};
    }
    block_info blk = allocate_bytes((size_t)count * sizeof(T), strategy);
    if (!blk.ptr) {
      return {};
    }
    asserts(asserts.believe_rn, blk.length == count * sizeof(T) && blk.ptr);
    return {reinterpret_cast<T *>(blk.ptr), count};
  }
  template <typename T>
  MJZ_CX_ND_ALLOC_FN block_info_ot<T> allocate_helper_(
      uintlen_t count, alloc_info strategy, const T *const) const noexcept {
    return allocate<T>(count, strategy);
  }
  template <typename T>
  MJZ_CX_FN success_t deallocate(block_info_ot<T> &&blk,
                                 alloc_info strategy) const noexcept {
    strategy = preapare_strategy<T>(strategy);

    MJZ_RELEASE {
      blk.ptr = nullptr;
      blk.length = 0;
    };
    MJZ_IF_CONSTEVAL {
      std::allocator<T>().deallocate(blk.ptr, (size_t)blk.length);
      return true;
    }
    return deallocate_bytes(
        block_info{reinterpret_cast<char *>(blk.ptr), blk.length * sizeof(T)},
        strategy);
  }
  template <typename T>
  MJZ_CX_FN may_bool_t is_owner(const block_info_ot<T> &block,
                                alloc_info strategy) const noexcept {
    strategy = preapare_strategy<T>(strategy);
    MJZ_IF_CONSTEVAL { return may_bool_t::idk; }
    return is_owner_of_bytes(block_info{reinterpret_cast<char *>(block.ptr),
                                        block.length * sizeof(T)},
                             strategy);
  }

  template <typename T>
  MJZ_CX_FN auto alloc_sigular_info_v(bool thread_safe,
                                      bool is_node) const noexcept {
    alloc_info info{get_default_info()};
    info.is_thread_safe = thread_safe;
    info.allocate_exactly_minsize = true;
    info.is_one_of_many_nodes = is_node;
    info.set_alignof_z(alignof(T));

    return info;
  };
  MJZ_CX_FN alloc_info
  alloc_sigular_preapare_info_v(uintlen_t size_,
                                               uintlen_t align_,
                                               bool thread_safe,
                                               bool is_node,bool exact_=true) const noexcept {
    alloc_info info{get_default_info()};
    info.is_thread_safe = thread_safe;
    info.allocate_exactly_minsize = exact_;
    info.is_one_of_many_nodes = is_node;
    info.size_multiplier =  size_;
    info.set_alignof_z(align_);
    return info;
  };
  template <typename T>
  MJZ_CX_FN T *allocate_singular_uninit(bool thread_safe,
                                        bool is_node) const noexcept {
    block_info_t<version_v, T> blk =
        allocate<T>(1, alloc_sigular_info_v<T>(thread_safe, is_node));
    asserts(asserts.assume_rn,
            (blk.length == 1 && blk.ptr) || (!blk.length && !blk.ptr));
    return blk.ptr;
  }
  template <typename T>
  MJZ_CX_FN void deallocate_singular_uninit(T *ptr, bool thread_safe,
                                            bool is_node) const noexcept {
    if (!ptr) return;
    block_info_t<version_v, T> blk{};
    blk.ptr = ptr;
    blk.length = 1;
    asserts(asserts.assume_rn,
            deallocate<T>(std::move(blk),
                          alloc_sigular_info_v<T>(thread_safe, is_node)));
  }
  template <typename T>
  MJZ_CX_FN T *allocate_plural_uninit(uintlen_t count,
                                      bool thread_safe) const noexcept {
    block_info_t<version_v, T> blk =
        allocate<T>(count, alloc_sigular_info_v<T>(thread_safe, false));
    asserts(asserts.assume_rn,
            (blk.length == count && blk.ptr) || (!blk.length && !blk.ptr));
    return blk.ptr;
  }
  template <typename T>
  MJZ_CX_FN void deallocate_plural_uninit(T *ptr, uintlen_t count,
                                          bool thread_safe) const noexcept {
    if (!ptr) return;
    block_info_t<version_v, T> blk{};
    blk.ptr = ptr;
    blk.length = count;
    asserts(asserts.assume_rn,
            deallocate<T>(std::move(blk),
                          alloc_sigular_info_v<T>(thread_safe, false)));
  }

  template <typename T>
  MJZ_CX_FN void deallocate_singular(T *ptr, bool thread_safe,
                                     bool is_node) const noexcept {
    if (!ptr) return;
    std::destroy_at(ptr);
    deallocate_singular_uninit<T>(ptr, thread_safe, is_node);
  }
  template <typename T, typename... Ts>
  MJZ_CX_FN T *allocate_singular(bool thread_safe, bool is_node,
                                 Ts &&...args) const noexcept {
    T *ptr = allocate_singular_uninit<T>(thread_safe, is_node);
    if (!ptr) return ptr;
    if constexpr (requires() {
                    {
                      std::construct_at(ptr, std::forward<Ts>(args)...)
                    } noexcept;
                  }) {
      std::construct_at(ptr, std::forward<Ts>(args)...);
      return ptr;
    } else {
      if (MJZ_NOEXCEPT { std::construct_at(ptr, std::forward<Ts>(args)...); }) {
        return ptr;
      }
      deallocate_singular_uninit<T>(ptr, thread_safe, is_node);
      return nullptr;
    }
  }
  template <typename T>
  MJZ_CX_FN void deallocate_plural(T *ptr, uintlen_t count,
                                   bool thread_safe) const noexcept {
    if (!ptr) return;
    std::destroy_n(ptr, count);
    deallocate_plural_uninit<T>(ptr, count, thread_safe);
  }
  template <typename T, typename... Ts>
  MJZ_CX_FN T *allocate_plural(uintlen_t count, bool thread_safe,
                               Ts &&...args) const noexcept {
    T *ptr = allocate_plural_uninit<T>(count, thread_safe);
    if (!ptr) return ptr;
    uintlen_t i{};
    if constexpr (requires() {
                    { std::construct_at(ptr, args...) } noexcept;
                  }) {
      for (; i < count; i++) {
        std::construct_at(ptr, args...);
      }
      return ptr;
    } else {
      if (MJZ_NOEXCEPT {
            for (; i < count; i++) {
              std::construct_at(ptr, args...);
            }
          }) {
        return ptr;
      }
      std::destroy_n(ptr, i);
      deallocate_plural_uninit<T>(ptr, count, thread_safe);
      return nullptr;
    }
  }

  template <typename T>
  MJZ_CX_FN T *allocate_node_uninit(bool thread_safe) const noexcept {
    return allocate_singular_uninit<T>(thread_safe, true);
  }
  template <typename T>
  MJZ_CX_FN void deallocate_node_uninit(T *ptr,
                                        bool thread_safe) const noexcept {
    deallocate_singular_uninit<T>(ptr, thread_safe, true);
  }
  template <typename T>
  MJZ_CX_FN T *allocate_obj_uninit(bool thread_safe) const noexcept {
    return allocate_singular_uninit<T>(thread_safe, false);
  }
  template <typename T>
  MJZ_CX_FN void deallocate_obj_uninit(T *ptr,
                                       bool thread_safe) const noexcept {
    deallocate_singular_uninit<T>(ptr, thread_safe, false);
  }

  template <typename T, typename... Ts>
  MJZ_CX_FN T *allocate_node(bool thread_safe, Ts &&...args) const noexcept {
    return allocate_singular<T>(thread_safe, true, std::forward<Ts>(args)...);
  }
  template <typename T>
  MJZ_CX_FN void deallocate_node(T *ptr, bool thread_safe) const noexcept {
    deallocate_singular<T>(ptr, thread_safe, true);
  }
  template <typename T, typename... Ts>
  MJZ_CX_FN T *allocate_obj(bool thread_safe, Ts &&...args) const noexcept {
    return allocate_singular<T>(thread_safe, false, std::forward<Ts>(args)...);
  }
  template <typename T>
  MJZ_CX_FN void deallocate_obj(T *ptr, bool thread_safe) const noexcept {
    deallocate_singular<T>(ptr, thread_safe, false);
  }

 public:
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN void dealloca_bytes(block_info &&blk,
                                uintlen_t align) const noexcept {
    alloc_info info = get_default_info();
    info.is_thread_safe = false;
    info.allocation_mode_val |= uint16_t(allocation_mode_e::alloca_mode);
    info.set_alignof_z(align);
    asserts(asserts.assume_rn, deallocate_bytes(std::move(blk), info));
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
  MJZ_CX_FN block_info alloca_bytes(uintlen_t min_size,
                                    uintlen_t align) const noexcept {
    alloc_info info = get_default_info();
    info.is_thread_safe = false;
    info.allocation_mode_val |= uint16_t(allocation_mode_e::alloca_mode);
    info.set_alignof_z(align);
    return allocate_bytes(min_size, info);
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
  template <uintlen_t align_v = 16>
  MJZ_CX_FN block_info alloca_bytes(
      stack_alloc_ns::stack_allocator_meta_t<version_v, align_v> &stack_alloc,
      uintlen_t min_size, uintlen_t align) const noexcept {
    return alloca_bytes(&stack_alloc, min_size, align);
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
  template <uintlen_t align_v = 16>
  MJZ_CX_FN block_info alloca_bytes(
      stack_alloc_ns::stack_allocator_meta_t<version_v, align_v> *stack_alloc,
      uintlen_t min_size, uintlen_t align) const noexcept {
    if (stack_alloc) {
      std::span<char> ret = stack_alloc->fn_alloca(min_size, align);
      if (ret.size()) {
        return {ret.data(), ret.size()};
      }
    }
    return alloca_bytes(min_size, align);
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
  template <uintlen_t align_v = 16>
  MJZ_CX_FN void dealloca_bytes(
      stack_alloc_ns::stack_allocator_meta_t<version_v, align_v> *stack_alloc,
      block_info &&blk, uintlen_t align) const noexcept {
    if (stack_alloc) {
      if (blk.ptr + blk.length == stack_alloc->sptr) {
        MJZ_RELEASE { blk = block_info{}; };
        return stack_alloc->fn_dealloca(std::span{blk.ptr, blk.length});
      }
    }
    return dealloca_bytes(std ::move(blk), align);
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
  template <uintlen_t align_v = 16>
  MJZ_CX_FN void dealloca_bytes(
      stack_alloc_ns::stack_allocator_meta_t<version_v, align_v> &stack_alloc,
      block_info &&blk, uintlen_t align) const noexcept {
    return dealloca_bytes(&stack_alloc, std::move(blk), align);
  }
  MJZ_CONSTANT(bool)
  has_thread_local_stack_v = !!thread_local_stack_size_v<version_v>;
  using thread_local_stack_ref_t =
      optional_ref_t<allocs_ns::stack_alloc_ns::stack_allocator_meta_t<
          version_v, uintlen_t(thread_local_stack_align_v<version_v>)>>;

  template <int idk_ = 0>
  MJZ_CX_FN thread_local_stack_ref_t thread_local_stack() const noexcept {
    if constexpr (has_thread_local_stack_v) {
      MJZ_IF_CONSTEVAL { return {}; }
      else {
        return []() noexcept {
          thread_local allocs_ns::stack_alloc_ns::areana_t<
              version_v, thread_local_stack_size_v<version_v>,
              uintlen_t(thread_local_stack_align_v<version_v>)>
              thread_local_stack_stroage;
          return &thread_local_stack_stroage;
        }();
      }
    } else {
      return {};
    }
  }

  template <class>
  friend class mjz_private_accessed_t;

 public:
  MJZ_CX_FN success_t release_all(uintlen_t monotonic_minsize,
                                  uintlen_t monotonic_min_align,
                                  uintlen_t stack_minsize,
                                  uintlen_t stack_min_align) const noexcept {
    return refresh_call_impl_(monotonic_minsize, monotonic_min_align,
                              stack_minsize, stack_min_align, true);
  }
  MJZ_CX_FN success_t refresh_cache(uintlen_t monotonic_minsize,
                                    uintlen_t monotonic_min_align,
                                    uintlen_t stack_minsize,
                                    uintlen_t stack_min_align) const noexcept {
    return refresh_call_impl_(monotonic_minsize, monotonic_min_align,
                              stack_minsize, stack_min_align, false);
  }

 private:
  MJZ_CX_FN success_t refresh_call_impl_(uintlen_t monotonic_minsize,
                                         uintlen_t monotonic_min_align,
                                         uintlen_t stack_minsize,
                                         uintlen_t stack_min_align,
                                         bool release_all) const noexcept {
    asserts(asserts.assume_rn, std::has_single_bit(monotonic_min_align) &&
                                   std::has_single_bit(stack_min_align));
    const auto refresh_call_fn = get_vtbl().refresh_call;
    if (!refresh_call_fn) {
      return false;
    }
    return run_grantee_table(*ref, refresh_call_fn, monotonic_minsize,
                             monotonic_min_align, stack_minsize,
                             stack_min_align, release_all);
  }

 private:
  template <typename T>
  MJZ_CX_ND_FN static alloc_info preapare_strategy(
      alloc_info strategy) noexcept {
    return strategy.consider_type(alias_t<T *>{});
  }
};

template <version_t version_v>
constexpr static const alloc_base_ref_t<version_v> empty_alloc{};

template <class T, version_t version_v, bool no_threads_v = false,
          bool do_throw = MJZ_CATCHES_EXCEPTIONS_>
class std_alloc_ref_t : public alloc_base_ref_t<version_v> {
 public:
  using self_t = std_alloc_ref_t;
  using alloc_base_ref_t<version_v>::alloc_base_ref_t;
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
  using is_always_equal = std::false_type;
  template <class U>
  struct rebind {
    using other = std_alloc_ref_t<U, version_v, no_threads_v, do_throw>;
  };
  MJZ_CX_ND_FN MJZ_MSVC_ONLY_CODE_(__declspec(allocator)) T *allocate(
      const size_t count) noexcept(!do_throw) {
    T *ret{};
    if constexpr (no_threads_v) {
      ret = this->template allocate_exact_no_threads<T>(uintlen_t(count));
    } else {
      ret = this->template allocate_exact<T>(uintlen_t(count));
    }
    if constexpr (do_throw) {
      if (!ret) {
        throw std::bad_alloc{};
      }
    }
    return ret;
  }

  MJZ_CX_FN void deallocate(T *const ptr, const size_t count) noexcept {
    if constexpr (no_threads_v) {
      return this->deallocate_exact_no_threads(ptr, uintlen_t(count));
    } else {
      return this->deallocate_exact(ptr, uintlen_t(count));
    }
  }
  MJZ_CX_ND_FN static size_type max_size() noexcept {
    return size_type(
        std::min(uintlen_t(std::min(static_cast<size_t>(PTRDIFF_MAX),
                                    static_cast<size_t>(-1))),
                 uintlen_t(-1) >> 8));
  }

  MJZ_CX_ND_FN bool operator==(const self_t &) const noexcept = default;
};

}  // namespace mjz::allocs_ns

#endif  // MJZ_ALLOCS_alloc_refs_FILE_HPP_