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

#include "heaps.hpp"
#include "views.hpp"
#ifndef MJZ_BYTE_STRING_base_abi_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_base_abi_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz ::bstr_ns {
  template <version_t version_v>
  struct cheap_base_str_info_t {
    const allocs_ns::alloc_base_ref_t<version_v> *alloc_ptr{};
    uintlen_t reserve_capacity{0};
    bool is_threaded{true};
    // mostly ignored for now
    bool add_null{true};
    // mostly ignored for now
    encodings_e encoding{encodings_e::ascii};
  };
  namespace abi_ns_ {
  template <version_t version_v, bool has_alloc_v_>
  struct alloc_t {};
  template <version_t version_v>
  struct alloc_t<version_v, true> {
    allocs_ns::alloc_base_ref_t<version_v> alloc_;
  };
  };  // namespace abi_ns_
};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_base_abi_LIB_HPP_FILE_