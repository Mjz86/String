#include "../threads/mutex_ref.hpp"
#include "alloc_ref.hpp"

#ifndef MJZ_ALLOCS_generic_alloc_FILE_HPP_
#define MJZ_ALLOCS_generic_alloc_FILE_HPP_

namespace mjz ::allocs_ns {
template <version_t version_v, class generic_alloc_init_t>
struct generic_alloc_t : alloc_base_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  using generic_alloc = generic_alloc_t<version_v, generic_alloc_init_t>;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>;
  using AI_t = std::remove_cvref_t<generic_alloc_init_t>;
  using alloc_vtable = alloc_vtable_t<version_v>;
  using obj_t = typename AI_t::obj_t;
  MJZ_NO_MV_NO_CPY(generic_alloc_t);

  template <class>
  friend class mjz_private_accessed_t;

 private:
  union data_u_t {
    obj_t obj;
    MJZ_NO_MV_NO_CPY(data_u_t);
    MJZ_DISABLE_WANINGS_START_;
    MJZ_CX_FN ~data_u_t() noexcept {}
    MJZ_CX_FN data_u_t() noexcept {}
    MJZ_DISABLE_WANINGS_END_;
  };
  alloc_ref upstream{};
  /*
   * the first bit is used to determine if its heap allocated or not.
   */
  uintlen_t reference_count{};
  data_u_t data{};

