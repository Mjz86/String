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
#include "../optional_ref.hpp"
#include "../versions.hpp"
#if MJZ_LOG_ALLOCATIONS_
#include "../outputs.hpp"
#endif
#include <stdlib.h>

#include <cstdlib>
#include <memory_resource>
#include <new>
#include <numeric>
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
constexpr static const bool check_the_alloc_info{MJZ_IN_DEBUG_MODE};

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

  MJZ_CX_FN bool operator==(const alloc_info_t &) const noexcept = default;

 private:
  MJZ_CX_ND_FN static uint8_t log2_of_align_val_create_z(size_t val) noexcept {
    uint8_t ret{};
    while (val >>= 1) {
      ret++;
    }
    return ret;
  }
  MJZ_CX_ND_FN static size_t log2_of_align_val_to_val_z(
      uint8_t log2_val) noexcept {
    return static_cast<size_t>(1ull << log2_val);
  }

 public:
  uintlen_t size_multiplier{1};
  uint16_t log2_of_align_val : 6 {0};
  uint16_t time_threashhold : 2 {3};
  uint16_t uses_upstream_forwarding : 1 {1};
  uint16_t uses_default_upstream_forwarding : 1 {1};
  uint16_t allocate_exactly_minsize : 1 {0};
  uint16_t uses_munual_alignment : 1 {1};
  uint16_t is_thread_safe : 1 {1};
  uint16_t uses_best_fit : 1 {1};
  uint16_t is_one_of_many_nodes : 1 {0};
  uint16_t can_do_allocator_regrowth : 1 {1};

 public:
  template <class T>
  MJZ_CX_ND_FN alloc_info_t &consider_type(T *) noexcept {
    size_multiplier = std::lcm(size_multiplier, sizeof(T));
    set_alignof_z(std::max(get_alignof_z(), alignof(T)));
    return *this;
  }
  MJZ_CX_ND_FN alloc_info_t &cant_bother_with_good_size() noexcept {
    allocate_exactly_minsize |=
        std::lcm(size_multiplier, get_alignof_z()) != get_alignof_z();
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
    using alloc_call = F_t<void(block_info &blk, alloc_info) noexcept>;
    using ref_call = F_t<void(bool add_vs_destroy) noexcept>;
    using handle =
        F_t<const void_struct_t *(const void_struct_t *) const noexcept>;

    template <class>
    friend class mjz_private_accessed_t;

   private:  // not useful rn.
    using allocate = F_t<block_info(uintlen_t, alloc_info) noexcept>;
    using deallocate = F_t<success_t(block_info &&, alloc_info) noexcept>;
    using add_ref = F_t<success_t(intlen_t) noexcept>;
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
    typename funcs_t::allocate allocate;
    typename funcs_t::deallocate deallocate;
    typename funcs_t::add_ref add_ref;
    typename funcs_t::num_ref num_ref;
    typename funcs_t::destroy_obj destroy_obj;
  };
  static_assert(std::gcd(cow_threashold_v<version_v>,
                         hardware_destructive_interference_size) ==
                hardware_destructive_interference_size);

 public:
  typename funcs_t::alloc_call alloc_call{};
  typename funcs_t::ref_call ref_call{};
  typename funcs_t::is_equal is_equal{};
  typename funcs_t::is_owner is_owner{};
  typename funcs_t::handle handle{};
  uintlen_t cow_threashold{cow_threashold_v<version_v>};
  alloc_info default_info{};
  MJZ_CX_FN bool operator==(const alloc_vtable_t &rhs) const noexcept = default;
};

