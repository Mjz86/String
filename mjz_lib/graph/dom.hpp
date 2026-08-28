

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

#ifndef MJZ_SRC_GRAPH_dom_FILE_
#define MJZ_SRC_GRAPH_dom_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN std::vector<uintlen_t> calculate_dominators_given_sccs_from_entry(
    uintlen_t entry_node, const basic_forest_t<version_v, R1> &scc_forest,
    const basic_forest_t<version_v, R2> &preds_of_node) noexcept {
  uintlen_t total_node_count = std::ranges::size(preds_of_node.range());
  std::vector<uintlen_t> monotonic_dominator_ordenal(total_node_count * 2,
                                                     uintlen_t(-1));
  auto frozen_ordenal = std::span<uintlen_t>(monotonic_dominator_ordenal)
                            .subspan(total_node_count);
  uintlen_t monotonic_ordenal_counter = 0;
  monotonic_dominator_ordenal[entry_node] = entry_node;
  frozen_ordenal[entry_node] = monotonic_ordenal_counter++;
  auto monotonic_scc_range = scc_forest.range() | std::views::reverse;
  for (auto &&scc : monotonic_scc_range) {
    for (uintlen_t node : scc) {
      if (node == entry_node)
        continue;
      frozen_ordenal[node] = monotonic_ordenal_counter++;
    }
  }
  auto preds = preds_of_node.range();
  for (auto &&scc : monotonic_scc_range) {
    bool changed = false;
    bool has_no_monotonic_progression = 1 < std::ranges::size(scc);
    do {
      changed = false;
      for (uintlen_t node_index : scc) {
        if (node_index == entry_node)
          continue;

        uintlen_t monotonic_dominator_ordenal_behind = uintlen_t(-1);

        for (uintlen_t behind_index : preds[node_index]) {
          bool behind_has_no_ordenal =
              monotonic_dominator_ordenal[behind_index] == uintlen_t(-1);
          if (behind_has_no_ordenal)
            continue;
          uintlen_t first_index_behind_monotonic_ordenal = behind_index;
          uintlen_t second_index_behind_monotonic_ordenal =
              monotonic_dominator_ordenal_behind;
          if (first_index_behind_monotonic_ordenal == uintlen_t(-1)) {
            monotonic_dominator_ordenal_behind =
                second_index_behind_monotonic_ordenal;
            continue;
          }
          if (second_index_behind_monotonic_ordenal == uintlen_t(-1)) {
            monotonic_dominator_ordenal_behind =
                first_index_behind_monotonic_ordenal;
            continue;
          }
          while (first_index_behind_monotonic_ordenal !=
                 second_index_behind_monotonic_ordenal) {
            while (frozen_ordenal[first_index_behind_monotonic_ordenal] >
                   frozen_ordenal[second_index_behind_monotonic_ordenal])
              first_index_behind_monotonic_ordenal = monotonic_dominator_ordenal
                  [first_index_behind_monotonic_ordenal];
            while (frozen_ordenal[second_index_behind_monotonic_ordenal] >
                   frozen_ordenal[first_index_behind_monotonic_ordenal])
              second_index_behind_monotonic_ordenal =
                  monotonic_dominator_ordenal
                      [second_index_behind_monotonic_ordenal];
          }
          monotonic_dominator_ordenal_behind =
              first_index_behind_monotonic_ordenal;
        }
        if (monotonic_dominator_ordenal[node_index] !=
            monotonic_dominator_ordenal_behind) {
          monotonic_dominator_ordenal[node_index] =
              monotonic_dominator_ordenal_behind;
          changed = has_no_monotonic_progression;
        }
      }
    } while (changed);
  }
  monotonic_dominator_ordenal.resize(total_node_count);

  return monotonic_dominator_ordenal;
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN void calculate_dominators_from_entry(
    std::vector<uintlen_t> &ret, uintlen_t entry_node,
    const basic_forest_t<version_v, R1> &edge_of_node_,
    const basic_forest_t<version_v, R2> &pred_of_node_) noexcept {
  auto adj = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(adj);
  uintlen_t ret_append_entact_sz = ret.size();
  if (!total_node_count) {
    return;
  }
  uintlen_t timer = 0;
  uintlen_t max_size_total = total_node_count * 8;
  asserts(total_node_count <= max_size_total);
  asserts(max_size_total <= max_size_total + ret_append_entact_sz);
  //
  ret.reserve(max_size_total + ret_append_entact_sz);
  ret.resize(total_node_count * 4 + ret_append_entact_sz, uintlen_t(-1));
  ret.resize(max_size_total + ret_append_entact_sz);
  //
  uintlen_t bumbp_alloc_ptr{ret_append_entact_sz};
  auto alloc_storage_fn_ = [&](uintlen_t sz) noexcept {
    bumbp_alloc_ptr += sz;
    asserts(bumbp_alloc_ptr <= ret_append_entact_sz + max_size_total);
    return bumbp_alloc_ptr - sz;
  };

  auto alloc_ptr_fn_ = [&](uintlen_t sz) noexcept {
    return std::span(ret).subspan(alloc_storage_fn_(sz), sz);
  };
  //
  auto immidiate_dominators = alloc_ptr_fn_(total_node_count);
  //
  auto linked_list_head = alloc_ptr_fn_(total_node_count);
  auto linked_list_next = alloc_ptr_fn_(total_node_count);
  auto semi_dominator_discovery_ordinal = alloc_ptr_fn_(total_node_count);
  //
  auto inverse_depth_first_ordenal = alloc_ptr_fn_(total_node_count);
  auto depth_first_parent = alloc_ptr_fn_(total_node_count);
  auto disjoint_set_union_label_path = alloc_ptr_fn_(total_node_count);
  auto disjoint_set_union_parent = alloc_ptr_fn_(total_node_count);
  // lifetimes dont overlap
  auto call_stack = disjoint_set_union_label_path;

  const auto pred = pred_of_node_.range();

  {
    uintlen_t current_node = entry_node;
    asserts(current_node < total_node_count);
    immidiate_dominators[current_node] = current_node;
    depth_first_parent[current_node] = uintlen_t(-1);
    uintlen_t call_stack_ptr{};
    bool fresh_call = true;
    uintlen_t i = 0;
    do {
      if (fresh_call) {
        inverse_depth_first_ordenal[timer] = current_node;
        semi_dominator_discovery_ordinal[current_node] = timer;
        i = 0;
        timer++;
      }
      if (i < std::ranges::size(adj[current_node])) {
        uintlen_t connection_node = adj[current_node][intlen_t(i)];
        if (uintlen_t(-1) !=
            semi_dominator_discovery_ordinal[connection_node]) {
          i++;
          fresh_call = false;
          continue;
        }
        depth_first_parent[connection_node] = current_node;
        call_stack[call_stack_ptr++] = i;
        i = 0;
        current_node = connection_node;
        fresh_call = true;
        continue;
      }
      if (!call_stack_ptr)
        break;
      i = call_stack[--call_stack_ptr];
      current_node = depth_first_parent[current_node];
      fresh_call = false;
      i++;
    } while (true);
    i = 0;
    for (i = 0; i < total_node_count; i++)
      disjoint_set_union_label_path[i] = disjoint_set_union_parent[i] = i;
  }

  auto disjoint_set_union_path_find =
      [&](const uintlen_t original_node) noexcept {
        uintlen_t root_node = original_node;
        uintlen_t prev_node = original_node;
        while (disjoint_set_union_parent[root_node] != root_node) {
          uintlen_t next = disjoint_set_union_parent[root_node];
          disjoint_set_union_parent[root_node] = prev_node;
          prev_node = root_node;
          root_node = next;
        }

        if (prev_node == root_node) {
          return disjoint_set_union_label_path[original_node];
        }

        while (prev_node != original_node) {
          uintlen_t next_down = disjoint_set_union_parent[prev_node];
          if (semi_dominator_discovery_ordinal
                  [disjoint_set_union_label_path[prev_node]] <
              semi_dominator_discovery_ordinal
                  [disjoint_set_union_label_path[next_down]]) {
            disjoint_set_union_label_path[next_down] =
                disjoint_set_union_label_path[prev_node];
          }
          disjoint_set_union_parent[prev_node] = root_node;
          prev_node = next_down;
        }

        disjoint_set_union_parent[original_node] = root_node;
        return disjoint_set_union_label_path[original_node];
      };
  for (uintlen_t k = timer - 1; 0 < k; k--) {

    uintlen_t current_ordered_node = inverse_depth_first_ordenal[k];

    for (uintlen_t connection_node : pred[current_ordered_node]) {
      if (uintlen_t(-1) == semi_dominator_discovery_ordinal[connection_node])
        continue;
      uintlen_t current_node = disjoint_set_union_path_find(connection_node);
      if (semi_dominator_discovery_ordinal[current_node] <
          semi_dominator_discovery_ordinal[current_ordered_node])
        semi_dominator_discovery_ordinal[current_ordered_node] =
            semi_dominator_discovery_ordinal[current_node];
    }
    linked_list_next[current_ordered_node] = std::exchange(
        linked_list_head
            [inverse_depth_first_ordenal
                 [semi_dominator_discovery_ordinal[current_ordered_node]]],
        +current_ordered_node);

    disjoint_set_union_parent[current_ordered_node] =
        depth_first_parent[current_ordered_node];
    for (uintlen_t connection_node = std::exchange(
             linked_list_head[depth_first_parent[current_ordered_node]],
             uintlen_t(-1));
         connection_node != uintlen_t(-1);
         connection_node = linked_list_next[connection_node]) {
      uintlen_t current_node = disjoint_set_union_path_find(connection_node);
      immidiate_dominators[connection_node] =
          semi_dominator_discovery_ordinal[current_node] <
                  semi_dominator_discovery_ordinal[connection_node]
              ? current_node
              : depth_first_parent[current_ordered_node];
    }
  }

  for (uintlen_t i = 1; i < timer; i++) {
    uintlen_t current_ordered_node = inverse_depth_first_ordenal[i];

    immidiate_dominators[current_ordered_node] =
        immidiate_dominators[current_ordered_node] !=
                inverse_depth_first_ordenal
                    [semi_dominator_discovery_ordinal[current_ordered_node]]
            ? immidiate_dominators[immidiate_dominators[current_ordered_node]]
            : immidiate_dominators[current_ordered_node];
  }
  ret_append_entact_sz += total_node_count;
  const bool plese_dont_emit_realoc = ret_append_entact_sz < ret.size();
  asserts(plese_dont_emit_realoc);
  MJZ_JUST_ASSUME_(plese_dont_emit_realoc);
  if (plese_dont_emit_realoc) {
    ret.resize(ret_append_entact_sz);
  }
  return;
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN std::vector<uintlen_t> calculate_post_dominators_from_exit(
    uintlen_t exit_node, const basic_forest_t<version_v, R1> &edge_of_node_,
    const basic_forest_t<version_v, R2> &pred_of_node_) noexcept {
  std::vector<uintlen_t> ret{};
  calculate_dominators_from_entry(ret, exit_node, pred_of_node_, edge_of_node_);
  return ret;
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN std::vector<uintlen_t> calculate_dominators_from_entry(
    uintlen_t entry_node, const basic_forest_t<version_v, R1> &edge_of_node_,
    const basic_forest_t<version_v, R2> &pred_of_node_) noexcept {
  std::vector<uintlen_t> ret{};
  calculate_dominators_from_entry(ret, entry_node, edge_of_node_,
                                  pred_of_node_);
  return ret;
}

template <version_t version_v,
          usable_index_range_c<version_v> immidiate_dominators_t>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
make_dominator_forest_given_dominators(
    immidiate_dominators_t &&immidiate_dominators) noexcept {
  return make_forest_given_parents_inludes<version_v>(
      immidiate_dominators,
      [](uintlen_t immidiate_dominator, uintlen_t node) noexcept {
        return immidiate_dominator != node &&
               immidiate_dominator != uintlen_t(-1);
      });
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_dom_FILE_