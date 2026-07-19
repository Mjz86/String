
#ifndef MJZ_SRC_GRAPH_algo_FILE_
#define MJZ_SRC_GRAPH_algo_FILE_
#include "../releasers.hpp"
namespace mjz::graph_ns {
template <version_t version_v>
struct MJZ_trivially_relocatable basic_index_range_t {
  uintlen_t i{};
  uintlen_t n{};
  MJZ_CX_FN std::ranges::iota_view<uintlen_t, uintlen_t> iota() const noexcept {
    return std::views::iota(i, i + n);
  }
  MJZ_CX_FN pair_t<uintlen_t, uintlen_t> bounds() const noexcept {
    return {i, i + n};
  }
  MJZ_CX_FN static basic_index_range_t from_bounds(uintlen_t i,
                                                   uintlen_t j) noexcept {
    if (i > j)
      i = j;
    return {i, j - i};
  }
  MJZ_CX_FN static basic_index_range_t
  from_union(const basic_index_range_t &j,
             const basic_index_range_t &i) noexcept {
    auto [small1, big1] = j.bounds();
    auto [small2, big2] = i.bounds();
    return from_bounds(std::min(small1, small2), std::max(big1, big2));
  }
  MJZ_CX_FN static basic_index_range_t
  from_insersection(const basic_index_range_t &j,
                    const basic_index_range_t &i) noexcept {
    auto [small1, big1] = j.bounds();
    auto [small2, big2] = i.bounds();
    return from_bounds(std::max(small1, small2), std::min(big1, big2));
  }
  MJZ_CX_FN bool has_inside(uintlen_t canidate_node_index,
                            bool empty_overlaps = false) const noexcept {
    if (!empty_overlaps && !n)
      return false;
    return i <= canidate_node_index && canidate_node_index < i + n;
  }
  MJZ_CX_FN bool has_inside(const basic_index_range_t &r,
                            bool empty_overlaps = false) const noexcept {
    if (!empty_overlaps && (!n || !r.n))
      return false;
    return i <= r.i && r.i + r.n <= i + n;
  }
  MJZ_CX_FN bool overlaps(const basic_index_range_t &r,
                          bool empty_overlaps = false) const noexcept {
    if (!empty_overlaps && (!n || !r.n))
      return false;
    return has_inside(r.i, empty_overlaps) || r.has_inside(i, empty_overlaps) ||
           r.i == i;
  }

  MJZ_CX_FN static std::partial_ordering
  partial_compare_range(const basic_index_range_t &lhs,
                        const basic_index_range_t &rhs,
                        bool empty_overlaps = false) noexcept {
    if (lhs.overlaps(rhs, empty_overlaps))
      return std::partial_ordering::unordered;
    return lhs.i <=> rhs.i;
  };

  MJZ_CX_FN std::strong_ordering
  operator<=>(const basic_index_range_t &rhs) const noexcept = default;
  MJZ_CX_FN bool
  operator==(const basic_index_range_t &rhs) const noexcept = default;
  MJZ_CX_FN explicit operator bool() const noexcept { return !!n; }
};
// a bloated reachability checker , it probably should never be used.
template <version_t version_v>
struct MJZ_maybe_trivially_relocatable reachability_graph_t {
  std::vector<std::vector<bool>> reachable{};

  MJZ_CX_FN uintlen_t add_node() noexcept {
    uintlen_t i = reachable.size();
    for (std::vector<bool> &behinds : reachable) {
      behinds.push_back(false);
    }
    reachable.push_back(std::vector<bool>(i + 1, false));
    reachable[i][i] = true;
    return i;
  }

