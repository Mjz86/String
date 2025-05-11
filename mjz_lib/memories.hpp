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

#include <concepts>
#include <cstring>

#include "packings.hpp"
#ifndef MJZ_MEMORIES_LIB_HPP_FILE_
#define MJZ_MEMORIES_LIB_HPP_FILE_
namespace mjz {
MJZ_FCONSTANT(uintlen_t) default_new_align_z = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
MJZ_FCONSTANT(std::align_val_t)
default_new_align_v = std::align_val_t(default_new_align_z);
MJZ_CONSTANT(uintlen_t)
cache_fast_align_v = 8 * sizeof(uintlen_t);
/*
O(1) time complexity in runtime
O(b_len*a_len)  time complexity in compile-time.
NOTE:
diffrence is due to constexpr's pointer comparason limitation.

IF THIS FUNCTION GIVES AN ERROR IN CONSTEXPR GCC < 12 :
https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89074
*/
MJZ_CX_AL_FN bool memory_has_overlap(const char *const a_ptr_, uintlen_t a_len,
                                     const char *const b_ptr_,
                                     uintlen_t b_len) noexcept {
  MJZ_IF_CONSTEVAL {
    if (b_len && a_len) {
      const char *a_ptr{a_ptr_};
      for (uintlen_t i{}; i < a_len; a_ptr++, i++) {  //-V2528
        const char *b_ptr{b_ptr_};
        for (uintlen_t j{}; j < b_len; b_ptr++, j++) {  //-V2528
          if (b_ptr == a_ptr) {
            return true;
          }
        }
      }
    } else if (!a_len) {
      const char *const a_ptr{a_ptr_};
      const char *b_ptr{b_ptr_};
      for (uintlen_t j{}; j < b_len; b_ptr++, j++) {  //-V2528
        if (b_ptr == a_ptr) {
          return true;
        }
      }
    } else if (!b_len) {
      const char *a_ptr{a_ptr_};
      const char *const b_ptr{b_ptr_};
      for (uintlen_t j{}; j < a_len; a_ptr++, j++) {  //-V2528
        if (b_ptr == a_ptr) {
          return true;
        }
      }
    } else {
      const char *const a_ptr{a_ptr_};
      const char *const b_ptr{b_ptr_};
      return b_ptr == a_ptr;
    }
    return false;
  }
  else {
    const uintlen_t a_ptr{std::bit_cast<uintptr_t>(a_ptr_)};
    const uintlen_t b_ptr{std::bit_cast<uintptr_t>(b_ptr_)};
    auto b_end = b_ptr + b_len;
    auto a_end = a_ptr + a_len;
    bool a_beg_is_in_b = (b_ptr <= a_ptr);
    a_beg_is_in_b &= (a_ptr < b_end);
    bool b_beg_is_in_a = (a_ptr <= b_ptr);
    b_beg_is_in_a &= (b_ptr < a_end);
    auto a_last = a_end - 1;
    auto b_last = b_end - 1;
    bool a_last_is_in_b = (b_ptr <= a_last);
    a_last_is_in_b &= (a_last < b_end);
    bool b_last_is_in_a = (a_ptr <= b_last);
    b_last_is_in_a &= (b_last < a_end);
    bool ret = a_beg_is_in_b;
    ret |= b_beg_is_in_a;
    ret |= a_last_is_in_b;
    ret |= b_last_is_in_a;
    return ret;
  }
}

MJZ_CX_AL_FN bool memory_is_inside(const char *const hey_stack,
                                   uintlen_t hey_len, const char *const needle,
                                   uintlen_t needle_len) noexcept {
  MJZ_IFN_CONSTEVAL {
    const uintlen_t hey_stack_{std::bit_cast<uintptr_t>(hey_stack)};
    const uintlen_t needle_{std::bit_cast<uintptr_t>(needle)};
    bool good= hey_stack_ <= needle_;
    good&= needle_ + needle_len <= hey_len + hey_stack_;
    return good;
  }
  uintlen_t i{};
  for (; i < hey_len; i++) {
      if(i + hey_stack == needle){
      break;
    }
  }
  if (i + hey_stack != needle) {
    return false;
  }
  return needle + needle_len <= hey_len + hey_stack;


} 
template <typename T>
  requires(std::is_trivially_copy_constructible_v<T> &&
           std::is_trivially_default_constructible_v<T> &&
           std::is_trivially_destructible_v<T>)
MJZ_NCX_FN T cpy_aligned_bitcast(const void *src) noexcept {
  T ret{};
  std::memcpy(&ret, std::assume_aligned<alignof(T)>(src), sizeof(src));
  return ret;
}

template <typename T>
  requires(std::is_trivially_copy_constructible_v<T> &&
           std::is_trivially_default_constructible_v<T> &&
           std::is_trivially_destructible_v<T>)
MJZ_NCX_FN void cpy_aligned_bitcast(void *dest, const T &src) noexcept {
  std::memcpy(std::assume_aligned<alignof(T)>(dest), &src, sizeof(src));
}
MJZ_CX_AL_FN char *memcpy_forward(char *dest, const char *src,
                                  uintlen_t len) noexcept {
  MJZ_IFN_CONSTEVAL {  // If dest or src is a null pointer or invalid pointer,
                       // the behavior is undefined. (NO! , len=0 defines this)
    if (!len) return dest;
    return reinterpret_cast<char *>(
        ::std::memmove(reinterpret_cast<void *>(dest),
                       reinterpret_cast<const void *>(src), size_t(len)));
  }
  char *d = dest;
  const char *s = src;
  while (len--) {
    *d++ = *s++;
  }
  return dest;
}

MJZ_CX_AL_FN char *memcpy_backward(char *dest, const char *src,
                                   uintlen_t len) noexcept {
  MJZ_IFN_CONSTEVAL {  // If dest or src is a null pointer or invalid pointer,
                       // the behavior is undefined. (NO! , len=0 defines this)
    if (!len) return dest;
    return reinterpret_cast<char *>(
        ::std::memmove(reinterpret_cast<void *>(dest),
                       reinterpret_cast<const void *>(src), size_t(len)));
  }
  char *d = dest;
  const char *s = src;
  while (len--) {
    d[len] = s[len];
  }
  return dest;
}

MJZ_CX_AL_FN char *memomve_overlap(char *dest, const char *src,
                                   uintlen_t len) noexcept {
  MJZ_IFN_CONSTEVAL {
    // If dest or src is a null pointer or invalid pointer, the behavior is
    // undefined (NO! , len=0 defines this)
    if (!len) return dest;
    return reinterpret_cast<char *>(
        ::std::memmove(reinterpret_cast<void *>(dest),
                       reinterpret_cast<const void *>(src), size_t(len)));
  }
  if (dest <= src) {
    return memcpy_forward(dest, src, len);
  }
  return memcpy_backward(dest, src, len);
}
/*
O(len) time complexity .
NOTE:
use memmove for potentialy overlaping memory.
*/
MJZ_CX_AL_FN char *memcpy(char *dest, const char *src, uintlen_t len) noexcept {
  MJZ_IFN_CONSTEVAL {
    // If dest or src is a null pointer or invalid pointer, the behavior is
    // undefined (NO! , len=0 defines this)
    if (!len) return dest;
    return reinterpret_cast<char *>(
        ::std::memcpy(reinterpret_cast<void *>(dest),
                      reinterpret_cast<const void *>(src), size_t(len)));
  }
  return memcpy_forward(dest, src, len);
}

/*
O(len) time complexity .
NOTE:
use memmove for potentialy overlaping memory.
*/
MJZ_CX_AL_FN char *memcpy_swap(char *dest, char *src, uintlen_t len) noexcept {
  char *d = dest;
  char *s = src;
  while (len--) {
    std::swap(*d++, *s++);
  }
  return dest;
}
/*
O(len) time complexity in runtime
O(len^2)  time complexity in compile-time.
NOTE:
diffrence is due to constexpr's pointer comparason limitation.
*/
MJZ_CX_AL_FN char *memmove(char *const dest, const char *const src,
                           const uintlen_t len) noexcept {
  const char *from = src;
  char *to = dest;
  if (from == to || len == 0) {
    return dest;
  }
  MJZ_IFN_CONSTEVAL {
    return reinterpret_cast<char *>(
        ::std::memmove(reinterpret_cast<void *>(dest),
                       reinterpret_cast<const void *>(src), size_t(len)));
  }
  if (!memory_has_overlap(dest, len, src, len)) {
    return memcpy(dest, src, len);
  } else {
    const ptrdiff_t n = static_cast<ptrdiff_t>(len);
    if (to > from && to - from < n) {
      /* to overlaps with from */
      /*  <from......>         */
      /*         <to........>  */
      /* copy in reverse, to avoid overwriting from */
      ptrdiff_t i{};
      for (i = n - 1; i >= 0; i--) {
        to[i] = from[i];
      }
      return dest;
    }
    if (from > to && from - to < n) {
      /* to overlaps with from */
      /*        <from......>   */
      /*  <to........>         */
      /* copy forwards, to avoid overwriting from */
      uintlen_t i{};
      for (i = 0; i < len; i++) {
        to[i] = from[i];
      }
      return dest;
    }
    memcpy(dest, src, len);  //-V2547
    return dest;
  }
}

/*
O(len) time complexity in runtime
O(len^2)  time complexity in compile-time.
NOTE:
diffrence is due to constexpr's pointer comparason limitation.
*/
MJZ_CX_AL_FN char *memmove_swap(char *const dest, char *const src,
                                const uintlen_t len) noexcept {
  char *from = src;
  char *to = dest;
  if (from == to || len == 0) {
    return dest;
  }
  if (!memory_has_overlap(dest, len, src, len)) {
    return memcpy_swap(dest, src, len);
  } else {
    const ptrdiff_t n = static_cast<ptrdiff_t>(len);
    if (to > from && to - from < n) {
      /* to overlaps with from */
      /*  <from......>         */
      /*         <to........>  */
      /* copy in reverse, to avoid overwriting from */
      ptrdiff_t i{};
      for (i = n - 1; i >= 0; i--) {
        std::swap(to[i], from[i]);
      }
      return dest;  //-V2547
    }
    if (from > to && from - to < n) {
      /* to overlaps with from */
      /*        <from......>   */
      /*  <to........>         */
      /* copy forwards, to avoid overwriting from */
      uintlen_t i{};
      for (i = 0; i < len; i++) {
        std::swap(to[i], from[i]);
      }
      return dest;
    }
    memcpy_swap(dest, src, len);
    return dest;
  }
}
template <typename T>
MJZ_CX_FN static auto mjz_memset_lambda_createor(T val) noexcept {
  return [val = std::move(val)](
             T &c, MJZ_UNUSED uintlen_t &i,
             MJZ_UNUSED const uintlen_t len) constexpr noexcept -> success_t {
    c = val;
    return 1;
  };
};
MJZ_CX_FN static auto mjz_memset_deafult_zero_lambda =
    mjz_memset_lambda_createor<char>(0);

template <typename T, class Lambda_t>
MJZ_CX_FN T *mjz_mem_iterate(T *dest, const uintlen_t len,
                             Lambda_t &&lambda) noexcept
  requires requires(uintlen_t &i, T *ptr) {
    {
      std::forward<Lambda_t>(lambda)(ptr[i], i, len)
    } noexcept -> std::convertible_to<success_t>;
  }
{
  for (uintlen_t i{}; i < len; i++) {
    if (!std::forward<Lambda_t>(lambda)(dest[i], i, len)) {
      break;
    }
  }
  return dest;
}
MJZ_CX_FN char *memset(char *ptr, uintlen_t len, char val) noexcept {
  MJZ_IFN_CONSTEVAL {
    if (!len) return ptr;
    return reinterpret_cast<char *>(
        ::std::memset(reinterpret_cast<void *>(ptr), val, size_t(len)));
  }
  for (uintlen_t i{}; i < len; i++) {
    ptr[i] = val;
  }
  return ptr;
}
#ifdef MJZ_KNOWN_L1_CACHE_LINE_SIZE
MJZ_CONSTANT(size_t)
hardware_constructive_interference_size = MJZ_KNOWN_L1_CACHE_LINE_SIZE;
MJZ_CONSTANT(size_t)
hardware_destructive_interference_size = MJZ_KNOWN_L1_CACHE_LINE_SIZE;
#else
#ifdef __cpp_lib_hardware_interference_size
/* Minimum offset between two objects to avoid false sharing. Guaranteed to be
 * at least alignof(std::max_align_t)*/
MJZ_CONSTANT(size_t)
hardware_constructive_interference_size =
    std::hardware_constructive_interference_size;
/* Maximum size of contiguous memory to promote true sharing. Guaranteed to be
 * at least alignof(std::max_align_t)*/
MJZ_CONSTANT(size_t)
hardware_destructive_interference_size =
    std::hardware_destructive_interference_size;
#else
MJZ_CONSTANT(size_t)
hardware_constructive_interference_size = 64;
MJZ_CONSTANT(size_t)
hardware_destructive_interference_size = 64;
#endif
#endif  // MJZ_KNOWN_L1_CACHE_LINE_SIZE
struct mjz_sharable_cache_line_t {
  alignas(hardware_destructive_interference_size) char buffer
      [hardware_destructive_interference_size];
};
struct mjz_unique_cache_line_t {
  alignas(hardware_constructive_interference_size) char buffer
      [hardware_constructive_interference_size];
};
static_assert(sizeof(uintlen_t) <= hardware_constructive_interference_size);
static_assert(
    log2_ceil_of_val_create(hardware_constructive_interference_size) ==
    log2_of_val_create(hardware_constructive_interference_size));

template <uintlen_t N>
MJZ_CX_FN alias_t<char (&)[N]> mjz_array_set(char (&array)[N],
                                             char val) noexcept {
  memset(array, N, val);
  return array;
}

template <typename T, class F_t>
  requires requires(F_t &&access_at, uintlen_t i, const T &value) {
    { bool(access_at(i) <= value) } noexcept;
    { bool(access_at(i) < value) } noexcept;
  }
MJZ_CX_FN uintlen_t mjz_binary_search(const T &value, uintlen_t array_size,
                                      F_t &&access_at,
                                      bool exclusive_search = false) noexcept {
  if (!array_size) return array_size;
  if (bool(access_at(array_size - 1) < value)) {
    return array_size;
  }
  if (!bool(access_at(0) <= value)) {
    return 0;
  }
  uintlen_t first{};
  uintlen_t count = uintlen_t(array_size);
  while (count) {
    uintlen_t count2{};
    // for small ones we can do a cache friendly linear seacrh
    if (8 < count) MJZ_MOSTLY_UNLIKELY {
        count2 = count >> 1;
      }
    auto mid = first + count2;
    bool result_{};
    if (exclusive_search) {
      result_ = bool(access_at(mid) <= value);
    } else {
      result_ = bool(access_at(mid) < value);
    }
    if (result_) {
      first = ++mid;
      count -= count2 + 1;
    } else {
      count = count2;
    }
  }
  return first;
}

template <typename T>
concept bitcastable_c = std::is_trivially_copy_constructible_v<T> &&
                        std::is_trivially_destructible_v<T>;

template <bitcastable_c T>
MJZ_CX_FN T cpy_bitcast(const char *src) noexcept {
  alignas(alignof(T)) std::array<char, sizeof(T)> buf{};
  memcpy(buf.data(), src, sizeof(T));
  return std::bit_cast<T>(buf);
}

template <bitcastable_c T>
MJZ_CX_FN auto make_bitcast(const T &src) noexcept {
  return std::bit_cast<std::array<char, sizeof(T)>>(src);
}
template <bitcastable_c T>
MJZ_CX_FN void cpy_bitcast(char *dest, const T &src) noexcept {
  alignas(alignof(T)) auto buf = make_bitcast<T>(src);
  memcpy(dest, buf.data(), sizeof(T));
  return;
}

template <class Like_what_t_, class U>
MJZ_CX_AL_FN auto &&forward_like(U &&x) noexcept {
  constexpr bool is_adding_const =
      std::is_const_v<std::remove_reference_t<Like_what_t_>>;
  if constexpr (std::is_lvalue_reference_v<Like_what_t_ &&>) {
    if constexpr (is_adding_const) {
      return std::as_const(x);
    } else {
      return static_cast<U &>(x);
    }
  } else {
    if constexpr (is_adding_const) {
      return std::move(std::as_const(x));
    } else {
      return std::move(x);
    }
  }
}

template <class Like_what_t_, class T>
using forward_like_t = decltype(forward_like<Like_what_t_>(std::declval<T>()));

template <class T, class Like_what_t_>
concept forward_like_c =
    std::same_as<T &&, forward_like_t<Like_what_t_, std::remove_cvref_t<T>>>;

template <class From_, class Like_to_what_t_>
concept forward_convert_like_c = std::convertible_to<
    From_ &&, forward_like_t<Like_to_what_t_, std::remove_cvref_t<From_>>>;



}  // namespace mjz
#endif  // MJZ_MEMORIES_LIB_HPP_FILE_