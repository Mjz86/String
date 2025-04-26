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


#include "../threads/atomic_ref.hpp"
#include "views.hpp"
#ifndef MJZ_BYTE_STRING_basic_lazy_str_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_basic_lazy_str_LIB_HPP_FILE_
namespace mjz ::bstr_ns {
namespace lazy_abi_ {
template <class T, version_t version_v>
concept lazy_reader_fn_c =
    requires(T& lazy_reader, basic_string_view_t<version_v> read_slice_) {
      { lazy_reader(read_slice_) } noexcept -> std::same_as<success_t>;
    };
template <version_t version_v>
struct basic_lazy_str_t;
template <version_t version_v>
struct basic_lazy_vtable_t {
  using obj = basic_lazy_str_t<version_v>;
  success_t (*iterate_fnp)(const obj&, uintlen_t offset, uintlen_t length,
                           lazy_reader_fn<version_v> lazy_reader) noexcept {};
  success_t (*copy_move_destroy_fnp)(obj* dest /*nullptr means destroy src*/,
                                     const obj& src,
                                     bool move_const_cast) noexcept {};
};

template <version_t version_v>
struct alignas(hardware_destructive_interference_size) heap_storage_base_t {
  uintlen_t reference_count{1};
};
template <version_t version_v, class T>
struct heap_storage_t : heap_storage_base_t<version_v> {
  T object;
  template <class... Us>
  MJZ_CX_FN heap_storage_t(Us&&... args) noexcept
      : object(std::forward<Us>(args)...) {};
  MJZ_CX_FN ~heap_storage_t() noexcept = default;
  MJZ_NO_MV_NO_CPY(heap_storage_t);
};
template <version_t version_v>
struct lazy_ref_t {
  allocs_ns::alloc_base_ref_t<version_v> alloc{};
  heap_storage_base_t<version_v>* object{};
  bool is_threaded{};
  MJZ_DEFAULTED_CLASS(lazy_ref_t);
};
MJZ_DISABLE_ALL_WANINGS_START_;
template <version_t version_v>
union alignas(alignof(uintlen_t)) lazy_storage_u {
  char dummy_{};
  char raw[sizeof(uintlen_t)*4];
  lazy_ref_t<version_v> lazy_ref;
  MJZ_UNSAFE_UNION(lazy_storage_u);
};
MJZ_DISABLE_ALL_WANINGS_END_;
template <version_t version_v>
struct basic_lazy_str_t {
  MJZ_CX_FN basic_lazy_str_t() noexcept {};
  MJZ_CX_FN basic_lazy_str_t(basic_lazy_str_t&& obj) noexcept {
    init_with(obj, true);
  }
  MJZ_CX_FN basic_lazy_str_t(const basic_lazy_str_t& obj) noexcept {
    init_with(obj, false);
  }
  MJZ_CX_FN basic_lazy_str_t& operator=(basic_lazy_str_t&& obj) noexcept {
    return init_with(obj, true);
  }
  MJZ_CX_FN basic_lazy_str_t& operator=(const basic_lazy_str_t& obj) noexcept {
    return init_with(obj, false);
  }
  MJZ_CX_FN ~basic_lazy_str_t() noexcept { reset(); }
  MJZ_CX_FN void reset() noexcept {
    if (!vtable) return;
    asserts(asserts.assume_rn,
            vtable->copy_move_destroy_fnp(nullptr, *this, true));
    vtable = nullptr;
  }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!vtable; }
  MJZ_CX_FN success_t
  iterate(uintlen_t offset, uintlen_t length,
          lazy_reader_fn_c<version_v> auto&& lazy_reader) const noexcept {
    if (!vtable) return false; 
    return vtable->iterate_fnp(
        *this, offset, length,
        +no_type_ns::make<lazy_reader_fnt<version_v>>(
            [&](base_string_view_t<version_v> read_slice) noexcept
                -> success_t {
              return lazy_reader(basic_string_view_t<version_v>(read_slice));
            }));
  }

