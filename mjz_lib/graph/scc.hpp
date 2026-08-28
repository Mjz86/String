

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

#ifndef MJZ_SRC_GRAPH_scc_FILE_
#define MJZ_SRC_GRAPH_scc_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {
template <version_t version_v, class R = std::span<const uintlen_t>,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN uintlen_t
calculate_strongly_connected_components_with_order_get_scc_count(
    std::vector<uintlen_t> &ret,
    const basic_forest_t<version_v, R> &edge_of_node_,
    RIR_t &&entry_node_order) noexcept {

  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  uintlen_t ret_append_entact_sz = ret.size();
  uintlen_t max_size_total = total_node_count * 7;
  asserts(total_node_count <= max_size_total);
  asserts(max_size_total <= max_size_total + ret_append_entact_sz);
  //
  ret.resize(max_size_total + ret_append_entact_sz);
  //
  uintlen_t bumbp_alloc_ptr{ret_append_entact_sz};
  auto alloc_storage_fn_ = [&](uintlen_t sz) noexcept {
    bumbp_alloc_ptr += sz;
    asserts(bumbp_alloc_ptr <= ret_append_entact_sz + max_size_total);
    return bumbp_alloc_ptr - sz;
  };

  //
  auto alloc_ptr_fn_ = [&](uintlen_t sz) noexcept {
    return std::span(ret).subspan(alloc_storage_fn_(sz), sz);
  };
  //
  auto ret_reserve_span = alloc_ptr_fn_(total_node_count * 2);
  auto scc_index_span = ret_reserve_span.subspan(uintlen_t(), total_node_count);
  auto scc_elem_span =
      ret_reserve_span.subspan(total_node_count, total_node_count);
  uintlen_t scc_count{};
  uintlen_t scc_elem_count{};
  //
  auto frozen_ordenal = alloc_ptr_fn_(total_node_count);
  auto monotone_lowest_ahead = alloc_ptr_fn_(total_node_count);
  auto active_stack = alloc_ptr_fn_(total_node_count);
  auto call_stack = alloc_ptr_fn_(total_node_count * 2);
  uintlen_t call_stack_ptr{};
  uintlen_t active_stack_ptr{};
  uintlen_t monotonic_ordenal_counter = 0;

  for (uintlen_t parent_node_i : std::forward<RIR_t>(entry_node_order)) {
    asserts(parent_node_i < total_node_count);
    if (frozen_ordenal[parent_node_i] != uintlen_t())
      continue;
    uintlen_t u = parent_node_i;
    bool fresh_call{true};

    uintlen_t i{};
    do {
      if (fresh_call) {
        frozen_ordenal[u] = monotone_lowest_ahead[u] =
            ++monotonic_ordenal_counter;
        active_stack[active_stack_ptr++] = u;
        i = 0;
      }
      fresh_call = false;
      if (i < edge_of_node[u].size()) {
        uintlen_t candidate_node_index = edge_of_node[u][intlen_t(i)];
        if (frozen_ordenal[candidate_node_index] == uintlen_t()) {
          call_stack[call_stack_ptr++] = u;
          call_stack[call_stack_ptr++] = i;
          u = candidate_node_index;
          fresh_call = true;
          continue;
        } else if (0 < intlen_t(frozen_ordenal[candidate_node_index])) {
          monotone_lowest_ahead[u] = std::min(
              monotone_lowest_ahead[u], frozen_ordenal[candidate_node_index]);
        }
        i++;
        fresh_call = false;
        continue;
      }

      if (monotone_lowest_ahead[u] == frozen_ordenal[u]) {
        uintlen_t active_base_ptr = active_stack_ptr;
        while (active_stack[--active_base_ptr] != u)
          ;
        scc_index_span[scc_count++] = scc_elem_count;
        for (uintlen_t j = active_base_ptr; j < active_stack_ptr; j++) {
          uintlen_t node = scc_elem_span[scc_elem_count++] = active_stack[j];
          frozen_ordenal[node] = 1 + ~frozen_ordenal[node];
        }
        active_stack_ptr = active_base_ptr;
      }
      if (!call_stack_ptr)
        break;
      i = call_stack[--call_stack_ptr];
      u = call_stack[--call_stack_ptr];
      uintlen_t candidate_node_index = edge_of_node[u][intlen_t(i)];
      monotone_lowest_ahead[u] =
          std::min(monotone_lowest_ahead[u],
                   monotone_lowest_ahead[candidate_node_index]);
      i++;
    } while (true);
  }
  for (uintlen_t i{}; i < scc_elem_count; i++) {
    ret_reserve_span[i + scc_count] = scc_elem_span[i];
  }

  ret_append_entact_sz += scc_count + scc_elem_count;
  const bool plese_dont_emit_realoc = ret_append_entact_sz <= ret.size();
  asserts(plese_dont_emit_realoc);
  MJZ_JUST_ASSUME_(plese_dont_emit_realoc);
  if (plese_dont_emit_realoc) {
    ret.resize(ret_append_entact_sz);
  }
  return scc_count;
}

template <version_t version_v, class R = std::span<const uintlen_t>,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN basic_iota_forest_t<version_v>
calculate_strongly_connected_components_with_order_iota(
    std::vector<uintlen_t> &ret,
    const basic_forest_t<version_v, R> &edge_of_node_,
    RIR_t &&entry_node_order) noexcept {
  uintlen_t base_offset = ret.size();
  uintlen_t scc_count =
      calculate_strongly_connected_components_with_order_get_scc_count(
          ret, edge_of_node_, std::forward<RIR_t>(entry_node_order));
  uintlen_t elem_offset = base_offset + scc_count;
  return basic_iota_forest_t{
      .edges_index = basic_index_range_t<version_v>::from_the_iota_bounds(
          base_offset, elem_offset),
      .nodes_index_index = basic_index_range_t<version_v>::from_the_iota_bounds(
          elem_offset, ret.size())};
}

template <version_t version_v, class R = std::span<const uintlen_t>,
          index_range_of_c<version_v> RIR_t>
MJZ_CX_FN treversal_result_t<version_v>
calculate_strongly_connected_components_with_order(
    const basic_forest_t<version_v, R> &edge_of_node_,
    RIR_t &&entry_node_order) noexcept {
  treversal_result_t<version_v> ret{};
  uintlen_t base_offset = ret.nodes_index.size();
  uintlen_t scc_count =
      calculate_strongly_connected_components_with_order_get_scc_count(
          ret.nodes_index, edge_of_node_,
          std::forward<RIR_t>(entry_node_order));
  uintlen_t elem_offset = base_offset + scc_count;
  auto eds = std::span(ret.nodes_index).subspan(elem_offset);
  ret.edges = std::vector(eds.begin(), eds.end());
  ret.nodes_index.resize(elem_offset);
  return ret;
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_strongly_connected_components_with_entry(
    uintlen_t entry_node,
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  uintlen_t total_node_count = std::ranges::size(edge_of_node_.range());
  return calculate_strongly_connected_components_with_order(
      edge_of_node_,
      std::views::iota(entry_node, entry_node + total_node_count) |
          std::views::transform([total_node_count](uintlen_t i) noexcept {
            return i < total_node_count ? i : i - total_node_count;
          }));
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v> calculate_strongly_connected_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  uintlen_t total_node_count = std::ranges::size(edge_of_node_.range());
  return calculate_strongly_connected_components_with_order(
      edge_of_node_,
      std::views::iota(total_node_count ^ total_node_count, total_node_count));
}
} // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_scc_FILE_