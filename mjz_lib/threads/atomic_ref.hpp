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
#include <atomic>
#include <thread>

#include "cx_atomic_ref.hpp"
#ifndef MJZ_THREADS_atomic_ref_LIB_HPP_FILE_
#define MJZ_THREADS_atomic_ref_LIB_HPP_FILE_
namespace mjz::threads_ns {
template <typename T>
  requires std::is_trivially_copyable_v<T>
class atomic_ref_t {
  MJZ_CONSTANT(bool) mutable_v { !std::is_const_v<T> };
  atomic_ref_t &operator=(const atomic_ref_t &) = delete;
  union ref_pair_t {
    MJZ_CX_FN ref_pair_t(const ref_pair_t &) = default;
    ref_pair_t &operator=(const ref_pair_t &) = delete;
    char dummmy{};
    using a_ref_t = std::atomic_ref<T>;
    T *a_ref;
    cx_atomic_ref_t<T> cx_ref;
    MJZ_CX_FN ref_pair_t() noexcept {}
    MJZ_CX_FN ref_pair_t(T &obj) noexcept {
      MJZ_IF_CONSTEVAL { std::construct_at(&cx_ref, obj); }
      else {
        std::construct_at(&a_ref, &obj);
      }
    }
    MJZ_CX_FN ~ref_pair_t() noexcept {
      MJZ_IF_CONSTEVAL { std::destroy_at(&cx_ref); }
      else {
        std::destroy_at(&a_ref);
      }
    }
    template <typename Self_t, class Lmabda_t>
    MJZ_CX_FN static auto perform(Self_t &&Self, Lmabda_t &&Lmabda) noexcept {
      static_assert(
          callable_anyret_c<Lmabda_t,
                            void(decltype(ref_pair_t::cx_ref) &) noexcept> &&
          callable_anyret_c<Lmabda_t, void(a_ref_t &) noexcept> &&
          partial_same_as<Self_t, ref_pair_t>);
      MJZ_IF_CONSTEVAL { return Lmabda(std::forward<Self_t>(Self).cx_ref); }
      else {
        return [&]() noexcept {
          a_ref_t a_ref_{*std::forward<Self_t>(Self).a_ref};
          return Lmabda(a_ref_);
        }();
      }
    }
  };
  ref_pair_t m{};
  template <class L_t>
  MJZ_CX_FN auto perform(L_t &&Lmabda) const noexcept {
    return ref_pair_t::perform(m, std::forward<L_t>(Lmabda));
  }
  template <class L_t>
  MJZ_CX_FN auto perform(L_t &&Lmabda) noexcept {
    return ref_pair_t::perform(m, std::forward<L_t>(Lmabda));
  }
  template <class L_t>
  MJZ_CX_FN auto perform(L_t &&Lmabda) const volatile noexcept {
    return ref_pair_t::perform(m, std::forward<L_t>(Lmabda));
  }
  template <class L_t>
  MJZ_CX_FN auto perform(L_t &&Lmabda) volatile noexcept {
    return ref_pair_t::perform(m, std::forward<L_t>(Lmabda));
  }

 public:
  MJZ_CX_FN explicit atomic_ref_t(T &obj) noexcept : m(obj) {}

  MJZ_CX_FN atomic_ref_t(const atomic_ref_t &) noexcept = default;
  MJZ_CONSTANT(bool)
  is_always_lock_free = std::atomic_ref<T>::is_always_lock_free;
  MJZ_CONSTANT(size_t)
  required_alignment = std::atomic_ref<T>::required_alignment;

  MJZ_CX_ND_FN bool is_lock_free() const noexcept {
    return perform(
        [&](auto &&ref) noexcept -> bool { return ref.is_lock_free(); });
  }

