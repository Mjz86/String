
/*MIT License

Copyright (c) 2026 Mjz86

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
#include "../memories.hpp"
#include "atomic_ref.hpp"

#ifndef MJZ_THREADS_jthread_worker_HPP_FILE_
#define MJZ_THREADS_jthread_worker_HPP_FILE_
MJZ_EXPORT namespace mjz::threads_ns {
  namespace worker_ns {
  struct alignas(hardware_destructive_interference_size) signal_worker_t {
    MJZ_CX_FN void wait_less(uintlen_t target) const noexcept {
      mjz::threads_ns ::atomic_ref_t<const uintlen_t> atom{atomic_active_count};
      do {
        uintlen_t now = atom.load(std::memory_order_acquire);
        if (now < target)
          return;
        atom.wait(now, std::memory_order_relaxed);
      } while (true);
    }

    MJZ_CX_FN void inc() noexcept {
      mjz::threads_ns ::atomic_ref_t<uintlen_t> atom{atomic_active_count};
      atom.fetch_add(1, std::memory_order_acq_rel);
      atom.notify_one();
    }

    MJZ_CX_FN void dec() noexcept {
      mjz::threads_ns ::atomic_ref_t<uintlen_t> atom{atomic_active_count};
      atom.fetch_sub(1, std::memory_order_acq_rel);
      atom.notify_one();
    }

  private:
    uintlen_t atomic_active_count{};
  };
  template <class data_t, callable_c<void(data_t &&) noexcept> auto lambda_fn>
  struct alignas(hardware_destructive_interference_size) worker_t {
    MJZ_NO_MV_NO_CPY(worker_t);

  private:
    std::optional<data_t> payload{};
    mjz::threads_ns::jthread<> th{};
    signal_worker_t *signal_work{};
    // alignas the worker_t at both boundaries
    alignas(hardware_destructive_interference_size) char c_lock{};

    enum lock_states_e : char { sleepy, go_signal, working };

  public:
    MJZ_CX_FN worker_t(signal_worker_t *signal_work_ptr = nullptr) noexcept
        : signal_work{signal_work_ptr} {
      mjz::threads_ns ::atomic_ref_t<char> lock{c_lock};
      lock.store(sleepy, std::memory_order_relaxed);
      th = mjz::threads_ns::jthread<>([this]() noexcept { runner(); });
    }
    MJZ_CX_FN bool accepting_work() const noexcept {
      mjz::threads_ns ::atomic_ref_t<const char> lock{c_lock};
      return sleepy == lock.load(std::memory_order_acquire);
    }
    MJZ_CX_FN ~worker_t() noexcept {
      shutdown();
      (void)th.join();
    }

    MJZ_CX_FN void shutdown() noexcept { assign_impl(std::nullopt); }
    template <class... Ts> MJZ_CX_FN void assign(Ts &&...args) noexcept {
      assign_impl(data_t(std::forward<Ts>(args)...));
    }
    MJZ_CX_FN bool waiting_accepting_work() const noexcept {
      mjz::threads_ns ::atomic_ref_t<const char> lock{c_lock};
      char c = lock.load(std::memory_order_acquire);
      if (sleepy == c)
        return true;
      lock.wait(c, std::memory_order_relaxed);
      return sleepy == lock.load(std::memory_order_acquire);
    }

  private:
    MJZ_CX_FN void runner() noexcept {
      mjz::threads_ns ::atomic_ref_t<char> lock{c_lock};
      do {
        char c{};
        do {
          c = lock.load(std::memory_order_acquire);
          if (c == go_signal && lock.compare_exchange_strong(
                                    c, working, std::memory_order_acq_rel))
            break;
          if (c == go_signal)
            continue;
          lock.wait(c, std::memory_order_relaxed);
        } while (true);

        MJZ_RAII_RELEASE {

          lock.store(sleepy, std::memory_order_release);
          lock.notify_one();
          if (signal_work) {
            signal_work->dec();
          }
        };
        if (!payload)
          return;
        lambda_fn(*std::move(payload));
        payload.reset();

      } while (true);
    }

    MJZ_CX_FN void assign_impl(std::optional<data_t> lam) noexcept {
      mjz::threads_ns ::atomic_ref_t<char> lock{c_lock};
      char c{};
      do {
        c = lock.load(std::memory_order_acquire);
        if (c == sleepy &&
            lock.compare_exchange_strong(c, working, std::memory_order_acq_rel))
          break;
        if (c == sleepy)
          continue;
        lock.wait(c, std::memory_order_relaxed);
      } while (true);
      payload = std::move(lam);
      if (signal_work) {
        signal_work->inc();
      }
      lock.store(go_signal, std::memory_order_release);
      lock.notify_one();
    }
  };

  template <class data_t, callable_c<void(data_t &&) noexcept> auto lambda_fn>
  struct alignas(hardware_destructive_interference_size)
      single_producer_worker_pool_t {
    MJZ_NO_MV_NO_CPY_DC(single_producer_worker_pool_t);
    using base = worker_t<data_t, lambda_fn>;
    template <class... Ts> MJZ_CX_FN success_t assign(Ts &&...args) noexcept {
      uintlen_t i{};
      do {
        i = 0;
        do {
          if (workers[i].accepting_work())
            break;
          i++;
        } while (i < ring_size_v);
        if (i < ring_size_v)
          break;
        signal_worker_thing.wait_less(ring_size_v);
      } while (true);
      workers[i].assign(std::forward<Ts>(args)...);
      return true;
    }
    MJZ_CX_FN single_producer_worker_pool_t(uintlen_t worker_count) noexcept
        : ring_size_v{worker_count}, workers{} {
      workers = std::allocator<base>().allocate(ring_size_v);
      for (base *p : std::views::iota(workers, workers + ring_size_v))
        std::construct_at(p, &signal_worker_thing);
    }

    MJZ_CX_FN ~single_producer_worker_pool_t() noexcept {
      for (base *p : std::views::iota(workers, workers + ring_size_v) |
                         std::views::reverse)
        std::destroy_at(p);
      std::allocator<base>().deallocate(workers, ring_size_v);
    }

    MJZ_CX_FN void shutdown() noexcept {
      for (uintlen_t j = 0; j < ring_size_v; j++) {
        workers[j].shutdown();
      }
    }
    MJZ_CX_FN void wait_idle() noexcept {
      for (uintlen_t j = 0; j < ring_size_v; j++) {
        while (!workers[j].waiting_accepting_work())
          ;
      }
    }

    MJZ_CX_FN uintlen_t thread_size() const noexcept { return ring_size_v; }

  private:
    uintlen_t ring_size_v{};
    base *workers{};
    signal_worker_t signal_worker_thing{};
  };
  }; // namespace worker_ns
};

#endif // MJZ_THREADS_jthread_worker_HPP_FILE_