

#ifndef mjz_string_lib_HPP_FILE_
#define mjz_string_lib_HPP_FILE_

/*
 * shows if we can assume an expression in an assumbtion macro
 *  0 means no assumbtion
 */

#ifndef MJZ_TRUST_LEVEL_
#define MJZ_TRUST_LEVEL_ 3
#endif  // !MJZ_TRUST_LEVEL_
/*
 * shows if we can assume an expression in an assert
 *  0 means no assumbtion
 */

#ifndef MJZ_SPIN_WITH_WAIT_
#define MJZ_SPIN_WITH_WAIT_ false
#endif  // !MJZ_SPIN_WITH_WAIT_

#ifndef MJZ_assert_TRUST_LEVEL_
#define MJZ_assert_TRUST_LEVEL_ 4
#endif  // !MJZ_assert_TRUST_LEVEL_
/*
 * shows if we can assume liklyhood of an operation.
 *  0 means no assumbtion */
#ifndef MJZ_LIKELYHOD_LEVEL_
#define MJZ_LIKELYHOD_LEVEL_ 2
#endif  // !MJZ_LIKELYHOD_LEVEL_
/*
 * shows the ABI version of the library
 */
#ifndef MJZ_LIB_NEWEST_VERSION_
#define MJZ_LIB_NEWEST_VERSION_ 0
#endif  // !MJZ_LIB_NEWEST_VERSION_
/*
 * use the msvc debug mode.
 */

#ifndef MJZ_CONTROL_IN_DEBUG_MODE_
#define MJZ_CONTROL_IN_DEBUG_MODE_ true
#endif  // !MJZ_CONTROL_IN_DEBUG_MODE_
/*
 * use the debug forcefully mode.
 */

#ifndef MJZ_TEST_MODE_
#define MJZ_TEST_MODE_ false
#endif  // !MJZ_TEST_MODE_
/*
 *  log allocations in the allocators when using new.
 */

#ifndef MJZ_LOG_NEW_ALLOCATIONS_
#define MJZ_LOG_NEW_ALLOCATIONS_ false
#endif  // !MJZ_LOG_NEW_ALLOCATIONS_
/*
 * dosent work in gcc :( , we shoud wait till c++26 to get
 * https://en.cppreference.com/w/cpp/language/structured_binding auto [...args]
 * = E{};
 */
#ifndef MJZ_aggregate_tuple_workaround
#define MJZ_aggregate_tuple_workaround false
#endif  // !MJZ_aggregate_tuple_workaround

#ifndef MJZ_KNOWN_L1_CACHE_LINE_SIZE
#define MJZ_KNOWN_L1_CACHE_LINE_SIZE 64
#endif  // !MJZ_KNOWN_L1_CACHE_LINE_SIZE

#ifndef MJZ_LOG_PRINT_FAILURE_
#define MJZ_LOG_PRINT_FAILURE_ true
#endif  // !MJZ_LOG_PRINT_FAILURE_

#ifndef MJZ_CATCHES_EXCEPTIONS_
#define MJZ_CATCHES_EXCEPTIONS_ true
#endif  // !MJZ_CATCHES_EXCEPTIONS_

#include <memory>
#include <type_traits>
#include <utility>

#ifndef _CONTAINER_DEBUG_LEVEL
#define _CONTAINER_DEBUG_LEVEL 0
#endif  // !_CONTAINER_DEBUG_LEVEL

#define MJZ_IN_DEBUG_MODE \
  (MJZ_TEST_MODE_ ||      \
   (MJZ_CONTROL_IN_DEBUG_MODE_ && (_CONTAINER_DEBUG_LEVEL > 0)))

#ifndef MJZ_WITH_iostream
#define MJZ_WITH_iostream MJZ_IN_DEBUG_MODE
#endif  // !MJZ_WITH_iostream

