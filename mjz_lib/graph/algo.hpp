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

#ifndef MJZ_SRC_GRAPH_algo_FILE_
#define MJZ_SRC_GRAPH_algo_FILE_
#include "bijective.hpp"
#include "color.hpp"
#include "csr.hpp"
#include "dijkstra.hpp"
#include "dom.hpp"
#include "dsu.hpp"
#include "idf.hpp"
#include "loop.hpp"
#include "optimize.hpp"
#include "scc.hpp"
#include "search.hpp"
#include "sequence.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v> struct calculate_too_much_t {

  std::vector<uintlen_t> immidiate_post_dominators{};
  std::vector<uintlen_t> immidiate_dominators{};
  treversal_result_t<version_v> edge_of_node{};
  treversal_result_t<version_v> pred_of_node{};
  treversal_result_t<version_v> strongly_connected_components{};
  pair_t<std::vector<intlen_t>, intlen_t> sequenced_components{};

  template <class R = std::span<const uintlen_t>,
            index_range_of_c<version_v> RIR_t>
  MJZ_CX_AL_FN static calculate_too_much_t
  make(const auto &range_of_range) noexcept {
    calculate_too_much_t ret{};
    ret.edge_of_node = make_basic_forest<version_v>(range_of_range);
    ret.pred_of_node =
        make_basic_inv_forest<version_v>(ret.edge_of_node.range());
    ret.strongly_connected_components =
        calculate_strongly_connected_components(ret.edge_of_node);
    auto [scc_ordered, sec, wc] =
        calculate_graph_toposorted_sequenced_components(
            ret.strongly_connected_components, ret.edge_of_node);
    ret.strongly_connected_components = std::move(scc_ordered);
    ret.sequenced_components = pair_t{std::move(sec), wc};
    uintlen_t total_node_count = std::ranges::size(ret.edge_of_node.range());

    ret.edge_of_node.nodes_index.push_back(ret.edge_of_node.edges.size());
    auto scc_range = ret.strongly_connected_components.range();

    for (auto &&scc : scc_range) {
      uintlen_t representative = scc[0];
      intlen_t wave_num = ret.sequenced_components.first[representative];
      if (wave_num == 1 || wave_num == -1) {
        for (uintlen_t node : scc) {
          ret.edge_of_node.edges.push_back(node);
        }
      }
    }

    ret.immidiate_dominators = calculate_dominators_from_entry(
        total_node_count, ret.edge_of_node,
        make_basic_inv_forest<version_v>(ret.edge_of_node.range()));
    uintlen_t i{};
    for (uintlen_t &id : ret.immidiate_dominators) {
      if (id == total_node_count) {
        id = i;
      }
      i++;
    }
    ret.edge_of_node.edges.resize(ret.edge_of_node.nodes_index.back());
    ret.edge_of_node.nodes_index.pop_back();
    ret.immidiate_dominators.pop_back();
    return ret;
  }
};

///////////

}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_algo_FILE_
