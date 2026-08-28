

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

#ifndef MJZ_SRC_GRAPH_search_FILE_
#define MJZ_SRC_GRAPH_search_FILE_
#include "csr.hpp"
#include "dsu.hpp"
MJZ_EXPORT
//
namespace mjz::graph_ns {

template <
    version_t version_v, mutable_index_range_c<version_v> zero_init_level_t,
    mutable_index_range_c<version_v> uninit_bfs_order_t,
    mutable_index_range_c<version_v> uninit_bfs_parent_t,
    class R = std::span<const uintlen_t>, index_range_of_c<version_v> RIR_t>
MJZ_CX_FN pair_t<uintlen_t, uintlen_t>
calculate_breadth_first_levels_and_parent(
    zero_init_level_t &&zero_init_level, uninit_bfs_order_t &&uninit_bfs_order,
    uninit_bfs_parent_t &&uninit_bfs_parent,
    const basic_forest_t<version_v, R> &edge_of_node_, RIR_t &&visit_order,
    auto &&entry_callback, auto &&tree_edge_callback,
    auto &&visited_edge_callback) noexcept
  requires requires(uintlen_t parent, uintlen_t entry_level, uintlen_t node,
                    uintlen_t adj) {
    { (void)entry_callback(+parent, +node, +entry_level) } noexcept;
    { (void)tree_edge_callback(+node, +adj) } noexcept;
    { (void)visited_edge_callback(+node, +adj) } noexcept;
  }
{

  auto adjs = edge_of_node_.range();
  uintlen_t head{};
  uintlen_t tail{};
  uintlen_t max_level{};
  for (uintlen_t graph_node : visit_order) {
    if (zero_init_level[graph_node])
      continue;
    uninit_bfs_parent[graph_node] = uintlen_t(-1);
    zero_init_level[graph_node] = 1;
    max_level = std::max(max_level, uintlen_t(1));
    uninit_bfs_order[head++] = graph_node;

    while (head != tail) {
      graph_node = uninit_bfs_order[tail++];
      uintlen_t level = zero_init_level[graph_node];
      (void)entry_callback(+uninit_bfs_parent[graph_node], +graph_node, +level);
      for (uintlen_t adj : adjs[graph_node]) {
        if (zero_init_level[adj]) {
          (void)visited_edge_callback(+graph_node, +adj);
          continue;
        }
        uninit_bfs_parent[adj] = graph_node;
        zero_init_level[adj] = level + 1;
        uninit_bfs_order[head++] = adj;
        max_level = std::max(max_level, level + 1);

        (void)tree_edge_callback(+graph_node, +adj);
      }
    }
  }
  return {max_level, head};
}

template <
    version_t version_v, mutable_index_range_c<version_v> zero_init_level_t,
    mutable_index_range_c<version_v> uninit_bfs_order_t,
    class R = std::span<const uintlen_t>, index_range_of_c<version_v> RIR_t>
MJZ_CX_FN pair_t<uintlen_t, uintlen_t> calculate_breadth_first_levels_no_parent(
    zero_init_level_t &&zero_init_level, uninit_bfs_order_t &&uninit_bfs_order,
    const basic_forest_t<version_v, R> &edge_of_node_, RIR_t &&visit_order,
    auto &&entry_callback, auto &&tree_edge_callback,
    auto &&visited_edge_callback) noexcept
  requires requires(uintlen_t entry_level, uintlen_t node, uintlen_t adj) {
    { (void)entry_callback(+node, +entry_level) } noexcept;
    { (void)tree_edge_callback(+node, +adj) } noexcept;
    { (void)visited_edge_callback(+node, +adj) } noexcept;
  }
{

  auto adjs = edge_of_node_.range();
  uintlen_t head{};
  uintlen_t tail{};
  uintlen_t max_level{};
  for (uintlen_t graph_node : visit_order) {
    if (zero_init_level[graph_node])
      continue;
    zero_init_level[graph_node] = 1;
    max_level = std::max(max_level, uintlen_t(1));
    uninit_bfs_order[head++] = graph_node;

    while (head != tail) {
      graph_node = uninit_bfs_order[tail++];
      uintlen_t level = zero_init_level[graph_node];
      (void)entry_callback(+graph_node, +level);
      for (uintlen_t adj : adjs[graph_node]) {
        if (zero_init_level[adj]) {
          (void)visited_edge_callback(+graph_node, +adj);
          continue;
        }
        zero_init_level[adj] = level + 1;
        uninit_bfs_order[head++] = adj;
        max_level = std::max(max_level, level + 1);

        (void)tree_edge_callback(+graph_node, +adj);
      }
    }
  }
  return {max_level, head};
}

template <version_t version_v,
          mutable_index_range_c<version_v> zero_init_interval_begin_t,
          mutable_index_range_c<version_v> zero_init_interval_end_t,
          mutable_index_range_c<version_v> uninit_dfs_parent_t,
          class R = std::span<const uintlen_t>,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN uintlen_t calculate_depth_first_intervals(
    zero_init_interval_begin_t &&zero_init_interval_begin,
    zero_init_interval_end_t &&zero_init_interval_end,
    uninit_dfs_parent_t &&uninit_dfs_parent,
    const basic_forest_t<version_v, R> &edge_of_node_, RIR_t &&visit_order,
    auto &&entry_callback, auto &&tree_edge_callback, auto &&back_edge_callback,
    auto &&cross_edge_callback, auto &&forward_edge_callback,
    auto &&exit_callback) noexcept
  requires requires(uintlen_t parent, uintlen_t entry_time, uintlen_t exit_time,
                    uintlen_t node, uintlen_t adj) {
    { (void)entry_callback(+parent, +node, +entry_time) } noexcept;
    { (void)tree_edge_callback(+node, +adj) } noexcept;
    { (void)back_edge_callback(+node, +adj) } noexcept;
    { (void)cross_edge_callback(+node, +adj) } noexcept;
    { (void)forward_edge_callback(+node, +adj) } noexcept;
    { (void)exit_callback(+parent, +node, +exit_time) } noexcept;
  }
{

  auto adjs = edge_of_node_.range();
  uintlen_t timer{};
  for (uintlen_t graph_node : visit_order) {
    if (zero_init_interval_begin[graph_node])
      continue;
    uninit_dfs_parent[graph_node] = uintlen_t(-1);
    do {
      uintlen_t graph_size = std::ranges::size(adjs[graph_node]);
      uintlen_t next_leaf_index = ~--zero_init_interval_end[graph_node];
      if (!next_leaf_index) {
        zero_init_interval_begin[graph_node] = ++timer;
        (void)entry_callback(+uninit_dfs_parent[graph_node], +graph_node,
                             +timer);
      }

      if (next_leaf_index < graph_size) {
        uintlen_t next_graph_node = adjs[graph_node][intlen_t(next_leaf_index)];
        uintlen_t start_time_next = zero_init_interval_begin[next_graph_node];
        if (start_time_next) {
          if (~zero_init_interval_end[next_graph_node] <
              std::ranges::size(adjs[next_graph_node])) {
            (void)back_edge_callback(+graph_node, +next_graph_node);
            continue;
          }
          if (zero_init_interval_begin[graph_node] <=
              zero_init_interval_begin[next_graph_node]) {
            (void)forward_edge_callback(+graph_node, +next_graph_node);
            continue;
          }
          (void)cross_edge_callback(+graph_node, +next_graph_node);
          continue;
        }
        (void)tree_edge_callback(+graph_node, +next_graph_node);
        uninit_dfs_parent[next_graph_node] = graph_node;
        graph_node = next_graph_node;
        continue;
      }

      zero_init_interval_end[graph_node] = ++timer;
      uintlen_t parent_graph = uninit_dfs_parent[graph_node];
      (void)exit_callback(+parent_graph, +graph_node, +timer);
      if (parent_graph == uintlen_t(-1))
        break;
      graph_node = parent_graph;

      continue;
    } while (true);
  }
  return timer;
}
template <version_t version_v,
          mutable_index_range_c<version_v> zero_init_interval_begin_t,
          mutable_index_range_c<version_v> zero_init_interval_end_t,
          mutable_index_range_c<version_v> uninit_dfs_parent_t,
          class R = std::span<const uintlen_t>,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN uintlen_t calculate_depth_first_intervals(
    zero_init_interval_begin_t &&zero_init_interval_begin,
    zero_init_interval_end_t &&zero_init_interval_end,
    uninit_dfs_parent_t &&uninit_dfs_parent,
    const basic_forest_t<version_v, R> &edge_of_node_,
    RIR_t &&visit_order) noexcept {
  auto dummy_callback = [](auto &&...) noexcept {};
  return calculate_depth_first_intervals(
      zero_init_interval_begin, zero_init_interval_end, uninit_dfs_parent,
      edge_of_node_, visit_order, dummy_callback, dummy_callback,
      dummy_callback, dummy_callback, dummy_callback, dummy_callback);
}

template <version_t version_v,
          mutable_index_range_c<version_v> zero_init_interval_begin_t,
          mutable_index_range_c<version_v> zero_init_interval_end_t,
          class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> immidiate_dominators_t,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN uintlen_t calculate_dominance_intervals_given_dominators_and_forest(
    zero_init_interval_begin_t &&zero_init_interval_begin,
    zero_init_interval_end_t &&zero_init_interval_end,
    const basic_forest_t<version_v, R> &dominator_forest,
    immidiate_dominators_t &&immidiate_dominators,
    RIR_t &&visit_order) noexcept {

  auto dom_tree_range = dominator_forest.range();
  uintlen_t timer{};
  for (uintlen_t tree_node :
       visit_order | std::views::filter([&](uintlen_t i) noexcept -> bool {
         return immidiate_dominators[i] == i ||
                immidiate_dominators[i] == uintlen_t(-1);
       })) {
    uintlen_t parent_tree = immidiate_dominators[tree_node];
    if (parent_tree == uintlen_t(-1)) {
      zero_init_interval_begin[tree_node] = ++timer;
      zero_init_interval_end[tree_node] = ++timer;
      continue;
    }
    if (tree_node != parent_tree)
      continue;
    if (zero_init_interval_begin[tree_node])
      continue;
    do {
      uintlen_t tree_size = std::ranges::size(dom_tree_range[tree_node]);
      uintlen_t next_leaf_index = zero_init_interval_end[tree_node]++;
      if (!next_leaf_index) {
        zero_init_interval_begin[tree_node] = ++timer;
      }

      if (next_leaf_index < tree_size) {
        tree_node = dom_tree_range[tree_node][intlen_t(next_leaf_index)];
        continue;
      }

      zero_init_interval_end[tree_node] = ++timer;
      parent_tree = immidiate_dominators[tree_node];
      if (tree_node == parent_tree || parent_tree == uintlen_t(-1))
        break;
      tree_node = parent_tree;
      continue;

    } while (true);
  }
  return timer;
}

template <version_t version_v, class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> immidiate_dominators_t>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, std::vector<uintlen_t>>
calculate_dominance_intervals_given_dominators_and_forest(
    const basic_forest_t<version_v, R> &dominator_forest,
    immidiate_dominators_t &&immidiate_dominatorsr) noexcept {
  auto begin_interval = std::vector<uintlen_t>(immidiate_dominatorsr.size());
  auto end_interval = std::vector<uintlen_t>(immidiate_dominatorsr.size());
  calculate_dominance_intervals_given_dominators_and_forest(
      begin_interval, end_interval, dominator_forest, immidiate_dominatorsr,
      std::views::iota(uintlen_t(), uintlen_t(immidiate_dominatorsr.size())));
  return {begin_interval, end_interval};
}

template <version_t version_v,
          usable_index_range_c<version_v> dominance_interval_begin_t,
          usable_index_range_c<version_v> dominance_interval_end_t,
          usable_index_range_c<version_v> depth_first_interval_begin_t,
          usable_index_range_c<version_v> depth_first_interval_end_t,
          class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> visit_range_t>
MJZ_CX_FN uintlen_t
for_each_unreducibles_given_depth_first_and_dominance_intervals(
    dominance_interval_begin_t &&dominance_interval_begin,
    dominance_interval_end_t &&dominance_interval_end,
    depth_first_interval_begin_t &&depth_first_interval_begin,
    depth_first_interval_end_t &&depth_first_interval_end,
    const basic_forest_t<version_v, R> &edge_of_node_,
    visit_range_t &&visit_range, auto &&call_back_unreducibles,
    auto &&call_back_reducibles) noexcept {
  auto dominates = [&](uintlen_t dom, uintlen_t sub) noexcept -> bool {
    if (dominance_interval_begin[dom] == 0 ||
        dominance_interval_begin[sub] == 0)
      return false;
    return dominance_interval_begin[dom] <= dominance_interval_begin[sub] &&
           dominance_interval_end[dom] >= dominance_interval_end[sub];
  };
  auto is_dfs_ancestor = [&](uintlen_t dom, uintlen_t sub) noexcept -> bool {
    if (depth_first_interval_begin[dom] == 0 ||
        depth_first_interval_begin[sub] == 0)
      return false;
    return depth_first_interval_begin[dom] <= depth_first_interval_begin[sub] &&
           depth_first_interval_end[dom] >= depth_first_interval_end[sub];
  };

  auto adjs = edge_of_node_.range();

  uintlen_t unreducibles_count{};
  for (uintlen_t node : visit_range) {
    for (uintlen_t next : adjs[node]) {
      const bool is_dfs_ancestor_ = is_dfs_ancestor(next, node);
      const bool is_dominator_ = dominates(next, node);
      if (!is_dfs_ancestor_ || is_dominator_) {
        (void)call_back_reducibles(+node, +next, is_dfs_ancestor_,
                                   is_dominator_);
        continue;
      }
      unreducibles_count++;
      (void)call_back_unreducibles(+node, +next);
    }
  }
  return unreducibles_count;
}

template <version_t version_v, mutable_index_range_c<version_v> range_t,
          usable_index_range_c<version_v> dominance_interval_begin_t,
          usable_index_range_c<version_v> dominance_interval_end_t,
          usable_index_range_c<version_v> depth_first_interval_begin_t,
          usable_index_range_c<version_v> depth_first_interval_end_t,
          class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> visit_range_t>
MJZ_CX_FN void union_unreducibles_given_depth_first_and_dominance_intervals(
    disjoint_set_union_t<version_v, range_t> &ret,
    dominance_interval_begin_t &&dominance_interval_begin,
    dominance_interval_end_t &&dominance_interval_end,
    depth_first_interval_begin_t &&depth_first_interval_begin,
    depth_first_interval_end_t &&depth_first_interval_end,
    const basic_forest_t<version_v, R> &edge_of_node_,
    visit_range_t &&visit_range) noexcept {
  ret.init(std::ranges::size(edge_of_node_.range()));
  for_each_unreducibles_given_depth_first_and_dominance_intervals(
      dominance_interval_begin, dominance_interval_end,
      depth_first_interval_begin, depth_first_interval_end, edge_of_node_,
      visit_range,
      [&](uintlen_t node, uintlen_t adj) noexcept {
        ret.find_and_unite_root(adj, node);
      },
      [](auto &&...) noexcept {});
}

template <version_t version_v,
          usable_index_range_c<version_v> dominance_interval_begin_t,
          usable_index_range_c<version_v> dominance_interval_end_t,
          usable_index_range_c<version_v> depth_first_interval_begin_t,
          usable_index_range_c<version_v> depth_first_interval_end_t,
          class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> visit_range_t>
MJZ_CX_FN uintlen_t count_unreducibles_given_dfs_and_dominance_intervals(
    dominance_interval_begin_t &&dominance_interval_begin,
    dominance_interval_end_t &&dominance_interval_end,
    depth_first_interval_begin_t &&depth_first_interval_begin,
    depth_first_interval_end_t &&depth_first_interval_end,
    const basic_forest_t<version_v, R> &edge_of_node_,
    visit_range_t &&visit_range) noexcept {
  return for_each_unreducibles_given_depth_first_and_dominance_intervals(
      dominance_interval_begin, dominance_interval_end,
      depth_first_interval_begin, depth_first_interval_end, edge_of_node_,
      visit_range, [](auto &&...) noexcept {}, [](auto &&...) noexcept {});
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_search_FILE_