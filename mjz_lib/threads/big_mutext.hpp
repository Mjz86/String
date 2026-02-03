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

#include "bit_mutex.hpp"

#ifndef MJZ_THREADS_big_mutex_LIB_HPP_FILE_
#define MJZ_THREADS_big_mutex_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz::threads_ns {
  template <partial_same_as<totally_empty_type_t> T_E_ = totally_empty_type_t>
  class big_mutex_t {

  private:
    MJZ_NO_MV_NO_CPY(big_mutex_t);
    union data_t {
      MJZ_NO_MV_NO_CPY(data_t);
      MJZ_DISABLE_ALL_WANINGS_START_;
      alignas(alignof(std::mutex)) char r_mutex_[sizeof(std::mutex)];
      bit_mutex_t<T_E_> cx_mutex;
      MJZ_DISABLE_ALL_WANINGS_END_;
      MJZ_DISABLE_ALL_WANINGS_START_;
      MJZ_CX_FN data_t() noexcept {
        MJZ_DISABLE_ALL_WANINGS_END_;
        MJZ_IF_CONSTEVAL { std::construct_at(&cx_mutex); }
        else {
          std::construct_at(reinterpret_cast<std::mutex *>(r_mutex_));
        }
      }
      MJZ_DISABLE_ALL_WANINGS_START_;
      MJZ_CX_FN std::mutex &get_r() noexcept {
        return *(reinterpret_cast<std::mutex *>(r_mutex_));
      }
      MJZ_CX_FN ~data_t() noexcept {
        MJZ_DISABLE_ALL_WANINGS_END_;
        MJZ_IF_CONSTEVAL {
          asserts(!cx_mutex, "you forgot to unlock this object!!");
          std::destroy_at(&cx_mutex);
        }
        else {
          std::destroy_at(&get_r());
        }
      }
    };
    data_t m{};

  public:
    MJZ_CX_FN ~big_mutex_t() noexcept = default;
    MJZ_CX_FN big_mutex_t() noexcept = default;
    MJZ_CX_ND_RES_OBJ_FN
    success_t try_lock(const bool need_to_wait,
                       uint64_t timeout_count) noexcept {
      MJZ_IFN_CONSTEVAL { return try_lock(); }
      return m.cx_mutex.try_lock(need_to_wait, timeout_count);
    }
    MJZ_CX_ND_RES_OBJ_FN success_t try_lock() noexcept {
      MJZ_IF_CONSTEVAL { return m.cx_mutex.try_lock(); }
      else {
        return m.get_r().try_lock();
      }
    }
    MJZ_CX_FN void lock() {
      MJZ_IF_CONSTEVAL { return m.cx_mutex.lock(); }
      else {
        MJZ_DISABLE_ALL_WANINGS_START_;
        std::ignore = std::unique_lock{m.get_r()}.release();
        MJZ_DISABLE_ALL_WANINGS_END_;
      }
    }
    MJZ_CX_FN void unlock() noexcept {
      MJZ_IF_CONSTEVAL { return m.cx_mutex.unlock(); }
      else {
        MJZ_DISABLE_ALL_WANINGS_START_;
        std::ignore = std::unique_lock{m.get_r(), std::adopt_lock};
        MJZ_DISABLE_ALL_WANINGS_END_;
      }
    }
  };
} // namespace mjz::threads_ns
#endif // MJZ_THREADS_big_mutex_LIB_HPP_FILE_