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

#include "user_include_context_.hpp"
#ifndef MJZ_string_lib_macros_HPP_FILE_
#define MJZ_string_lib_macros_HPP_FILE_

////////////////////////////
///////////////////
#include "choose_include_marco_.hpp"
/////////////////

#include "include_externals.hpp"

////////////////////////////

#define MJZ_string_lib_macros_ true

#if MJZ_USE_CXX_MODULES_
#define MJZ_EXPORT export
#else
#define MJZ_EXPORT
#endif

/*
 * shows if we can assume an expression in an assumbtion macro
 * 0 means no assumbtion
 */

#ifndef MJZ_TRUST_LEVEL_
#define MJZ_TRUST_LEVEL_ 3
#endif // !MJZ_TRUST_LEVEL_
/*
 * for some reason , notify does not work on esp32 , this is for that :(
 */

#ifndef MJZ_SLEEP_WITH_WAIT_
#define MJZ_SLEEP_WITH_WAIT_ 0ull
#endif // !MJZ_SLEEP_WITH_WAIT_

#ifndef MJZ_assert_TRUST_LEVEL_
#define MJZ_assert_TRUST_LEVEL_ 4
#endif // !MJZ_assert_TRUST_LEVEL_
/*
 * shows if we can assume liklyhood of an operation.
 * 0 means no assumbtion */
#ifndef MJZ_LIKELYHOD_LEVEL_
#define MJZ_LIKELYHOD_LEVEL_ 2
#endif // !MJZ_LIKELYHOD_LEVEL_
/*
 * shows the ABI version of the library
 */
#ifndef MJZ_LIB_NEWEST_VERSION_
#define MJZ_LIB_NEWEST_VERSION_ 0
#endif // !MJZ_LIB_NEWEST_VERSION_
/*
 * use the msvc debug mode.
 */

#ifndef MJZ_CONTROL_IN_DEBUG_MODE_
#define MJZ_CONTROL_IN_DEBUG_MODE_ true
#endif // !MJZ_CONTROL_IN_DEBUG_MODE_
/*
 * use the debug forcefully mode.
 */

#ifndef MJZ_TEST_MODE_
#define MJZ_TEST_MODE_ false
#endif // !MJZ_TEST_MODE_

#ifndef MJZ_STR_RC_SPERATE_CACHE_LINE_
#define MJZ_STR_RC_SPERATE_CACHE_LINE_ false
#endif // !MJZ_STR_RC_SPERATE_CACHE_LINE_
/*
 * log allocations in the allocators when using new.
 */

#ifndef MJZ_PMR_GLOBAL_ALLOCATIONS_
#define MJZ_PMR_GLOBAL_ALLOCATIONS_ false
#endif // !MJZ_PMR_GLOBAL_ALLOCATIONS_

#ifndef MJZ_cow_threashold_CACHE_LINE_N_
#define MJZ_cow_threashold_CACHE_LINE_N_ 4
#endif // !MJZ_cow_threashold_CACHE_LINE_N_
// esp32 issues for some reason :(
// note that enabling this may cause type punning
#ifndef MJZ_CALL_LIB_MEMCPY_ON_CPYBITCAST
#define MJZ_CALL_LIB_MEMCPY_ON_CPYBITCAST true
#undef MJZ_MEMCPY_ON_CPYBITCAST_PUNNED
#define MJZ_MEMCPY_ON_CPYBITCAST_PUNNED false
#else
#ifndef MJZ_MEMCPY_ON_CPYBITCAST_PUNNED
#define MJZ_MEMCPY_ON_CPYBITCAST_PUNNED false
#endif // !MJZ_MEMCPY_ON_CPYBITCAST_PUNNED
#endif // !MJZ_CALL_LIB_MEMCPY_ON_CPYBITCAST

#ifndef MJZ_SANE_MEMMOVE_IMPLS
#define MJZ_SANE_MEMMOVE_IMPLS true
#endif // !MJZ_SANE_MEMMOVE_IMPLS

#ifndef MJZ_LOG_ALLOC_ALLOCATIONS_
#define MJZ_LOG_ALLOC_ALLOCATIONS_ false
#endif // !MJZ_LOG_ALLOC_ALLOCATIONS_
#ifndef MJZ_LOG_NEW_ALLOCATIONS_
#define MJZ_LOG_NEW_ALLOCATIONS_ false
#endif // !MJZ_LOG_NEW_ALLOCATIONS_

#define MJZ_LOG_ALLOCATIONS_                                                   \
  (MJZ_LOG_NEW_ALLOCATIONS_ || MJZ_LOG_ALLOC_ALLOCATIONS_)
/*
 * dosent work in gcc :( , we shoud wait till c++26 to get
 * https://en.cppreference.com/w/cpp/language/structured_binding auto [...args]
 * = E{};
 */
#ifndef MJZ_aggregate_tuple_workaround
#define MJZ_aggregate_tuple_workaround false
#endif // !MJZ_aggregate_tuple_workaround

#ifndef MJZ_KNOWN_L1_CACHE_LINE_SIZE
#define MJZ_KNOWN_L1_CACHE_LINE_SIZE 64
#endif // !MJZ_KNOWN_L1_CACHE_LINE_SIZE

#ifndef MJZ_LOG_PRINT_FAILURE_
#define MJZ_LOG_PRINT_FAILURE_ true
#endif // !MJZ_LOG_PRINT_FAILURE_

#ifndef MJZ_CATCHES_EXCEPTIONS_
#define MJZ_CATCHES_EXCEPTIONS_ true
#endif // !MJZ_CATCHES_EXCEPTIONS_

