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
#ifndef MJZ_THREADS_lock_guard_LIB_HPP_FILE_
#define MJZ_THREADS_lock_guard_LIB_HPP_FILE_
namespace mjz::threads_ns {
template <class my_mutex_t>
class MJZ_NODISCRAD_FOR(
    " you forgot to use lock_guard_t , or you forgot to use std::ignore")
    lock_guard_t {
  my_mutex_t *ptr{};
  MJZ_NO_CPY(lock_guard_t);

 public:
  MJZ_CX_FN lock_guard_t(lock_guard_t &&obj) noexcept
      : ptr(std::exchange(obj.ptr, nullptr)) {}
  MJZ_CX_FN lock_guard_t &operator=(lock_guard_t &&obj) noexcept {
    if (ptr) {
      ptr->unlock();
    }
    ptr = std::exchange(obj.ptr, nullptr);
    return *this;
  }
  MJZ_CX_FN lock_guard_t(my_mutex_t &ref, const bool need_to_wait,
                         uint64_t timeout_count = defult_timeout) noexcept
      : ptr(&ref) {
    if (!ref.try_lock(need_to_wait, timeout_count)) {
      ptr = nullptr;
    }
  }
  MJZ_CX_FN
  lock_guard_t(my_mutex_t &ref) noexcept(noexcept(my_mutex_t{}.lock()))
      : ptr(&ref) {
    ref.lock();
  }
  MJZ_CX_FN lock_guard_t() noexcept = default;
  MJZ_CX_FN ~lock_guard_t() noexcept {
    if (ptr) {
      ptr->unlock();
    }
  }
  MJZ_CX_ND_FN explicit operator bool() const noexcept { return !!ptr; }
};
template <typename T>
lock_guard_t(T) -> lock_guard_t<T>;
};  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_lock_guard_LIB_HPP_FILE_