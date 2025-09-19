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
#include "base.hpp"
#ifndef MJZ_THREADS_cx_atomic_ref_LIB_HPP_FILE_
#define MJZ_THREADS_cx_atomic_ref_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz::threads_ns {
  /* the illusion of atomic_ref in compile time */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  class cx_atomic_ref_t {
    T &ref;
    MJZ_MCONSTANT(bool) mutable_v { !std::is_const_v<T> };
    /*  struct byte_T_t {
      alignas(alignof(T)) char a[sizeof(T)];
      MJZ_CX_ND_FN std::strong_ordering operator<=>(
          const byte_T_t&) const noexcept = default;
    };*/

  public:
    MJZ_CX_FN explicit cx_atomic_ref_t(T &obj) noexcept : ref(obj) {}

    MJZ_CX_FN cx_atomic_ref_t(const cx_atomic_ref_t &) noexcept = default;
    MJZ_CX_FN cx_atomic_ref_t &operator=(const cx_atomic_ref_t &) = delete;
    MJZ_MCONSTANT(bool) is_always_lock_free = true;
    MJZ_MCONSTANT(size_t)
    required_alignment = std::atomic_ref<T>::required_alignment;

    MJZ_CX_ND_FN bool is_lock_free() const noexcept { return true; }

    MJZ_CX_FN void
    store(const T obj,
          const std::memory_order = std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      ref = obj;
    }
    MJZ_CX_ND_FN T
    load(const std::memory_order = std::memory_order::relaxed) const noexcept {
      return ref;
    }
    MJZ_CX_FN void
    wait(const T Expected,
         const std::memory_order = std::memory_order::relaxed) const noexcept {
      asserts(
          /*std::bit_cast<byte_T_t>(Expected) != std::bit_cast<byte_T_t>(ref)*/
          Expected != ref, "i'm waiting forever at compile time ! :( ", true);
      return;
    }

    MJZ_CX_FN T operator=(const T obj) const noexcept
      requires(mutable_v)
    {
      store(obj);
      return obj;
    }

    MJZ_CX_ND_FN T exchange(
        const T value,
        const std::memory_order = std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return std::exchange(ref, value);
    }

    MJZ_CX_ND_FN success_t compare_exchange_weak(
        T &Expected, const T Desired,
        const std::memory_order = std::memory_order::relaxed,
        const std::memory_order = std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      if (/*std::bit_cast<byte_T_t>(Expected) !=
          std::bit_cast<byte_T_t>(ref) */
          Expected != ref) {
        Expected = ref;
        return false;
      }
      Expected = std::exchange(ref, Desired);
      return true;
    }

    MJZ_CX_ND_FN success_t compare_exchange_strong(
        T &Expected, const T Desired,
        const std::memory_order = std::memory_order::relaxed,
        const std::memory_order = std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      asserts(compare_exchange_weak(Expected, Desired),
              " compare_exchange_strong failed, shall we retry "
              "forever?, you may use compare_exchange_weak if you "
              "expect failure of this function at compile-time");
      return true;
    }

    MJZ_CX_FN operator T() const noexcept { return load(); }

    MJZ_CX_FN void notify_one() const noexcept { return; }

    MJZ_CX_FN void notify_all() const noexcept { return; }

    MJZ_CX_FN
    T fetch_add(const T Operand, const std::memory_order =
                                     std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return exchange(ref + Operand);
    }

    MJZ_CX_FN T fetch_and(
        const T Operand,
        const std::memory_order = std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return exchange(ref & Operand);
    }
    MJZ_CX_FN
    T fetch_or(const T Operand, const std::memory_order =
                                    std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return exchange(ref | Operand);
    }
    MJZ_CX_FN
    T fetch_xor(const T Operand, const std::memory_order =
                                     std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return exchange(ref ^ Operand);
    }
    MJZ_CX_FN
    T operator++(int) const noexcept
      requires(mutable_v)
    {
      return ref++;
    }
    MJZ_CX_FN
    T operator++() const noexcept
      requires(mutable_v)
    {
      return ++ref;
    }
    MJZ_CX_FN
    T operator--(int) const noexcept
      requires(mutable_v)
    {
      return ref--;
    }
    MJZ_CX_FN
    T operator--() const noexcept
      requires(mutable_v)
    {
      return --ref;
    }
    using difference_type = T;

    MJZ_CX_ND_FN auto &&as_normal_() volatile noexcept {
      return *const_cast<cx_atomic_ref_t *>(this);
    }
    MJZ_CX_ND_FN auto &&as_normal_() const volatile noexcept {
      return *const_cast<const cx_atomic_ref_t *>(this);
    }

    MJZ_CX_FN T fetch_add(const T Operand, const std::memory_order =
                                               std::memory_order::relaxed) const
        volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().fetch_add(Operand);
    }

    MJZ_CX_ND_FN static T negate_obj_(const T val) noexcept
      requires(mutable_v)
    {
      if constexpr (std::integral<T>) {
        return static_cast<T>(std::make_unsigned_t<T>() -
                              static_cast<std::make_unsigned_t<T>>(val));
      } else {
        return -val;
      }
    }
    MJZ_CX_FN
    T fetch_sub(const T Operand, const std::memory_order =
                                     std::memory_order::relaxed) const noexcept
      requires(mutable_v)
    {
      return fetch_add(negate_obj_(Operand));
    }
    MJZ_CX_FN
    T fetch_sub(const T Operand,
                const std::memory_order = std::memory_order::relaxed) const
        volatile noexcept
      requires(mutable_v)
    {
      return fetch_add(negate_obj_(Operand));
    }

    MJZ_CX_FN
    T fetch_and(const T Operand,
                const std::memory_order = std::memory_order::relaxed) const
        volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().fetch_and(Operand);
    }
    MJZ_CX_FN
    T fetch_or(const T Operand,
               const std::memory_order = std::memory_order::relaxed) const
        volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().fetch_or(Operand);
    }
    MJZ_CX_FN
    T fetch_xor(const T Operand,
                const std::memory_order = std::memory_order::relaxed) const
        volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().fetch_xor(Operand);
    }
    MJZ_CX_FN
    T operator++(int) const volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().operator++(0);
    }
    MJZ_CX_FN
    T operator++() const volatile noexcept { return as_normal_().operator++(); }
    MJZ_CX_FN
    T operator--(int) const volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().operator--(0);
    }
    MJZ_CX_FN

    T operator--() const volatile noexcept
      requires(mutable_v)
    {
      return as_normal_().operator--();
    }
    MJZ_CX_FN
    T operator+=(const T Operand) const noexcept
      requires(mutable_v)
    {
      return static_cast<T>(this->fetch_add(Operand) + Operand);
    }
    MJZ_CX_FN

    T operator+=(const T Operand) const volatile noexcept
      requires(mutable_v)
    {
      return static_cast<T>(as_normal_().fetch_add(Operand) + Operand);
    }
    MJZ_CX_FN

    T operator-=(const T Operand) const noexcept
      requires(mutable_v)
    {
      return static_cast<T>(fetch_sub(Operand) - Operand);
    }
    MJZ_CX_FN

    T operator-=(const T Operand) const volatile noexcept
      requires(mutable_v)
    {
      return static_cast<T>(as_normal_().fetch_sub(Operand) - Operand);
    }
    MJZ_CX_FN
    T operator&=(const T Operand) const noexcept
      requires(mutable_v)
    {
      return static_cast<T>(this->fetch_and(Operand) & Operand);
    }
    MJZ_CX_FN
    T operator&=(const T Operand) const volatile noexcept
      requires(mutable_v)
    {
      return static_cast<T>(as_normal_().fetch_and(Operand) & Operand);
    }
    MJZ_CX_FN
    T operator|=(const T Operand) const noexcept
      requires(mutable_v)
    {
      return static_cast<T>(this->fetch_or(Operand) | Operand);
    }
    MJZ_CX_FN

    T operator|=(const T Operand) const volatile noexcept
      requires(mutable_v)
    {
      return static_cast<T>(as_normal_().fetch_or(Operand) | Operand);
    }
    MJZ_CX_FN
    T operator^=(const T Operand) const noexcept
      requires(mutable_v)
    {
      return static_cast<T>(this->fetch_xor(Operand) ^ Operand);
    }
    MJZ_CX_FN
    T operator^=(const T Operand) const volatile noexcept
      requires(mutable_v)
    {
      return static_cast<T>(as_normal_().fetch_xor(Operand) ^ Operand);
    }
  };
  template <typename T> cx_atomic_ref_t(T) -> cx_atomic_ref_t<T>;
}; // namespace mjz::threads_ns
#endif //  MJZ_THREADS_cx_atomic_ref_LIB_HPP_FILE_