#ifdef _CONTAINER_DEBUG_LEVEL
#define STD_DEGUG_MODE _CONTAINER_DEBUG_LEVEL
#else
#define STD_DEGUG_MODE 0
#endif // !_CONTAINER_DEBUG_LEVEL

#ifndef MJZ_IN_DEBUG_MODE
#define MJZ_IN_DEBUG_MODE                                                      \
  (MJZ_TEST_MODE_ || (MJZ_CONTROL_IN_DEBUG_MODE_ && (STD_DEGUG_MODE > 0)))
#endif // !MJZ_IN_DEBUG_MODE

#ifndef MJZ_DO_DEBUG_COUT
#define MJZ_DO_DEBUG_COUT (MJZ_IN_DEBUG_MODE || MJZ_LOG_ALLOCATIONS_)
#endif // !MJZ_DO_DEBUG_COUT

#ifndef MJZ_WITH_iostream
#define MJZ_WITH_iostream MJZ_DO_DEBUG_COUT
#endif // !MJZ_WITH_iostream

#ifndef MJZ_uintlen_t_as_64_bit
#define MJZ_uintlen_t_as_64_bit false
#endif // !MJZ_TRUST_LEVEL_

/*
 * gives curunt line number
 */
#define MJZ_LINE_() __LINE__
/*
 * gives curunt file name
 */
#define MJZ_FILE_() __FILE__
#define MJZ_UNIQUE_VAR_NAME_HELPER_0_(X, L) X##_##L##_unique_var_
#define MJZ_UNIQUE_VAR_NAME_HELPER_1_(X, L) MJZ_UNIQUE_VAR_NAME_HELPER_0_(X, L)
/*
 * gives uniqe variable name for a line and an
 */
#define MJZ_UNIQUE_VAR_NAME(X) MJZ_UNIQUE_VAR_NAME_HELPER_1_(X, __LINE__)
// nothing
#define MJZ_JUST_NOTHING()
// makes a c++ attribute
#define MJZ_AS_CPP_ATTREBUTE(X)                                                \
  MJZ_JUST_NOTHING()                                                           \
  [MJZ_JUST_NOTHING()[X] MJZ_JUST_NOTHING()] MJZ_JUST_NOTHING()
// for a variable that is not used for now
#define MJZ_MAYBE_UNUSED MJZ_AS_CPP_ATTREBUTE(maybe_unused)
// for a variable that is not used for now
#define MJZ_WILL_USE MJZ_MAYBE_UNUSED
// for a variable that is not used
#define MJZ_UNUSED MJZ_MAYBE_UNUSED
// for a function that will not give control back to the caller
#define MJZ_NORETURN MJZ_AS_CPP_ATTREBUTE(noreturn)

// for a case expression that has no break
#define MJZ_FALLTHROUGH MJZ_AS_CPP_ATTREBUTE(fallthrough)
// for a case expression that has no break
#define MJZ_NO_BRAKE MJZ_FALLTHROUGH
// for a function/object that holds value or is a reasorce
#define MJZ_NODISCRAD MJZ_AS_CPP_ATTREBUTE(nodiscard)
// for a function/object that holds value or is a reasorce
#define MJZ_NODISCRAD_FOR(R_reason_R_)                                         \
  MJZ_AS_CPP_ATTREBUTE(nodiscard(R_reason_R_))
// for a function/object that is not good to use anymore
#define MJZ_DEPRECATED MJZ_AS_CPP_ATTREBUTE(deprecated)
// for a function/object that is not good to use anymore
#define MJZ_DEPRECATED_R(R_reason_R_)                                          \
  MJZ_AS_CPP_ATTREBUTE(deprecated(R_reason_R_))
#define MJZ_CONSTEXPR constexpr
// for a constexpr function atribute
#define MJZ_USED_CXIL_FN MJZ_JUST_NOTHING() MJZ_CONSTEXPR MJZ_JUST_NOTHING()
#define MJZ_USED_NCXIL_FN MJZ_JUST_NOTHING() inline MJZ_JUST_NOTHING()
// for a typical constexpr function atribute
#define MJZ_CX_FN MJZ_MAYBE_UNUSED MJZ_USED_CXIL_FN
#define MJZ_IL_CX_FN MJZ_MAYBE_UNUSED MJZ_USED_CXIL_FN inline
#define MJZ_NCX_FN MJZ_MAYBE_UNUSED MJZ_USED_NCXIL_FN
#define MJZ_IL_NCX_FN MJZ_MAYBE_UNUSED MJZ_USED_NCXIL_FN inline
// for a typical no discard of return constexpr function atribute
#define MJZ_CX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_CXIL_FN
#define MJZ_NCX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_NCXIL_FN
#define MJZ_IL_CX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_CXIL_FN inline
#define MJZ_IL_NCX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_NCXIL_FN inline
// for a typical no discard of return constexpr function atribute
#define MJZ_CX_NDF_FN(R_reason_R_)                                             \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_) MJZ_USED_CXIL_FN
#define MJZ_NCX_NDF_FN(R_reason_R_)                                            \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_) MJZ_USED_NCXIL_FN

#define MJZ_CX_ND_ALLOC_FN                                                     \
  MJZ_CX_NDF_FN("This function allocates memory and returns a raw pointer. "   \
                "Discarding the return value will cause a memory leak.")

#define MJZ_CX_ND_RES_OBJ_FN                                                   \
  MJZ_CX_NDF_FN(                                                               \
      "This function returns whether the operation succeeded in modifying "    \
      "object state. "                                                         \
      "It is dangerous to ignore the return value.")

