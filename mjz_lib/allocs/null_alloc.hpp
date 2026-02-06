
#include "../threads/mutex_ref.hpp"
#include "alloc_ref.hpp"

#ifndef MJZ_ALLOCS_NULL_alloc_FILE_HPP_
#define MJZ_ALLOCS_NULL_alloc_FILE_HPP_

MJZ_EXPORT namespace mjz ::allocs_ns {
  template <version_t version_v> struct null_alloc_t : alloc_base_t<version_v> {

  private:
    using generic_alloc = null_alloc_t<version_v>;
    using alloc_base = alloc_base_t<version_v>;
    using block_info = block_info_t<version_v>;
    using alloc_info = alloc_info_t<version_v>;
    using ref_count = ref_count_t<version_v>;
    using alloc_speed = alloc_speed_t<version_v>;
    using alloc_ref = alloc_base_ref_t<version_v>;

    using alloc_vtable = alloc_vtable_t<version_v>;
    MJZ_NO_MV_NO_CPY(null_alloc_t);

  public:
    MJZ_CX_FN null_alloc_t() noexcept : alloc_base{vtable_val_f()} {}
    MJZ_CX_FN ~null_alloc_t() noexcept {}

  private:
    MJZ_CX_FN static alloc_vtable vtable_val_f() noexcept {

      return {alloc_info{},   cow_threashold_v<version_v>,
              &allocate_call, &deallocate_call,
              nullptr,        nullptr,
              &is_owner,      nullptr};
    }

    MJZ_CX_FN static may_bool_t is_owner(const alloc_base *, const block_info &,
                                         alloc_info) noexcept {
      return may_bool_t::no;
    }

    MJZ_CX_FN static void deallocate_call(alloc_base *, block_info,
                                          alloc_info) noexcept {

      return;
    }
    MJZ_CX_FN static block_info allocate_call(alloc_base *, uintlen_t,
                                              alloc_info) noexcept {
      return {};
    }

  public:
    MJZ_CX_FN alloc_ref operator()() & noexcept { return +*this; }
    MJZ_CX_FN alloc_ref operator+() & noexcept { return alloc_ref{this, true}; }
  };

}; // namespace mjz::allocs_ns

#endif // MJZ_ALLOCS_NULL_alloc_FILE_HPP_