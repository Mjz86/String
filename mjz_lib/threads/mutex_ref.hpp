#include "atomic_ref.hpp"
#ifndef MJZ_THREADS_mutex_ref_LIB_HPP_FILE_
#define MJZ_THREADS_mutex_ref_LIB_HPP_FILE_
namespace mjz::threads_ns {
template <std::integral T>
class mutex_ref_t : private atomic_ref_t<T> {
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(T) one { 1 };
  MJZ_CONSTANT(T) zero { 0 };

 private:
  MJZ_NCX_FN bool test_and_set_m() noexcept {
    return this->exchange(one, std::memory_order_acquire) != zero;
  }
  MJZ_NCX_FN void wait_m() noexcept {
    return this->wait(one, std::memory_order_acquire);
  }
  MJZ_NCX_FN void unlock_m() noexcept {
    this->store(false, std::memory_order_release);
    if (this->load(std::memory_order_acquire) == zero) MJZ_IS_LIKELY {
        this->notify_one();
      }
  }

  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_ncx(const bool need_to_wait,
                         uint64_t timeout_count) noexcept {
    MJZ_IF_CONSTEVAL { return true; }
    uint64_t i{};
    while (i < timeout_count && this->test_and_set_m()) {
      i++;
    }
    if (i < timeout_count) {
      return true;
    }
    if (!need_to_wait) {
      return false;
    }
    while (this->test_and_set_m()) {
      this->wait_m();
    }
    return true;
  }
  MJZ_CX_FN void unlock_ncx() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    this->unlock_m();
  }
  MJZ_CX_FN void unlock_cx() noexcept {
    MJZ_IFN_CONSTEVAL { return; }
    asserts(!!this->load(), " the mutex is not locked , why did you unlock??");
    this->store(zero);
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_cx(const bool need_to_wait) noexcept {
    MJZ_IFN_CONSTEVAL { return false; }
    if (need_to_wait) {
      asserts(!this->load(),
              " the mutex is  already locked , why did you re-lock??");
      this->store(one);
      return true;
    }
    if (this->load()) {
      return false;
    }
    this->store(one);
    return true;
  }

 public:
  MJZ_CX_FN mutex_ref_t(atomic_ref_t<T>&& obj) noexcept
      : atomic_ref_t<T>{std::move(obj)} {};
  MJZ_CX_FN mutex_ref_t(const mutex_ref_t&) noexcept = default;
  MJZ_CX_FN mutex_ref_t(mutex_ref_t&&) noexcept = default;
  MJZ_CX_FN mutex_ref_t& operator=(const mutex_ref_t&) noexcept = delete;
  MJZ_CX_FN mutex_ref_t& operator=(mutex_ref_t&&) noexcept = delete;
  MJZ_CX_FN ~mutex_ref_t() noexcept = default;

  MJZ_CX_FN void unlock() noexcept {
    MJZ_IFN_CONSTEVAL { return this->unlock_ncx(); }
    return this->unlock_cx();
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock(const bool need_to_wait, uint64_t timeout_count) noexcept {
    MJZ_IFN_CONSTEVAL {
      return this->try_lock_ncx(need_to_wait, timeout_count);
    }
    return this->try_lock_cx(need_to_wait);
  }

  MJZ_CX_ND_RES_OBJ_FN success_t try_lock(uint64_t timeout_count) noexcept {
    return try_lock(false, timeout_count);
  }
  MJZ_CONSTANT(uint64_t)
  defult_timeout = threads_ns::defult_timeout;
  MJZ_CX_ND_RES_OBJ_FN success_t try_lock() noexcept {
    return this->try_lock(defult_timeout);
  }
  MJZ_CX_FN void lock() noexcept(!MJZ_IN_DEBUG_MODE) {
    if (this->try_lock(true, defult_timeout)) MJZ_IS_LIKELY {
        return;
      }
    asserts.panic(" what happened !? why did the un-fail-able fail?! ");
  }
  /*
   *returns true if its locked
   */
  MJZ_CX_ND_FN
  explicit operator bool() const noexcept {
    return !!this->load(std::memory_order_acquire);
  }
};
}  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_mutex_ref_LIB_HPP_FILE_