#ifndef MJZ_uintlen_t_as_64_bit
#define MJZ_uintlen_t_as_64_bit false
#endif  // !MJZ_TRUST_LEVEL_

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
#define MJZ_AS_CPP_ATTREBUTE(X) \
  MJZ_JUST_NOTHING()            \
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
#define MJZ_NODISCRAD_FOR(R_reason_R_) \
  MJZ_AS_CPP_ATTREBUTE(nodiscard(R_reason_R_))
// for a function/object that is not good to use anymore
#define MJZ_DEPRECATED MJZ_AS_CPP_ATTREBUTE(deprecated)
// for a function/object that is not good to use anymore
#define MJZ_DEPRECATED_R(R_reason_R_) \
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
// for a typical no discard of return  constexpr function atribute
#define MJZ_CX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_CXIL_FN
#define MJZ_NCX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_NCXIL_FN
#define MJZ_IL_CX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_CXIL_FN inline
#define MJZ_IL_NCX_ND_FN MJZ_MAYBE_UNUSED MJZ_NODISCRAD MJZ_USED_NCXIL_FN inline
// for a typical no discard of return  constexpr function atribute
#define MJZ_CX_NDF_FN(R_reason_R_) \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_) MJZ_USED_CXIL_FN
#define MJZ_NCX_NDF_FN(R_reason_R_) \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_) MJZ_USED_NCXIL_FN

#define MJZ_CX_ND_ALLOC_FN                                         \
  MJZ_CX_NDF_FN(                                                   \
      "This function allocates memory and returns a raw pointer. " \
      "Discarding the return value will cause a memory leak.")

