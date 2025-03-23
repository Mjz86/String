#include "alloc_ref.hpp"
#include "../threads/atomic_ref.hpp"
#if MJZ_WITH_iostream
#include <memory_resource> 
#else
#define MJZ_ALLOCS_pmr_adaptor_FILE_HPP_
#endif
#ifndef MJZ_ALLOCS_pmr_adaptor_FILE_HPP_
#define MJZ_ALLOCS_pmr_adaptor_FILE_HPP_
namespace mjz::allocs_ns {
template <version_t version_v>
class pmr_adaptor_t : public std::pmr::memory_resource {
 public:
  MJZ_NO_MV_NO_CPY(pmr_adaptor_t);
  MJZ_NCX_FN pmr_adaptor_t(const alloc_base_ref_t<version_v>& alloc) noexcept
      : ref(alloc) {}

 protected:
  const alloc_base_ref_t<version_v>& ref;
  MJZ_NCX_FN void* do_allocate(size_t count, size_t align) override final {
    alloc_info_t<version_v> info{};
    void* ptr =
        ref.allocate_bytes(
               count, info.set_alignof_z(align).make_allocate_exactly_minsize())
            .ptr;
    if (ptr) return ptr;
    return std::pmr::null_memory_resource()->allocate(count, align);
  }
  MJZ_NCX_FN void do_deallocate(void* ptr, size_t count,
                                size_t align) override final {
    alloc_info_t<version_v> info{};
    asserts(asserts.assume_rn,
            ref.deallocate_bytes(
                block_info_t<version_v>{reinterpret_cast<char*>(ptr), count},
                info.set_alignof_z(align).make_allocate_exactly_minsize()));
  }
  MJZ_NCX_FN
  bool do_is_equal(const memory_resource& self) const noexcept override final {
    return &self == this;
  };
};
template <version_t version_v>
struct pmr_alloc_t : alloc_base_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  using pmr_alloc = pmr_alloc_t<version_v>;
  using alloc_base = alloc_base_t<version_v>;
  using block_info = block_info_t<version_v>;
  using alloc_info = alloc_info_t<version_v>;
  using ref_count = ref_count_t<version_v>;
  using alloc_speed = alloc_speed_t<version_v>;
  using alloc_ref = alloc_base_ref_t<version_v>; 
  using alloc_vtable = alloc_vtable_t<version_v>; 
  MJZ_NO_MV_NO_CPY(pmr_alloc_t);

  template <class>
  friend class mjz_private_accessed_t;

 private:
  std::pmr::polymorphic_allocator<char> undelying{};
  /*
   * the first bit is used to determine if its heap allocated or not.
   */
  uintlen_t reference_count{}; 

