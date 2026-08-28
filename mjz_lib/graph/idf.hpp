

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

#ifndef MJZ_SRC_GRAPH_idf_FILE_
#define MJZ_SRC_GRAPH_idf_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v,
          usable_index_range_c<version_v> immidiate_dominators_t,
          class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_inverse_dominance_frontiers_given_dominators(
    immidiate_dominators_t &&immidiate_dominators,
    const basic_forest_t<version_v, R> &preds_of_node) noexcept {
  uintlen_t total_node_count = std::ranges::size(preds_of_node.range());
  treversal_result_t<version_v> ret{};
  ret.nodes_index.reserve(total_node_count);
  ret.edges.reserve(total_node_count + std::ranges::size(preds_of_node.edges));

  auto preds = preds_of_node.range();
  auto current_inverse_frontiers =
      std::vector<uintlen_t>(total_node_count, uintlen_t(-1));

  for (uintlen_t y = 0; y < total_node_count; ++y) {
    uintlen_t current_inverse_frontiers_head = uintlen_t(-1);
    uintlen_t current_inverse_frontiers_count{};
    auto pred_range = preds[y];
    for (uintlen_t p : pred_range) {
      uintlen_t runner = p;
      while (runner != immidiate_dominators[y] && runner != uintlen_t(-1) &&
             current_inverse_frontiers[runner] == uintlen_t(-1)) {
        current_inverse_frontiers[runner] = current_inverse_frontiers_head;
        current_inverse_frontiers_head = runner;
        current_inverse_frontiers_count++;
        runner = immidiate_dominators[runner];
      }
    }

    uintlen_t offset_of_node = ret.edges.size();
    ret.nodes_index.push_back(offset_of_node);
    ret.edges.resize(offset_of_node + current_inverse_frontiers_count);
    while (current_inverse_frontiers_head != uintlen_t(-1)) {
      ret.edges[offset_of_node++] = current_inverse_frontiers_head;
      current_inverse_frontiers_head = std::exchange(
          current_inverse_frontiers[current_inverse_frontiers_head],
          uintlen_t(-1));
    }
  }

  return ret;
}

template <version_t version_v,
          usable_index_range_c<version_v> immidiate_dominators_t,
          class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_dominance_frontiers_given_dominators(
    immidiate_dominators_t &&immidiate_dominators,
    const basic_forest_t<version_v, R> &preds_of_node) noexcept {
  return make_basic_inv_forest<version_v>(
      calculate_inverse_dominance_frontiers_given_dominators(
          std::forward<immidiate_dominators_t>(immidiate_dominators),
          preds_of_node)
          .range());
}

template <version_t version_v,
          usable_index_range_c<version_v> immidiate_post_dominators_t,
          class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_dominance_frontiers_given_post_dominators(
    immidiate_post_dominators_t &&immidiate_post_dominators,
    const basic_forest_t<version_v, R> &edges_of_node) noexcept {
  return make_basic_inv_forest<version_v>(
      calculate_inverse_dominance_frontiers_given_dominators(
          std::forward<immidiate_post_dominators_t>(immidiate_post_dominators),
          edges_of_node)
          .range());
}

template <version_t version_v, class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> definion_range_t>
MJZ_CX_FN uintlen_t
calculate_iterated_dominance_frontiers_given_dominance_frontiers(
    std::vector<uintlen_t> &ret,
    const basic_forest_t<version_v, R> &dominance_frontiers_forest,
    definion_range_t &&definion_range) noexcept {
  auto dominance_frontiers_range = dominance_frontiers_forest.range();
  uintlen_t total_node_count = std::ranges::size(dominance_frontiers_range);
  if (!total_node_count) {
    return 0;
  }

  uintlen_t ret_append_entact_sz = ret.size();
  uintlen_t max_size_total = total_node_count * 2;
  asserts(total_node_count <= max_size_total);
  //
  ret.resize(max_size_total + ret_append_entact_sz);
  //
  uintlen_t bump_alloc_ptr{ret_append_entact_sz};
  auto alloc_ptr_fn_ = [&](uintlen_t sz) noexcept {
    uintlen_t old = bump_alloc_ptr;
    bump_alloc_ptr += sz;
    asserts(bump_alloc_ptr <= ret_append_entact_sz + max_size_total);
    return std::span(ret).subspan(old, sz);
  };

  auto iterated_dominance_frontiers = alloc_ptr_fn_(total_node_count);
  auto work_que = alloc_ptr_fn_(total_node_count);

  uintlen_t iterated_dominance_frontiers_count{};
  uintlen_t work_head{};
  uintlen_t work_tail{};

  for (uintlen_t node : std::forward<definion_range_t>(definion_range)) {
    if (node >= total_node_count || (work_que[node] & 1))
      continue;
    work_que[node] |= 1;
    work_que[work_tail++] |= node << 1;
  }

  while (work_head < work_tail) {
    for (uintlen_t node :
         dominance_frontiers_range[work_que[work_head++] >> 1]) {
      if ((iterated_dominance_frontiers[node] & 1))
        continue;
      iterated_dominance_frontiers[node] |= 1;
      iterated_dominance_frontiers[iterated_dominance_frontiers_count++] |=
          node << 1;

      if (work_que[node] & 1)
        continue;
      work_que[node] |= 1;
      work_que[work_tail++] |= node << 1;
    }
  }

  for (uintlen_t i = 0; i < iterated_dominance_frontiers_count; ++i) {
    ret[ret_append_entact_sz + i] = iterated_dominance_frontiers[i] >> 1;
  }

  ret_append_entact_sz += iterated_dominance_frontiers_count;
  const bool please_dont_emit_realloc = ret_append_entact_sz <= ret.size();
  asserts(please_dont_emit_realloc);
  MJZ_JUST_ASSUME_(please_dont_emit_realloc);
  if (please_dont_emit_realloc) {
    ret.resize(ret_append_entact_sz);
  }
  return iterated_dominance_frontiers_count;
}

template <version_t version_v, class R = std::span<const uintlen_t>,
          usable_index_range_c<version_v> definion_range_t>
MJZ_CX_FN std::vector<uintlen_t>
calculate_iterated_dominance_frontiers_given_dominance_frontiers(
    const basic_forest_t<version_v, R> &dominance_frontiers_forest,
    definion_range_t &&definion_range) noexcept {
  std::vector<uintlen_t> ret{};
  calculate_iterated_dominance_frontiers_given_dominance_frontiers(
      ret, dominance_frontiers_forest,
      std::forward<definion_range_t>(definion_range));
  return ret;
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_idf_FILE_