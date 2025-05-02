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


#include <optional>

#include "asserts.hpp"
#ifndef MJZ_OPTIONALS_LIB_HPP_FILE_
#define MJZ_OPTIONALS_LIB_HPP_FILE_

namespace mjz {
template <typename T>
struct optional_ref_t {
   
  single_object_pointer_t<T> ptr{};
  MJZ_CX_ND_FN T &operator*() const noexcept { 
    return *ptr;
  }
  MJZ_CX_ND_FN T &value() const noexcept { 
    return *ptr;
  }
  MJZ_CX_ND_FN T &value_or(T &&) const noexcept = delete;
  MJZ_CX_ND_FN T &value_or(T &other) const noexcept {
    if (*this) {
      return *ptr;
    }
    return other;
  }
  MJZ_CX_ND_FN
  single_object_pointer_t<T> operator->() const noexcept { 
    return ptr;
  }
  MJZ_CX_ND_FN explicit operator bool() const noexcept { return !!ptr; }
  MJZ_CX_ND_FN bool operator!() const noexcept { return !ptr; }
  MJZ_CX_ND_FN bool has_value() const noexcept { return !!ptr; }

  MJZ_CX_ND_FN
  single_object_pointer_t<T> get_ptr() const noexcept { return ptr; }
  MJZ_CX_ND_FN
  single_object_pointer_t<T> get() const noexcept { return ptr; }

  template <typename U>
  MJZ_CX_ND_FN decltype(auto) operator->*(U &&arg) const noexcept { 
    return ptr->*std::forward<U>(arg);
  }
  MJZ_CX_FN optional_ref_t(single_object_pointer_t<T> ptr_ = nullptr) noexcept
      : ptr(ptr_) {}

  MJZ_CX_FN optional_ref_t(const optional_ref_t &opt) noexcept : ptr(opt.ptr) {}

  MJZ_CX_FN optional_ref_t(optional_ref_t &&opt) noexcept
      : ptr(std::exchange(opt.ptr, {})) {}
  MJZ_CX_FN optional_ref_t(T &ref) noexcept : ptr(&ref) {}
  MJZ_CX_FN optional_ref_t(std::nullopt_t) noexcept {}
  MJZ_CX_FN void reset() noexcept { ptr = nullptr; }

  MJZ_CX_FN optional_ref_t &operator=(nullopt_t) noexcept {
    reset();
    return *this;
  }
  MJZ_CX_FN bool operator==(const optional_ref_t &opt) const noexcept {
    return ptr == opt.ptr;
  }
  MJZ_CX_FN optional_ref_t &operator=(const optional_ref_t &opt) noexcept {
    ptr = opt.ptr;
    return *this;
  }
  MJZ_CX_FN optional_ref_t &operator=(optional_ref_t &&opt) noexcept {
    ptr = std::exchange(opt.ptr, {});
    return *this;
  }
  MJZ_CX_FN optional_ref_t &operator=(

      single_object_pointer_t<T> ptr_) noexcept {
    ptr = ptr_;
    return *this;
  }
  MJZ_CX_FN optional_ref_t &operator=(T &other) noexcept {
    if (!ptr) {
      ptr = &other;
    } else {
      *ptr = other;
    }
    return *this;
  }
  MJZ_DEPRECATED_R("may cause confution")
  MJZ_CX_FN optional_ref_t &operator=(T &&other) noexcept
    requires(false)
  {
    if (ptr) {
      *ptr = std::move(other);
    }
    return *this;
  }
  MJZ_CX_FN
  auto perform_else(callable_anyret_c<void(T &) noexcept> auto &&LT,
                    callable_anyret_c<void() noexcept> auto &&LF) noexcept
      -> decltype(bool{} ? LT(just_some_invalid_obj<T &>()) : LF()) {
    return !*this ? LF() : LT(*ptr);
  }
  MJZ_CX_FN
  auto perform_else(callable_anyret_c<void(T &) noexcept> auto &&LT,
                    callable_anyret_c<void() noexcept> auto &&LF) const noexcept
      -> decltype(bool{} ? LT(just_some_invalid_obj<T &>()) : LF()) {
    return !*this ? LF() : LT(*ptr);
  }
  MJZ_CX_FN
  void perform(callable_c<void(T &) noexcept> auto &&LT) noexcept {
    if (!*this) return;
    LT(*ptr);
  }
  MJZ_CX_FN
  void perform(callable_c<void(const T &) noexcept> auto &&LT) const noexcept {
    if (!*this) return;
    LT(*ptr);
  }