#define MJZ_CX_ND_RES_OBJ_FN                                                \
  MJZ_CX_NDF_FN(                                                            \
      "This function returns whether the operation succeeded in modifying " \
      "object state. "                                                      \
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
#define MJZ_CE_NCF_FN(R_reason_R_)                \
  MJZ_MAYBE_UNUSED MJZ_NODISCRAD_FOR(R_reason_R_) \
  MJZ_CONSTEVAL inline

#define MJZ_JUST_SUBOPTIMAL \
  MJZ_DEPRECATED_R("this is slow , use the newer function insted")
#define MJZ_SUBOPTIMAL(F_FUNCTION_NAME_F_)                                \
  MJZ_DEPRECATED_R("this is slow , use the newer  \"" #F_FUNCTION_NAME_F_ \
                   "\"  function  insted")
#define MJZ_VARY_SLOW MJZ_DEPRECATED_R("[info]:this is vary slow")
#if MJZ_LIKELYHOD_LEVEL_ > 3
#define MJZ_MAYBE_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_MAYBE_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_MAYBE_LIKELY
#define MJZ_MAYBE_UNLIKELY
#endif  // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 2
#define MJZ_HALF_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_HALF_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_HALF_LIKELY
#define MJZ_HALF_UNLIKELY
#endif  // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 1
#define MJZ_MOSTLY_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_MOSTLY_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_MOSTLY_LIKELY
#define MJZ_MOSTLY_UNLIKELY
#endif  // MJZ_LIKELYHOD_
#if MJZ_LIKELYHOD_LEVEL_ > 0
#define MJZ_IS_LIKELY MJZ_AS_CPP_ATTREBUTE(likely)
#define MJZ_IS_UNLIKELY MJZ_AS_CPP_ATTREBUTE(unlikely)
#else
#define MJZ_IS_LIKELY
#define MJZ_IS_UNLIKELY
#endif  // MJZ_LIKELYHOD_
#define MJZ_CONSTANT(_TYPE_) MJZ_WILL_USE constexpr static _TYPE_ const
#define MJZ_FCONSTANT(_TYPE_) MJZ_WILL_USE constexpr _TYPE_ const
#define MJZ_AS_CONSTEXPR(result_, CODE) \
  MJZ_CONSTANT(auto) result_ { [&]() mutable CODE() }

#define MJZ_STD_is_constant_evaluated_FUNCTION_RET_ \
  (::std::is_constant_evaluated())
#define MJZ_CONSTEVAL consteval
#define MJZ_IF_CONSTEVAL_ \
  if (MJZ_STD_is_constant_evaluated_FUNCTION_RET_) MJZ_IS_UNLIKELY
#define MJZ_IFN_CONSTEVAL_ \
  if (!MJZ_STD_is_constant_evaluated_FUNCTION_RET_) MJZ_IS_LIKELY

#define MJZ_STD_is_constant_evaluated_FUNCTION_RET \
  []() noexcept -> bool {                          \
    MJZ_IF_CONSTEVAL_ { return true; }             \
    else {                                         \
      return false;                                \
    }                                              \
  }()

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
#define JUST_MJZ_FORCED_INLINE_ __forceinline
#define JUST_MJZ_NO_INLINE_ __declspec(noinline)
#define MJZ_no_unique_address                   \
  MJZ_AS_CPP_ATTREBUTE(msvc::no_unique_address) \
  MJZ_AS_CPP_ATTREBUTE(no_unique_address)
#define MJZ_JUST_ASSUME_(X_expression_) __assume(X_expression_)
#define MJZ_MACRO_PRAGMA_(WHAT_) __pragma(WHAT_)
#undef MJZ_MSVC_ONLY_PRAGMA_
#undef MJZ_MSVC_ONLY_CODE_
#undef MJZ_restrict
#define MJZ_restrict __restrict
#define MJZ_MSVC_ONLY_PRAGMA_(X) MJZ_MACRO_PRAGMA_(X)
#define MJZ_MSVC_ONLY_CODE_(X) X
#ifndef MJZ_VERBOSE_FORMAT_ERROR
#define MJZ_VERBOSE_FORMAT_ERROR false
#endif  // !MJZ_VERBOSE_FORMAT_ERROR
#elif defined(__GNUC__)
#undef MJZ_GCC_ATTRIBUTES_
#undef MJZ_GCC_ONLY_CODE_
#undef MJZ_restrict
#undef MJZ_GCC_ONLY_PRAGMA_
#define MJZ_restrict __restrict__
#define MJZ_GCC_ONLY_CODE_(X) X
#define MJZ_GCC_ATTRIBUTES_(X) __attribute__((X))
#define JUST_MJZ_FORCED_INLINE_ MJZ_GCC_ATTRIBUTES_(always_inline)
#define JUST_MJZ_NO_INLINE_ MJZ_GCC_ATTRIBUTES_(noinline) noinline
#define MJZ_MACRO_PRAGMA_(X) _Pragma(#X)
#if __has_cpp_attribute(assume)
#define MJZ_JUST_ASSUME_(X_expression_) \
  MJZ_AS_CPP_ATTREBUTE(assume(X_expression_))
#else
#define MJZ_JUST_ASSUME_(X_expression_) []() noexcept {}()
#endif
#define MJZ_no_unique_address MJZ_AS_CPP_ATTREBUTE(no_unique_address)
#define MJZ_GCC_ONLY_PRAGMA_(X) MJZ_MACRO_PRAGMA_(X)
#else
#define MJZ_MACRO_PRAGMA_(WHAT_) \
  static_assert(false, " relplace with pragma macro ")
#define MJZ_JUST_ASSUME_(X_expression_) \
  MJZ_AS_CPP_ATTREBUTE(assume(X_expression_))
#define MJZ_no_unique_address MJZ_AS_CPP_ATTREBUTE(no_unique_address)
#define JUST_MJZ_FORCED_INLINE_
#define JUST_MJZ_NO_INLINE_
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
#define MJZ_FORCED_INLINE JUST_MJZ_FORCED_INLINE_
#define MJZ_NO_INLINE JUST_MJZ_NO_INLINE_
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


used for understanding the consequenses of a    valid pointer, true boolean flag
, template specilization, value of a function argument  ,active enum member,
active union member...
*/
#define MJZ_ALWAYS_BELIEVE(X_expression_)

/*
this is purely a documentation macro.
THIS DOSE NOT GENERATE ANY CODE !!
INVISIBLE FOR THE COMPILER !!


used for understanding the consequenses of a    valid pointer, true boolean flag
, template specilization, value of a function argument  ,active enum member,
active union member... IF the first statement is true

*/
#define MJZ_ALWAYS_BELIEVE_IF(X_IF_STATEMENT_expression_, X_expression_)

#define MJZ_COMMENT(X_expression_)

#define MJZ_ASSIGN_IN_IF(DEST_D_, SRC_S_) (bool(DEST_D_ = SRC_S_))

#define MJZ_NO_INIT

#define MJZ_PRECONDITION_(X_expression_) MJZ_CONDITION_(X_expression_)
#define MJZ_POSTCONDITION_(X_expression_) MJZ_CONDITION_(X_expression_)
#define MJZ_RJUST_ASSUME_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_JUST_ASSUME_(ASSUMBTION_EXPRESION_)
#define MJZ_RASSUME_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_ASSUME_(ASSUMBTION_EXPRESION_)
#define MJZ_RBELIEVE_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_BELIEVE_(ASSUMBTION_EXPRESION_)
#define MJZ_REXPECT_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_EXPECT_(ASSUMBTION_EXPRESION_)
#define MJZ_RPRETEND_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_PRETEND_(ASSUMBTION_EXPRESION_)
#define MJZ_RCONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_CONDITION_(ASSUMBTION_EXPRESION_)
#define MJZ_RPOSTCONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_POSTCONDITION_(ASSUMBTION_EXPRESION_)
#define MJZ_RPRECONDITION_(ASSUMBTION_EXPRESION_, reason_of_such_assumbtion) \
  MJZ_PRECONDITION_(ASSUMBTION_EXPRESION_)

#define MJZ_NO_MV(CLASS_NAME)         \
  CLASS_NAME(CLASS_NAME &&) = delete; \
  CLASS_NAME &operator=(CLASS_NAME &&) = delete;
#define MJZ_NO_CPY(CLASS_NAME)             \
  CLASS_NAME(const CLASS_NAME &) = delete; \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;

#define MJZ_NO_MV_NO_CPY(CLASS_NAME)             \
  CLASS_NAME(CLASS_NAME &&) = delete;            \
  CLASS_NAME(const CLASS_NAME &) = delete;       \
  CLASS_NAME &operator=(CLASS_NAME &&) = delete; \
  CLASS_NAME(CLASS_NAME &) = delete;             \
  CLASS_NAME &operator=(CLASS_NAME &) = delete;  \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;

#define MJZ_DEFAULTED_CLASS(CLASS_NAME)                              \
  MJZ_CX_FN CLASS_NAME() noexcept = default;                         \
  MJZ_CX_FN ~CLASS_NAME() noexcept = default;                        \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &) noexcept = default;             \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &&) noexcept = default;            \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept = default;       \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &) noexcept = default;  \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &&) noexcept = default; \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept = default;

