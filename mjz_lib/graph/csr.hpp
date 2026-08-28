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

#ifndef MJZ_SRC_GRAPH_CSR_FILE_
#define MJZ_SRC_GRAPH_CSR_FILE_
#include "base.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

// the densest way i know to pack a graph
template <version_t version_v, usable_index_range_c<version_v> T>
struct MJZ_maybe_trivially_relocatable basic_forest_t {
  T edges{};
  T nodes_index{};

  template <usable_index_range_c<version_v> R1,
            usable_index_range_c<version_v> R2>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  transform_impl_(R1 &&index_range, R2 &&inv_index_range) const noexcept;

  template <usable_index_range_c<version_v> R>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  transform(R &&index_range) const noexcept {
    return transform_impl_(index_range,
                           range_bijective_inv<version_v>(index_range));
  }
  template <usable_index_range_c<version_v> R>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  inv_transform(R &&index_range) const noexcept {
    return transform_impl_(range_bijective_inv<version_v>(index_range),
                           index_range);
  }
  MJZ_CX_FN auto basic_index_range() const noexcept {
    uintlen_t nodes_index_sz = std::ranges::size(nodes_index);
    return std::views::iota(uintlen_t(0), nodes_index_sz) |
           std::views::transform([this, nodes_index_sz,
                                  edges_sz = uintlen_t(std::ranges::size(
                                      edges))](uintlen_t node_i) noexcept {
             uintlen_t begin_index = uintlen_t(nodes_index[node_i]);
             bool in_node = node_i + 1 < nodes_index_sz;
             uintlen_t end_index = uintlen_t(nodes_index[node_i + in_node]);
             end_index = bit_branchless_teranary(in_node, end_index, edges_sz);
             return basic_index_range_t<version_v>{
                 .i = begin_index, .n = end_index - begin_index};
           });
  }

  MJZ_CX_FN auto index_range() const noexcept {
    return basic_index_range() |
           std::views::transform(
               [](basic_index_range_t<version_v> bir_) noexcept {
                 return bir_.iota();
               });
  }

  MJZ_CX_FN auto range() const noexcept {
    return basic_index_range() |
           std::views::transform(
               [it = std::ranges::begin(edges)](
                   basic_index_range_t<version_v> iotaview) noexcept {
                 auto [begin_index, end_index] = iotaview.bounds();
                 return std::ranges::subrange(it + intlen_t(begin_index),
                                              it + intlen_t(end_index));
               });
  }

  MJZ_CX_FN std::optional<pair_t<uintlen_t, uintlen_t>>
  get_edge_from_to_opt(uintlen_t edge_index) const noexcept {
    std::optional<uintlen_t> from_opt = get_edge_from_opt(edge_index);
    if (!from_opt)
      return std::nullopt;
    uintlen_t to_ =
        uintlen_t(*(std::ranges::begin(edges) + intlen_t(edge_index)));
    uintlen_t from_ = *from_opt;
    return pair_t{+from_, +to_};
  }
  MJZ_CX_FN pair_t<uintlen_t, uintlen_t>
  get_edge_from_to(uintlen_t edge_index) const noexcept {
    return *get_edge_from_to_opt(edge_index);
  }

  MJZ_CX_FN std::optional<uintlen_t>
  get_edge_to_opt(uintlen_t edge_index) const noexcept {
    if (edge_index < std::ranges::size(edges))
      return get_edge_to(edge_index);
    return std::nullopt;
  }
  MJZ_CX_FN uintlen_t get_edge_to(uintlen_t edge_index) const noexcept {
    return uintlen_t(*(std::ranges::begin(edges) + intlen_t(edge_index)));
  }
  MJZ_CX_FN std::optional<uintlen_t>
  get_edge_from_opt(uintlen_t edge_index) const noexcept {
    if (edge_index >= std::ranges::size(edges))
      return std::nullopt;
    auto it = std::ranges::upper_bound(nodes_index, edge_index);
    return uintlen_t(it - std::ranges::begin(nodes_index)) - 1;
  }
  MJZ_CX_FN uintlen_t get_edge_from(uintlen_t edge_index) const noexcept {
    return *get_edge_from_opt(edge_index);
  }