#define MJZ_IN_DEBUG_MODE_CODE(X)
#define MJZ_OUT_DEBUG_MODE_CODE(X)
#if MJZ_IN_DEBUG_MODE
#undef MJZ_IN_DEBUG_MODE_CODE
#define MJZ_IN_DEBUG_MODE_CODE(X) X
#else
#undef MJZ_OUT_DEBUG_MODE_CODE
#define MJZ_OUT_DEBUG_MODE_CODE(X) X
#endif
#define MJZ_CE_FN MJZ_MAYBE_UNUSED MJZ_CONSTEVAL inline
#define MJZ_CE_NC_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_CONSTEVAL inline
#define MJZ_CE_NCF_FN(R_reason_R_)                                             \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_)                              \
  MJZ_CONSTEVAL inline

#define MJZ_JUST_SUBOPTIMAL                                                    \
  MJZ_DEPRECATED_R("this is slow , use the newer function insted")
#define MJZ_SUBOPTIMAL(F_FUNCTION_NAME_F_)                                     \
  MJZ_DEPRECATED_R("this is slow , use the newer \"" #F_FUNCTION_NAME_F_       \
                   "\" function insted")
#define MJZ_VARY_SLOW MJZ_DEPRECATED_R("[info]:this is vary slow")
#if MJZ_LIKELYHOD_LEVEL_ > 3
#define MJZ_MAYBE_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_MAYBE_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_MAYBE_LIKELY
#define MJZ_MAYBE_UNLIKELY
#endif // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 2
#define MJZ_HALF_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_HALF_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_HALF_LIKELY
#define MJZ_HALF_UNLIKELY
#endif // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 1
#define MJZ_MOSTLY_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_MOSTLY_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_MOSTLY_LIKELY
#define MJZ_MOSTLY_UNLIKELY
#endif // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 0
#define MJZ_IS_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_IS_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_IS_LIKELY
#define MJZ_IS_UNLIKELY
#endif // MJZ_LIKELYHOD_
#define MJZ_FCONSTANT(_TYPE_) MJZ_WILL_USE constexpr _TYPE_ const
#define MJZ_MCONSTANT(_TYPE_) MJZ_WILL_USE static inline constexpr _TYPE_ const

#define MJZ_EXPAND_(X) X

#define MJZ_STD_is_constant_evaluated_FUNCTION_RET_                            \
  (::std::is_constant_evaluated())
#define MJZ_CONSTEVAL consteval
#define MJZ_IF_CONSTEVAL_                                                      \
  if (MJZ_STD_is_constant_evaluated_FUNCTION_RET_)                             \
  MJZ_IS_UNLIKELY
#define MJZ_IFN_CONSTEVAL_                                                     \
  if (!MJZ_STD_is_constant_evaluated_FUNCTION_RET_)                            \
  MJZ_IS_LIKELY
namespace mjz {
static constexpr const inline auto is_at_consteval_ = []() noexcept -> bool {
  MJZ_IF_CONSTEVAL_ { return true; }
  else {
    return []() noexcept {
      MJZ_IN_DEBUG_MODE_CODE(static) bool debug_helper = false;
      return debug_helper;
    }();
  }
};

struct totally_empty_type_t {};
using nullptr_t = std::nullptr_t;
MJZ_FCONSTANT(totally_empty_type_t) totally_empty_type{};
static_assert(std::is_empty_v<totally_empty_type_t>);
using void_struct_t = totally_empty_type_t;
}; // namespace mjz
#define MJZ_STD_is_constant_evaluated_FUNCTION_RET ::mjz::is_at_consteval_()

#define MJZ_IF_CONSTEVAL if (MJZ_STD_is_constant_evaluated_FUNCTION_RET)
#define MJZ_IFN_CONSTEVAL if (!MJZ_STD_is_constant_evaluated_FUNCTION_RET)

/*
 *add other compiler specific macro definitions.
 */

#define MJZ_MSVC_ONLY_PRAGMA_(X)
#define MJZ_GCC_ONLY_PRAGMA_(X)
#define MJZ_MSVC_ONLY_CODE_(X)
#define MJZ_GCC_ONLY_CODE_(X)
#define MJZ_GCC_ATTRIBUTES_(X)
#define MJZ_restrict
#if 1 < _MSC_VER
#define MJZ_JUST_FORCED_INLINE_ __forceinline
#define MJZ_JUST_NO_INLINE_ __declspec(noinline)
#define MJZ_no_unique_address MJZ_AS_CPP_ATTREBUTE(msvc::no_unique_address)
#define MJZ_JUST_ASSUME_(X_expression_) __assume(X_expression_)
#define MJZ_MACRO_PRAGMA_(WHAT_) __pragma(WHAT_)
#undef MJZ_MSVC_ONLY_PRAGMA_
#undef MJZ_MSVC_ONLY_CODE_
#undef MJZ_restrict
#define MJZ_restrict __restrict
#define MJZ_MSVC_ONLY_PRAGMA_(X) MJZ_MACRO_PRAGMA_(X)

#define MJZ_uint128_t_impl_t_                                                  \
  MJZ_MSVC_ONLY_CODE_(std::) MJZ_MSVC_ONLY_CODE_(_Unsigned128)
#define MJZ_MSVC_ONLY_CODE_(X) X
#ifndef MJZ_VERBOSE_FORMAT_ERROR
#define MJZ_VERBOSE_FORMAT_ERROR false
#endif // !MJZ_VERBOSE_FORMAT_ERROR
#define MJZ_DISABLED_MSVC_WANINGS_                                             \
  5264 26495 4180 4412 4455 4494 4514 4574 4582 4583 4587 4588 4619 4623 4625 4626 4643 4648 4702 4793 4820 4988 5026 5027 5045 6294 4710 4711 4868 4866 5246 4702 6385 26115 26110 6236 26495 6287 28020 26816 6386

#define MJZ_WARNINGS_IGNORE_BEGIN_BASE_IMPL_()                                 \
  MJZ_MSVC_ONLY_PRAGMA_(warning(push))

