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
#include "traits.hpp"
#ifndef MJZ_ASSERTS_LIB_HPP_FILE_
#define MJZ_ASSERTS_LIB_HPP_FILE_

MJZ_EXPORT namespace mjz {

  struct mjz_assert_t {
    mjz_assert_t(auto) = delete;
    MJZ_CX_AL_FN mjz_assert_t(totally_empty_type_t) noexcept {}
    MJZ_CX_AL_FN void operator=(auto) const noexcept = delete;

    MJZ_CX_AL_FN const mjz_assert_t &
    operator()(success_t value, const char *str = "assert",
               bool consteval_only = false) const noexcept {
      MJZ_DISABLE_ALL_WANINGS_START_;
      if (consteval_only || !MJZ_IN_DEBUG_MODE) {
        MJZ_IFN_CONSTEVAL { return *this; }
      }
      MJZ_DISABLE_ALL_WANINGS_END_;
      if (!value) {
        /* if you see an error hear , go to the caller , they have more context
         * for the assert*/
        if (!consteval_only) {
          panic(str);
        }
        unreachable(str);
      }
      return *this;
    }

    MJZ_NORETURN MJZ_NCX_AL_FN void static unreachable(
        MJZ_UNUSED const char *const str = "assert") noexcept {
      MJZ_DISABLE_ALL_WANINGS_START_;
      MJZ_UNREACHABLE();
      MJZ_DISABLE_ALL_WANINGS_END_;
    }
    MJZ_NORETURN MJZ_NCX_FN static void panic_handle_deafult() noexcept {
#if MJZ_PAINC_TRACE_ && MJZ_WITH_iostream
#ifdef __cpp_lib_stacktrace
      std::cout << std::stacktrace::current();
#endif
#endif
      std::terminate();
    }

    MJZ_CX_FN static void deafult_breakpoint_handler() noexcept {}

  private:
    struct handle_fn_ptr_t_ {
      static inline std::atomic<decltype(&panic_handle_deafult)> a{
          &panic_handle_deafult};
      static inline std::atomic<decltype(&deafult_breakpoint_handler)> d{
          MJZ_IN_DEBUG_MODE ? &deafult_breakpoint_handler : nullptr};
    };

  public:
    MJZ_CX_AL_FN static void breakpoint() noexcept {
      MJZ_IFN_CONSTEVAL {
        auto bp = handle_fn_ptr_t_::d.load(std::memory_order_acquire);
        if (bp) {
          return bp();
        }
      }
      return panic_handler();
    }
    MJZ_CX_AL_FN static void breakpoint_if_debugging() noexcept {
      MJZ_IF_CONSTEVAL { return; }
      auto bp = handle_fn_ptr_t_::d.load(std::memory_order_acquire);
      if (bp) {
        return bp();
      }
    }
    MJZ_CX_AL_FN static bool is_debugger_present() noexcept {
      MJZ_IF_CONSTEVAL { return false; }
      return !!handle_fn_ptr_t_::d.load(std::memory_order_acquire);
    }
    MJZ_NCX_AL_FN static auto &panic_handle_fn_ptr() noexcept {
      return handle_fn_ptr_t_::a;
    }
    MJZ_NORETURN static void panic_handler() noexcept {
      MJZ_DISABLE_ALL_WANINGS_START_;
      panic_handle_fn_ptr().load(std::memory_order_acquire)();
      MJZ_UNREACHABLE();
      MJZ_DISABLE_ALL_WANINGS_END_;
    }
    MJZ_NORETURN MJZ_NCX_AL_FN void
    panic(MJZ_UNUSED const char *const str = "assert") const noexcept {
      panic_handler();
    }

    MJZ_NORETURN MJZ_NCX_AL_FN void not_implemented_yet(
        MJZ_UNUSED const char *const str = "assert") const noexcept {
      panic(str);
    }

    MJZ_NORETURN MJZ_NCX_AL_FN void
    not_implemented_yet_v(auto &&...) const noexcept {
      panic();
    }

    MJZ_CX_AL_FN const mjz_assert_t &ignore_v(auto &&...) const noexcept {
      return *this;
    }

    MJZ_CX_AL_FN const mjz_assert_t &not_optimal(auto &&...) const noexcept {
      return *this;
    }
    MJZ_CX_AL_FN const mjz_assert_t &
    as_consteval(success_t value) const noexcept {
      (*this)(value, "assert", true);
      return *this;
    }
    enum mjz_asserts_e {
      condition_rn = MJZ_TRUST_LEVEL_OF_CONDITION_,
      pre_condition = condition_rn,
      post_condition = condition_rn,
      assume_rn = MJZ_TRUST_LEVEL_OF_ASSUME_,
      believe_rn = MJZ_TRUST_LEVEL_OF_BELIEVE_,
      expect_rn = MJZ_TRUST_LEVEL_OF_EXPECT_,
      pretend_rn = MJZ_TRUST_LEVEL_OF_PRETEND_,
      debug = MJZ_assert_TRUST_LEVEL_,
    };
    MJZ_CX_AL_FN const mjz_assert_t &
    operator[](mjz_asserts_e mjz_asserts_v) const noexcept {
      return (*this)(
          mjz_asserts_v, [&]() noexcept { return false; }, "assertion", false,
          false);
    }
    MJZ_CX_AL_FN const mjz_assert_t &operator|(bool B) const noexcept {
      return (*this)(B);
    }

    MJZ_CX_AL_FN const mjz_assert_t &
    operator()(mjz_asserts_e mjz_assertv_e,
               callable_c<success_t(void) noexcept> auto &&value,
               const char *str = "assert", bool consteval_only = false,
               bool can_assume = true,
               bool always_call = false) const noexcept {
      if (always_call) {
        return force_call(value, mjz_assertv_e, str, consteval_only,
                          can_assume);
      }
      if (consteval_only) {
        MJZ_IFN_CONSTEVAL { return *this; }
      }
      if (mjz_assertv_e < debug && !MJZ_IN_DEBUG_MODE) {
        if (can_assume) {
          // warning: assumption is ignored because it contains (potential)
          // side-effects [-Wassume] MJZ_JUST_ASSUME_(value());
          std::ignore = 0;
        }
        return *this;
      } else {
        (*this)(value(), str, false);
      }
      return *this;
    }
    MJZ_CX_AL_FN const mjz_assert_t &
    operator()(mjz_asserts_e mjz_assertv_e, bool value,
               const char *str = "assert", bool consteval_only = false,
               bool can_assume = true) const noexcept {
      if (consteval_only) {
        MJZ_IFN_CONSTEVAL { return *this; }
      }
      if (mjz_assertv_e < debug && !MJZ_IN_DEBUG_MODE) {
        if (can_assume) {
          MJZ_JUST_ASSUME_(value);
          if (!value)
            unreachable(str);
        }
        return *this;
      }
      if (!value) {
        panic(str);
      }
      return *this;
    }
    MJZ_CX_AL_FN const mjz_assert_t &
    debug_only(const char *str = "assert",
               bool consteval_only = false) const noexcept {
      return (*this)(debug, false, str, consteval_only, false);
    }
    MJZ_CX_AL_FN const mjz_assert_t &
    force_call(callable_c<success_t(void) noexcept> auto &&value,
               mjz_asserts_e mjz_assertv_e = debug, const char *str = "assert",
               bool consteval_only = false,
               bool can_assume = true) const noexcept {
      return (*this)(mjz_assertv_e, value(), str, consteval_only, can_assume);
    }

    MJZ_DEPRECATED_R("confusion")
    MJZ_CX_FN explicit operator bool() const noexcept = delete;
    MJZ_DEPRECATED_R("confusion")
    MJZ_CX_FN bool operator!() const noexcept = delete;
    MJZ_DEPRECATED_R("confusion")
    MJZ_CX_FN void operator&() const noexcept = delete;
  };
  static_assert(std::is_empty_v<mjz_assert_t>);
  MJZ_FCONSTANT(mjz_assert_t) asserts{totally_empty_type};
} // namespace mjz

#endif // MJZ_ASSERTS_LIB_HPP_FILE_