  MJZ_CX_FN
  void else_do(callable_c<void() noexcept> auto &&LF) const noexcept {
    if (!!*this) return;
    LF();
  }
};
template <is_totaly_trivial T>
struct char_storage_as_temp_t : public optional_ref_t<T> {
  MJZ_NO_MV_NO_CPY(char_storage_as_temp_t);

  template <class>
  friend class mjz_private_accessed_t;

 private:
  using optional_ref_t<T>::ptr;
  std::remove_const_t<T> buffer_obj{};
  std::conditional_t<std::is_const_v<T>, const char, char> *ptr_to_real_obj{};
  MJZ_CX_FN char_storage_as_temp_t(
      std::conditional_t<std::is_const_v<T>, const char, char>
          *ptr_to_obj) noexcept {
    ptr = nullptr;
    if (!ptr_to_obj) {
      return;
    }
    ptr_to_real_obj = ptr_to_obj;
    MJZ_IFN_CONSTEVAL {
      ptr = reinterpret_cast<T *>(ptr_to_obj);
      return;
    }
    struct A {
      char array_buf[sizeof(T)]{};
    } a;
    mjz::memcpy(a.array_buf, ptr_to_obj, sizeof(T));
    buffer_obj = std::bit_cast<T>(a);
    ptr = &buffer_obj;
  }

 public:
  struct size_val_t_ {
    MJZ_CE_FN size_val_t_(auto var) {
      asserts([](std::integral auto size) {
        return sizeof(T) <= uintlen_t(size);
      }(var));
    }
  };
  struct align_val_t_ {
    MJZ_CE_FN align_val_t_(auto var) {
      asserts([]<typename Var_t>(Var_t alignment)
                requires(std::same_as<Var_t, std::align_val_t> ||
                         std::integral<Var_t>)
              { return alignof(T) <= uintlen_t(alignment); }(var));
    }
  };

  MJZ_CX_FN char_storage_as_temp_t() noexcept
      : char_storage_as_temp_t(nullptr) {}
  MJZ_CX_ND_FN char_storage_as_temp_t(
      std::conditional_t<std::is_const_v<T>, const char, char> *ptr_to_obj,
      size_val_t_, align_val_t_) noexcept
      : char_storage_as_temp_t(ptr_to_obj) {}
  MJZ_CX_ND_FN char_storage_as_temp_t(
      std::conditional_t<std::is_const_v<T>, const char, char> *ptr_to_obj,
      uintlen_t size, std::align_val_t alignment, totally_empty_type_t) noexcept
      : char_storage_as_temp_t(
            (static_cast<size_t>(alignment) < alignof(T) || size < sizeof(T))
                ? nullptr
                : ptr_to_obj) {}
  MJZ_CX_FN ~char_storage_as_temp_t() noexcept {
    if constexpr (!std::is_const_v<T>) {
      if (ptr != &buffer_obj) {
        return;
      }
      struct A {
        char array_buf[sizeof(T)]{};
      } a{std::bit_cast<A>(buffer_obj)};
      mjz::memcpy(this->ptr_to_real_obj, a.array_buf, sizeof(T));
    }
    ptr = nullptr;
  }
};
template <typename T>
struct runtime_only_union_mimic_t {
  alignas(alignof(T)) char buffer[sizeof(T)];

  MJZ_CX_ND_FN T *get_ptr() noexcept {
    MJZ_IF_CONSTEVAL { return nullptr; }
    return reinterpret_cast<T *>(buffer);
  }
  MJZ_CX_ND_FN const T *get_ptr() const noexcept {
    MJZ_IF_CONSTEVAL { return nullptr; }
    return reinterpret_cast<const T *>(buffer);
  }
  MJZ_NCX_FN auto &&get_ref() noexcept { return *std::launder(get_ptr()); }
  MJZ_NCX_FN auto &&get_ref() const noexcept {
    return *std::launder(get_ptr());
  };
};
template <typename T>
struct better_runtime_only_union_mimic_t : runtime_only_union_mimic_t<T> {
  using runtime_only_union_mimic_t<T>::get_ptr;
  using runtime_only_union_mimic_t<T>::get_ref;
  using runtime_only_union_mimic_t<T>::buffer;
  MJZ_CX_FN ~better_runtime_only_union_mimic_t() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    std::destroy_at(get_ptr());
  }
  template <typename... Ts>
  MJZ_CX_FN better_runtime_only_union_mimic_t(Ts &&...args) noexcept(
      noexcept(T(std::forward<Ts>(args)...))) {
    MJZ_IF_CONSTEVAL { return; }
    std::construct_at(get_ptr(), std::forward<Ts>(args)...);
  }
  MJZ_CX_FN better_runtime_only_union_mimic_t(
      better_runtime_only_union_mimic_t &&other) noexcept {
    MJZ_IF_CONSTEVAL { return; }
    std::construct_at(get_ptr(), std::move(other.get_ref()));
  };
  MJZ_CX_FN better_runtime_only_union_mimic_t(
      const better_runtime_only_union_mimic_t &other) noexcept {
    MJZ_IF_CONSTEVAL { return; }
    std::construct_at(get_ptr(), other.get_ref());
  };
  MJZ_CX_FN better_runtime_only_union_mimic_t &operator=(
      better_runtime_only_union_mimic_t &&other) noexcept {
    MJZ_IF_CONSTEVAL { return *this; }
    get_ref() = std::move(other.get_ref());
    return *this;
  };
  MJZ_CX_FN better_runtime_only_union_mimic_t &operator=(
      const better_runtime_only_union_mimic_t &other) noexcept {
    MJZ_IF_CONSTEVAL { return *this; }
    get_ref() = other.get_ref();
    return *this;
  }
};

