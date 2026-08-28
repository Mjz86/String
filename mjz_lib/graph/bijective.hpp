
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

#ifndef MJZ_SRC_GRAPH_bijective_FILE_
#define MJZ_SRC_GRAPH_bijective_FILE_
#include "base.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> make_view_vector_range(R &&range_) noexcept {
  return std::vector<uintlen_t>(std::ranges::begin(range_),
                                std::ranges::end(range_));
}

template <version_t version_v>
MJZ_CX_FN std::vector<uintlen_t>
make_index_vector_range(const uintlen_t sz) noexcept {
  return make_view_vector_range<version_v>(std::views::iota(uintlen_t(0), sz));
}
template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN auto original_order_transform(R &&transform_range_) noexcept {
  return std::views::transform(
      [transform_range = std::forward<R>(transform_range_)](
          uintlen_t i) noexcept { return transform_range[i]; });
}

template <version_t version_v>
MJZ_CX_FN std::vector<uintlen_t>
optimize_transformer_ordering(uintlen_t sz, auto &&index_transformer) noexcept {
  auto ret = make_index_vector_range<version_v>(sz);
  std::ranges::sort(ret, std::ranges::less{}, index_transformer);
  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN std::vector<std::iter_value_t<it_t>>
make_reorder_bijective_transform_indexies(index_range_t &&map,
                                          it_t &&data) noexcept {
  std::vector<std::iter_value_t<it_t>> ret{};
  auto v =
      map | std::views::transform([&](uintlen_t i) noexcept -> decltype(auto) {
        return *(data + intlen_t(i));
      });
  ret.insert(ret.begin(), v.begin(), v.end());

  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN std::vector<std::iter_value_t<it_t>>
make_reorder_inv_bijective_transform_indexies(index_range_t &&map,
                                              it_t &&data) noexcept {
  return make_reorder_bijective_transform_indexies<version_v>(
      range_bijective_inv<version_v>(map), std::forward<it_t>(data));
}

template <version_t version_v, mutable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN void
reorder_range_bijective_transform_indexies(index_range_t &&map,
                                           it_t &&data) noexcept {
  intlen_t sz = intlen_t(std::ranges::distance(map));
  auto mp = std::ranges::begin(map);
  for (intlen_t i{}; i < sz; i++) {
    intlen_t old_i = i;
    intlen_t new_i = intlen_t(*(mp + old_i));
    if (new_i == old_i)
      continue;
    std::remove_cvref_t<decltype(*data)> first = std::move(*(data + i));
    do {
      *(data + old_i) = std::move(*(data + new_i));
      *(mp + old_i) = uintlen_t(old_i);
      old_i = new_i;
      new_i = intlen_t(*(mp + old_i));
    } while (new_i != i);
    *(data + old_i) = std::move(first);
    *(mp + old_i) = uintlen_t(old_i);
  }
}
template <std::ranges::random_access_range it_range_t>
  requires std::random_access_iterator<std::ranges::range_value_t<it_range_t>>
MJZ_CX_FN void reorder_range_bijective_transform_iterators(
    it_range_t &&map,
    std::ranges::range_value_t<it_range_t> begining) noexcept {
  auto en = std::ranges::end(map);
  auto bg = std::ranges::begin(map);
  for (auto it = bg; it != en; ++it) {
    auto i = it - bg;
    auto old_i = i;
    auto new_i = *(bg + old_i) - begining;
    if (new_i == old_i)
      continue;
    auto first = std::move(*(begining + i));
    do {
      *(begining + old_i) = std::move(*(begining + new_i));
      *(bg + old_i) = old_i + begining;
      old_i = new_i;
      new_i = *(bg + old_i) - begining;
    } while (new_i != i);
    *(begining + old_i) = std::move(first);
    *(bg + old_i) = old_i + begining;
  }
}

}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_bijective_FILE_