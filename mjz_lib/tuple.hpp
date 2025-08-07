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

#ifndef MJZ_TUPLE_LIB_HPP_FILE_
#define MJZ_TUPLE_LIB_HPP_FILE_
#include <tuple>

#include "memories.hpp"
namespace mjz {

template <class, class>
struct tuple_strorage_t {};
MJZ_DISABLE_ALL_WANINGS_START_;
template <class T, size_t my_I>
struct tuple_elem_storage_t {
  MJZ_DISABLE_ALL_WANINGS_END_;
  T data;
  template <size_t I>
    requires(my_I == I)
  MJZ_CX_FN decltype(auto) get() && noexcept {
    if constexpr (std::is_reference_v<T>) {
      return (data);
    } else {
      return static_cast<T &&>(data);
    }
  }
  template <size_t I>
    requires(my_I == I)
  MJZ_CX_FN decltype(auto) get() const && noexcept {
    if constexpr (std::is_reference_v<T>) {
      return (data);
    } else {
      return static_cast<const T &&>(data);
    }
  }
  template <size_t I>
    requires(my_I == I)
  MJZ_CX_FN decltype(auto) get() & noexcept {
    return (data);
  }
  template <size_t I>
    requires(my_I == I)
  MJZ_CX_FN decltype(auto) get() const & noexcept {
    return (data);
  }

  struct type_ {
    using type = T;
  };
  template <size_t I>
    requires(my_I == I)
  MJZ_CX_FN static type_ type_get() noexcept {
    return {};
  };
  using const_tr = std::conditional_t<std::is_reference_v<T>, T, const T &>;

  MJZ_CX_FN bool operator==(const tuple_elem_storage_t &rhs) const
      noexcept(noexcept(std::declval<const_tr>() == std::declval<const_tr>()))
    requires(requires() {
      std::declval<const_tr>() == std::declval<const_tr>();
    })
  {
    return get<my_I>() == rhs.template get<my_I>();
  };

  MJZ_CX_FN auto operator<=>(const tuple_elem_storage_t &rhs) const
      noexcept(noexcept(std::declval<const_tr>() <=> std::declval<const_tr>()))
    requires(requires() {
      std::declval<const_tr>() <=> std::declval<const_tr>();
    })
  {
    return get<my_I>() <=> rhs.template get<my_I>();
  }
};
MJZ_DISABLE_ALL_WANINGS_START_;
template <typename... Ts, size_t... Is>
  requires(!!sizeof...(Ts))
struct MJZ_MSVC_ONLY_CODE_(__declspec(empty_bases))
    tuple_strorage_t<void(Ts...), std::index_sequence<Is...>>
    : tuple_elem_storage_t<Ts, Is>... {
  MJZ_DISABLE_ALL_WANINGS_END_;
  using tuple_elem_storage_t<Ts, Is>::get...;
  using tuple_elem_storage_t<Ts, Is>::type_get...;
  MJZ_DEFAULTED_CLASS(tuple_strorage_t);
  template <typename... Us>
    requires(std::convertible_to<Us, Ts> && ...)
  MJZ_CX_AL_FN tuple_strorage_t(Us &&...args) noexcept(
      (noexcept(tuple_elem_storage_t<Ts, Is>{std::forward<Us>(args)}) && ...))
      : tuple_elem_storage_t<Ts, Is>{std::forward<Us>(args)}... {}

  MJZ_CX_FN bool operator==(const tuple_strorage_t &rhs) const noexcept(
      (noexcept(std::declval<const tuple_elem_storage_t<Ts, Is> &>() ==
                std::declval<const tuple_elem_storage_t<Ts, Is> &>()) &&
       ...))
    requires(requires() {
      std::declval<const tuple_elem_storage_t<Ts, Is> &>() ==
          std::declval<const tuple_elem_storage_t<Ts, Is> &>();
    } && ...)
  {
    return ((*static_cast<tuple_elem_storage_t<Ts, Is> const *>(this) ==
             *static_cast<tuple_elem_storage_t<Ts, Is> const *>(&rhs)) &&
            ...);
  };
  MJZ_CX_FN auto operator<=>(const tuple_strorage_t &rhs) const noexcept(
      (noexcept(std::declval<const tuple_elem_storage_t<Ts, Is> &>() <=>
                std::declval<const tuple_elem_storage_t<Ts, Is> &>()) &&
       ...))
    requires(requires() {
      std::declval<const tuple_elem_storage_t<Ts, Is> &>() <=>
          std::declval<const tuple_elem_storage_t<Ts, Is> &>();
    } && ...)
  {
    std::common_comparison_category_t<
        decltype(std::declval<const tuple_elem_storage_t<Ts, Is> &>() <=>
                 std::declval<const tuple_elem_storage_t<Ts, Is> &>())...>
        res{};
    ((res = (*static_cast<tuple_elem_storage_t<Ts, Is> const *>(this) <=>
             *static_cast<tuple_elem_storage_t<Ts, Is> const *>(&rhs)),
      res != 0) ||
     ...);

    return res;
  };
};

MJZ_DISABLE_ALL_WANINGS_START_;
template <typename... Ts>
struct tuple_t
    : tuple_strorage_t<void(Ts...), std::make_index_sequence<sizeof...(Ts)>> {
  MJZ_DISABLE_ALL_WANINGS_END_;
  using base_t_ =
      tuple_strorage_t<void(Ts...), std::make_index_sequence<sizeof...(Ts)>>;
  MJZ_DEFAULTED_CLASS(tuple_t);
  template <typename... Us>
    requires(std::convertible_to<Us, Ts> && ...)
  MJZ_CX_AL_FN tuple_t(Us &&...args) noexcept(noexcept(base_t_{
      std::forward<Us>(args)...}))
      : base_t_{std::forward<Us>(args)...} {}
  using base_t_::get;
  using base_t_::type_get;
  MJZ_CX_FN bool operator==(const tuple_t &) const = default;

  MJZ_CX_FN auto operator<=>(const tuple_t &rhs) const
      noexcept(noexcept(std::declval<const base_t_ &>() <=>
                        std::declval<const base_t_ &>()))
    requires(requires() {
      std::declval<const base_t_ &>() <=> std::declval<const base_t_ &>();
    })
  {
    return (const base_t_ &)*this <=> (const base_t_ &)rhs;
  };
  using is_mjz_tuple_c_uuid_098765987654345678 = void;
};
template <typename... Ts>
tuple_t(Ts &&...) noexcept -> tuple_t<std::remove_cvref_t<Ts>...>;

MJZ_DISABLE_ALL_WANINGS_START_;
template <class first_t, class second_t>
struct pair_t {
  MJZ_DISABLE_ALL_WANINGS_END_;
  MJZ_no_unique_address first_t first{};
  MJZ_no_unique_address second_t second{};