#define MJZ_WARNINGS_IGNORE_END_BASE_IMPL_() MJZ_MSVC_ONLY_PRAGMA_(warning(pop))

#define MJZ_WARNINGS_IGNORE_BEGIN_IMPL_                                        \
  MJZ_MSVC_ONLY_PRAGMA_(warning(push, 0))                                      \
  MJZ_MSVC_ONLY_PRAGMA_(warning(disable : MJZ_DISABLED_MSVC_WANINGS_))

#define MJZ_WARNINGS_IGNORE_END_IMPL_ MJZ_MSVC_ONLY_PRAGMA_(warning(pop));

#define CLANG_NO_GCC_CODE_IMPL(X)
#define GCC_NO_CLANG_CODE_IMPL(X)
#elif defined(__GNUC__)

#if defined(__clang__)
#define CLANG_NO_GCC_CODE_IMPL(X) X
#define GCC_NO_CLANG_CODE_IMPL(X)
#else

#define CLANG_NO_GCC_CODE_IMPL(X)
#define GCC_NO_CLANG_CODE_IMPL(X) X

#endif

#ifdef __SIZEOF_INT128__
#define MJZ_uint128_t_impl_t_ unsigned __int128
#endif
#undef MJZ_GCC_ATTRIBUTES_
#undef MJZ_GCC_ONLY_CODE_
#undef MJZ_restrict
#undef MJZ_GCC_ONLY_PRAGMA_

#define MJZ_MACRO_PRAGMA_(X) _Pragma(#X)

#define MJZ_GCC_ONLY_CODE_(X) X
#define MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_(warnoption0)                         \
  MJZ_MACRO_PRAGMA_(GCC diagnostic ignored warnoption0)

#define MJZ_WARNINGS_IGNORE_BEGIN_BASE_IMPL_()                                 \
  MJZ_MACRO_PRAGMA_(GCC diagnostic push)

#define MJZ_WARNINGS_IGNORE_END_BASE_IMPL_()                                   \
  MJZ_MACRO_PRAGMA_(GCC diagnostic pop)

#define MJZ_WARNINGS_IGNORE_BEGIN_IMPL_                                        \
  MJZ_WARNINGS_IGNORE_BEGIN_BASE_IMPL_();                                      \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wall");                                  \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wextra");                                \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Walloca");                               \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wcast-align");                           \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wcast-qual");                            \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wctor-dtor-privacy");                    \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wdeprecated-copy-dtor");                 \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wdouble-promotion");                     \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wenum-conversion");                      \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wfloat-equal");                          \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wformat-signedness");                    \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wformat=2");                             \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wmismatched-tags");                      \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wmultichar");                            \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wnon-virtual-dtor");                     \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Woverloaded-virtual");                   \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wpointer-arith");                        \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wrange-loop-construct");                 \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wshadow");                               \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wuninitialized");                        \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wvla");                                  \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wwrite-strings");                        \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wsign-conversion");                      \
  GCC_NO_CLANG_CODE_IMPL(                                                      \
      MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wmaybe-uninitialized"));             \
  MJZ_WARNINGS_IGNORE_BEGIN_IMPL00_("-Wdelete-non-"                            \
                                    "virtual-dtor")

#define MJZ_WARNINGS_IGNORE_END_IMPL_ MJZ_WARNINGS_IGNORE_END_BASE_IMPL_()

#define MJZ_restrict __restrict__
#define MJZ_GCC_ATTRIBUTES_(X) __attribute__((X))
#define MJZ_JUST_FORCED_INLINE_ MJZ_GCC_ATTRIBUTES_(always_inline) inline
#define MJZ_JUST_NO_INLINE_ MJZ_GCC_ATTRIBUTES_(noinline)
#if __has_cpp_attribute(assume)
#define MJZ_JUST_ASSUME_(X_expression_)                                        \
  MJZ_AS_CPP_ATTREBUTE(assume(X_expression_))
#else
#define MJZ_JUST_ASSUME_(X_expression_) []() noexcept {}()
#endif
#define MJZ_no_unique_address MJZ_AS_CPP_ATTREBUTE(no_unique_address)
#define MJZ_GCC_ONLY_PRAGMA_(X) MJZ_MACRO_PRAGMA_(X)
#else
#define MJZ_MACRO_PRAGMA_(WHAT_)                                               \
  static_assert(false, " relplace with pragma macro ")
#define MJZ_JUST_ASSUME_(X_expression_)                                        \
  MJZ_AS_CPP_ATTREBUTE(assume(X_expression_))
#define MJZ_no_unique_address MJZ_AS_CPP_ATTREBUTE(no_unique_address)
#define MJZ_JUST_FORCED_INLINE_
#define MJZ_JUST_NO_INLINE_
#endif

#ifdef __cpp_lib_trivially_relocatable
#define MJZ_trivially_relocatable [[trivially_relocatable]]
#define MJZ_maybe_trivially_relocatable [[maybe_trivially_relocatable]]
#else
#define MJZ_maybe_trivially_relocatable
#define MJZ_trivially_relocatable
#endif
#define MJZ_TRUST_LEVEL_OF_ASSUME_ 0
#define MJZ_TRUST_LEVEL_OF_BELIEVE_ 1
#define MJZ_TRUST_LEVEL_OF_EXPECT_ 2
#define MJZ_TRUST_LEVEL_OF_CONDITION_ 3
#define MJZ_TRUST_LEVEL_OF_PRETEND_ 4
#if MJZ_TRUST_LEVEL_OF_ASSUME_ < MJZ_TRUST_LEVEL_
#define MJZ_ASSUME_(X_expression_) MJZ_JUST_ASSUME_(X_expression_)
#else
#define MJZ_ASSUME_(X_expression_)
#endif