 public: 
  MJZ_CX_FN pmr_alloc_t(std::pmr::polymorphic_allocator<char> pmr,
                            bool fast_table = false) noexcept
      : undelying{std::move(pmr)} {
    this->vt_ptr = fast_table ? &vt_stack_obj : &vt_obj; 
  }
  MJZ_CX_FN ~pmr_alloc_t() noexcept {
    asserts(asserts.condition_rn, 0 == reference_count,
            "the ref count must be 0 (deleted) , there is still an alive "
            "allocator reference . we must have been destructed indirectly "
            "by now !?"); 
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  MJZ_CX_FN static bool inequals(const alloc_ref& other,
                                 const alloc_base* This) noexcept { 
      return As(other.get_ptr()).undelying != As(This).undelying; 
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  template <version_t>
  friend struct pmr_alloc_maker_t;
  MJZ_CX_FN static auto&& As(alloc_base* ptr) noexcept {
    asserts(asserts.condition_rn, !!ptr);
    return static_cast<pmr_alloc&>(*ptr);
  }
  MJZ_CX_FN static auto&& As(const alloc_base* ptr) noexcept {
    asserts(asserts.condition_rn, !!ptr);
    return static_cast<const pmr_alloc&>(*ptr);
  }
  
  MJZ_NCX_FN block_info obj_allocate(uintlen_t minsize, alloc_info ai) noexcept { 
      block_info blk{};
    MJZ_NOEXCEPT{
        blk.ptr = reinterpret_cast<char*>(this->undelying.allocate_bytes(minsize, ai.get_alignof_z()));
      blk.length = minsize;
    };
    return blk;
  }
  MJZ_NCX_FN success_t obj_deallocate(block_info&& blk,
                                      alloc_info ai) noexcept {
    MJZ_NOEXCEPT {
      undelying.deallocate_bytes(blk.ptr, blk.length, ai.get_alignof_z());
    };
    return true;
  }

 public:
  MJZ_CX_FN static may_bool_t is_owner(const alloc_base*  ,
                                       const block_info&  ,
                                       alloc_info  ) noexcept { 
    return may_bool_t::idk;
  }

  MJZ_CX_FN
  alloc_relations_e static is_equal(const alloc_base* This,
                                    const alloc_ref& other) noexcept {
    if (This == other.get_ptr()) return alloc_relations_e::equal;
    if (!other || &other.get_vtbl() != &vt_obj || inequals(other, This)) {
      return alloc_relations_e::none;
    }
    return alloc_relations_e::equal;
  }
  MJZ_CX_FN static block_info allocate(alloc_base* This, uintlen_t minsize,
                                       alloc_info ai) noexcept {
    return As(This).obj_allocate(minsize, ai);
  }
  MJZ_CX_FN static success_t deallocate(alloc_base* This, block_info&& blk,
                                        alloc_info ai) noexcept {
    return As(This).obj_deallocate(std::move(blk), ai);
  }
  MJZ_CX_FN static success_t add_ref(alloc_base* This,
                                     intlen_t delta) noexcept {
    threads_ns:: atomic_ref_t<uintlen_t> rc{As(This).reference_count};
    delta *= 2;
    asserts(asserts.condition_rn, delta > 0 || uintlen_t(-delta) <= rc,
            "the ref count cant be negetive!");
    // negative makes it overflow , but with the correct result.
    rc += uintlen_t(delta);

    return true;
  }
  MJZ_CX_FN static ref_count num_ref(const alloc_base* This) noexcept {
    return ref_count(
        threads_ns::atomic_ref_t<const uintlen_t>(As(This).reference_count)
            .load() /
        2);
  }
 
  template <class>
  friend class mjz_private_accessed_t;

 private:
  using blk_t_ = typename alloc_base_ref_t<version_v>::template block_info_ot<
      pmr_alloc>;

 public:
  MJZ_CX_FN static success_t destroy_obj(alloc_base* This) noexcept {
    threads_ns::atomic_ref_t<uintlen_t> rc(As(This).reference_count);
    asserts(asserts.condition_rn, rc < 2,
            "the ref count must be zero to destroy the allocator!, this ref "
            "must be uniqe!");
    if (rc == 1) {
      rc = 0;
      std::destroy_at(This);

      std::pmr::polymorphic_allocator<char> upstream = As(This).undelying;
      MJZ_NOEXCEPT { upstream.deallocate_object(std::addressof(As(This)), 1); };
    }
    return true;
  }

  template <class>
  friend class mjz_private_accessed_t;

 private: 
  MJZ_CX_FN static alloc_ref static_create_obj(
      std::pmr::polymorphic_allocator<char> pmr) noexcept { 
      pmr_alloc* ptr{}; 
      MJZ_NOEXCEPT { ptr = pmr.template allocate_object<pmr_alloc>(1); };
    if (!ptr) return {};
    std::construct_at(ptr,pmr);
    ptr->reference_count = 0b11;

    return alloc_ref{static_cast<alloc_base*>(ptr), false};
  } 
  MJZ_CX_FN static pmr_alloc static_create_stack_obj(
      std::pmr::polymorphic_allocator<char> pmr, bool fast_table) noexcept {
    return pmr_alloc{pmr, fast_table};
  }

 public:
  MJZ_DEPRECATED_R("be careful! we dont want non ref counted refrences!")
  MJZ_CX_FN pmr_alloc* operator&() noexcept { return this; }
  MJZ_DEPRECATED_R("be careful! we dont want non ref counted refrences!")
  MJZ_CX_FN const pmr_alloc* operator&() const noexcept { return this; }
  MJZ_CX_FN alloc_ref operator()() & noexcept { return +*this; }
  MJZ_CX_FN alloc_ref operator+() & noexcept { return alloc_ref{this, true}; }

  template <class>
  friend class mjz_private_accessed_t;

 private: 

  MJZ_CONSTANT(alloc_vtable)
  vt_obj{&is_owner,    &is_equal,
         &allocate,    &deallocate,
         &add_ref,     &num_ref,
         &destroy_obj,   nullptr};
  MJZ_CONSTANT(alloc_vtable)
  vt_stack_obj{&is_owner,&is_equal,
               &allocate, &deallocate,
               nullptr,   nullptr,
               nullptr,    nullptr};
};

template <version_t version_v>
struct pmr_alloc_maker_t {
  MJZ_CE_FN
  pmr_alloc_maker_t(totally_empty_type_t) noexcept {}
  using A_t=std::pmr::polymorphic_allocator<char>;
  MJZ_NCX_FN alloc_base_ref_t<version_v> operator()(
      A_t pmr = A_t{}) const noexcept {
    return pmr_alloc_t<version_v>::static_create_obj(pmr);
  } 
  MJZ_NCX_FN auto on_stack(A_t pmr = A_t{},
                          bool fast_table = false) const noexcept {
    return pmr_alloc_t<version_v>::static_create_stack_obj(pmr,
                                fast_table);
  };
};

template <version_t version_v>
MJZ_CONSTANT(pmr_alloc_maker_t<version_v>)
pmr_alloc_maker{totally_empty_type};
};      // namespace mjz::allocs_ns
#endif  // MJZ_ALLOCS_pmr_adaptor_FILE_HPP_