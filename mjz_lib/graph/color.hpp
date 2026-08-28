

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

#ifndef MJZ_SRC_GRAPH_color_FILE_
#define MJZ_SRC_GRAPH_color_FILE_
#include "csr.hpp"
#include "optimize.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_graph_greedy_by_order(const basic_forest_t<version_v, T> &graph,
                            R &&order) noexcept {
  uintlen_t color_count{};
  auto adjs = graph.range();
  uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> graph_color(sz, uintlen_t(-1));
  for (uintlen_t i : std::forward<R>(order)) {
    uintlen_t selected_color{color_count};
    for (uintlen_t j : adjs[i]) {
      if ((graph_color[j] | 1) == uintlen_t(-1))
        continue;
      graph_color[graph_color[j] >> 1] &= ~uintlen_t(1);
    }

    for (uintlen_t j{}; j < color_count; j++) {
      if (!(graph_color[j] & 1))
        continue;
      selected_color = j;
      break;
    }
    for (uintlen_t j : adjs[i]) {
      if ((graph_color[j] | 1) == uintlen_t(-1))
        continue;
      graph_color[graph_color[j] >> 1] |= uintlen_t(1);
    }
    graph_color[i] = (selected_color << 1) | 1;
    color_count = std::max(color_count, selected_color + 1);
  }
  for (uintlen_t &c : graph_color)
    c = uintlen_t(intlen_t(c) >> 1);
  return {graph_color, color_count};
}

template <version_t version_v, class T>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_graph_greedy(const basic_forest_t<version_v, T> &graph) noexcept {
  return color_graph_greedy_by_order(
      graph, std::views::iota(uintlen_t(0),
                              uintlen_t(std::ranges::size(graph.range()))));
}

template <version_t version_v>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t> color_timeline(
    std::span<const basic_index_range_t<version_v>> timelines) noexcept {
  // empty timelines get colored with  uintlen_t(-1)
  std::vector<uintlen_t> ret(timelines.size() * 4, uintlen_t(-1));
  uintlen_t bumbp_alloc_ptr{timelines.size()};
  auto alloc_ptr_fn_ = [&](uintlen_t sz) noexcept {
    bumbp_alloc_ptr += sz;
    asserts(bumbp_alloc_ptr <= ret.size());
    return std::span(ret).subspan(bumbp_alloc_ptr - sz, sz);
  };
  auto free_list = alloc_ptr_fn_(timelines.size());
  auto intersected_time = alloc_ptr_fn_(timelines.size() * 2);
  uintlen_t color_count{};
  uintlen_t free_list_ptr{};
  uintlen_t intersected_time_ptr{};

  for (uintlen_t i{}; i < timelines.size(); i++) {
    if (!timelines[i].n)
      continue;
    intersected_time[intersected_time_ptr++] = (i << 1) | 1;
    intersected_time[intersected_time_ptr++] = (i << 1);
  }

  auto intersected_time_slice =
      intersected_time.subspan(uintlen_t(), intersected_time_ptr);
  std::ranges::sort(
      intersected_time_slice, std::ranges::less{}, [&](uintlen_t i) noexcept {
        auto [begi, endi] = timelines[i >> 1].bounds();
        return (i & 1) ? pair_t{begi * 2 + 1, i} : pair_t{endi * 2, i};
      });
  for (uintlen_t index_and_bit : intersected_time_slice) {
    uintlen_t index = index_and_bit >> 1;
    if (!(index_and_bit & 1)) {
      free_list[free_list_ptr++] = ret[index];
      continue;
    }
    free_list[free_list_ptr] = color_count;
    color_count += !free_list_ptr;
    free_list_ptr += !free_list_ptr;
    ret[index] = free_list[--free_list_ptr];
  }
  ret.resize(timelines.size());
  return {std::move(ret), color_count};
}

template <version_t version_v, class T>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_chordal_graph(const basic_forest_t<version_v, T> &graph) noexcept {
  return color_graph_greedy_by_order(
      graph, optimize_maximum_cardinality_search_ordering(graph));
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_color_FILE_