#if MJZ_TRUST_LEVEL_OF_BELIEVE_ < MJZ_TRUST_LEVEL_
#define MJZ_FORCED_INLINE MJZ_JUST_FORCED_INLINE_
#define MJZ_NO_INLINE MJZ_JUST_NO_INLINE_
#define MJZ_BELIEVE_(X_expression_) MJZ_JUST_ASSUME_(X_expression_)
#else
#define MJZ_BELIEVE_(X_expression_)
#define MJZ_FORCED_INLINE inline
#define MJZ_NO_INLINE
#endif
#if MJZ_TRUST_LEVEL_OF_EXPECT_ < MJZ_TRUST_LEVEL_
#define MJZ_EXPECT_(X_expression_) MJZ_JUST_ASSUME_(X_expression_)
#else
#define MJZ_EXPECT_(X_expression_)
#endif
#if MJZ_TRUST_LEVEL_OF_CONDITION_ < MJZ_TRUST_LEVEL_
#define MJZ_CONDITION_(X_expression_) MJZ_JUST_ASSUME_(X_expression_)
#else
#define MJZ_CONDITION_(X_expression_)
#endif
#if MJZ_TRUST_LEVEL_OF_PRETEND_ < MJZ_TRUST_LEVEL_
#define MJZ_PRETEND_(X_expression_) MJZ_JUST_ASSUME_(X_expression_)
#else
#define MJZ_PRETEND_(X_expression_)
#endif
/*
this is purely a documentation macro.
THIS DOSE NOT GENERATE ANY CODE !!
INVISIBLE FOR THE COMPILER !!


used for understanding the consequenses of a valid pointer, true boolean flag
, template specilization, value of a function argument ,active enum member,
active union member...
*/
#define MJZ_ALWAYS_BELIEVE(X_expression_)

/*
this is purely a documentation macro.
THIS DOSE NOT GENERATE ANY CODE !!
INVISIBLE FOR THE COMPILER !!


used for understanding the consequenses of a valid pointer, true boolean flag
, template specilization, value of a function argument ,active enum member,
active union member... IF the first statement is true

*/
#define MJZ_ALWAYS_BELIEVE_IF(X_IF_STATEMENT_expression_, X_expression_)

#define MJZ_COMMENT(X_expression_)

#define MJZ_ASSIGN_IN_IF(DEST_D_, SRC_S_) (bool(DEST_D_ = SRC_S_))

#define MJZ_NO_INIT

#define MJZ_PRECONDITION_(X_expression_) MJZ_CONDITION_(X_expression_)
#define MJZ_POSTCONDITION_(X_expression_) MJZ_CONDITION_(X_expression_)
#define MJZ_RJUST_ASSUME_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)    \
  MJZ_JUST_ASSUME_(ASSUMBTION_EXPRESION_)
#define MJZ_RASSUME_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)         \
  MJZ_ASSUME_(ASSUMBTION_EXPRESION_)
#define MJZ_RBELIEVE_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)        \
  MJZ_BELIEVE_(ASSUMBTION_EXPRESION_)
#define MJZ_REXPECT_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)         \
  MJZ_EXPECT_(ASSUMBTION_EXPRESION_)
#define MJZ_RPRETEND_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)        \
  MJZ_PRETEND_(ASSUMBTION_EXPRESION_)
#define MJZ_RCONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)      \
  MJZ_CONDITION_(ASSUMBTION_EXPRESION_)
#define MJZ_RPOSTCONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)  \
  MJZ_POSTCONDITION_(ASSUMBTION_EXPRESION_)
#define MJZ_RPRECONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion)   \
  MJZ_PRECONDITION_(ASSUMBTION_EXPRESION_)

#define MJZ_NO_MV(CLASS_NAME)                                                  \
  CLASS_NAME(CLASS_NAME &&) = delete;                                          \
  CLASS_NAME &operator=(CLASS_NAME &&) = delete;
#define MJZ_NO_CPY(CLASS_NAME)                                                 \
  CLASS_NAME(const CLASS_NAME &) = delete;                                     \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;

#define MJZ_NO_MV_NO_CPY(CLASS_NAME)                                           \
  CLASS_NAME(CLASS_NAME &&) = delete;                                          \
  CLASS_NAME(const CLASS_NAME &) = delete;                                     \
  CLASS_NAME &operator=(CLASS_NAME &&) = delete;                               \
  CLASS_NAME(CLASS_NAME &) = delete;                                           \
  CLASS_NAME &operator=(CLASS_NAME &) = delete;                                \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;

#define MJZ_DEFAULTED_CLASS(CLASS_NAME)                                        \
  MJZ_CX_FN CLASS_NAME() noexcept = default;                                   \
  MJZ_CX_FN ~CLASS_NAME() noexcept = default;                                  \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &) noexcept = default;                       \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &&) noexcept = default;                      \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept = default;                 \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &) noexcept = default;            \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &&) noexcept = default;           \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept = default;

#define MJZ_ASSIGN_LIKE_CONSTRUCT(CLASS_NAME)                                  \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &&src_0_) noexcept {              \
    if (this == &src_0_)                                                       \
      return *this;                                                            \
    std::destroy_at(this);                                                     \
    std::construct_at(this, std::move(src_0_));                                \
    return *this;                                                              \
  }                                                                            \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &&src_0_) noexcept {        \
    if (this == &src_0_)                                                       \
      return *this;                                                            \
    std::destroy_at(this);                                                     \
    std::construct_at(this, (const CLASS_NAME &&)(src_0_));                    \
    return *this;                                                              \
  }                                                                            \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &src_0_) noexcept {               \
    if (this == &src_0_)                                                       \
      return *this;                                                            \
    std::destroy_at(this);                                                     \
    std::construct_at(this, src_0_);                                           \
    return *this;                                                              \
  }                                                                            \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &src_0_) noexcept {         \
    if (this == &src_0_)                                                       \
      return *this;                                                            \
    std::destroy_at(this);                                                     \
    std::construct_at(this, src_0_);                                           \
    return *this;                                                              \
  }