  MJZ_CX_FN void store(
      const T obj,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> void { return ref.store(obj, mo); });
  }
  MJZ_CX_ND_FN T
  load(const std::memory_order mo = std::memory_order_seq_cst) const noexcept {
    return perform([&](auto &&ref) noexcept -> T { return ref.load(mo); });
  }
  MJZ_CX_FN void wait(
      const T Expected,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept {
    return perform(
        [&](auto &&ref) noexcept -> void { return ref.wait(Expected, mo); });
  }
  MJZ_CX_FN void try_wait(
      const T Expected,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept {
#if MJZ_SLEEP_WITH_WAIT_
    if (load(mo) == Expected) {
      std::this_thread::sleep_for(
          std::chrono_literals::operator""ns(MJZ_SLEEP_WITH_WAIT_));
    }
#else
    wait(Expected, mo);
#endif  // MJZ_SLEEP_WITH_WAIT_
  }
  MJZ_CX_FN void try_notify_one() const noexcept {
#if !MJZ_SLEEP_WITH_WAIT_
    notify_one();
#endif  // MJZ_SLEEP_WITH_WAIT_
  }

  MJZ_CX_FN void try_notify_all() const noexcept {
#if !MJZ_SLEEP_WITH_WAIT_
    notify_all();
#endif  // MJZ_SLEEP_WITH_WAIT_
  }

  MJZ_CX_FN T operator=(const T obj) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref = obj; });
  }

  MJZ_CX_ND_FN T exchange(
      const T value,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.exchange(value, mo); });
  }

  MJZ_CX_ND_FN success_t compare_exchange_weak(
      T &Expected, const T Desired,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> success_t {
      return ref.compare_exchange_weak(Expected, Desired, mo);
    });
  }

  MJZ_CX_ND_FN success_t compare_exchange_strong(
      T &Expected, const T Desired,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> success_t {
      return ref.compare_exchange_strong(Expected, Desired, mo);
    });
  }
  MJZ_CX_ND_FN success_t compare_exchange_weak(
      T &Expected, const T Desired, const std::memory_order success,
      const std::memory_order failure) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> success_t {
      return ref.compare_exchange_weak(Expected, Desired, success, failure);
    });
  }

  MJZ_CX_ND_FN success_t compare_exchange_strong(
      T &Expected, const T Desired, const std::memory_order success,
      const std::memory_order failure) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> success_t {
      return ref.compare_exchange_strong(Expected, Desired, success, failure);
    });
  }

  MJZ_CX_FN operator T() const noexcept {
    return perform([&](auto &&ref) noexcept -> T { return T(ref); });
  }

  MJZ_CX_FN void notify_one() const noexcept {
    return perform(
        [&](auto &&ref) noexcept -> void { return ref.notify_one(); });
  }

  MJZ_CX_FN void notify_all() const noexcept {
    return perform(
        [&](auto &&ref) noexcept -> void { return ref.notify_all(); });
  }

  MJZ_CX_FN
  T fetch_add(const T Operand, const std::memory_order mo =
                                   std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_add(Operand, mo); });
  }

  MJZ_CX_FN T fetch_and(
      const T Operand,
      const std::memory_order mo = std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_and(Operand, mo); });
  }
  MJZ_CX_FN
  T fetch_or(const T Operand, const std::memory_order mo =
                                  std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_or(Operand, mo); });
  }
  MJZ_CX_FN
  T fetch_xor(const T Operand, const std::memory_order mo =
                                   std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_xor(Operand, mo); });
  }
  MJZ_CX_FN
  T operator++(int) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref++; });
  }
  MJZ_CX_FN
  T operator++() const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ++ref; });
  }
  MJZ_CX_FN
  T operator--(int) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref--; });
  }
  MJZ_CX_FN
  T operator--() const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return --ref; });
  }
  using difference_type = T;

  MJZ_CX_FN T fetch_add(const T Operand, const std::memory_order mo =
                                             std::memory_order_seq_cst) const
      volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_add(Operand, mo); });
  }

  MJZ_CX_FN
  T fetch_sub(const T Operand, const std::memory_order mo =
                                   std::memory_order_seq_cst) const noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_sub(Operand, mo); });
  }
  MJZ_CX_FN
  T fetch_sub(const T Operand,
              const std::memory_order mo = std::memory_order_seq_cst) const
      volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_sub(Operand, mo); });
  }
  MJZ_CX_FN
  T fetch_and(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_and(Operand); });
  }
  MJZ_CX_FN
  T fetch_and(const T Operand,
              const std::memory_order mo = std::memory_order_seq_cst) const
      volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_and(Operand, mo); });
  }

  MJZ_CX_FN
  T fetch_or(const T Operand,
             const std::memory_order mo = std::memory_order_seq_cst) const
      volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_or(Operand, mo); });
  }
  MJZ_CX_FN
  T fetch_xor(const T Operand,
              const std::memory_order mo = std::memory_order_seq_cst) const
      volatile noexcept
    requires(mutable_v)
  {
    return perform(
        [&](auto &&ref) noexcept -> T { return ref.fetch_xor(Operand, mo); });
  }
  MJZ_CX_FN
  T operator++(int) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref++; });
  }
  MJZ_CX_FN
  T operator++() const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ++ref; });
  }
  MJZ_CX_FN
  T operator--(int) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref--; });
  }
  MJZ_CX_FN

  T operator--() const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return --ref; });
  }
  MJZ_CX_FN
  T operator+=(const T Operand) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref += Operand; });
  }
  MJZ_CX_FN

  T operator+=(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref += Operand; });
  }
  MJZ_CX_FN

  T operator-=(const T Operand) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref -= Operand; });
  }
  MJZ_CX_FN

  T operator-=(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref -= Operand; });
  }
  MJZ_CX_FN
  T operator&=(const T Operand) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref &= Operand; });
  }
  MJZ_CX_FN
  T operator&=(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref &= Operand; });
  }
  MJZ_CX_FN
  T operator|=(const T Operand) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref |= Operand; });
  }
  MJZ_CX_FN

  T operator|=(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref |= Operand; });
  }
  MJZ_CX_FN
  T operator^=(const T Operand) const noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref ^= Operand; });
  }
  MJZ_CX_FN
  T operator^=(const T Operand) const volatile noexcept
    requires(mutable_v)
  {
    return perform([&](auto &&ref) noexcept -> T { return ref ^= Operand; });
  }
};
template <typename T>
atomic_ref_t(T) -> atomic_ref_t<T>;
};  // namespace mjz::threads_ns
#endif  //  MJZ_THREADS_atomic_ref_LIB_HPP_FILE_