  MJZ_CX_FN bool has_directed_path(uintlen_t behind,
                                   uintlen_t ahead) const noexcept {
    return reachable[behind][ahead];
  }
  MJZ_CX_FN void add_directed_path(uintlen_t behind, uintlen_t ahead) noexcept {
    std::vector<bool> &behinds = reachable[behind];
    if (reachable[behind][ahead])
      return;
    behinds[ahead] = true;
    uintlen_t sz = reachable.size();
    for (uintlen_t i = 0; i < sz; ++i) {
      if (reachable[i][behind]) {
        for (uintlen_t j = 0; j < sz; ++j) {
          if (reachable[ahead][j]) {
            reachable[i][j] = true;
          }
        }
      }
    }
  }
};

template <typename T, class U, version_t version_v>
concept usable_type_range_exact_c =
    std::ranges::random_access_range<T> && std::ranges::sized_range<T> &&
    std::convertible_to<std::ranges::range_reference_t<T>, U>;
template <typename T, class U, version_t version_v>
concept usable_type_range_c =
    usable_type_range_exact_c<std::remove_cvref_t<T>, U, version_v>;

template <typename T, version_t version_v>
concept usable_index_range_exact_c =
    usable_type_range_exact_c<T, uintlen_t, version_v>;
template <typename T, version_t version_v>
concept usable_index_range_c =
    usable_index_range_exact_c<std::remove_cvref_t<T>, version_v>;

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> range_injective_inv(R &&index_range) noexcept {
  uintlen_t sz = std::ranges::size(index_range);
  std::vector<uintlen_t> inverse_index_range(sz);
  for (uintlen_t i : std::views::iota(uintlen_t(0), sz)) {
    inverse_index_range[index_range[i]] = i;
  }
  return inverse_index_range;
}

// the densest way i know to pack a graph
template <version_t version_v, usable_index_range_c<version_v> T>
struct MJZ_maybe_trivially_relocatable basic_forest_t {
  T edges{};
  // index into node vector
  T nodes_index{};

  template <usable_index_range_c<version_v> R1,
            usable_index_range_c<version_v> R2>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  transform_impl_(R1 &&index_range, R2 &&inv_index_range) const noexcept;

  template <usable_index_range_c<version_v> R>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  transform(R &&index_range) const noexcept {
    return transform_impl_(index_range,
                           range_injective_inv<version_v>(index_range));
  }
  template <usable_index_range_c<version_v> R>
  MJZ_CX_FN basic_forest_t<version_v, std::vector<uintlen_t>>
  inv_transform(R &&index_range) const noexcept {
    return transform_impl_(range_injective_inv<version_v>(index_range),
                           index_range);
  }

