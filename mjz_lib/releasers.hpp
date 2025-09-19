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
#include "tuple.hpp"
#ifndef MJZ_RELEASER_LIB_HPP_FILE_
#define MJZ_RELEASER_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz {
  template <class unique_accessor_id_t>
  class mjz_private_accessed_t {};
  struct totally_empty_type_t {};
  using nullptr_t = std::nullptr_t;
  MJZ_FCONSTANT(totally_empty_type_t) totally_empty_type{};
  static_assert(std::is_empty_v<totally_empty_type_t>);
  using void_struct_t = totally_empty_type_t;
  template <>
  class mjz_private_accessed_t<void_struct_t(void_struct_t *)> {
   public:
    template <class T>
    MJZ_CX_AL_FN static T mptr_static_cast(auto p) noexcept {
      if constexpr (requires() { static_cast<T>(p); }) {
        return static_cast<T>(p);
      } else if constexpr (std::same_as<std::remove_cvref_t<decltype(p)>,
                                        nullptr_t>) {
        return nullptr;
      } else if constexpr (std::same_as<std::remove_cvref_t<decltype(p)>,
                                        void_struct_t *>) {
        return std::remove_pointer_t<
            std::remove_cvref_t<T>>::mptr_static_cast_pv_fn_(p);
      } else {
        if (!p) return nullptr;
        return p->template mptr_static_cast_pv_fn_<T>();
      }
    }

    template <class T>
    MJZ_CX_AL_FN static decltype(auto) up_cast(T *ptr) noexcept {
      return mptr_static_cast<void_struct_t *>(ptr);
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) up_cast(const T *ptr) noexcept {
      return mptr_static_cast<const void_struct_t *>(ptr);
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) down_cast(void_struct_t *ptr) noexcept {
      return mptr_static_cast<T *>(ptr);
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) down_cast(
        const void_struct_t *ptr) noexcept {
      return mptr_static_cast<const T *>(ptr);
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) up_cast(T &ptr) noexcept {
      return *up_cast(std::addressof(ptr));
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) up_cast(const T &ptr) noexcept {
      return *up_cast(std::addressof(ptr));
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) down_cast(void_struct_t &ptr) noexcept {
      return *down_cast<T>(std::addressof(ptr));
    }
    template <class T>
    MJZ_CX_AL_FN static decltype(auto) down_cast(
        const void_struct_t &ptr) noexcept {
      return *down_cast<T>(std::addressof(ptr));
    }
  };
  using void_struct_cast_t =
      mjz_private_accessed_t<void_struct_t(void_struct_t *)>;
  enum class may_bool_t : char { no = false, yes = true, idk = 2, err = 3 };
  MJZ_FCONSTANT(bool)
  SYSTEM_is_little_endian_{std::endian::little == std::endian::native};
#if MJZ_uintlen_t_as_64_bit
  using uintlen_t = uint64_t;
  using ushortlen_t = uint32_t;
#else
  using uintlen_t = std::conditional_t<(sizeof(uint32_t) <= sizeof(uintptr_t)),
                                       uintptr_t, uint32_t>;
  using ushortlen_t =
      std::conditional_t<(sizeof(uint32_t) <= sizeof(uintlen_t)), uint32_t,
                         uint16_t>;
