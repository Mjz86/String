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

namespace mjz {
struct mjz_assert_t {
  mjz_assert_t(auto) = delete;
  MJZ_CX_AL_FN mjz_assert_t(totally_empty_type_t) noexcept {}
  MJZ_CX_AL_FN void operator=(auto) const noexcept = delete;

  MJZ_CX_AL_FN const mjz_assert_t &operator()(
      success_t value, const char *str = "assert",
      bool consteval_only = false) const noexcept {
    MJZ_DISABLE_ALL_WANINGS_START_;
    if (consteval_only || !MJZ_IN_DEBUG_MODE) {
      MJZ_IFN_CONSTEVAL { return *this; }
    }
    MJZ_DISABLE_ALL_WANINGS_END_;
    if (!value) {
      /* if you see an error hear , go to the caller , they have more context
       * for the assert*/
      str -= uintlen_t(value) / uintlen_t(value);
      if (!consteval_only) {
        MJZ_IFN_CONSTEVAL { panic(str); }
      }
    }
    return *this;
  }

  
  MJZ_CX_AL_A_FN(MJZ_NORETURN)
  void unreachable(
      MJZ_UNUSED const char *const str = "assert") const noexcept {
    MJZ_DISABLE_ALL_WANINGS_START_;
    MJZ_UNREACHABLE();
    MJZ_DISABLE_ALL_WANINGS_END_;
  }

  MJZ_CX_AL_A_FN(MJZ_NORETURN)
  void panic(
      MJZ_UNUSED const char *const str = "assert") const noexcept {
    MJZ_DISABLE_ALL_WANINGS_START_;
    MJZ_IFN_CONSTEVAL {
#if MJZ_WITH_iostream
      std::abort();
#endif
    }
    MJZ_UNREACHABLE();
    MJZ_DISABLE_ALL_WANINGS_END_;
  }

  MJZ_CX_AL_A_FN(MJZ_NORETURN)
  void not_implemented_yet(
      MJZ_UNUSED const char *const str = "assert") const noexcept {
    panic(str);
  }

  MJZ_CX_AL_A_FN(MJZ_NORETURN)
  void not_implemented_yet_v(auto &&...) const noexcept {
    panic();
  }

  MJZ_CX_AL_FN const mjz_assert_t &ignore_v(auto &&...) const noexcept {
    return *this;
  }

  MJZ_CX_AL_FN const mjz_assert_t &not_optimal(auto &&...) const noexcept {
    return *this;
  }
  MJZ_CX_AL_FN const mjz_assert_t &as_consteval(
      success_t value) const noexcept {
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
  MJZ_CX_AL_FN const mjz_assert_t &operator[](
      mjz_asserts_e mjz_asserts_v) const noexcept {
    return (*this)(
        mjz_asserts_v, [&]() noexcept { return false; }, "assertion", false,
        false);
  }
  MJZ_CX_AL_FN const mjz_assert_t &operator|(bool B) const noexcept {
    return (*this)(B);
  }

  MJZ_CX_AL_FN const mjz_assert_t &operator()(
      mjz_asserts_e mjz_assertv_e,
      callable_c<success_t(void) noexcept> auto &&value,
      const char *str = "assert", bool consteval_only = false,
      bool can_assume = true, bool always_call = false) const noexcept {
    if (always_call) {
      return force_call(value, mjz_assertv_e, str, consteval_only, can_assume);
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
  MJZ_CX_AL_FN const mjz_assert_t &operator()(
      mjz_asserts_e mjz_assertv_e, bool value, const char *str = "assert",
      bool consteval_only = false, bool can_assume = true) const noexcept {
    if (consteval_only) {
      MJZ_IFN_CONSTEVAL { return *this; }
    }
    if (mjz_assertv_e < debug && !MJZ_IN_DEBUG_MODE) {
      if (can_assume) {
        MJZ_JUST_ASSUME_(value);
        if (!value) unreachable(str);
      }
      return *this;
    }
    if (!value) {
      panic(str);
    }
    return *this;
  }
  MJZ_CX_AL_FN const mjz_assert_t &debug_only(
      const char *str = "assert", bool consteval_only = false) const noexcept {
    return (*this)(debug, false, str, consteval_only, false);
  }
  MJZ_CX_AL_FN const mjz_assert_t &force_call(
      callable_c<success_t(void) noexcept> auto &&value,
      mjz_asserts_e mjz_assertv_e = debug, const char *str = "assert",
      bool consteval_only = false, bool can_assume = true) const noexcept {
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
MJZ_CONSTANT(mjz_assert_t) asserts{totally_empty_type};
}  // namespace mjz

#endif  // MJZ_ASSERTS_LIB_HPP_FILE_