#define MJZ_UNSAFE_UNION(CLASS_NAME)                                           \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept {}                         \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept {               \
    return *this;                                                              \
  }                                                                            \
  MJZ_CX_FN CLASS_NAME() noexcept {}                                           \
  MJZ_CX_FN ~CLASS_NAME() noexcept {}

#define MJZ_STATIC_CLASS(CLASS_NAME)                                           \
  MJZ_CX_FN CLASS_NAME() noexcept = delete;                                    \
  MJZ_CX_FN ~CLASS_NAME() noexcept = delete;                                   \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &) noexcept = delete;                        \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &&) noexcept = delete;                       \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept = delete;                  \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &) noexcept = delete;             \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &&) noexcept = delete;            \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept = delete;

#define MJZ_NO_MV_NO_CPY_DC(CLASS_NAME)                                        \
  MJZ_CX_FN CLASS_NAME() = default;                                            \
  MJZ_NO_MV_NO_CPY(CLASS_NAME);

#define MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME)              \
  mjz_type_of_##VAR_NAME##_t

#define MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, CODE_C_)      \
  UNION_OR_STRUCT_UOS_ MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_,         \
                                                 VAR_NAME) CODE_C_;            \
  MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME) VAR_NAME

#define MJZ_NO_MV_NO_CPY_var_of_nameless_type_CNB(UNION_OR_STRUCT_UOS_,        \
                                                  VAR_NAME, CODE_C_NB_)        \
  MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, {                   \
    CODE_C_NB_;                                                                \
    MJZ_NO_MV_NO_CPY(                                                          \
        MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME));            \
  })

#define MJZ_NO_MV_NO_CPY_var_of_nameless_type_CNB_DC(UNION_OR_STRUCT_UOS_,     \
                                                     VAR_NAME, CODE_C_NB_)     \
  MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, {                   \
    CODE_C_NB_;                                                                \
    MJZ_NO_MV_NO_CPY_DC(                                                       \
        MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME));            \
  })

#define MJZ_PACKING_START_                                                     \
  MJZ_MACRO_PRAGMA_(pack(push))                                                \
  MJZ_MACRO_PRAGMA_(pack(1))

#define MJZ_PACKING_END_ MJZ_MACRO_PRAGMA_(pack(pop))

#define MJZ_DISABLE_ALL_WANINGS_START_ MJZ_WARNINGS_IGNORE_BEGIN_IMPL_

#define MJZ_DISABLE_ALL_WANINGS_END_ MJZ_WARNINGS_IGNORE_END_IMPL_

#define MJZ_NUMBEROF(Array_) (sizeof(Array_) / sizeof(Array_[0]))

#define MJZ_CX_AL_FN                                                           \
  MJZ_MAYBE_UNUSED                                                             \
  MJZ_GCC_ONLY_CODE_(__attribute__((always_inline)))                           \
  MJZ_CONSTEXPR MJZ_MSVC_ONLY_CODE_(__forceinline)

#define MJZ_NCX_AL_FN                                                          \
  MJZ_MAYBE_UNUSED                                                             \
  MJZ_GCC_ONLY_CODE_(__attribute__((always_inline)))                           \
  inline MJZ_MSVC_ONLY_CODE_(__forceinline)

#define MJZ_CX_NL_FN                                                           \
  MJZ_MAYBE_UNUSED                                                             \
  MJZ_GCC_ONLY_CODE_(__attribute__((noinline)))                                \
  MJZ_CONSTEXPR MJZ_MSVC_ONLY_CODE_(__declspec(noinline))

#define MJZ_CX_AL_A_FN(X) X MJZ_CX_AL_FN

#define MJZ_CX_AL_ND_FN MJZ_CX_AL_A_FN(MJZ_NODISCRAD)
#define MJZ_CX_AL_NDR_FN(REASON) MJZ_CX_AL_A_FN(MJZ_NODISCRAD_FOR(REASON))

#define MJZ_JUST_UNREACHABLE_()                                                \
  MJZ_GCC_ONLY_CODE_(__builtin_unreachable())                                  \
  MJZ_MSVC_ONLY_CODE_(MJZ_JUST_ASSUME_(false))

#define MJZ_UNREACHABLE() MJZ_JUST_UNREACHABLE_()

#define MJZ_NEVER_RUN_PATH() MJZ_UNREACHABLE()

#define MJZ_TO_STRING(X) #X

#define MJZ_TO_STRING_V(X) MJZ_TO_STRING(X)

//-V:MJZ_NOT_CONSTEXPR: 779 , 1080 , 571 , 2535 , 3522
#define MJZ_NOT_CONSTEXPR()                                                    \
  ;                                                                            \
  MJZ_IF_CONSTEVAL {                                                           \
    MJZ_MSVC_ONLY_PRAGMA_(warning(push));                                      \
    MJZ_MSVC_ONLY_PRAGMA_(warning(disable : 4702));                            \
    ::MJZ_NORETURN_SPECIAL_namespace_::STATUS_ACSESS_VOILATION_bye_mjz_();     \
    MJZ_ASSUME_(false);                                                        \
    MJZ_MSVC_ONLY_PRAGMA_(warning(pop));                                       \
  }

