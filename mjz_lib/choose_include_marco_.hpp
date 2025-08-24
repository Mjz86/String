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
#ifndef MJZ_USE_cpp_lib_to_chars_int
#define MJZ_USE_cpp_lib_to_chars_int false
#endif
#ifndef MJZ_USE_cpp_lib_to_chars_float
#define MJZ_USE_cpp_lib_to_chars_float false
#endif
#define MJZ_CAN_USE_LIB_STD_charconv_ false
#if MJZ_USE_cpp_lib_to_chars_float || MJZ_USE_cpp_lib_to_chars_int
#ifdef __cpp_lib_to_chars

#undef MJZ_CAN_USE_LIB_STD_charconv_
#define MJZ_CAN_USE_LIB_STD_charconv_ true
#endif
#endif

#ifndef MJZ_CAN_USE_LIB_STD_FORMAT_
#define MJZ_CAN_USE_LIB_STD_FORMAT_ false
#else
#ifndef __cpp_lib_format
#undef MJZ_CAN_USE_LIB_STD_FORMAT_
#define MJZ_CAN_USE_LIB_STD_FORMAT_ false
#endif
#endif  // !MJZ_CAN_USE_LIB_STD_FORMAT_
#ifdef MJZ_USE_CXX_MODULES_
#ifndef __cpp_modules
#undef MJZ_USE_CXX_MODULES_
#define MJZ_USE_CXX_MODULES_ false
#endif
#else
#define MJZ_USE_CXX_MODULES_ false
#endif


#ifndef MJZ_STD_HAS_SIMD_LIB_
#define MJZ_STD_HAS_SIMD_LIB_ false
#else

#if defined(__cpp_lib_experimental_parallel_simd) || \
    defined(_LIBCPP_ENABLE_EXPERIMENTAL)
#define MJZ_STD_HAS_SIMD_LIB_ true
#endif
#endif  // ! MJZ_STD_HAS_SIMD_LIB_

#ifdef MJZ_USE_CXX_STD_MODULES_
#ifndef __cpp_lib_modules
#undef MJZ_USE_CXX_STD_MODULES_
#define MJZ_USE_CXX_STD_MODULES_ false
#endif
#else
#define MJZ_USE_CXX_STD_MODULES_ false
#endif

#if !MJZ_USE_CXX_MODULES_
#undef MJZ_USE_CXX_STD_MODULES_
#define MJZ_USE_CXX_STD_MODULES_ false
#endif


#ifndef MJC_MANUAL_CXX_INCLUDES_
#define MJC_MANUAL_CXX_INCLUDES_ false
#endif