#define MJZ_UNSAFE_UNION(CLASS_NAME)                             \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept {}           \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept { \
    return *this;                                                \
  }                                                              \
  MJZ_CX_FN CLASS_NAME() noexcept {}                             \
  MJZ_CX_FN ~CLASS_NAME() noexcept {}

#define MJZ_STATIC_CLASS(CLASS_NAME)                                \
  MJZ_CX_FN CLASS_NAME() noexcept = delete;                         \
  MJZ_CX_FN ~CLASS_NAME() noexcept = delete;                        \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &) noexcept = delete;             \
  MJZ_CX_FN CLASS_NAME(CLASS_NAME &&) noexcept = delete;            \
  MJZ_CX_FN CLASS_NAME(const CLASS_NAME &) noexcept = delete;       \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &) noexcept = delete;  \
  MJZ_CX_FN CLASS_NAME &operator=(CLASS_NAME &&) noexcept = delete; \
  MJZ_CX_FN CLASS_NAME &operator=(const CLASS_NAME &) noexcept = delete;

#define MJZ_NO_MV_NO_CPY_DC(CLASS_NAME) \
  MJZ_CX_FN CLASS_NAME() = default;     \
  MJZ_NO_MV_NO_CPY(CLASS_NAME);

#define MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME) \
  mjz_type_of_##VAR_NAME##_t