  MJZ_CX_FN static auto batchy_range_filter(uintlen_t min,
                                            uintlen_t max) noexcept {
    return std::views::filter([min, max](auto &&pan) noexcept {
      uintlen_t sz = std::ranges::size(pan);
      return min <= sz && sz < max;
    });
  }
  MJZ_CX_FN auto batchy_range(uintlen_t min, uintlen_t max) const noexcept {
    return range() | batchy_range_filter(min, max);
  }
  MJZ_CX_FN auto cyclic_range() const noexcept {
    return batchy_range(2, uintlen_t(-1));
  }
  MJZ_CX_FN auto acyclic_range() const noexcept { return batchy_range(0, 2); }
};

template <version_t version_v>
using treversal_result_t = basic_forest_t<version_v, std::vector<uintlen_t>>;

template <version_t version_v>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
make_basic_forest(const auto &range_of_range) noexcept {
  treversal_result_t<version_v> ret{};
  if constexpr (requires() {
                  { range_of_range } -> std::ranges::sized_range;
                }) {
    ret.nodes_index.reserve(std::ranges::size(range_of_range));
  }

  uintlen_t accumulate{};
  for (auto &&range : range_of_range) {
    accumulate += std::ranges::size(range);
  }
  ret.edges.reserve(accumulate);
  for (auto &&range : range_of_range) {
    ret.nodes_index.push_back(ret.edges.size());
    ret.edges.insert(ret.edges.end(), std::ranges::begin(range),
                     std::ranges::end(range));
  }
  return ret;
}
template <version_t version_v>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
make_basic_inv_forest(const auto &range_of_range) noexcept {
  treversal_result_t<version_v> ret{};

  static_assert(requires() {
    { range_of_range } -> std::ranges::sized_range;
  });
  ret.nodes_index =
      std::vector<uintlen_t>(std::ranges::size(range_of_range), 0);

  for (auto &&range : range_of_range) {
    for (uintlen_t edge : range) {
      ret.nodes_index[edge]++;
    }
  }
  uintlen_t accumulate{};
  for (uintlen_t &node_index : ret.nodes_index) {
    accumulate += node_index;
    node_index = accumulate;
  }
  ret.edges = std::vector<uintlen_t>(accumulate, 0);
  uintlen_t node_index{};

  for (auto &&range : range_of_range) {
    for (uintlen_t edge : range) {
      ret.edges[--ret.nodes_index[edge]] = node_index;
    }
    node_index++;
  }
  return ret;
}

template <version_t version_v, typename T>
MJZ_CX_FN pair_t<treversal_result_t<version_v>, std::vector<T>>
map_basic_forest_edges_data(const auto &range_of_range,
                            auto &&range_of_edge_datas) noexcept {
  pair_t<treversal_result_t<version_v>, std::vector<T>> ret_{};
  auto &[ret, dret] = ret_;
  if constexpr (requires() {
                  { range_of_range } -> std::ranges::sized_range;
                }) {
    ret.nodes_index.reserve(std::ranges::size(range_of_range));
  }

  uintlen_t accumulate{};
  for (auto &&range : range_of_range) {
    accumulate += std::ranges::size(range);
  }
  dret.reserve(accumulate);
  ret.edges.reserve(accumulate);
  auto &&itd = std::ranges::begin(range_of_edge_datas);
  for (auto &&range : range_of_range) {
    auto &&drange = *itd;
    ret.nodes_index.push_back(ret.edges.size());
    ret.edges.insert(ret.edges.end(), std::ranges::begin(range),
                     std::ranges::end(range));
    dret.insert(dret.end(), std::ranges::begin(drange),
                std::ranges::end(drange));
    (void)++itd;
  }
  return ret_;
}
template <version_t version_v, typename T>
MJZ_CX_FN pair_t<treversal_result_t<version_v>, std::vector<T>>
map_make_basic_inv_forest_edges_data(const auto &range_of_range,
                                     auto &&range_of_edge_datas) noexcept {
  pair_t<treversal_result_t<version_v>, std::vector<T>> ret_{};
  auto &[ret, dret] = ret_;

  static_assert(requires() {
    { range_of_range } -> std::ranges::sized_range;
  });
  ret.nodes_index =
      std::vector<uintlen_t>(std::ranges::size(range_of_range), 0);

  for (auto &&range : range_of_range) {
    for (uintlen_t edge : range) {
      ret.nodes_index[edge]++;
    }
  }
  uintlen_t accumulate{};
  for (uintlen_t &node_index : ret.nodes_index) {
    accumulate += node_index;
    node_index = accumulate;
  }
  ret.edges = std::vector<uintlen_t>(accumulate, 0);
  dret = std::vector<T>(accumulate);
  uintlen_t node_index{};

  auto itd = std::ranges::begin(range_of_edge_datas);
  for (auto &&range : range_of_range) {
    auto &&drange = *itd;
    auto &&ititd = std::ranges::begin(drange);
    for (uintlen_t edge : range) {
      uintlen_t rindex = --ret.nodes_index[edge];
      ret.edges[rindex] = node_index;
      dret[rindex] = std::ranges::iter_move(ititd);
      (void)++ititd;
    }
    node_index++;
    (void)++itd;
  }
  return ret_;
}