 private:
  MJZ_CX_FN basic_lazy_str_t& init_with(const basic_lazy_str_t& src,
                                        bool move_construct) noexcept {
    reset();
    if (!src.vtable) return *this;
    if (src.vtable->copy_move_destroy_fnp(this, src, move_construct))
      return *this;
    reset();
    return *this;
  }
  template <class T>
  struct vtable_t : basic_lazy_vtable_t<version_v> {
    MJZ_CX_FN static success_t iterate_fn_(
        const basic_lazy_str_t& object, uintlen_t offset, uintlen_t length,
        lazy_reader_fn<version_v> lazy_reader) noexcept {
      return ref(object)(
          offset, length,
          [&lazy_reader](
              basic_string_view_t<version_v> read_slice)   noexcept
              -> success_t {
            return lazy_reader.run(read_slice.unsafe_handle());
          });
    }

    MJZ_CX_FN static success_t copy_move_destroy_fn_(
        basic_lazy_str_t* dest_p /*nullptr means destroy src*/,
        const basic_lazy_str_t& src, bool move_const_cast) noexcept {
      if (!dest_p) {
        asserts(asserts.assume_rn, move_const_cast);
        destroy_fn(const_cast<basic_lazy_str_t&>(src));
        return true;
      }

      const vtable_t* This(static_cast<const vtable_t*>(src.vtable));
      basic_lazy_str_t& dest = *dest_p;
      auto& src_ref = ref(src);
      if constexpr (small_enough) {
        if (!move_const_cast) {
          return This->init(dest, src.storage.lazy_ref.alloc,
                            src.storage.lazy_ref.is_threaded, src_ref);
        }
        MJZ_RELEASE { destroy_fn(const_cast<basic_lazy_str_t&>(src)); };
        return This->init(dest, src.storage.lazy_ref.alloc,
                          src.storage.lazy_ref.is_threaded, std::move(src_ref));
      }
      if (move_const_cast) {
        basic_lazy_str_t& src_obj_ref = const_cast<basic_lazy_str_t&>(src);
        dest.vtable = std::exchange(src_obj_ref.vtable, {});
        dest.storage.lazy_ref = std::exchange(src_obj_ref.storage.lazy_ref, {});
        return true;
      }
      dest.vtable = src.vtable;
      dest.storage.lazy_ref = src.storage.lazy_ref;
      if (dest.storage.lazy_ref.is_threaded) {
        threads_ns::atomic_ref_t<uintlen_t>(
            dest.storage.lazy_ref.object->reference_count).fetch_add(1,std::memory_order_acquire);
      } else {
        dest.storage.lazy_ref.object->reference_count++;
      }
      return true;
    }

    MJZ_CONSTANT(bool)
    small_enough = requires(const T val) {
      requires(sizeof(T) <= sizeof(lazy_storage_u<version_v>));
      requires(alignof(T) <= alignof(lazy_storage_u<version_v>));
      requires std::is_nothrow_copy_constructible_v<T>;
      requires std::is_nothrow_move_constructible_v<T>;
    };
    using hs_t = heap_storage_t<version_v, T>;
    template <typename... Ts>
    MJZ_CX_FN success_t
    init(basic_lazy_str_t& dest,
         const allocs_ns::alloc_base_ref_t<version_v>& alloc, bool is_threaded,
         Ts&&... args) const& noexcept {
      auto alloc_path = [&]() noexcept {
        hs_t* ref = alloc.template allocate_node<hs_t>(
            is_threaded, std::forward<Ts>(args)...);
        if (!ref) return false;
        std::construct_at(&dest.storage.lazy_ref);
        dest.storage.lazy_ref.alloc = alloc;
        dest.storage.lazy_ref.is_threaded = is_threaded;
        dest.storage.lazy_ref.object = ref;
        dest.vtable = this;
        return true;
      };

      MJZ_IF_CONSTEVAL { return alloc_path(); }
      if constexpr (!small_enough) {
        return alloc_path();
      }
      auto ptr = reinterpret_cast<void*>(&dest.storage.raw[0]);
      T* ref{reinterpret_cast<T*>(ptr)};
      bool good{};
      if constexpr (requires() {
                      {
                        std::construct_at(ref, std::forward<Ts>(args)...)
                      } noexcept;
                    }) {
        good = !!std::construct_at(ref, std::forward<Ts>(args)...);
      } else {
        MJZ_NOEXCEPT {
          good = !!std::construct_at(ref, std::forward<Ts>(args)...);
        };
      }
      if (!good) return false;
      dest.vtable = this;
      return true;
    }
    MJZ_CX_FN static T& ref(basic_lazy_str_t& obj) noexcept {
      if constexpr (small_enough) {
        MJZ_IFN_CONSTEVAL {
          return *std::launder(reinterpret_cast<T*>(&obj.storage.raw[0]));
        }
      }
      return static_cast<hs_t*>(obj.storage.lazy_ref.object)->object;
    }
    MJZ_CX_FN static const T& ref(const basic_lazy_str_t& obj) noexcept {
      if constexpr (small_enough) {
        MJZ_IFN_CONSTEVAL {
          return *std::launder(reinterpret_cast<const T*>(&obj.storage.raw[0]));
        }
      }
      return static_cast<const hs_t*>(obj.storage.lazy_ref.object)->object;
    }