  MJZ_CX_FN auto range() const noexcept {
    uintlen_t nodes_index_sz = std::ranges::size(nodes_index);
    return std::views::iota(uintlen_t(0), nodes_index_sz) |
           std::views::transform([this, nodes_index_sz,
                                  edges_sz = uintlen_t(std::ranges::size(
                                      edges))](uintlen_t node_i) noexcept {
             uintlen_t begin_index = uintlen_t(nodes_index[node_i]);
             uintlen_t end_index = (node_i + 1 < nodes_index_sz)
                                       ? uintlen_t(nodes_index[node_i + 1])
                                       : edges_sz;
             auto it = std::ranges::begin(edges);
             return std::ranges::subrange(it + intlen_t(begin_index),
                                          it + intlen_t(end_index));
           });
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
  // for scc
  MJZ_CX_FN auto cyclic_range() const noexcept {
    return batchy_range(2, uintlen_t(-1));
  }

  // for scc
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
template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v> calculate_strongly_connected_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  // credit to
  // https://www.geeksforgeeks.org/dsa/tarjan-algorithm-find-strongly-connected-components/
  // i stil dont fully comprehend what i've optimized lol , maybe not?
  std::vector<uintlen_t> active_nodes{};
  treversal_result_t<version_v> ret{};
  std::vector<std::pair<uintlen_t, uintlen_t>> call_stack{};
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  active_nodes.reserve(total_node_count);
  call_stack.reserve(total_node_count);
  ret.nodes_index.reserve(total_node_count);
  ret.edges.reserve(total_node_count);
  auto frozen_ordenal = std::vector<uintlen_t>(total_node_count, uintlen_t());
  auto monotone_lowest_ahead =
      std::vector<uintlen_t>(total_node_count, uintlen_t());
  auto is_active_set = std::vector<bool>(total_node_count, false);
  uintlen_t monotonic_ordenal_counter = 0;

  // Call the recursive helper function to find SCCs
  // in DFS tree with vertex i
  for (uintlen_t j = 0; j < total_node_count; j++) {
    if (frozen_ordenal[j] != uintlen_t())
      continue;
    uintlen_t u = j;
    // Initialize discovery time and monotone_lowest_ahead value
    bool fresh_call{true};

    uintlen_t i{};
    do {
      if (fresh_call) {
        frozen_ordenal[u] = monotone_lowest_ahead[u] =
            ++monotonic_ordenal_counter;
        if (active_nodes.size() == active_nodes.capacity()) [[unlikely]] {
          std::terminate();
        }
        // Push current vertex to stack and mark it as in stack
        active_nodes.push_back(u);
        is_active_set[u] = true;
        i = 0;
      }
      fresh_call = false;
      // Go through all vertices adjacent to this
      if (i < edge_of_node[u].size()) {
        uintlen_t canidate_node_index = edge_of_node[u][i];
        // If canidate_node_index is not visited yet, then recur for it
        // Case 1: Tree edge
        if (frozen_ordenal[canidate_node_index] == uintlen_t()) {
          if (call_stack.size() == call_stack.capacity()) [[unlikely]] {
            std::terminate();
          }
          call_stack.push_back({u, i});
          u = canidate_node_index;
          i = 0;
          fresh_call = true;
          continue;
        }

        // Update monotone_lowest_ahead value of u only if canidate_node_index
        // is still in stack Case 2: Back edge (not cross edge)
        else if (is_active_set[canidate_node_index]) {
          monotone_lowest_ahead[u] = std::min(
              monotone_lowest_ahead[u], frozen_ordenal[canidate_node_index]);
        }
        i++;
        continue;
      }

      // If u is head node of SCC, pop the stack and store the SCC
      if (monotone_lowest_ahead[u] == frozen_ordenal[u]) {
        auto rev = active_nodes | std::views::reverse;
        auto scc_begin = std::ranges::find(rev, u);
        uintlen_t scc_size = uintlen_t(++scc_begin - rev.begin());
        auto scc_root_it = active_nodes.end() - ptrdiff_t(scc_size);
        for (uintlen_t k :
             std::ranges::subrange(scc_root_it, active_nodes.end()))
          is_active_set[k] = false;
        ret.nodes_index.push_back(ret.edges.size());
        ret.edges.insert(ret.edges.end(), scc_root_it, active_nodes.end());
        active_nodes.erase(scc_root_it, active_nodes.end());
        // Pop all vertices from stack till u is found
        // Store one strongly connected component
      }
      if (!call_stack.size())
        break;
      u = call_stack.back().first;
      i = call_stack.back().second;
      call_stack.pop_back();
      uintlen_t canidate_node_index = edge_of_node[u][i];
      // Check if the subtree rooted with canidate_node_index has a
      // connection to one of the ancestors of u
      monotone_lowest_ahead[u] = std::min(
          monotone_lowest_ahead[u], monotone_lowest_ahead[canidate_node_index]);
      i++;
    } while (true);
  }

  return ret;
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_graph_sequenced_components(
    const basic_forest_t<version_v, R> &scc_forest_,
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  auto scc_forest = scc_forest_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<uintlen_t> in_degree(std::ranges::size(scc_forest), uintlen_t(0));
  // scc index
  std::vector<uintlen_t> scc_indexies(total_node_count, uintlen_t(-1));
  std::vector<intlen_t> sequence_number(total_node_count, intlen_t());
  std::vector<uintlen_t> wave_stack{};
  std::vector<uintlen_t> next_wave_stack{};
  wave_stack.reserve(in_degree.size());
  next_wave_stack.reserve(in_degree.size());
  uintlen_t monotonic_id_counter{};
  for (auto &&scc : scc_forest) {
    for (uintlen_t node : scc) {
      scc_indexies[node] = monotonic_id_counter;
    }
    monotonic_id_counter++;
  }
  monotonic_id_counter = 0;
  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      if (scc_indexies[monotonic_id_counter] == scc_indexies[next])
        continue;
      in_degree[scc_indexies[next]]++;
    }
    monotonic_id_counter++;
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (!in_degree[i])
      next_wave_stack.push_back(i);
  }
  intlen_t wave_index_sequence{};
  while (next_wave_stack.size()) {
    wave_index_sequence++;
    wave_stack.clear();
    std::swap(next_wave_stack, wave_stack);
    for (uintlen_t i : wave_stack) {
      bool was_self_sequencal = false;
      for (uintlen_t node : scc_forest[i]) {
        for (uintlen_t next : edge_of_node[node]) {
          if (scc_indexies[node] == scc_indexies[next]) {
            was_self_sequencal = true;
            continue;
          }
          if (--in_degree[scc_indexies[next]])
            continue;
          next_wave_stack.push_back(scc_indexies[next]);
        }
      }
      for (uintlen_t node : scc_forest[i]) {
        sequence_number[node] =
            was_self_sequencal ? -wave_index_sequence : wave_index_sequence;
      }
    }
  };
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
calculate_acyclic_graph_sequenced_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<uintlen_t> in_degree(total_node_count, uintlen_t(0));
  // sequence_number of 0 means unsequenced
  std::vector<uintlen_t> sequence_number(total_node_count, uintlen_t());
  std::vector<uintlen_t> wave_stack{};
  std::vector<uintlen_t> next_wave_stack{};
  wave_stack.reserve(in_degree.size());
  next_wave_stack.reserve(in_degree.size());
  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      in_degree[next]++;
    }
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (!in_degree[i])
      next_wave_stack.push_back(i);
  }
  uintlen_t wave_index_sequence{};
  while (next_wave_stack.size()) {
    wave_index_sequence++;
    wave_stack.clear();
    std::swap(next_wave_stack, wave_stack);
    for (uintlen_t node : wave_stack) {
      for (uintlen_t next : edge_of_node[node]) {
        if (--in_degree[next])
          continue;
        next_wave_stack.push_back(next);
      }
      sequence_number[node] = wave_index_sequence;
    }
  };
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN std::vector<uintlen_t> calculate_dominators_from_entry_given_sccs(
    uintlen_t entry_node, const basic_forest_t<version_v, R> &scc_forest,
    const basic_forest_t<version_v, R> &preds_of_node) noexcept {
  uintlen_t total_node_count = std::ranges::size(preds_of_node.range());
  std::vector<uintlen_t> monotonic_dominator_ordenal(total_node_count,
                                                     uintlen_t(-1));
  monotonic_dominator_ordenal[entry_node] = entry_node;
  std::vector<uintlen_t> frozen_ordenal(total_node_count, uintlen_t(-1));
  uintlen_t monotonic_ordenal_counter = 0;
  auto monotonic_scc_range = scc_forest.range() | std::views::reverse;
  for (auto &&scc : monotonic_scc_range) {
    for (uintlen_t node : scc) {
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

  return monotonic_dominator_ordenal;
}

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> make_view_vector_range(R &&range_) noexcept {
  const uintlen_t sz = std::ranges::size(range_);
  std::vector<uintlen_t> ret{};
  ret.reserve(sz);
  for (uintlen_t val : range_) {
    ret.push_back(val);
  }
  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN auto original_order_transform(R &&transform_range_) noexcept {
  return std::views::transform(
      [transform_range = std::forward<R>(transform_range_)](
          uintlen_t i) noexcept { return transform_range[i]; });
}
template <version_t version_v>
MJZ_CX_FN std::vector<uintlen_t>
make_index_vector_range(const uintlen_t sz) noexcept {
  return make_view_vector_range<version_v>(std::views::iota(uintlen_t(0), sz));
}

template <version_t version_v>
MJZ_CX_FN std::vector<uintlen_t>
optimize_transformer_ordering(uintlen_t sz, auto &&index_transformer) noexcept {
  auto ret = make_index_vector_range<version_v>(sz);
  std::ranges::sort(ret, std::ranges::less{}, index_transformer);
  return ret;
}

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
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_graph_greedy_by_order(const basic_forest_t<version_v, T> &graph,
                            R &&order) noexcept {
  uintlen_t color_count{};
  auto adjs = graph.range();
  uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> graph_color(sz, uintlen_t(-1));
  std::vector<bool> temp_colors(sz, false);
  for (uintlen_t i : std::forward<R>(order)) {
    uintlen_t selected_color{color_count};
    for (uintlen_t j : adjs[i]) {
      if (graph_color[j] == uintlen_t(-1))
        continue;
      temp_colors[graph_color[j]] = true;
    }

    for (uintlen_t j{}; j < color_count; j++) {
      if (temp_colors[j])
        continue;
      selected_color = j;
      break;
    }
    for (uintlen_t j : adjs[i]) {
      if (graph_color[j] == uintlen_t(-1))
        continue;
      temp_colors[graph_color[j]] = false;
    }
    graph_color[i] = selected_color;
    color_count = std::max(color_count, selected_color + 1);
  }
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
  std::vector<uintlen_t> ret(timelines.size(), uintlen_t(-1));
  std::vector<uintlen_t> free_list{};
  uintlen_t color_count{};
  std::vector<pair_t<uintlen_t, uintlen_t>> intersected_time;
  intersected_time.reserve(timelines.size() * 2);
  free_list.reserve(timelines.size());
  for (uintlen_t i{}; i < timelines.size(); i++) {
    auto [begi, endi] = timelines[i].bounds();
    // empty timelines get colored with  uintlen_t(-1)
    if (begi == endi)
      continue;
    intersected_time.push_back({begi * 2 + 1, i});
    intersected_time.push_back({endi * 2, i});
  }
  std::ranges::sort(intersected_time);
  for (auto &&[time_, index] : intersected_time) {
    if (uintlen_t(-1) != ret[index]) {
      free_list.push_back(ret[index]);
      continue;
    }
    if (!free_list.size()) {
      free_list.push_back(color_count++);
    }
    ret[index] = free_list.back();
    free_list.pop_back();
  }
  return {ret, color_count};
}
template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, std::vector<intlen_t>>
optimize_maximum_cardinality_search_ordering_pair(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  auto adjs = suboptimal_indexed_graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> ret(sz, uintlen_t(-1));
  std::vector<intlen_t> priorities(sz, intlen_t(0));
  if (!sz)
    return {ret, priorities};
  uintlen_t edges_sz = std::ranges::size(suboptimal_indexed_graph.edges);

  struct linked_list_elem_t {
    uintlen_t back_index = uintlen_t(-1);
    uintlen_t elem_index = uintlen_t(-1);
  };
  std::vector<linked_list_elem_t> priority_ranks{};
  priority_ranks.reserve(sz * 2 + edges_sz);
  priority_ranks.resize(sz);
  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    linked_list_elem_t prev = priority_ranks[0];
    priority_ranks[0].back_index = priority_ranks.size();
    priority_ranks[0].elem_index = i;
    priority_ranks.push_back(prev);
  }
  uintlen_t highest_priority{};
  uintlen_t position_node_index{};
  while (position_node_index != sz) {
    while (priority_ranks[highest_priority].elem_index == uintlen_t(-1)) {
      if (!highest_priority)
        return {ret, priorities};
      --highest_priority;
    }
    linked_list_elem_t canidate_node_index = priority_ranks[highest_priority];
    priority_ranks[highest_priority] =
        priority_ranks[canidate_node_index.back_index];
    if (priorities[canidate_node_index.elem_index] < 0)
      continue;
    priorities[canidate_node_index.elem_index] =
        ~priorities[canidate_node_index.elem_index];
    ret[position_node_index++] = canidate_node_index.elem_index;
    for (uintlen_t neighbor :
         adjs[canidate_node_index.elem_index] | std::views::reverse) {
      if (priorities[neighbor] < 0)
        continue;
      priorities[neighbor]++;
      highest_priority =
          std::max(highest_priority, uintlen_t(priorities[neighbor]));
      uintlen_t prio_index = uintlen_t(priorities[neighbor]);
      linked_list_elem_t prev = priority_ranks[prio_index];
      priority_ranks[prio_index].back_index = priority_ranks.size();
      priority_ranks[prio_index].elem_index = neighbor;
      priority_ranks.push_back(prev);
    }
  }
  return {ret, priorities};
}

template <version_t version_v, class T>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_cardinality_search_ordering(
    const basic_forest_t<version_v, T> &graph) noexcept {
  return optimize_maximum_cardinality_search_ordering_pair(
             graph,
             std::views::iota(uintlen_t(),
                              uintlen_t(std::ranges::size(graph.range()))))
      .first;
}

template <version_t version_v, class T>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_chordal_graph(const basic_forest_t<version_v, T> &graph) noexcept {
  return color_graph_greedy_by_order(
      graph, optimize_maximum_cardinality_search_ordering(graph));
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_bucket_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph, R &&order,
    auto &&bucket_fn, uintlen_t bucket_max) noexcept {
  const uintlen_t sz = std::ranges::size(suboptimal_indexed_graph.range());
  std::vector<uintlen_t> ret(sz, uintlen_t(-1));

  if (!sz)
    return ret;
  struct linked_list_elem_t {
    uintlen_t back_index = uintlen_t(-1);
    uintlen_t elem_index = uintlen_t(-1);
  };
  std::vector<linked_list_elem_t> priority_ranks{};
  priority_ranks.resize(1 + bucket_max);
  uintlen_t max_deg{};
  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    uintlen_t deg = bucket_fn(i);
    max_deg = std::max(deg, max_deg);
    linked_list_elem_t prev = priority_ranks[deg];
    priority_ranks[deg].back_index = priority_ranks.size();
    priority_ranks[deg].elem_index = i;
    priority_ranks.push_back(prev);
  }
  uintlen_t highest_priority{max_deg};
  uintlen_t position_node_index{};
  while (position_node_index != sz) {
    while (priority_ranks[highest_priority].elem_index == uintlen_t(-1)) {
      if (!highest_priority)
        return ret;
      --highest_priority;
    }
    linked_list_elem_t canidate_node_index = priority_ranks[highest_priority];
    priority_ranks[highest_priority] =
        priority_ranks[canidate_node_index.back_index];

    ret[position_node_index++] = canidate_node_index.elem_index;
  }
  return ret;
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_dgree_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  return optimize_maximum_bucket_ordering(
      suboptimal_indexed_graph, order,
      [adjs = suboptimal_indexed_graph.range()](uintlen_t i) noexcept
          -> uintlen_t { return std::ranges::size(adjs[i]); },
      std::ranges::size(suboptimal_indexed_graph.edges));
}

template <version_t version_v, class T>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_dgree_ordering(
    const basic_forest_t<version_v, T> &graph) noexcept {
  return optimize_maximum_dgree_ordering(
      graph, std::views::iota(uintlen_t(),
                              uintlen_t(std::ranges::size(graph.range()))));
}

///////////

}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_algo_FILE_