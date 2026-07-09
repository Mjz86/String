
#ifndef MJZ_SRC_GRAPH_algo_FILE_
#define MJZ_SRC_GRAPH_algo_FILE_
#include "../byte_str/formatting/basic_formatters.hpp"
#include "../byte_str/formatting/format.hpp"
namespace mjz::graph_ns {
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

// the densest way i know to pack a graph
template <version_t version_v, typename T>
  requires(std::ranges::random_access_range<T> && std::ranges::sized_range<T> &&
           std::convertible_to<std::ranges::range_reference_t<T>, uintlen_t>)
struct MJZ_maybe_trivially_relocatable basic_forest_t {
  T edges{};
  // index into node vector
  T nodes_index{};

  MJZ_CX_FN auto range() const noexcept {
    uintlen_t nodes_index_sz = std::ranges::size(nodes_index);
    uintlen_t edges_sz = std::ranges::size(edges);
    return std::views::iota(uintlen_t(0), nodes_index_sz) |
           std::views::transform(
               [this, edges_sz, nodes_index_sz](uintlen_t node_i) noexcept {
                 uintlen_t begin_index = uintlen_t(nodes_index[node_i]);
                 uintlen_t end_index = (node_i + 1 < nodes_index_sz)
                                           ? uintlen_t(nodes_index[node_i + 1])
                                           : edges_sz;
                 auto it = std::ranges::begin(edges);
                 return std::ranges::subrange(it + intlen_t(begin_index),
                                              it + intlen_t(end_index));
               });
  }
  MJZ_CX_FN auto batchy_range(uintlen_t min, uintlen_t max) const noexcept {
    return range() | std::views::filter([min, max](auto &&pan) noexcept {
             uintlen_t sz = std::ranges::size(pan);
             return min <= sz && sz < max;
           });
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
        uintlen_t v = edge_of_node[u][i];
        // If v is not visited yet, then recur for it
        // Case 1: Tree edge
        if (frozen_ordenal[v] == uintlen_t()) {
          if (call_stack.size() == call_stack.capacity()) [[unlikely]] {
            std::terminate();
          }
          call_stack.push_back({u, i});
          u = v;
          i = 0;
          fresh_call = true;
          continue;
        }

        // Update monotone_lowest_ahead value of u only if v is still in stack
        // Case 2: Back edge (not cross edge)
        else if (is_active_set[v]) {
          monotone_lowest_ahead[u] =
              std::min(monotone_lowest_ahead[u], frozen_ordenal[v]);
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
      uintlen_t v = edge_of_node[u][i];
      // Check if the subtree rooted with v has a
      // connection to one of the ancestors of u
      monotone_lowest_ahead[u] =
          std::min(monotone_lowest_ahead[u], monotone_lowest_ahead[v]);
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
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_algo_FILE_