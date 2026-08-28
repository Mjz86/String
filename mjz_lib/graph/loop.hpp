

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

#ifndef MJZ_SRC_GRAPH_loop_FILE_
#define MJZ_SRC_GRAPH_loop_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, usable_index_range_c<version_v> interval_begin_t,
          usable_index_range_c<version_v> interval_end_t,
          class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_natural_loops_given_dominance_intervals(
    interval_begin_t &&interval_begin, interval_end_t &&interval_end,
    const basic_forest_t<version_v, R1> &edge_of_node_,
    const basic_forest_t<version_v, R2> &preds_of_node) noexcept {
  auto adjs = edge_of_node_.range();
  auto preds = preds_of_node.range();
  treversal_result_t<version_v> ret{};

  uintlen_t total_node_count = std::ranges::size(adjs);
  uintlen_t max_natual_loop_count = std::ranges::size(edge_of_node_.edges);
  ret.nodes_index.reserve(max_natual_loop_count);
  ret.edges.reserve(total_node_count + max_natual_loop_count);

  auto dominates = [&](uintlen_t dom, uintlen_t sub) noexcept -> bool {
    if (interval_begin[dom] == 0 || interval_begin[sub] == 0)
      return false;
    return interval_begin[dom] <= interval_begin[sub] &&
           interval_end[dom] >= interval_end[sub];
  };

  auto work_stack = std::vector<uintlen_t>(total_node_count);
  uintlen_t work_stack_ptr{};
  for (uintlen_t u = 0; u < total_node_count; ++u) {
    for (uintlen_t v : adjs[u]) {
      if (!dominates(v, u))
        continue;
      uintlen_t loop_start_offset = ret.edges.size();
      ret.nodes_index.push_back(loop_start_offset);
      ret.edges.push_back(v);

      work_stack[v] |= 1;
      if (u != v) {
        work_stack[u] |= 1;
        work_stack[work_stack_ptr++] |= u << 1;
        ret.edges.push_back(u);
      }
      while (work_stack_ptr) {
        uintlen_t curr = work_stack[--work_stack_ptr] >> 1;
        work_stack[work_stack_ptr] &= 1;
        for (uintlen_t pred : preds[curr]) {
          if (work_stack[pred] & 1)
            continue;
          work_stack[pred] |= 1;
          work_stack[work_stack_ptr++] |= (pred << 1);
          ret.edges.push_back(pred);
        }
      }
      for (uintlen_t i :
           std::views::iota(loop_start_offset, ret.edges.size())) {
        work_stack[ret.edges[i]] = 0;
      }
    }
  }

  return ret;
}

template <version_t version_v,
          mutable_index_range_c<version_v> uninit_inner_most_loops_t,
          mutable_index_range_c<version_v> uninit_outer_most_loop_of_loop_t,
          mutable_index_range_c<version_v> uninit_nest_of_loop_t,
          class R = std::span<const uintlen_t>>
MJZ_CX_FN void calculate_nesting_of_natural_loops(
    const basic_forest_t<version_v, R> &natural_loops_,
    uninit_inner_most_loops_t &&inner_most_loops,
    uninit_outer_most_loop_of_loop_t &&outer_most_loop_of_loop,
    uninit_nest_of_loop_t &&uninit_nest_of_loop) noexcept {
  uintlen_t node_count = std::ranges::size(inner_most_loops);
  uintlen_t loop_count = std::ranges::size(outer_most_loop_of_loop);
  std::vector<uintlen_t> temporary(loop_count + node_count + 1, uintlen_t(-1));
  auto natural_loops = natural_loops_.range();
  auto linked_list_next =
      std::span<uintlen_t>(temporary).subspan(uintlen_t(), loop_count);
  auto linked_list_head =
      std::span<uintlen_t>(temporary).subspan(loop_count, node_count + 1);
  uintlen_t max_loop_size{};
  uintlen_t loop_index{};
  std::ranges::fill(inner_most_loops, uintlen_t(-1));

  for (auto &&loop : natural_loops) {
    uintlen_t loop_size = std::ranges::size(loop);
    uninit_nest_of_loop[loop_index] = 0;
    outer_most_loop_of_loop[loop_index] = loop_index;
    linked_list_next[loop_index] =
        std::exchange(linked_list_head[loop_size], loop_index++);
    max_loop_size = std::max(max_loop_size, loop_size);
  }
  uintlen_t current_loop_size{};
  for (uintlen_t head : linked_list_head) {
    if (max_loop_size < current_loop_size++)
      break;
    for (; head != uintlen_t(-1); head = linked_list_next[head]) {
      for (uintlen_t block : natural_loops[head]) {
        uintlen_t closest_loop = inner_most_loops[block];
        if (closest_loop == uintlen_t(-1)) {
          inner_most_loops[block] = head;
          continue;
        }
        if (head == std::exchange(outer_most_loop_of_loop[closest_loop], head))
          continue;
        uninit_nest_of_loop[closest_loop]++;
      }
    }
  }
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_loop_FILE_