#define MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, CODE_C_) \
  UNION_OR_STRUCT_UOS_ MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_,    \
                                                 VAR_NAME) CODE_C_;       \
  MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME) VAR_NAME

#define MJZ_NO_MV_NO_CPY_var_of_nameless_type_CNB(UNION_OR_STRUCT_UOS_, \
                                                  VAR_NAME, CODE_C_NB_) \
  MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, {            \
    CODE_C_NB_;                                                         \
    MJZ_NO_MV_NO_CPY(                                                   \
        MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME));     \
  })

#define MJZ_NO_MV_NO_CPY_var_of_nameless_type_CNB_DC(UNION_OR_STRUCT_UOS_, \
                                                     VAR_NAME, CODE_C_NB_) \
  MJZ_var_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME, {               \
    CODE_C_NB_;                                                            \
    MJZ_NO_MV_NO_CPY_DC(                                                   \
        MJZ_name_of_nameless_type(UNION_OR_STRUCT_UOS_, VAR_NAME));        \
  })

#define MJZ_DISABLED_MSVC_WANINGS_ \
  5264 26495 4180 4412 4455 4494 4514 4574 4582 4583 4587 4588 4619 4623 4625 4626 4643 4648 4702 4793 4820 4988 5026 5027 5045 6294 4710 4711 4868 4866 5246
#define MJZ_WARNING_LEVEL_ 3
#define MJZ_PACKING_START_      \
  MJZ_MACRO_PRAGMA_(pack(push)) \
  MJZ_MACRO_PRAGMA_(pack(1))

#define MJZ_PACKING_END_ MJZ_MACRO_PRAGMA_(pack(pop))

#define MJZ_DISABLE_WANINGS_START_      \
  MJZ_MSVC_ONLY_PRAGMA_(warning(push)); \
  MJZ_MSVC_ONLY_PRAGMA_(warning(disable : MJZ_DISABLED_MSVC_WANINGS_));

#define MJZ_DISABLE_WANINGS_END_ MJZ_MSVC_ONLY_PRAGMA_(warning(pop))

#define MJZ_DISABLE_ALL_WANINGS_START_    \
  MJZ_MSVC_ONLY_PRAGMA_(warning(push, 0)) \
  MJZ_MSVC_ONLY_PRAGMA_(warning(          \
      disable : MJZ_DISABLED_MSVC_WANINGS_ 4702 6385 26115 26110 6236 26495 6287 28020 26816))

#define MJZ_DISABLE_ALL_WANINGS_END_ MJZ_MSVC_ONLY_PRAGMA_(warning(pop));

#define MJZ_PADDING_TEST_START_         \
  MJZ_MSVC_ONLY_PRAGMA_(warning(push)); \
  MJZ_MSVC_ONLY_PRAGMA_(warning(4 : 4820));

#define MJZ_PADDING_TEST_END_ MJZ_MSVC_ONLY_PRAGMA_(warning(pop));

#define MJZ_PACKING_START_NW_ \
  MJZ_DISABLE_WANINGS_START_; \
  MJZ_PACKING_START_;

#define MJZ_PACKING_END_NW_ \
  MJZ_PACKING_END_          \
  MJZ_DISABLE_WANINGS_END_

#define MJZ_PACKING_START_NAW_   \
  MJZ_DISABLE_ALL_WANINGS_START_ \
  MJZ_PACKING_START_

#define MJZ_PACKING_END_NAW_ \
  MJZ_PACKING_END_           \
  MJZ_DISABLE_ALL_WANINGS_END_

#define MJZ_Trust_me_bro(CODE) CODE

#define MJZ_W_Trust_me_bro(CODE) \
  MJZ_DISABLE_WANINGS_START_     \
  CODE MJZ_DISABLE_WANINGS_END_

