
#include "heaps.hpp"
#include "views.hpp"
#ifndef MJZ_BYTE_STRING_base_abi_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_base_abi_LIB_HPP_FILE_
namespace mjz ::bstr_ns {

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