  using const_tr_0_ = std::conditional_t<std::is_reference_v<first_t>, first_t,
                                         const first_t &>;
  using const_tr_1_ = std::conditional_t<std::is_reference_v<second_t>,
                                         second_t, const second_t &>;
  MJZ_CX_FN bool operator==(const pair_t &rhs) const noexcept(
      noexcept(std::declval<const_tr_0_>() == std::declval<const_tr_0_>() &&
               std::declval<const_tr_1_>() == std::declval<const_tr_1_>()))
    requires(requires() {
      std::declval<const_tr_0_>() == std::declval<const_tr_0_>() &&
          std::declval<const_tr_1_>() == std::declval<const_tr_1_>();
    })
  {
    return first == rhs.first && second == rhs.second;
  }
  MJZ_CX_FN auto operator<=>(const pair_t &rhs) const noexcept(
      noexcept((std::declval<const_tr_0_>() <=> std::declval<const_tr_0_>(),
                std::declval<const_tr_1_>() <=> std::declval<const_tr_1_>())))
    requires(requires() {
      std::declval<const_tr_0_>() <=> std::declval<const_tr_0_>();
      std::declval<const_tr_1_>() <=> std::declval<const_tr_1_>();
    })
  {
    std::common_comparison_category_t<
        decltype(std::declval<const_tr_0_>() <=> std::declval<const_tr_0_>()),
        decltype(std::declval<const_tr_1_>() <=> std::declval<const_tr_1_>())>
        res{};
    ((res = (first <=> rhs.first), res != 0) ||
     (res = (second <=> rhs.second), res != 0));
    return res;
  }

  template <size_t I>
    requires(I < 2)
  MJZ_CX_FN decltype(auto) get() && noexcept {
    if constexpr (!!I) {
      if constexpr (std::is_reference_v<second_t>) {
        return (second);
      } else {
        return static_cast<second_t &&>(second);
      }
    } else {
      if constexpr (std::is_reference_v<first_t>) {
        return (first);
      } else {
        return static_cast<first_t &&>(first);
      }
    }
  }
  template <size_t I>
    requires(I < 2)
  MJZ_CX_FN decltype(auto) get() const && noexcept {
    if constexpr (!!I) {
      if constexpr (std::is_reference_v<second_t>) {
        return (second);
      } else {
        return static_cast<const second_t &&>(second);
      }
    } else {
      if constexpr (std::is_reference_v<first_t>) {
        return (first);
      } else {
        return static_cast<const first_t &&>(first);
      }
    }
  }
  template <size_t I>
    requires(I < 2)
  MJZ_CX_FN decltype(auto) get() & noexcept {
    if constexpr (!!I) {
      return (second);
    } else {
      return (first);
    }
  }
  template <size_t I>
    requires(I < 2)
  MJZ_CX_FN decltype(auto) get() const & noexcept {
    if constexpr (!!I) {
      return (second);
    } else {
      return (first);
    }
  }
  using is_mjz_tuple_c_uuid_098765987654345678 = void;
};

template <class T>
concept is_mjz_tuple_c_ = requires() {
  typename std::remove_cvref_t<T>::is_mjz_tuple_c_uuid_098765987654345678;
};

}  // namespace mjz
namespace std {
template <class... Types>
struct tuple_size<::mjz::pair_t<Types...>>
    : std::integral_constant<std::size_t, 2> {};
template <std::size_t I, class first_t, class second_t>
  requires(I < 2)
struct tuple_element<I, ::mjz::pair_t<first_t, second_t>> {
  using type = std::conditional_t<!!I, second_t, first_t>;
};

template <class... Types>
struct tuple_size<::mjz::tuple_t<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};
template <std::size_t I, typename... Ts>
struct tuple_element<I, ::mjz::tuple_t<Ts...>> {
  using type = decltype(::mjz::tuple_t<Ts...>::template type_get<I>())::type;
};
template <size_t index_, ::mjz::is_mjz_tuple_c_ T>
MJZ_CX_ND_FN decltype(auto) get(T&& p) noexcept {
  return std::forward<T>(p).template get<index_>();
}
};  // namespace std

#endif  // MJZ_TUPLE_LIB_HPP_FILE_