 public:
  template <partial_same_as<generic_alloc_init_t> T>
  MJZ_CX_FN generic_alloc_t(alloc_ref &&upstream_alloc, T &&ai,
                            bool fast_table = false) noexcept
      : upstream{std::move(upstream_alloc)} {
    this->vt_ptr = fast_table ? &vt_stack_obj : &vt_obj;
    std::construct_at(&data.obj, std::forward<T>(ai));
  }
  MJZ_CX_FN ~generic_alloc_t() noexcept {
    asserts(asserts.condition_rn, 0 == reference_count,
            "the ref count must be 0 (deleted) , there is still an alive "
            "allocator reference . we must have been destructed indirectly "
            "by now !?");
    std::destroy_at(&data.obj);
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CONSTANT(bool)
  Const_inequals = !requires(const obj_t &obj) {
    { obj != obj } noexcept -> std::same_as<bool>;
  };
  MJZ_CONSTANT(bool)
  has_handle = requires(const alloc_base *This, const void_struct_t *ptr) {
    {
      As(This).data.obj.handle(ptr)
    } noexcept -> std::same_as<const void_struct_t *>;
  };
  MJZ_CX_FN static bool inequals(const alloc_ref &other,
                                 const alloc_base *This) noexcept {
    if constexpr (!Const_inequals) {
      return As(other.get_ptr()).get() != As(This).get();
    }
    return &As(other.get_ptr()).get() != &As(This).get();
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <version_t>
  friend struct generic_alloc_maker_t;
  MJZ_CX_FN static auto &&As(alloc_base *ptr) noexcept {
    asserts(asserts.condition_rn, !!ptr);
    return static_cast<generic_alloc &>(*ptr);
  }
  MJZ_CX_FN static auto &&As(const alloc_base *ptr) noexcept {
    asserts(asserts.condition_rn, !!ptr);
    return static_cast<const generic_alloc &>(*ptr);
  }
  MJZ_CX_FN bool obj_is_owner(const block_info &blk,
                              alloc_info ai) const noexcept {
    static_assert(
        requires() {
          { get().is_owner(blk, ai) } noexcept -> std::convertible_to<bool>;
        }, " we shall know the truth!");
    return get().is_owner(blk, ai);
  }
  MJZ_CX_FN block_info obj_allocate(uintlen_t minsize, alloc_info ai) noexcept {
    if (block_info obj = get().allocate(minsize, ai); !!obj.ptr) {
      return obj;
    }
    if (ai.uses_upstream_forwarding && upstream) {
      return upstream.allocate_bytes(minsize, ai);
    }
    if (ai.uses_default_upstream_forwarding && !upstream) {
      return upstream.allocate_bytes(minsize, ai);
    }

    return {};
  }
  MJZ_CX_FN success_t obj_deallocate(block_info &&blk, alloc_info ai) noexcept {
    if (obj_is_owner(blk, ai)) {
      return get().deallocate(std::move(blk), ai);
    }
    if (ai.uses_upstream_forwarding && upstream) {
      return upstream.deallocate_bytes(std::move(blk), ai);
    }
    if (ai.uses_default_upstream_forwarding && !upstream) {
      return upstream.deallocate_bytes(std::move(blk), ai);
    }
    asserts(asserts.expect_rn, blk.ptr == nullptr,
            "why is the wrong allocator being used! UB!!");
    return false;
  }

 public:
  MJZ_CX_FN static may_bool_t is_owner(const alloc_base *This,
                                       const block_info &blk,
                                       alloc_info ai) noexcept {
    if (As(This).obj_is_owner(blk, ai)) return may_bool_t::yes;
    if (ai.uses_upstream_forwarding && As(This).upstream) {
      return As(This).upstream.is_owner(blk, ai);
    }
    if (ai.uses_default_upstream_forwarding && !As(This).upstream) {
      return As(This).upstream.is_owner(blk, ai);
    }
    return may_bool_t::no;
  }

  MJZ_CX_FN
  alloc_relations_e static is_equal(const alloc_base *This,
                                    const alloc_ref &other) noexcept {
    if (This == other.get_ptr()) return alloc_relations_e::equal;
    if (!other || &other.get_vtbl() != &vt_obj || inequals(other, This)) {
      return alloc_relations_e::none;
    }
    return alloc_relations_e::equal;
  }
  MJZ_CX_FN static block_info allocate(alloc_base *This, uintlen_t minsize,
                                       alloc_info ai) noexcept {
    return As(This).obj_allocate(minsize, ai);
  }
  MJZ_CX_FN static success_t deallocate(alloc_base *This, block_info &&blk,
                                        alloc_info ai) noexcept {
    return As(This).obj_deallocate(std::move(blk), ai);
  }
  MJZ_CX_FN static success_t add_ref(alloc_base *This,
                                     intlen_t delta) noexcept {
    threads_ns::atomic_ref_t<uintlen_t> rc{As(This).reference_count};
    delta *= 2;
    if constexpr(MJZ_IN_DEBUG_MODE) {
      asserts(
          asserts.condition_rn,
          delta > 0 || uintlen_t(-delta) <= rc.load(std::memory_order_relaxed),
          "the ref count cant be negetive!");
    }
    // negative makes it overflow , but with the correct result.
    rc.fetch_add(uintlen_t(delta),
                 0<uintlen_t(delta) ? std::memory_order_acquire : std::memory_order_release);

    return true;
  }
  MJZ_CX_FN static ref_count num_ref(const alloc_base *This,std::memory_order mo) noexcept {
    return ref_count(
        threads_ns::atomic_ref_t<const uintlen_t>(As(This).reference_count)
            .load(mo) /
        2);
  }

  MJZ_CX_FN
  static const void_struct_t *handle(const alloc_base *This,
                                     const void_struct_t *ptr) noexcept {
    if constexpr (has_handle) {
      return As(This).get().handle(ptr);
    }
    return nullptr;
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  using blk_t_ = typename alloc_base_ref_t<version_v>::template block_info_ot<
      generic_alloc>;

 public:
  MJZ_CX_FN static success_t destroy_obj(alloc_base *This) noexcept {
    threads_ns::atomic_ref_t<uintlen_t> rc(As(This).reference_count);

    if constexpr (MJZ_IN_DEBUG_MODE) {
      asserts(asserts.condition_rn, rc.load(std::memory_order_relaxed) < 2,
              "the ref count must be zero to destroy the allocator!, this ref "
              "must be uniqe!");
    }
    if (rc.load(std::memory_order_relaxed) == 1) {
      rc.store(0, std::memory_order_relaxed) ;
      std::destroy_at(This);
      As(This).upstream.deallocate(
          blk_t_{std::addressof(As(This)), 1},
          alloc_info::this_s_alloc_info(
              std::align_val_t(alignof(generic_alloc_init_t))));
    }
    return true;
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <partial_same_as<generic_alloc_init_t> T>
  MJZ_CX_FN static alloc_ref static_create_obj(alloc_ref &&where,
                                               T &&ai) noexcept {
    blk_t_ blk = where.allocate_helper_(
        1,
        alloc_info::this_s_alloc_info(
            std::align_val_t(alignof(generic_alloc_init_t))),
        alias_t<generic_alloc *>{});
    std::construct_at(blk.ptr, std::move(where), std::forward<T>(ai));
    blk.ptr->reference_count = 0b11;

    return alloc_ref{static_cast<alloc_base *>(blk.ptr), false};
  }
  template <partial_same_as<generic_alloc_init_t> T>
  MJZ_CX_FN static generic_alloc static_create_stack_obj(
      alloc_ref &&where, T &&ai, bool fast_table) noexcept {
    return generic_alloc{std::move(where), std::forward<T>(ai), fast_table};
  }

 public:
  MJZ_DEPRECATED_R("be careful! we dont want non ref counted refrences!")
  MJZ_CX_FN generic_alloc *operator&() noexcept { return this; }
  MJZ_DEPRECATED_R("be careful! we dont want non ref counted refrences!")
  MJZ_CX_FN const generic_alloc *operator&() const noexcept { return this; }
  MJZ_CX_FN alloc_ref operator()() & noexcept { return +*this; }
  MJZ_CX_FN alloc_ref operator+() & noexcept { return alloc_ref{this, true}; }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_FN auto &&get() noexcept { return *std::launder(&data.obj); }
  MJZ_CX_FN auto &&get() const noexcept { return *std::launder(&data.obj); }

  MJZ_CONSTANT(alloc_vtable)
  vt_obj{&is_owner,    Const_inequals ? nullptr : &is_equal,
         &allocate,    &deallocate,
         &add_ref,     &num_ref,
         &destroy_obj, has_handle ? &handle : nullptr};
  MJZ_CONSTANT(alloc_vtable)
  vt_stack_obj{&is_owner, Const_inequals ? nullptr : &is_equal,
               &allocate, &deallocate,
               nullptr,   nullptr,
               nullptr,   has_handle ? &handle : nullptr};
};

template <version_t version_v>
struct generic_alloc_maker_t {
  MJZ_CE_FN
  generic_alloc_maker_t(totally_empty_type_t) noexcept {}
  template <class T>
  MJZ_CX_FN alloc_base_ref_t<version_v> operator()(
      alloc_base_ref_t<version_v> where, T &&ai) const noexcept {
    return generic_alloc_t<
        version_v, std::remove_cvref_t<T>>::static_create_obj(std::move(where),
                                                              std::forward<T>(
                                                                  ai));
  }
  template <class T>
  MJZ_CX_FN auto on_stack(alloc_base_ref_t<version_v> where, T &&ai,
                          bool fast_table = false) const noexcept {
    return generic_alloc_t<version_v, std::remove_cvref_t<T>>::
        static_create_stack_obj(std::move(where), std::forward<T>(ai),
                                fast_table);
  };
};
namespace lock_details_ns {
/*
 * for the  threads_ns::mutex_ref_t<char> destructor "not being called"
 * warning.
 */
MJZ_DISABLE_ALL_WANINGS_START_;
template <version_t version_v>
MJZ_CX_FN auto lock_gaurd_maker(bool is_threaded, char *byte_mutex_ptr,
                                bool has_lock = true,
                                bool not_lock = false) noexcept {
  struct A {
    MJZ_NO_MV_NO_CPY(A);
    union U {
      threads_ns::mutex_ref_t<char> mutex;
      char dummy_;
      MJZ_NO_MV_NO_CPY(U);
      MJZ_CX_FN U() noexcept {}
      MJZ_CX_FN ~U() noexcept {}
    } u;
    MJZ_DISABLE_ALL_WANINGS_END_;
    bool has_mutex{};
    bool is_not_threaded{};
    MJZ_CX_FN ~A() noexcept {
      if (!has_mutex) return;
      u.mutex.unlock();
      std::destroy_at(&u.mutex);
    }
    MJZ_CX_FN A(threads_ns::mutex_ref_t<char> m) noexcept {
      std::construct_at(&u.mutex, std::move(m));
      has_mutex = true;
      u.mutex.lock();
    }
    MJZ_CX_FN A(bool is_not_threaded_, totally_empty_type_t) noexcept
        : is_not_threaded{is_not_threaded_} {}
    MJZ_CX_FN explicit operator bool() const noexcept {
      return is_not_threaded || has_mutex;
    };
  };
  if (not_lock) {
    return A{true, totally_empty_type};
  }
  if (!has_lock) {
    return A{!is_threaded, totally_empty_type};
  }
  if (!byte_mutex_ptr) {
    return A{false, totally_empty_type};
  }
  if (!is_threaded) return A{true, totally_empty_type};
  return A{threads_ns::atomic_ref_t<char>(*byte_mutex_ptr)};
}
};  // namespace lock_details_ns

template <version_t version_v>
MJZ_CONSTANT(generic_alloc_maker_t<version_v>)
generic_alloc_maker{totally_empty_type};
};  // namespace mjz::allocs_ns

#endif  // MJZ_ALLOCS_generic_alloc_FILE_HPP_