#define MJZ_WA_Trust_me_bro(CODE) \
  MJZ_DISABLE_ALL_WANINGS_START_  \
  CODE MJZ_DISABLE_ALL_WANINGS_END_

#define MJZ_P_Trust_me_bro(CODE) \
  MJZ_PACKING_START_             \
  CODE MJZ_PACKING_END_

#define MJZ_WP_Trust_me_bro(CODE) \
  MJZ_PACKING_START_NW_           \
  CODE MJZ_PACKING_END_NW_

#define MJZ_WAP_Trust_me_bro(CODE) \
  MJZ_PACKING_START_NAW_           \
  CODE MJZ_PACKING_END_NAW_

#define MJZ_MSASSERT(EXPRESTION_, MASSAGE_) \
  static_assert(bool(EXPRESTION_), MASSAGE_)
#define MJZ_SASSERT(EXPRESTION_) \
  MJZ_MSASSERT(EXPRESTION_, "  requremets are not met ")

#define MJZ_offsetof(type_, member_) uintptr_t(offsetof(type_, member_))

#define MJZ_NUMBEROF(Array_) (sizeof(Array_) / sizeof(Array_[0]))

namespace MJZ_NORETURN_SPECIAL_namespace_ {
struct mjz_unreachable_t {
  MJZ_NORETURN MJZ_CX_FN void operator()(bool B_false) const noexcept {
    MJZ_IF_CONSTEVAL {
      std::ignore = reinterpret_cast<const uint8_t &>("UB!!!!!! VVVVVVV"[0]);
    }
    else {
      MJZ_JUST_ASSUME_(B_false);
#ifdef __cpp_lib_unreachable
      ::std::unreachable();
#else
      MJZ_JUST_ASSUME_(false);
      while (!B_false);
      while (B_false);
      for (;;);
      ;
#endif
    }
    for (;;);
  }
};

};  // namespace MJZ_NORETURN_SPECIAL_namespace_
#define MJZ_UNREACHABLE() \
  ::MJZ_NORETURN_SPECIAL_namespace_::mjz_unreachable_t()(false)

#define MJZ_NEVER_RUN_PATH() MJZ_UNREACHABLE()

#define MJZ_TO_STRING(X) #X

#define MJZ_TO_STRING_V(X) MJZ_TO_STRING(X)

namespace MJZ_NORETURN_SPECIAL_namespace_ {
MJZ_DISABLE_WANINGS_START_;
MJZ_NORETURN
MJZ_WILL_USE constexpr static void inline STATUS_ACSESS_VOILATION_bye_mjz_() noexcept {  //-V1082
  MJZ_NEVER_RUN_PATH();
}
MJZ_DISABLE_WANINGS_END_;
};  // namespace MJZ_NORETURN_SPECIAL_namespace_

#define MJZ_NOT_IMPLEMENTATED_HEAPER_()                                    \
  do {                                                                     \
    MJZ_RASSUME_(                                                          \
        false,                                                             \
        "this cntrol path IS UNDEFINED  and will be optimized out and :( " \
        "\n runing this has UNDEFINED BEHAVIOUR .");                       \
    ::MJZ_NORETURN_SPECIAL_namespace_::STATUS_ACSESS_VOILATION_bye_mjz_(); \
  } while (0)

#define MJZ_NOT_IMPLEMENTATED()      \
  MJZ_NOT_IMPLEMENTATED_HEAPER_();   \
  do {                               \
    MJZ_NOT_IMPLEMENTATED_HEAPER_(); \
  } while (true)

#define MJZ_NOT_IMPLEMENTATED_YET() MJZ_NOT_IMPLEMENTATED()
#define MJZ_NOT_IMPLEMENTATED_YET_v() \
  [] {                                \
    MJZ_NOT_IMPLEMENTATED();          \
    return;                           \
  }()
#define MJZ_UB() MJZ_NOT_IMPLEMENTATED()