    MJZ_CX_FN static void destroy_fn(basic_lazy_str_t& obj) noexcept {
      MJZ_RELEASE { obj.vtable = nullptr; };
      if constexpr (small_enough) {
        MJZ_IFN_CONSTEVAL {
          std::destroy_at(&ref(obj));
          obj.storage.dummy_ = 0;
          return;
        }
      }
      auto& rc_ = obj.storage.lazy_ref.object->reference_count;
      if (obj.storage.lazy_ref.is_threaded) {
        if (1==threads_ns::atomic_ref_t<uintlen_t>(rc_).fetch_sub(
                     1, std::memory_order_acq_rel))
          return;
      } else {
        if (--rc_) return;
      }
      obj.storage.lazy_ref.alloc.deallocate_node(
          static_cast<hs_t*>(obj.storage.lazy_ref.object),
          obj.storage.lazy_ref.is_threaded);
      std::destroy_at(&obj.storage.lazy_ref);
      obj.storage.dummy_ = 0;
      return;
    }
    MJZ_CX_FN vtable_t() noexcept {
      this->copy_move_destroy_fnp = &copy_move_destroy_fn_;
      this->iterate_fnp = &iterate_fn_;
    }
  };

  template <class T>
  constexpr static const vtable_t<std::remove_cvref_t<T>> vtable_var_{};
  const basic_lazy_vtable_t<version_v>* vtable{};
  lazy_storage_u<version_v> storage{};

 public:
  template <class T, typename... Ts>
  MJZ_CX_FN success_t init(const allocs_ns::alloc_base_ref_t<version_v>& alloc,
                           bool is_threaded, Ts&&... args) noexcept {
    return vtable_var_<T>.init(*this, alloc, is_threaded,
                               std::forward<Ts>(args)...);
  }
};

};  // namespace lazy_abi_

template <class T, version_t version_v>
concept lazy_generatorable_c =
    requires(uintlen_t offset, uintlen_t length, const T& obj) {
      {
        obj(offset, length,
            [_ = std::array<uintlen_t, 2>{}](
                basic_string_view_t<version_v>)   noexcept
                -> success_t { return true; })
      } noexcept -> std::same_as<success_t>;
    };
template <version_t version_v>
struct lazy_generator_str_t : void_struct_t {
 private:
  struct m_t : lazy_abi_::basic_lazy_str_t<version_v> {
    uintlen_t length{};
    uintlen_t offset{};
  } m{};

 public:
  MJZ_CX_FN m_t& unsafe_handle(unsafe_ns::i_know_what_im_doing_t) noexcept {
    return m;
  }

  MJZ_CX_FN const m_t& unsafe_handle(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return m;
  }

 public:
  MJZ_CX_FN lazy_generator_str_t() noexcept = default;
  template <lazy_generatorable_c<version_v> lambda_t>
  MJZ_CX_FN lazy_generator_str_t(
      lambda_t&& generator, uintlen_t length, uintlen_t offset=0,
      const allocs_ns::alloc_base_ref_t<version_v>& alloc =
          allocs_ns::alloc_base_ref_t<version_v>{},
      bool is_threaded=true) noexcept {
    if (!m.template init<std::remove_cvref_t<lambda_t>>(alloc, is_threaded,
                                                        generator))
      return;
    m.length = length;
    m.offset = offset;
  }
  MJZ_CX_FN success_t iterate(
      lazy_abi_::lazy_reader_fn_c<version_v> auto&& lazy_reader) const noexcept

  {
    return m.iterate(m.offset, m.length, lazy_reader);
  }
};

};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_basic_lazy_str_LIB_HPP_FILE_
