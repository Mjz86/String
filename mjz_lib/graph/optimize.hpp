

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

#ifndef MJZ_SRC_GRAPH_optimize_FILE_
#define MJZ_SRC_GRAPH_optimize_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {
template <version_t version_v, class T>
MJZ_CX_FN pair_t<std::vector<uintlen_t>,
                 basic_forest_t<version_v, std::vector<uintlen_t>>>
optimize_transformer_ordering_graph(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    auto &&index_transformer) noexcept {
  auto order = optimize_transformer_ordering<version_v>(
      std::ranges::size(suboptimal_indexed_graph.range()), index_transformer);
  return {order, organize_graph_edges(
                     suboptimal_indexed_graph,
                     [&](uintlen_t lhs, uintlen_t rhs) noexcept {
                       return index_transformer(lhs) < index_transformer(rhs);
                     })
                     .transform(order)};
}

template <version_t version_v, class T>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
trim_to_directed_graph_edges(const basic_forest_t<version_v, T> &graph,
                             auto &&comp = std::ranges::less{}) noexcept {
  treversal_result_t<version_v> ret{};

  ret.nodes_index.reserve(std::ranges::size(graph.nodes_index));
  ret.edges.reserve(std::ranges::size(graph.edges));
  auto adjs = graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  for (uintlen_t i : std::views::iota(uintlen_t(0), sz)) {
    ret.nodes_index.push_back(ret.edges.size());
    for (uintlen_t j : adjs[i]) {
      if (!comp(i, j))
        continue;
      ret.edges.push_back(j);
    }
  }
  return ret;
}

template <version_t version_v, class T>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
organize_graph_edges(const basic_forest_t<version_v, T> &graph,
                     auto &&comp = std::ranges::less{}) noexcept {
  treversal_result_t<version_v> ret{};
  ret.nodes_index.reserve(std::ranges::size(graph.nodes_index));
  ret.edges.reserve(std::ranges::size(graph.edges));
  auto adjs = graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  for (uintlen_t i : std::views::iota(uintlen_t(0), sz)) {
    uintlen_t old = ret.edges.size();
    ret.nodes_index.push_back(old);
    uintlen_t n = std::ranges::size(adjs[i]);
    ret.edges.resize(old + n);
    std::ranges::partial_sort_copy(adjs[i],
                                   std::span(ret.edges).subspan(old, n), comp);
  }
  return ret;
}
template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_cardinality_search_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  auto adjs = suboptimal_indexed_graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> ret(sz * 4, uintlen_t(-1));
  if (!sz)
    return ret;
  auto priority_list_next = std::span<uintlen_t>(ret).subspan(sz, sz);
  auto priority_list_head = std::span<uintlen_t>(ret).subspan(sz * 2, sz);
  auto priority_of_node_compl = std::span<uintlen_t>(ret).subspan(sz * 3, sz);

  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    priority_list_next[i] = std::exchange(priority_list_head[0], i);
  }
  uintlen_t highest_priority{};
  uintlen_t position_node_index{};
  MJZ_RAII_RELEASE { ret.resize(position_node_index); };
  while (position_node_index != sz) {
    while (priority_list_head[highest_priority] == uintlen_t(-1)) {
      if (!highest_priority)
        return (ret);
      --highest_priority;
    }
    uintlen_t candidate_node_index = priority_list_head[highest_priority];
    priority_list_head[highest_priority] =
        priority_list_next[candidate_node_index];
    if (!priority_of_node_compl[candidate_node_index])
      continue;
    priority_of_node_compl[candidate_node_index] = 0;
    ret[position_node_index++] = candidate_node_index;
    for (uintlen_t neighbor :
         adjs[candidate_node_index] | std::views::reverse) {
      if (!priority_of_node_compl[neighbor])
        continue;
      uintlen_t prio_index = ~--priority_of_node_compl[neighbor];
      highest_priority = std::max(highest_priority, prio_index);
      priority_list_next[neighbor] =
          std::exchange(priority_list_head[prio_index], neighbor);
    }
  }
  return (ret);
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_bucket_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph, R &&order,
    auto &&bucket_fn, uintlen_t bucket_max) noexcept {
  auto adjs = suboptimal_indexed_graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> ret(sz * 2 + 1 + bucket_max, uintlen_t(-1));
  if (!sz)
    return ret;
  uintlen_t position_node_index{};
  MJZ_RAII_RELEASE { ret.resize(position_node_index); };
  auto priority_list_next = std::span<uintlen_t>(ret).subspan(sz, sz);
  auto priority_list_head = std::span<uintlen_t>(ret).subspan(sz * 2);

  uintlen_t max_deg{};
  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    uintlen_t deg = bucket_fn(i);
    asserts(deg <= bucket_max);
    max_deg = std::max(deg, max_deg);
    priority_list_next[i] = std::exchange(priority_list_head[deg], i);
  }
  uintlen_t highest_priority{max_deg};
  while (position_node_index != sz) {
    while (priority_list_head[highest_priority] == uintlen_t(-1)) {
      if (!highest_priority)
        return ret;
      --highest_priority;
    }
    uintlen_t candidate_node_index = priority_list_head[highest_priority];
    priority_list_head[highest_priority] =
        priority_list_next[candidate_node_index];
    ret[position_node_index++] = candidate_node_index;
  }
  return ret;
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_degree_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  return optimize_maximum_bucket_ordering(
      suboptimal_indexed_graph, order,
      [adjs = suboptimal_indexed_graph.range()](uintlen_t i) noexcept
          -> uintlen_t { return std::ranges::size(adjs[i]); },
      std::ranges::size(suboptimal_indexed_graph.edges));
}

template <version_t version_v, class T>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_degree_ordering(
    const basic_forest_t<version_v, T> &graph) noexcept {
  return optimize_maximum_degree_ordering(
      graph, std::views::iota(uintlen_t(),
                              uintlen_t(std::ranges::size(graph.range()))));
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_optimize_FILE_