#endif
  using intlen_t = std::make_signed_t<uintlen_t>;
  using shortlen_t = std::make_signed_t<ushortlen_t>;
  template <typename T>
  using single_object_pointer_t = T *;
  template <typename T>
  using alias_t = T;

  template <size_t I, typename T, typename... Ts>
  struct type_at_index_helper_t {
    using type_at = typename type_at_index_helper_t<I - 1, Ts...>::type_at;
  };
  template <typename T, typename... Ts>
  struct type_at_index_helper_t<0, T, Ts...> {
    using type_at = T;
  };
  template <size_t I, typename... Ts>
  using type_at_index_t = typename type_at_index_helper_t<I, Ts...>::type_at;
  template <class T, class U>
  struct compare_tamplate_codes_helper_t : std::false_type {};
  template <template <class...> class TC, class... Ts, class... Us>
  struct compare_tamplate_codes_helper_t<TC<Ts...>, TC<Us...>>
      : std::true_type {};
  template <class T, template <class...> class TC>
  struct compare_tamplates_helper_t : std::false_type {};
  template <template <class...> class TC, class... Ts>
  struct compare_tamplates_helper_t<TC<Ts...>, TC> : std::true_type {};
  template <class T, class U>
  concept compare_tamplate_codes_c = compare_tamplate_codes_helper_t<T, U>()();
  template <class T, template <class...> class TC>
  concept compare_tamplates_c = compare_tamplates_helper_t<T, TC>()();
  template <size_t size_of_myt>
  using uint_sizeof_t =
      type_at_index_t<size_of_myt, uint8_t, uint8_t, uint16_t, uint32_t,
                      uint32_t, uint64_t, uint64_t, uint64_t, uint64_t>;
  template <size_t S>
  using uint_size_of_t = uint_sizeof_t<S>;
  /*this actually performas better in general cases than
   * forced_branchless_teranary  */
  template <class T>
  MJZ_CX_AL_ND_FN T(branchless_teranary)(std::same_as<bool> auto if_expression,
                                         const T &then_val,
                                         const T &else_val) noexcept {
    if constexpr (std::is_scalar_v<std::remove_cvref_t<T>>) {
      T then_val_ = *(&then_val);
      T else_val_ = *(&else_val);
      MJZ_IFN_CONSTEVAL_ {
        using cast_t = uint_size_of_t<sizeof(T)>;
        const cast_t mask = cast_t((~cast_t(if_expression)) + 1);
        return std::bit_cast<T>(
            cast_t((std::bit_cast<cast_t>(then_val) & mask) |
                   (std::bit_cast<cast_t>(else_val) & ~mask)));
      }
      else {
        return if_expression ? then_val_ : else_val_;
      }
    } else {
      const T &then_val_ = *(&then_val);
      const T &else_val_ = *(&else_val);
      return if_expression ? then_val_ : else_val_;
    }
  }

  template <class T>
  MJZ_CX_AL_ND_FN T(forced_branchless_teranary)(
      std::same_as<bool> auto if_expression, const T &then_val,
      const T &else_val) noexcept {
    if constexpr (std::is_scalar_v<T>) {
      MJZ_IFN_CONSTEVAL_ {
        using cast_t = uint_size_of_t<sizeof(T)>;
        const cast_t mask = (~cast_t(if_expression)) + 1;
        return std::bit_cast<T>((std::bit_cast<cast_t>(then_val) & mask) |
                                (std::bit_cast<cast_t>(else_val) & ~mask));
      }
    }
    return alias_t<T[2]>{else_val, then_val}[if_expression];
  }

  template <class>
  class my_totatlly_empty_template1_class_t {};
  MJZ_CX_AL_ND_FN uint8_t log2_of_val_create(std::integral auto val) noexcept {
    return uint8_t(std::bit_width(val) - (val != 0));
  }
  MJZ_CX_AL_ND_FN uint64_t log2_of_val_to_val(uint8_t log2_val) noexcept {
    return static_cast<uint64_t>(1ull << log2_val);
  }
  MJZ_CX_AL_ND_FN uint8_t log2_ceil_of_val_create(
      std::integral auto val) noexcept
    requires(std::is_unsigned_v<std::remove_cvref_t<decltype(val)>>)
  {
    return uint8_t(std::bit_width(val) - std::has_single_bit(val));
  }

  template <uint64_t max_val>
  using uint_with_max_of_t =
      uint_size_of_t<size_t(log2_ceil_of_val_create(max_val + 1) / 8) +
                     size_t(!!(log2_ceil_of_val_create(max_val + 1) % 8))>;
  template <uint64_t min_max_val>
  using int_with_max_of_t = std::make_signed_t<uint_with_max_of_t<min_max_val>>;

  template <typename T>
    requires(std::is_integral_v<std::remove_cvref_t<T>>)
  constexpr inline auto signedof(T val) noexcept {
    return std::bit_cast<std::make_signed_t<T>>(val);
  }
  template <typename T>
    requires(std::is_integral_v<std::remove_cvref_t<T>>)
  constexpr inline auto unsignedof(T val) noexcept {
    return std::bit_cast<std::make_unsigned_t<T>>(val);
  }

  template <typename T>
  constexpr inline std::add_const_t<T &&> constof(T && val) noexcept {
    return std::forward<std::add_const_t<T>>(val);
  }

  template <typename T>
    requires(!std::is_integral_v<std::remove_cvref_t<T>>)
  MJZ_CX_FN decltype(auto) unsignedof(T && obj_for_unsignedcast) noexcept {
    return std::forward<T>(obj_for_unsignedcast);
  };
  template <typename T>
    requires(!std::is_integral_v<std::remove_cvref_t<T>>)
  MJZ_CX_FN decltype(auto) signedof(T && obj_for_signedcast) noexcept {
    return std::forward<T>(obj_for_signedcast);
  };

  template <class releaser_LAMBDA_t, typename... reasorces_t>
    requires requires(releaser_LAMBDA_t l, reasorces_t &&...args) {
      releaser_LAMBDA_t(std::move(l));
      l(std::forward<reasorces_t>(args)...);
    }
  class releaser_t {
   public:
    using tuple_t_0_ = tuple_t<reasorces_t...>;
    releaser_t(releaser_t &&) = delete;
    releaser_t(const releaser_t &) = delete;
    releaser_t &operator=(releaser_t &&) = delete;
    releaser_t(releaser_t &) = delete;
    releaser_t &operator=(releaser_t &) = delete;
    releaser_t &operator=(const releaser_t &) = delete;
    releaser_t() = delete;
    MJZ_CX_AL_FN releaser_t(releaser_LAMBDA_t &&releaser_lambda,
                            reasorces_t &&...args) noexcept(requires() {
      { releaser_LAMBDA_t(std::move(releaser_lambda)) } noexcept;
      { tuple_t_0_(std::forward<reasorces_t>(args)...) } noexcept;
    })
        : m_releaser_lambda_(std::move(releaser_lambda)),
          data(std::forward<reasorces_t>(args)...) {}
    MJZ_CX_AL_FN ~releaser_t() noexcept(requires(reasorces_t &&...args) {
      { m_releaser_lambda_(std::forward<reasorces_t>(args)...) } noexcept;
    }) {
      call();
    }

    template <class>
    friend class mjz_private_accessed_t;

   private:
    releaser_LAMBDA_t m_releaser_lambda_;
    template <size_t INDEX>
    using type_at = std::tuple_element_t<INDEX, tuple_t_0_>;

    template <typename... Ts>
      requires(sizeof...(Ts) < sizeof...(reasorces_t))
    MJZ_CX_AL_FN void call(Ts &&...args) {
      call(std::forward<Ts>(args)...,
           std::forward<type_at<sizeof...(Ts)>>(std::get<sizeof...(Ts)>(data)));
    }
    MJZ_CX_AL_FN void call(reasorces_t &&...args) {
      m_releaser_lambda_(std::forward<reasorces_t>(args)...);
    }

   public:
    tuple_t_0_ data;
    MJZ_NO_DYNAMIC_ALLOCATOR(releaser_t);
  };
  template <class releaser_LAMBDA_t>
    requires requires(releaser_LAMBDA_t l) {
      releaser_LAMBDA_t(std::move(l));
      l();
    }
  class releaser_t<releaser_LAMBDA_t> {
   public:
    MJZ_NO_MV_NO_CPY(releaser_t);
    releaser_t() = delete;
    MJZ_CX_AL_FN
    releaser_t(releaser_LAMBDA_t &&releaser_lambda) noexcept(requires() {
      { releaser_LAMBDA_t(std::move(releaser_lambda)) } noexcept;
    }) : m_releaser_lambda_(std::move(releaser_lambda)) {}
    MJZ_CX_AL_FN ~releaser_t() { m_releaser_lambda_(); }

    template <class>
    friend class mjz_private_accessed_t;

   private:
    releaser_LAMBDA_t m_releaser_lambda_;
    MJZ_NO_DYNAMIC_ALLOCATOR(releaser_t);
  };

  template <typename T>
  using nullable_t = T;
  template <typename T>
  using no_null_t = T;
  MJZ_MSVC_ONLY_PRAGMA_(optimize("", off));
  MJZ_CX_NL_FN void just_do(auto &&...) noexcept {}
  MJZ_MSVC_ONLY_PRAGMA_(optimize("", on));
  template <typename T>
  MJZ_CX_NL_FN T just_ret(T && arg) noexcept {
    return std::forward<T>(arg);
  }
  template <typename T>
  using non_null_rvalue_ptr_t = T *const;

  // no perfect forward
  template <typename T>
  using Rvalueref_t = T &&;
  template <class T>
    requires(!std::is_const_v<T> && !std::is_abstract_v<T> &&
             !std::is_reference_v<T> && std::is_class_v<T>)
  class conditional_releaser_t : private T {
    template <class>
    friend class mjz_private_accessed_t;
    conditional_releaser_t(const conditional_releaser_t &) = delete;
    conditional_releaser_t(conditional_releaser_t &) = delete;
    conditional_releaser_t &operator=(conditional_releaser_t &) = delete;
    conditional_releaser_t &operator=(const conditional_releaser_t &) = delete;
    bool has_lambda;

    MJZ_CX_FN T &operator*() noexcept { return *this; }

   public:
    MJZ_MCONSTANT(bool)
    noexcept_v = requires(T lamda) {
      { std::move(lamda)() } noexcept;
    };
    MJZ_CX_FN conditional_releaser_t(T &&lambda_,
                                     bool has_lambda_ = true) noexcept
        : T(std::move(lambda_)), has_lambda(has_lambda_) {}
    MJZ_CX_AL_FN void run_now() noexcept(noexcept_v) {
      if (has_lambda) {
        has_lambda = false;
        std::move (**this)();
      }
    }
    MJZ_CX_AL_FN void dont_run() noexcept { has_lambda = false; }
    MJZ_CX_AL_ND_FN explicit operator bool() const noexcept {
      return has_lambda;
    }
    MJZ_CX_AL_ND_FN bool operator!() const noexcept { return !has_lambda; }
    MJZ_CX_AL_FN ~conditional_releaser_t() noexcept(noexcept_v) { run_now(); }

    MJZ_CX_AL_FN conditional_releaser_t(
        conditional_releaser_t &&other) noexcept(requires(T other_l) {
      { T(std::move(other_l)) } noexcept;
    })
        : T(std::move(*other)),
          has_lambda(std::exchange(other.has_lambda, false)) {}
    MJZ_CX_AL_FN conditional_releaser_t &operator=(
        conditional_releaser_t &&other) noexcept(requires(T other_l, T lamda) {
      { lamda = std::move(other_l) } noexcept;
    }) {
      has_lambda = std::exchange(other.has_lambda, false);
      **this = std::move(*other);
      return *this;
    }

    MJZ_NO_DYNAMIC_ALLOCATOR(conditional_releaser_t);
  };

};  // namespace mjz
#endif  // MJZ_RELEASER_LIB_HPP_FILE_