//-V:MJZ_NOT_CONSTEXPR: 779 , 1080 , 571 , 2535 , 3522
#define MJZ_NOT_CONSTEXPR()                                                \
  ;                                                                        \
  MJZ_IF_CONSTEVAL {                                                       \
    MJZ_MSVC_ONLY_PRAGMA_(warning(push));                                  \
    MJZ_MSVC_ONLY_PRAGMA_(warning(disable : 4702));                        \
    ::MJZ_NORETURN_SPECIAL_namespace_::STATUS_ACSESS_VOILATION_bye_mjz_(); \
    MJZ_ASSUME_(false);                                                    \
    MJZ_MSVC_ONLY_PRAGMA_(warning(pop));                                   \
  }

//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESV_GET(PTR_, alignof_Ptr_) \
  (::std::assume_aligned<alignof_Ptr_>(PTR_))

//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN_GET(PTR_, alignof_Ptr_)          \
  ([](auto *ptr_just_ptr_) noexcept {                          \
    MJZ_ASSUME_(ptr_just_ptr_ != nullptr);                     \
    return ::std::assume_aligned<alignof_Ptr_>(ptr_just_ptr_); \
  }(PTR_))
//-V:MJZ_ASSUME_ALIGNESV_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN_GET_C(PTR_, alignof_Ptr_)       \
  ([&](auto *ptr_just_ptr_) noexcept {                        \
    MJZ_ASSUME_(ptr_just_ptr_ != nullptr);                    \
    return ptr_just_ptr_; /* no dynamic assume was provided*/ \
  }(PTR_))
//-V:MJZ_ASSUME_ALIGNES_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNES_GET(PTR_, TYPE_) \
  MJZ_ASSUME_ALIGNESV_GET(PTR_, alignof(TYPE_))
//-V:MJZ_ASSUME_ALIGNES_GET: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESNN_GET(PTR_, TYPE_) \
  MJZ_ASSUME_ALIGNESVNN_GET(PTR_, alignof(TYPE_))

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESV(PTR_, TYPE_A_) \
  MJZ_ASSUME_((PTR_) == ::std::assume_aligned<TYPE_A_>(PTR_))
//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNES(PTR_, TYPE_) \
  MJZ_ASSUME_ALIGNESV(PTR_, alignof(TYPE_))

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESVNN(PTR_, TYPE_A_)                    \
  MJZ_ASSUME_((PTR_) == ::std::assume_aligned<TYPE_A_>(PTR_) && \
              (PTR_) != nullptr)

//-V:MJZ_ASSUME_ALIGNES: 3546 , 2571,1080
#define MJZ_ASSUME_ALIGNESNN(PTR_, TYPE_) \
  MJZ_ASSUME_ALIGNESVNN(PTR_, alignof(TYPE_))

#define MJZ_UNSAFE_CODE_BLOCK(BLOCK_) BLOCK_

#define MJZ_ASSUME_IF_THEN(CONDITION_STATEMENT_, THEN_STATEMENT_) \
  MJZ_ASSUME_(!static_cast<bool>(CONDITION_STATEMENT_) ||         \
              static_cast<bool>(THEN_STATEMENT_))

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

#define MJZ_BAD_COMPILER(EXP) \
  MJZ_MSASSERT(               \
      EXP, "[unexpected standard violation] bad compiler macro definition")

#define MJZ_BAD_MCOMPILER(EXP, AND_MASSAGE)                          \
  MJZ_MSASSERT(EXP,                                                  \
               "[unexpected standard violation] bad compiler macro " \
               "definition ; " AND_MASSAGE)

#ifndef MJZ_VERBOSE_FORMAT_ERROR
#define MJZ_VERBOSE_FORMAT_ERROR true
#endif  // !MJZ_VERBOSE_FORMAT_ERROR


#define MJZ_CX_AL_FN MJZ_GCC_ONLY_CODE_(__attribute__((always_inline))) MJZ_CX_FN MJZ_MSVC_ONLY_CODE_(__forceinline)


#endif  // !mjz_string_lib_HPP_FILE_