#define MJZ_NO_DYNAMIC_ALLOCATOR(T_TYPE_T_)                                    \
  void operator delete(void *ptr) noexcept = delete;                           \
  void operator delete[](void *ptr) noexcept = delete;                         \
  void operator delete(void *ptr, std::align_val_t al) noexcept = delete;      \
  void operator delete[](void *ptr, std::align_val_t al) noexcept = delete;    \
  void operator delete(void *ptr, std::size_t sz) = delete;                    \
  void operator delete[](void *ptr, std::size_t sz) noexcept = delete;         \
  void operator delete(void *ptr, std::size_t sz,                              \
                       std::align_val_t al) noexcept = delete;                 \
  void operator delete[](void *ptr, std::size_t sz, std::align_val_t al) =     \
      delete;                                                                  \
  void operator delete(void *ptr, const std::nothrow_t &tag) noexcept =        \
      delete;                                                                  \
  void operator delete[](void *ptr, const std::nothrow_t &tag) noexcept =      \
      delete;                                                                  \
  void operator delete(void *ptr, std::align_val_t al,                         \
                       const std::nothrow_t &tag) noexcept = delete;           \
  void operator delete[](void *ptr, std::align_val_t al,                       \
                         const std::nothrow_t &tag) noexcept = delete;         \
  void operator delete(void *ptr, void *place) noexcept = delete;              \
  void operator delete[](void *ptr, void *place) noexcept = delete;            \
  void operator delete(T_TYPE_T_ *ptr, std::destroying_delete_t) = delete;     \
  void operator delete(T_TYPE_T_ *ptr, std::destroying_delete_t,               \
                       std::align_val_t al) = delete;                          \
  void operator delete(T_TYPE_T_ *ptr, std::destroying_delete_t,               \
                       std::size_t sz) = delete;                               \
  void operator delete(T_TYPE_T_ *ptr, std::destroying_delete_t,               \
                       std::size_t sz, std::align_val_t al) = delete;          \
  void *operator new(std::size_t count) = delete;                              \
  void *operator new[](std::size_t count) = delete;                            \
  void *operator new(std::size_t count, std::align_val_t al) = delete;         \
  void *operator new[](std::size_t count, std::align_val_t al) = delete;       \
  void *operator new(std::size_t count, const std::nothrow_t &tag) = delete;   \
  void *operator new[](std::size_t count, const std::nothrow_t &tag) = delete; \
  void *operator new(std::size_t count, std::align_val_t al,                   \
                     const std::nothrow_t &) = delete;                         \
  void *operator new[](std::size_t count, std::align_val_t al,                 \
                       const std::nothrow_t &) = delete;                       \
  void *operator new(std::size_t count, void *ptr) = delete;                   \
  void *operator new[](std::size_t count, void *ptr) = delete
#define MJZ_MSASSERT static_assert
#define MJZ_SASSERT(X) static_assert(X, "")
#define MJZ_BAD_COMPILER(EXP)                                                  \
  MJZ_MSASSERT(                                                                \
      EXP, "[unexpected standard violation] bad compiler macro definition")

#define MJZ_BAD_MCOMPILER(EXP, AND_MASSAGE)                                    \
  MJZ_MSASSERT(EXP, "[unexpected standard violation] bad compiler macro "      \
                    "definition ; " AND_MASSAGE)

#ifndef MJZ_VERBOSE_FORMAT_ERROR
#define MJZ_VERBOSE_FORMAT_ERROR true
#endif // !MJZ_VERBOSE_FORMAT_ERROR