template <version_t version_v>
struct alloc_base_t : void_struct_t {
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
              vtable_.cow_threashold += alias_t<uintlen_t[2]>{
                  0, min}[vtable_.cow_threashold != vtable_val.cow_threashold];
              return vtable_;
            },
            void_struct_t{}) {}

 private:
  MJZ_CX_FN alloc_base_t(auto &&lam, void_struct_t) noexcept : vtable(lam()) {}
};

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
  using bptr_t = const alloc_base *;
  alignas(sizeof(uintlen_t)) alloc_base *ref{};
  template <typename... Ts>
  MJZ_CX_FN auto run(auto &&fn, Ts &&...args) const noexcept {
    return fn(get_ptr(), std::forward<Ts>(args)...);
  }

 public:
  MJZ_CX_FN alloc_vtable_t<version_v> get_vtbl() const noexcept {
    constexpr const alloc_base dummy{};
    bptr_t vptr = alias_t<bptr_t[2]>{&dummy, this->ref}[!!this->ref];
    return vptr->vtable;
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
    if (!get_vtbl().ref_call) return true;
    MJZ_RELEASE { this->ref = nullptr; };
    run(get_vtbl().ref_call, false);
    return true;
  }
  MJZ_CX_FN
  success_t add_ref() const noexcept {
    if (!get_vtbl().ref_call) return true;
    run(get_vtbl().ref_call, true);
    return true;
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
    if (!get_vtbl().is_owner) return may_bool_t::idk;
    return run(get_vtbl().is_owner, blk, ai);
  }
  MJZ_CX_FN
  alloc_relations_e is_equal(const alloc_ref &ar) const noexcept {
    bool no_call{};
    bool is_eq = this->ref == ar.ref;
    is_eq |= !get_vtbl().is_equal;
    if (no_call) {
      return alias_t<alloc_relations_e[2]>{alloc_relations_e::none,
                                           alloc_relations_e::equal}[is_eq];
    }
    return run(get_vtbl().is_equal, ar);
  }
  MJZ_CX_FN bool operator==(const alloc_ref &ar) const noexcept {
    return is_equal(ar) == alloc_relations_e::equal;
  }

 private:
  MJZ_NCX_FN static void *nothrow_new_th(size_t size,
                                         std::align_val_t align) noexcept {
    void *ptr{};
    if constexpr (!uses_pmr_sync_v<version_v>) {
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
    if constexpr (!uses_pmr_async_v<version_v>) {
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
    if constexpr (!uses_pmr_async_v<version_v>) {
      return nothrow_delete_th(ptr, size, align);
    } else if (is_threaded) {
      return nothrow_delete_th(ptr, size, align);
    } else {
      return nothrow_delete_nth(ptr, size, align);
    }
  }

  MJZ_NCX_FN static void nothrow_delete_th(void *ptr, size_t size,
                                           std::align_val_t align) noexcept {
    if constexpr (!uses_pmr_sync_v<version_v>) {
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

  MJZ_CX_FN static block_info global_alloc(uintlen_t minsize,
                                           alloc_info ai) noexcept {
    auto align_val = ai.get_alignof();
    uintlen_t size = minsize;
    MJZ_IFN_CONSTEVAL {
      void *ptr{};
      ptr = nothrow_new((size_t)size, align_val, ai.is_thread_safe);
#if MJZ_LOG_NEW_ALLOCATIONS_
      MJZ_NOEXCEPT { mjz_debug_cout::println("[new:", size, "]"); };
#endif
      size = alias_t<uintlen_t[2]>{0, size}[!!ptr];
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
      MJZ_NOEXCEPT { mjz_debug_cout::println("[delete:", blk.length, "]"); };
#endif
      nothrow_delete(blk.ptr, blk.length, ai.get_alignof(), ai.is_thread_safe);
      return;
    }
    delete[] blk.ptr;
    return;
  }
  MJZ_CX_FN block_info local_alloc(uintlen_t minsize,
                                   alloc_info ai) const noexcept {
    block_info blk{};
    if (!minsize) return blk;
    blk.length = minsize;
    run(get_vtbl().alloc_call, blk, ai);
#if MJZ_LOG_ALLOC_ALLOCATIONS_
    MJZ_NOEXCEPT { mjz_debug_cout::println("[alloc:", blk.length, "]"); };
#endif
    /* lets assume this isnt needed beacuse of c++20:
   new (ptr) char[(size_t)size]*/
    return blk;
  }
  MJZ_CX_FN void local_dealloc(block_info blk, alloc_info ai) const noexcept {
    if (!blk.ptr) return;
#if MJZ_LOG_ALLOC_ALLOCATIONS_
    MJZ_NOEXCEPT { mjz_debug_cout::println("[dealloc:", blk.length, "]"); };
#endif
    run(get_vtbl().alloc_call, blk, ai);
    blk = block_info{};
    return;
  }
  MJZ_CX_FN
  block_info allocate_bytes_impl_(uintlen_t minsize,
                                  alloc_info ai) const noexcept {
    if (!get_vtbl().alloc_call) {
      return global_alloc(minsize, ai);
    }
    return local_alloc(minsize, ai);
  }
  MJZ_CX_FN
  success_t deallocate_bytes_impl_(block_info blk,
                                   alloc_info ai) const noexcept {
    if (!get_vtbl().alloc_call) {
      global_dealloc(blk, ai);
      return true;
    }
    local_dealloc(blk, ai);
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
    alloc_info ai{};
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
    ai.allocate_exactly_minsize = true;
    minsize += sizeof(alloc_log_info);
  }
  MJZ_CX_FN void allocate_bytes_log_fix(MJZ_MAYBE_UNUSED block_info &ret,
                                        MJZ_MAYBE_UNUSED alloc_info
                                            old_ai) const noexcept {
    if (!ret.ptr) return;
    ret.length -= sizeof(alloc_log_info);
    char *alloc_log_info_ptr = ret.ptr + ret.length;
    MJZ_IF_CONSTEVAL {
      cx_alloc_log_info log{};
      log.ai = old_ai;
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
    if (!blk.ptr) return;
    MJZ_RELEASE {
      ai.allocate_exactly_minsize = true;
      blk.length += sizeof(alloc_log_info);
    };
    char *alloc_log_info_ptr = blk.ptr + blk.length;
    MJZ_IF_CONSTEVAL {
      cx_alloc_log_info log{};
      log.ai = ai;
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
      block_info ret = allocate_bytes_impl_(minsize, ai);
      allocate_bytes_log_fix(ret, old_ai);
      return ret;
    } else {
      return allocate_bytes_impl_(minsize, ai);
    }
  }
  MJZ_CX_FN alloc_info get_default_info() const noexcept {
    return get_vtbl().default_info;
  }

  MJZ_CX_FN
  success_t deallocate_bytes(block_info &&blk, alloc_info ai) const noexcept {
    if constexpr (check_the_alloc_info<version_v>) {
      deallocate_bytes_log_check_fix(blk, ai);
      MJZ_RELEASE { blk = block_info{}; };
      return deallocate_bytes_impl_(blk, ai);
    } else {
      return deallocate_bytes_impl_(blk, ai);
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
    if (!get_vtbl().handle) return nullptr;
    return run(get_vtbl().handle, input);
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
  MJZ_CX_FN static auto alloc_sigular_info_v(bool thread_safe,
                                             bool is_node) noexcept {
    alloc_info info{};
    if (thread_safe) {
      info.make_threaded();
    }
    info.allocate_exactly_minsize = true;
    info.is_one_of_many_nodes = is_node;
    info.set_alignof_z(alignof(T));
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

  template <class>
  friend class mjz_private_accessed_t;

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
    using other = std_alloc_ref_t<U, version_v,no_threads_v, do_throw>;
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
    }else {
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