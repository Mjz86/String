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

#ifndef MJZ_SRC_GRAPH_base_FILE_
#define MJZ_SRC_GRAPH_base_FILE_
#include "../releasers.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <typename T>
concept random_access_iterator_c =
    std::random_access_iterator<std::remove_cvref_t<T>>;

template <typename T, version_t version_v>
concept usable_range_exact_c =
    std::ranges::random_access_range<T> && std::ranges::sized_range<T>;
template <typename T, class U, version_t version_v>
concept usable_type_range_exact_c =
    usable_range_exact_c<T, version_v> &&
    std::convertible_to<std::ranges::range_reference_t<T>, U>;
template <typename T, class U, version_t version_v>
concept usable_type_range_c =
    usable_type_range_exact_c<std::remove_cvref_t<T>, U, version_v>;

template <typename T, version_t version_v>
concept index_range_of_c =
    std::ranges::range<T> &&
    std::convertible_to<std::ranges::range_reference_t<T>, uintlen_t>;

template <typename T, version_t version_v>
concept usable_index_range_exact_c =
    usable_type_range_exact_c<T, uintlen_t, version_v>;
template <typename T, version_t version_v>
concept usable_index_range_c =
    usable_index_range_exact_c<std::remove_cvref_t<T>, version_v>;

template <typename T, version_t version_v>
concept mutable_index_range_exact_c =
    usable_type_range_exact_c<T, uintlen_t &, version_v>;
template <typename T, version_t version_v>
concept mutable_index_range_c =
    mutable_index_range_exact_c<std::remove_cvref_t<T>, version_v>;

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> range_bijective_inv(R &&index_range) noexcept {
  uintlen_t sz = std::ranges::size(index_range);
  std::vector<uintlen_t> inverse_index_range(sz);
  for (uintlen_t i : std::views::iota(uintlen_t(0), sz)) {
    inverse_index_range[index_range[i]] = i;
  }
  return inverse_index_range;
}

template <version_t version_v> struct basic_iota_forest_t {
  basic_index_range_t<version_v> edges_index{};
  basic_index_range_t<version_v> nodes_index_index{};
};
}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_base_FILE_