/* suppress the deleted copy/move assignment or construction stuff */
MJZ_DISABLE_WANINGS_START_;
template <class lam_t>
concept multilambda_segment_ne_c = requires(lam_t &&arg) {
  { (std::remove_cvref_t<lam_t>(std::forward<lam_t>(arg))) } noexcept;
};

template <class... lambdas_t>
struct multilambda_t : public std::remove_cvref_t<lambdas_t>... {
  using std::remove_cvref_t<lambdas_t>::operator()...;
  MJZ_CX_FN multilambda_t(lambdas_t &&...args) noexcept(
      (multilambda_segment_ne_c<lambdas_t> && ...))
      : std::remove_cvref_t<lambdas_t>(std::forward<lambdas_t>(args))... {}
};
MJZ_DISABLE_WANINGS_END_;

template <class... lambdas_t>
multilambda_t(lambdas_t &&...) -> multilambda_t<lambdas_t &&...>;

class success_ret_arg_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  success_t *ptr;

 public:
  MJZ_CX_FN success_ret_arg_t(success_t *who = nullptr) noexcept : ptr(who) {}
  MJZ_CX_FN success_ret_arg_t(success_t &who) noexcept : ptr(&who) {}
  MJZ_CX_FN bool operator!() const noexcept { return !ptr; }
  MJZ_CX_FN operator bool() const noexcept { return !!ptr; }

  MJZ_CX_FN void set(success_t B) noexcept {
    if (ptr) {
      *ptr = B;
    }
  }
  MJZ_CX_FN success_t get(success_t Default = true) noexcept {
    if (ptr) {
      return *ptr;
    }
    return Default;
  }
};

template <is_totaly_trivial T>
struct trivial_optional_t {
  T val;
  bool has_val;
};

namespace no_type_ns {
template <typename>
struct typeless_function_t {
  MJZ_CX_FN void run(auto &&...) noexcept;
};
template <typename ret_t, typename... args_t>
struct typeless_function_t<ret_t(args_t...) noexcept> {
  void_struct_t *obj;
  ret_t (*fn)(void_struct_t &, args_t...) noexcept;
  MJZ_CX_FN ret_t run(args_t... args) noexcept {
    return fn(*obj, std::forward<args_t>(args)...);
  }
};
MJZ_DISABLE_WANINGS_START_;
template <typename, typename T>
struct function_holder_t {
  MJZ_CX_FN void operator+() noexcept;
};
template <typename T, typename ret_t, typename... args_t>
  requires callable_c<T, ret_t(args_t...) noexcept>
struct function_holder_t<ret_t(args_t...) noexcept, T> : void_struct_t {
  T lambda;
  MJZ_CX_FN function_holder_t(T &&obj) noexcept
      : lambda{std::forward<T>(obj)} {}
  MJZ_CX_FN static ret_t run(void_struct_t &me, args_t... args) noexcept {
    return static_cast<function_holder_t &>(me).lambda(
        std::forward<args_t>(args)...);
  }
  MJZ_CX_FN auto operator+() noexcept {
    return typeless_function_t<ret_t(args_t...) noexcept>{this, &run};
  }
};

template <typename fn_t>
MJZ_CX_FN auto make(callable_c<fn_t> auto &&lambda) noexcept {
  return function_holder_t<fn_t, decltype(lambda)>{
      std::forward<decltype(lambda)>(lambda)};
}

MJZ_DISABLE_WANINGS_END_;
};  // namespace no_type_ns
}  // namespace mjz
#endif  // MJZ_OPTIONALS_LIB_HPP_FILE_