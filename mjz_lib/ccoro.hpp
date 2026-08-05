

#include "optional_ref.hpp"
#ifndef MJZ_CCORO_LIB_HPP_FILE_
#define MJZ_CCORO_LIB_HPP_FILE_
MJZ_EXPORT
namespace mjz {
inline namespace ccoro_ns {

struct base_coroutine_frame_ABI_t;
struct base_coroutine_frame_t;
struct base_coroutine_handle_t {
  using ptr_t = single_object_pointer_t<const base_coroutine_frame_ABI_t>;
  ptr_t ptr{};
  MJZ_CX_FN static base_coroutine_handle_t noop() noexcept;
  MJZ_CX_FN base_coroutine_handle_t() noexcept = default;

  template <class frame_t>
    requires std::derived_from<frame_t, base_coroutine_frame_t>
  MJZ_CX_FN base_coroutine_handle_t(frame_t *p) noexcept : ptr{p} {}
  MJZ_CX_FN base_coroutine_handle_t(ptr_t p, void_struct_t) noexcept : ptr{p} {}
  MJZ_CX_FN base_coroutine_handle_t(std::nullptr_t) noexcept : ptr{} {}
  MJZ_CX_ND_FN ptr_t address() const noexcept { return ptr; }
  MJZ_CX_ND_FN bool is_done() const noexcept;
  MJZ_CX_ND_FN bool was_fatal() const noexcept;
  MJZ_CX_ND_FN bool is_none() const noexcept;
  MJZ_CX_ND_FN bool is_noop() const noexcept;
  MJZ_CX_ND_FN bool can_exe() const noexcept;
  MJZ_CX_ND_FN bool on_start() const noexcept;
  MJZ_CX_ND_FN bool was_canceled() const noexcept;
  MJZ_CX_ND_FN bool has_work() const noexcept;
  MJZ_CX_ND_FN bool can_resume() const noexcept;
  MJZ_CX_FN success_t request_cancel() noexcept;
  MJZ_CX_FN success_t resume() noexcept;
  MJZ_CX_FN success_t destroy() noexcept;
  MJZ_CX_FN success_t cancel_destroy() noexcept;
  MJZ_CX_FN success_t execute() noexcept;
  MJZ_CX_ND_FN pair_t<base_coroutine_handle_t, success_t>
  execute_stack() noexcept;
  MJZ_CX_FN success_t cancel() noexcept;
  MJZ_CX_ND_FN explicit operator bool() const noexcept { return !is_none(); }

  MJZ_CX_ND_FN optional_ref_t<base_coroutine_frame_t> frame() noexcept;
  MJZ_CX_ND_FN optional_ref_t<const base_coroutine_frame_t>
  frame() const noexcept;

  template <std::derived_from<base_coroutine_frame_t> frame_t>
  MJZ_CX_ND_FN optional_ref_t<frame_t> promise() noexcept {
    return static_cast<frame_t *>(frame().get());
  }
  template <std::derived_from<base_coroutine_frame_t> frame_t>
  MJZ_CX_ND_FN optional_ref_t<const frame_t> promise() const noexcept {
    return static_cast<const frame_t *>(frame().get());
  }
  MJZ_CX_FN bool
  operator==(const base_coroutine_handle_t &) const noexcept = default;
};

struct base_coroutine_frame_ABI_t {
  using state_machine_mfnp_t =
      base_coroutine_handle_t (base_coroutine_frame_ABI_t::*)() noexcept;
  using state_machine_fnp_t =
      base_coroutine_handle_t (*)(base_coroutine_frame_ABI_t &) noexcept;
  MJZ_CX_ND_FN static base_coroutine_handle_t
  noop_impl(base_coroutine_frame_ABI_t &) noexcept {
    return nullptr;
  }
  constexpr static inline auto noop_impl_fnp_v =
      &base_coroutine_frame_ABI_t::noop_impl;
  intptr_t program_counter{1};
  state_machine_fnp_t state_machine_fnp{noop_impl_fnp_v};
  MJZ_CX_ND_FN bool is_noop() const noexcept {
    return state_machine_fnp == noop_impl_fnp_v;
  }

  MJZ_CX_ND_FN bool is_done() const noexcept {
    return !is_none() && program_counter == 0;
  }
  MJZ_CX_ND_FN bool was_fatal() const noexcept {
    return program_counter && !state_machine_fnp;
  }

  MJZ_CX_ND_FN bool is_none() const noexcept { return !state_machine_fnp; }
  MJZ_CX_ND_FN bool can_exe() const noexcept {
    return !(is_noop() || is_none());
  }
  MJZ_CX_ND_FN bool on_start() const noexcept {
    return !is_none() && (program_counter == 1 || program_counter == -1);
  }
  MJZ_CX_ND_FN bool was_canceled() const noexcept {
    return !is_none() && program_counter < 0;
  }
  MJZ_CX_ND_FN bool has_work() const noexcept {
    return !is_none() && 0 < program_counter;
  }
  MJZ_CX_ND_FN bool can_resume() const noexcept {
    return !is_none() && 0 != program_counter;
  }
  MJZ_CX_ND_FN intptr_t suspend_counter() noexcept {
    return std::max(-program_counter, program_counter);
  }
};

struct base_coroutine_frame_t : base_coroutine_frame_ABI_t {
  using base_coroutine_frame_ABI_t::state_machine_fnp_t;
  constexpr static inline base_coroutine_frame_ABI_t noop_frame_v{};
  constexpr static inline auto noop_impl_handle_v =
      base_coroutine_handle_t(&noop_frame_v, void_struct_t{});
  MJZ_CX_ND_FN static base_coroutine_handle_t noop() noexcept {
    return noop_impl_handle_v;
  }

  MJZ_CX_ND_FN base_coroutine_handle_t coro_handle() noexcept {
    return base_coroutine_handle_t(this);
  }
  MJZ_CX_FN success_t request_cancel() noexcept {
    if (program_counter <= 0)
      return false;
    program_counter = -program_counter;
    return true;
  }
  MJZ_CX_FN success_t resume() noexcept { return can_resume() && execute(); }
  MJZ_CX_FN success_t destroy() noexcept { return is_done() && execute(); }

  MJZ_CX_FN success_t cancel_destroy() noexcept {
    return cancel() && destroy();
  }
  MJZ_CX_FN success_t cancel() noexcept {
    request_cancel();
    while (was_canceled()) {
      if (!execute())
        return false;
    }
    return true;
  }

  // any call to suspend inside a call stack has to register the handle for
  // suspener.
  // much easier implement than forcing a tail call using C macros ,
  // however the downside is that invocation goes through this brucratic loop.
  MJZ_CX_ND_FN pair_t<base_coroutine_handle_t, success_t>
  execute_stack() noexcept {
    base_coroutine_handle_t task{this};
    base_coroutine_handle_t last{noop()};
    while (task.can_exe()) {
      base_coroutine_frame_t &task_frame = *task.frame();
      last = std::exchange(task, task_frame.state_machine_fnp(task_frame));
    };
    return {last, !!task};
  }

  MJZ_CX_FN success_t execute() noexcept {
    base_coroutine_handle_t last{this};
    while (true) {
      auto [task, good] = last.execute_stack();
      if (!good || !task.can_exe() || task == base_coroutine_handle_t{this})
        return good;
      last = task;
    };
    return false;
  }

  MJZ_CX_FN base_coroutine_handle_t achieved_final_suspend() noexcept {
    program_counter = 0;
    return noop();
  }
  MJZ_CX_FN base_coroutine_handle_t
  achieved_final_transfer(base_coroutine_handle_t next) noexcept {
    program_counter = 0;
    return next;
  }

  MJZ_CX_FN base_coroutine_handle_t end_final_suspend() noexcept {
    asserts(asserts.assume_rn, 0 == program_counter);
    state_machine_fnp = nullptr;
    return noop();
  }
  MJZ_CX_FN base_coroutine_handle_t
  end_final_transfer(base_coroutine_handle_t next) noexcept {
    end_final_suspend();
    return next;
  }

  MJZ_CX_FN base_coroutine_handle_t
  set_handle(state_machine_fnp_t handle) noexcept {
    asserts(asserts.assume_rn, !!handle);
    program_counter = 1;
    state_machine_fnp = handle;
    return noop();
  }

  MJZ_CX_FN base_coroutine_handle_t jump_reletive(intptr_t rel) noexcept {
    program_counter += rel;
    return program_counter == rel ? reached_fatal() : noop();
  }
  MJZ_CX_FN base_coroutine_handle_t jump_absolute(intptr_t absl) noexcept {
    return jump_reletive(absl - program_counter);
  }
  MJZ_CX_FN base_coroutine_handle_t reached_nth_suspend(intptr_t n) noexcept {
    return jump_absolute(was_canceled() ? -n : n);
  }

  MJZ_CX_FN base_coroutine_handle_t
  reached_nth_transfer(intptr_t n, base_coroutine_handle_t next) noexcept {
    base_coroutine_handle_t p = reached_nth_suspend(n);
    if (!p)
      return p;
    return next;
  }

  MJZ_CX_FN base_coroutine_handle_t reached_retry() noexcept {
    return coro_handle();
  }
  MJZ_CX_FN base_coroutine_handle_t reached_fatal() noexcept {
    state_machine_fnp = nullptr;
    return base_coroutine_handle_t(nullptr);
  }
};

MJZ_CX_ND_FN optional_ref_t<const base_coroutine_frame_t>
base_coroutine_handle_t::frame() const noexcept {
  if (is_noop())
    return nullptr;
  return static_cast<const base_coroutine_frame_t *>(ptr);
}
MJZ_CX_ND_FN optional_ref_t<base_coroutine_frame_t>
base_coroutine_handle_t::frame() noexcept {
  return const_cast<base_coroutine_frame_t *>(std::as_const(*this).frame().ptr);
}

MJZ_CX_ND_FN base_coroutine_handle_t base_coroutine_handle_t::noop() noexcept {
  return base_coroutine_frame_t::noop();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::is_noop() const noexcept {
  return ptr && ptr->is_noop();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::can_exe() const noexcept {
  return ptr && ptr->can_exe();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::is_done() const noexcept {
  return ptr && ptr->is_done();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::was_fatal() const noexcept {
  return ptr && ptr->was_fatal();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::is_none() const noexcept {
  return !ptr || ptr->is_none();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::on_start() const noexcept {
  return ptr && ptr->on_start();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::was_canceled() const noexcept {
  return ptr && ptr->was_canceled();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::has_work() const noexcept {
  return ptr && ptr->has_work();
}
MJZ_CX_ND_FN bool base_coroutine_handle_t::can_resume() const noexcept {
  return ptr && ptr->can_resume();
}
MJZ_CX_FN success_t base_coroutine_handle_t::request_cancel() noexcept {
  return frame() && frame()->request_cancel();
}
MJZ_CX_FN success_t base_coroutine_handle_t::resume() noexcept {
  return frame() && frame()->resume();
}
MJZ_CX_FN success_t base_coroutine_handle_t::destroy() noexcept {
  return frame() && frame()->destroy();
}

MJZ_CX_FN success_t base_coroutine_handle_t::cancel() noexcept {
  return frame() && frame()->cancel();
}
MJZ_CX_FN success_t base_coroutine_handle_t::cancel_destroy() noexcept {
  return frame() && frame()->cancel_destroy();
}

MJZ_CX_FN success_t base_coroutine_handle_t::execute() noexcept {
  return frame() && frame()->execute();
}
MJZ_CX_ND_FN pair_t<base_coroutine_handle_t, success_t>
base_coroutine_handle_t::execute_stack() noexcept {
  if (!frame())
    return {noop(), true};
  return frame()->execute_stack();
}

struct base_coroutine_stack_frame_t : base_coroutine_frame_t {
  base_coroutine_handle_t caller{base_coroutine_frame_t::noop()};
};

}; // namespace ccoro_ns
}; // namespace mjz
#endif // MJZ_CCORO_LIB_HPP_FILE_