namespace mjz {
typedef bool success_t;

template <std::same_as<void> = void> struct releaser_helper_t {
private:
  template <class releaser_LAMBDA_t>
    requires requires(releaser_LAMBDA_t l) {
      releaser_LAMBDA_t(std::move(l));
      l();
    }
  class releaser_0_t_ {
  public:
    MJZ_NO_MV_NO_CPY(releaser_0_t_);
    releaser_0_t_() = delete;
    MJZ_CX_AL_FN
    releaser_0_t_(releaser_LAMBDA_t &&releaser_lambda) noexcept(requires() {
      { releaser_LAMBDA_t(std::move(releaser_lambda)) } noexcept;
    }) : m_releaser_lambda_(std::move(releaser_lambda)) {}
    MJZ_CX_AL_FN ~releaser_0_t_() { m_releaser_lambda_(); }

    //

  private:
    releaser_LAMBDA_t m_releaser_lambda_;
    MJZ_NO_DYNAMIC_ALLOCATOR(releaser_0_t_);
  };

public:
  MJZ_CE_FN releaser_helper_t() noexcept {}
  template <typename T>
  MJZ_CX_AL_NDR_FN("this should be stored for it to be called at end of scope")
  auto operator->*(T &&fn) const
      noexcept(noexcept(releaser_0_t_{std::forward<T>(fn)}))
          -> decltype(releaser_0_t_{std::forward<T>(fn)}) {
    return releaser_0_t_{std::forward<T>(fn)};
  };
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN explicit operator bool() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN bool operator!() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN void operator&() const noexcept = delete;
};
static_assert(std::is_empty_v<releaser_helper_t<>>);

template <class Lmabda_t, bool no_exeptions = false>
MJZ_CX_AL_FN success_t run_and_block_exeptions(Lmabda_t &&code) noexcept {
  if constexpr (requires(Lmabda_t &&code_) {
                  { std::forward<Lmabda_t>(code_)() } noexcept;
                }) {
    std::forward<Lmabda_t>(code)();
    return true;
  } else if constexpr (no_exeptions) {
    std::forward<Lmabda_t>(code)();
    return true;
  } else {
    MJZ_DISABLE_ALL_WANINGS_START_;
#if MJZ_CATCHES_EXCEPTIONS_
    try {
      std::forward<Lmabda_t>(code)();
    } catch (...) {
      return false;
    }
#else
    std::forward<Lmabda_t>(code)();
#endif
    MJZ_DISABLE_ALL_WANINGS_END_;

    return true;
  }
}
template <std::same_as<void> = void> struct noexcept_er_helper_t {
public:
  MJZ_CE_FN noexcept_er_helper_t() noexcept {}
  template <class Lmabda_t>
  MJZ_CX_FN success_t operator->*(Lmabda_t &&fn) const noexcept {
    return run_and_block_exeptions(std::forward<Lmabda_t>(fn));
  };

  template <class Lmabda_t>
  MJZ_CX_FN success_t operator*(Lmabda_t &&fn) const noexcept {
    success_t ret{};
    run_and_block_exeptions(
        [&]() noexcept(noexcept(success_t(std::forward<Lmabda_t>(fn)()))) {
          ret = success_t(std::forward<Lmabda_t>(fn)());
        });
    return ret;
  };
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN explicit operator bool() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN bool operator!() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN void operator&() const noexcept = delete;
};
static_assert(std::is_empty_v<noexcept_er_helper_t<>>);

/*
this is a temporary RAII object for being used for something like this:
void f(){
std::mutex m;
m.lock();
MJZ_W_RELEASE(m){m.unlock();};
...use...m...
}
void g(){
 auto p= new int;
  MJZ_W_RELEASE(p) { delete p;};
...use...p...
}
*/

#define MJZ_NOEXCEPT ::mjz::noexcept_er_helper_t<>{}->*[&]() mutable -> void

/*
this is a temporary RAII object for being used for something like this:
m.lock();
MJZ_RELEASE {m.unlock();};
p= new int;
MJZ_RELEASE  { delete p; p=nullptr;};
...use...m...
...use...p...
*/
#define MJZ_RELEASE                                                            \
  MJZ_UNUSED const auto &&MJZ_UNIQUE_VAR_NAME(releaserr_on_line) =             \
      ::mjz::releaser_helper_t<>{}->*[&]() mutable noexcept -> void

template <size_t align_v> MJZ_CX_FN auto *assume_aligned(auto *ptr) noexcept {
  /*/usr/lib/llvm-19/bin/../include/c++/v1/__memory/assume_aligned.h:30:36:
  note: alignment of the base pointee object (1 byte) is less than the asserted
  8 bytes 30 |     (void)__builtin_assume_aligned(__ptr, _Np); | ^
  /usr/lib/llvm-19/bin/../include/c++/v1/__memory/assume_aligned.h:43:10: note:
  in call to '__assume_aligned<8UL, char>(&{*new char[64]#1}[0])'

    note that assume_aligned<8UL, char>(&{*new char[64]#1}[0]) failed!  even tho
  its at offset 0 of new char[N] !


    */
  MJZ_IF_CONSTEVAL_ { return ptr; }
  return ::std::assume_aligned<align_v>(ptr);
}

#define MJZ_BAD_DEBUG_0_                                                       \
  MJZ_IFN_CONSTEVAL {                                                          \
    std::cout << MJZ_EXPAND_(                                                  \
                                                                               \
        __FILE__ MJZ_TO_STRING_V( : MJZ_LINE_())) "hereeeeeeeeeeeeeeee!\n";    \
  }

//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESV_GET(PTR_, alignof_Ptr_)                            \
  (::mjz::assume_aligned<alignof_Ptr_>(PTR_))

//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN_GET(PTR_, alignof_Ptr_)                          \
  ([](auto *ptr_just_ptr_) noexcept {                                          \
    MJZ_ASSUME_(ptr_just_ptr_ != nullptr);                                     \
    return ::mjz::assume_aligned<alignof_Ptr_>(ptr_just_ptr_);                 \
  }(PTR_))
//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN_GET_C(PTR_, alignof_Ptr_)                        \
  ([&](auto *ptr_just_ptr_) noexcept {                                         \
    MJZ_ASSUME_(ptr_just_ptr_ != nullptr);                                     \
    return ptr_just_ptr_; /* no dynamic assume was provided*/                  \
  }(PTR_))
//-V:MJZ_ASSUME_ALIGNES_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNES_GET(PTR_, TYPE_)                                    \
  MJZ_ASSUME_ALIGNESV_GET(PTR_, alignof(TYPE_))
//-V:MJZ_ASSUME_ALIGNES_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESNN_GET(PTR_, TYPE_)                                  \
  MJZ_ASSUME_ALIGNESVNN_GET(PTR_, alignof(TYPE_))

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESV(PTR_, TYPE_A_)                                     \
  MJZ_ASSUME_((PTR_) == ::mjz::assume_aligned<TYPE_A_>(PTR_))
//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNES(PTR_, TYPE_)                                        \
  MJZ_ASSUME_ALIGNESV(PTR_, alignof(TYPE_))

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN(PTR_, TYPE_A_)                                   \
  MJZ_ASSUME_((PTR_) == ::mjz::assume_aligned<TYPE_A_>(PTR_) &&                \
              (PTR_) != nullptr)

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESNN(PTR_, TYPE_)                                      \
  MJZ_ASSUME_ALIGNESVNN(PTR_, alignof(TYPE_))

#define MJZ_UNSAFE_CODE_BLOCK(BLOCK_) BLOCK_

#define MJZ_ASSUME_IF_THEN(CONDITION_STATEMENT_, THEN_STATEMENT_)              \
  MJZ_ASSUME_(!static_cast<bool>(CONDITION_STATEMENT_) ||                      \
              static_cast<bool>(THEN_STATEMENT_))

}; // namespace mjz

#endif // !MJZ_string_lib_macros_HPP_FILE_