template <version_t version_v>
MJZ_CX_FN pair_t<treversal_result_t<version_v>, std::vector<uintlen_t>>
map_make_basic_inv_forest_edges_bijection(const auto &range_of_range) noexcept {
  pair_t<treversal_result_t<version_v>, std::vector<uintlen_t>> ret_{};
  auto &[ret, bijective_edge_map] = ret_;

  static_assert(requires() {
    { range_of_range } -> std::ranges::sized_range;
  });
  ret.nodes_index =
      std::vector<uintlen_t>(std::ranges::size(range_of_range), 0);

  for (auto &&range : range_of_range) {
    for (uintlen_t edge : range) {
      ret.nodes_index[edge]++;
    }
  }
  uintlen_t accumulate{};
  for (uintlen_t &node_index : ret.nodes_index) {
    accumulate += node_index;
    node_index = accumulate;
  }
  ret.edges = std::vector<uintlen_t>(accumulate, 0);
  bijective_edge_map = std::vector<uintlen_t>(accumulate);
  uintlen_t node_index{};

  uintlen_t edge_index{};
  for (auto &&range : range_of_range) {
    for (uintlen_t edge : range) {
      uintlen_t rindex = --ret.nodes_index[edge];
      ret.edges[rindex] = node_index;
      bijective_edge_map[rindex] = edge_index++;
    }
    node_index++;
  }
  return ret_;
}

template <version_t version_v, usable_index_range_c<version_v> T>
template <usable_index_range_c<version_v> R1,
          usable_index_range_c<version_v> R2>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
basic_forest_t<version_v, T>::transform_impl_(
    R1 &&index_range, R2 &&inverse_index_range) const noexcept {
  treversal_result_t<version_v> ret{};
  ret.nodes_index.reserve(std::ranges::size(nodes_index));
  ret.edges.reserve(std::ranges::size(edges));
  auto edge_view = range();
  for (uintlen_t i : std::forward<R1>(index_range)) {
    ret.nodes_index.push_back(ret.edges.size());
    auto range_ =
        edge_view[i] | std::views::transform([&](uintlen_t j) noexcept {
          return inverse_index_range[j];
        });
    ret.edges.insert(ret.edges.end(), std::ranges::begin(range_),
                     std::ranges::end(range_));
  }
  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> parents_t>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
make_forest_given_parents_inludes(parents_t &&parents,
                                  auto &&inludes_edge_fn) noexcept {
  const uintlen_t total_node_count = std::ranges::size(parents);
  treversal_result_t<version_v> ret{};
  ret.nodes_index = std::vector<uintlen_t>(total_node_count);
  uintlen_t node_index{};
  for (uintlen_t parent : parents) {
    if (inludes_edge_fn(+parent, +node_index)) {
      ret.nodes_index[parent]++;
    }
    node_index++;
  }

  uintlen_t accumulate{};
  for (uintlen_t &index_ : ret.nodes_index) {
    accumulate += index_;
    index_ = accumulate;
  }

  ret.edges = std::vector<uintlen_t>(accumulate);
  node_index = 0;
  for (uintlen_t parent : parents) {
    if (inludes_edge_fn(+parent, +node_index)) {
      ret.edges[--ret.nodes_index[parent]] = node_index;
    }
    node_index++;
  }
  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> parents_t>
MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
make_forest_given_parents(parents_t &&parents) noexcept {
  return make_forest_given_parents_inludes<version_v>(
      parents, [](auto &&...) noexcept { return true; });
}
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_CSR_FILE_