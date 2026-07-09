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

#ifndef MJZ_SRC_GRAPH_base_FILE_
#define MJZ_SRC_GRAPH_base_FILE_
#include "../byte_str/formatting/basic_formatters.hpp"
#include "../byte_str/formatting/format.hpp"
namespace mjz::graph_ns {
template <class T, version_t version_v>
concept dependency_state_c = requires(std::remove_cvref_t<T> s) {
  requires std::is_enum_v<std::remove_cvref_t<T>>;
};
template <version_t version_v>
struct MJZ_maybe_trivially_relocatable base_node_id_t {
private:
  uintlen_t value = uintlen_t(-1);

public:
  MJZ_CX_FN explicit operator bool() const noexcept {
    return value != uintlen_t(-1);
  }
  MJZ_CX_FN bool operator!() const noexcept { return value == uintlen_t(-1); }
  MJZ_CX_FN std::strong_ordering
  operator<=>(const base_node_id_t &) const noexcept = default;
  MJZ_CX_FN bool operator==(const base_node_id_t &) const noexcept = default;

  MJZ_CX_FN base_node_id_t() = default;
  MJZ_CX_FN uintlen_t index() const noexcept { return value; };
  MJZ_CX_FN explicit base_node_id_t(uintlen_t i) noexcept : value{i} {}
  MJZ_CX_FN uintlen_t
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return this->value;
  };
};
}; // namespace mjz::graph_ns

namespace mjz::graph_ns {
template <typename T>
  requires std::is_enum_v<T>
MJZ_CX_FN std::underlying_type_t<T> to_underlying(T e) noexcept {
  return static_cast<std::underlying_type_t<T>>(e);
}
template <version_t version_v>
MJZ_CX_FN decltype(auto)
state_to_str_impl_(dependency_state_c<version_v> auto e) noexcept {
  if constexpr (requires() {
                  {
                    state_to_str(e)
                  } noexcept -> mjz::bstr_ns::format_ns::
                      basic_format_specs_formatted_c<version_v>;
                }) {
    return state_to_str(e);
  } else {
    return to_underlying(e);
  }
}

template <version_t version_v,
          dependency_state_c<version_v> auto max_invalid_state_v>
struct MJZ_maybe_trivially_relocatable base_state_space_t {
  using states_e = std::remove_cvref_t<decltype(max_invalid_state_v)>;
  static_assert(!!to_underlying(max_invalid_state_v));
  states_e current{};
  states_e trigger{};
  MJZ_CX_ND_FN bool is_asleep() const noexcept {
    /*
    is_unrecoverable : latch forever error
    not is_incomplete: latch forever sucsuss
    is_triggered active : in-dgree became 0 , activate event.
    is_triggered passive: latch till we call defuse.
    is_asleep : non of the above
    */
    return !(!is_incomplete() || is_triggered() || is_unrecoverable());
  }
  MJZ_CX_ND_FN bool is_unrecoverable() const noexcept {
    if (max_invalid_state_v <= current && is_incomplete())
      MJZ_MOSTLY_UNLIKELY { return true; }
    return false;
  };
  MJZ_CX_ND_FN bool is_incomplete() const noexcept {
    return std::min(trigger, current) < max_invalid_state_v;
  };
  // a latch state
  MJZ_CX_ND_FN bool is_triggered() const noexcept {
    return trigger <= current && current < max_invalid_state_v;
  }
  MJZ_CX_ND_FN
  bool operator==(const base_state_space_t &) const noexcept = default;
  MJZ_CX_FN static std::unsigned_integral auto decay(states_e lhs) noexcept {
    return to_underlying(lhs);
  }
  MJZ_CX_FN static states_e next_state(states_e lhs) noexcept {
    if (lhs == max_invalid_state_v)
      return max_invalid_state_v;
    return states_e(decay(lhs) + 1);
  }
  MJZ_CX_FN success_t refresh(base_state_space_t fresh) noexcept {
    base_state_space_t me = *this;
    if (me.is_unrecoverable())
      MJZ_MOSTLY_UNLIKELY
    return false;
    if (fresh.current < me.current)
      MJZ_MOSTLY_UNLIKELY {
        return error(
            " the 'current' of a consumer must not exceed its provider ");
      }
    *this = fresh;
    return !fresh.is_unrecoverable();
  }
  MJZ_CX_ND_FN bool error(auto &&) noexcept {
    //  A then B is solved , but we suddenly changed A under B's feet to
    //  be for example C then A , but we have sent a signal already that A
    //  is ready
    trigger = states_e(decay(current = max_invalid_state_v) - 1);
    return false;
  }
  MJZ_CX_FN decltype(auto)
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return state_to_str_impl_<version_v>(current);
  };
};

template <version_t version_v,
          dependency_state_c<version_v> auto max_invalid_state_v>
struct MJZ_maybe_trivially_relocatable previous_states_t {
  using states_e =
      typename base_state_space_t<version_v, max_invalid_state_v>::states_e;
  base_node_id_t<version_v> id{};
  std::optional<states_e> forward{};
  std::optional<states_e> backward{};
};
template <version_t version_v,
          dependency_state_c<version_v> auto max_invalid_state_v>
struct MJZ_maybe_trivially_relocatable directed_state_space_t
    : base_state_space_t<version_v, max_invalid_state_v> {
  using base = base_state_space_t<version_v, max_invalid_state_v>;
  using node_id_t = base_node_id_t<version_v>;
  // can be more optimal but ok as is
  std::vector<node_id_t> connections{};
  // active_connections is latched to one of -2 or -1  when is_triggered,
  // no amount of connections.size() will go up to uintlen_t(-2).
  // active_connections grows until reinitilized.
  uintlen_t active_connections{};
  MJZ_CX_AL_FN bool refresh_dependancy_triggers(base old, base fresh) noexcept {
    base me = old;
    if (!me.refresh(fresh))
      return false;
    me = *this;
    if (fresh.current < me.trigger || me.trigger <= old.current)
      return false;

    return dec_zero_in_dgree();
  }
  MJZ_CX_AL_FN bool can_trigger() const noexcept {
    return !this->is_unrecoverable() && !this->is_triggered() &&
           active_connections == connections.size() && this->is_incomplete();
  }

  MJZ_CX_AL_FN success_t refresh_dependancy_defuse(base fresh) noexcept {
    base me = *this;
    if (fresh.is_unrecoverable())
      MJZ_MOSTLY_UNLIKELY { return false; }
    if (!fresh.is_incomplete()) {
      dec_zero_in_dgree();
      return true;
    }
    if (fresh.current < me.current)
      MJZ_MOSTLY_UNLIKELY {

        return this->error(
            " the 'current' of a consumer must not exceed its provider ");
      }
    if (fresh.current < me.trigger) {
      return true;
    }
    dec_zero_in_dgree();
    return true;
  }

  MJZ_CX_AL_FN success_t make_dependancy(base fresh, node_id_t id) noexcept {
    base me = *this;
    if (fresh.is_unrecoverable())
      MJZ_MOSTLY_UNLIKELY { return false; }
    if (!fresh.is_incomplete()) {
      dec_zero_in_dgree();
      connections.push_back(id);
      return true;
    }
    if (fresh.current < me.current)
      MJZ_MOSTLY_UNLIKELY {
        return this->error(
            " the 'current' of a consumer must not exceed its provider ");
      }
    connections.push_back(id);
    if (fresh.current < me.trigger) {
      return true;
    }
    dec_zero_in_dgree();
    return true;
  }
  MJZ_CX_ND_FN bool is_actively_triggered() const noexcept {
    return this->is_triggered() && active_connections == uintlen_t(-1);
  }
  MJZ_CX_ND_FN bool is_passively_triggered() const noexcept {
    return this->is_triggered() && active_connections == uintlen_t(-2);
  }
  MJZ_CX_AL_FN auto actively_trigger() noexcept {
    active_connections = uintlen_t(-1);
    return this->trigger = this->current;
  }
  MJZ_CX_AL_FN auto passively_trigger() noexcept {
    active_connections = uintlen_t(-2);
    return this->trigger = this->current;
  }

  MJZ_CX_AL_FN success_t defuse(base fresh) noexcept {
    if (fresh.is_triggered())
      MJZ_MOSTLY_UNLIKELY {
        return this->error(
            " any ignite must be eventually call defuse before the node "
            "or its dependancies can update , calling defuse with "
            "exepectation of an ignite is not allowed ");
      }
    active_connections = 0;
    return this->refresh(fresh);
  }

  MJZ_CX_AL_FN bool dec_zero_in_dgree() noexcept {
    if (this->is_triggered())
      return false;
    return ++active_connections == connections.size();
  }
  MJZ_CX_AL_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state_direct() const noexcept {
    base direc = *this;
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "current({}),trigger({}),active({}),needed({}),"
                       "connections({})">(),
        state_to_str_impl_<version_v>(direc.current),
        state_to_str_impl_<version_v>(direc.trigger),
        intlen_t(active_connections), connections.size(), connections);
  }
  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return format_node_state_direct();
  };
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

template <version_t version_v, class event_t, auto max_invalid_state_v>
struct MJZ_maybe_trivially_relocatable basic_dependency_graph_t {
public:
  using state_space_t = base_state_space_t<version_v, max_invalid_state_v>;
  // if max_invalid_state_v is in node_state , the node had a fatal error
  using states_e = state_space_t::states_e;

  using node_id_t = base_node_id_t<version_v>;
  ///////////////////////////////

  using node_direction_t =
      directed_state_space_t<version_v, max_invalid_state_v>;
  struct MJZ_maybe_trivially_relocatable dependency_node_t {
    std::array<node_direction_t, 2> directions{};
  };

private:
  std::vector<dependency_node_t> nodes{};
  uintlen_t expected_edges_per_node_v{0};
  std::vector<node_id_t> applied_list{};
  std::vector<node_id_t> apply_list{};
  std::vector<event_t> event_list{};
  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state(node_id_t me) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "[id({}),forward:({}),backward:({})]\n">(),
        me.index(), dependancy(me, true), dependancy(me, false));
  }

  MJZ_CX_AL_FN node_direction_t &dependancy(node_id_t me,
                                            bool direction) noexcept {
    return nodes[me.index()].directions[!direction];
  }
  MJZ_CX_AL_FN const node_direction_t &
  dependancy(node_id_t me, bool direction) const noexcept {
    return nodes[me.index()].directions[!direction];
  }
  MJZ_CX_AL_FN bool make_resolution_query(node_id_t id,
                                          bool direction) noexcept {
    node_direction_t &node_dependancy = dependancy(id, direction);
    node_direction_t &node_opposite = dependancy(id, !direction);
    if (!node_dependancy.can_trigger())
      return false;
    node_dependancy.actively_trigger();
    bool duplicate_event_request = node_opposite.is_actively_triggered();
    if (duplicate_event_request)
      return true;
    apply_list.push_back(id);
    return true;
  }
  MJZ_CX_AL_FN bool execute_resolution(node_id_t id, auto &...pram) noexcept {
    node_direction_t &node_forward = dependancy(id, true);
    node_direction_t &node_backward = dependancy(id, false);
    bool forward_active = node_forward.is_actively_triggered();
    bool backward_active = node_backward.is_actively_triggered();
    asserts(forward_active || backward_active);
    previous_states_t<version_v, max_invalid_state_v> signal{id};
    if (forward_active) {
      signal.forward = node_forward.passively_trigger();
    }
    if (backward_active) {
      signal.backward = node_backward.passively_trigger();
    }
    if constexpr (requires() { event_t(std::move(signal), pram...); }) {
      event_list.emplace_back(std::move(signal), pram...);
    } else {
      event_list.emplace_back(std::move(signal));
    }
    return true;
  }
  /*
  multithread constraints:
  if we want to multithread all work must be multithread done
  between ignite begin and join end. any write to *this (the graph) must be done
  exclusively, however if its read-only we can do that on all threads. the time
  frame from   create begin to create end  MUST NOT be multithreaded.
  also do NOT call into the run_resolution_queries! inside these.
  */
  MJZ_CX_AL_FN void execute_resolution_ignite(auto &...pram) noexcept {
    for (event_t &e : event_list) {
      static_assert(requires() {
        { e.ignite(pram...) } noexcept;
      });
      e.ignite(pram...);
    };
  } /*

  almost always we want to defuse the node to trigger the next events ,
  defuse is a graph mutating operation,
  to have a syncronous after the last node ends (without writing destructor
  boiler-plate) one can use defuse and call graph handle on it.

  note:
  btw theres a little qurirk in the engine and i love it :
  trigger freedom , assuming we dont go backward, defuse
  can launch a node from ast into code gen in 1 event, or make it stay in ast ,
  if it doesnt chage the current and puts trigger as next state of current ,
  it will automatically retrigger itself in the next event ( becuze it
  already was ready ) lol.
  it will just spin us until were bored , but notice , this is as if were
  waiting for an infinitley far behind dependancy to catch us , so the dependats
  are ok


  note:
  to effectively use this for parralel work,
  we can do read-only work inside ignite to join in multithreaded batches ,
  and flush the modification buffer of each batch in defuse,
  this way no lock is introduced,
  also , if any worker event does heavy work , it can yeild via my quirk of
  trigger, this way each wave is bounded to end pridictably and the thread pool
  will not be waiting that much. however this graph structure is thread
  agnostic.

  note:
  each defuse costs us O(indeg(v)+outdeg(v)) , v being our node, so ,
  each ignite to join should also cost at most O(deg(v)) , if it surpasses it
  must yeild , this way the cost of each yeild is never more than the cost of
  our work, and we can say , let K be the number of events in the wave, this way
  the time complexity of each wave is O(K+E_K), where E_K is all edges of the
  nodes in event K. while one might expect the cost of the defuse micro-tick to
  dominate, i theorize that a lock-free/locked implementation of this micro-tick
  will be slower than using a flush buffer,
  however this is all just my view, i may be wrong.

  */
  MJZ_CX_AL_FN void execute_resolution_join(auto &...pram) noexcept {
    MJZ_RAII_RELEASE { event_list.clear(); };

    for (event_t &e : event_list | std::views::reverse) {
      static_assert(requires() {
        { e.join(pram...) } noexcept;
      });
      e.join(pram...);
    };
    for (event_t &e : event_list | std::views::reverse) {
      static_assert(requires() {
        { std::move(e).defuse(*this, pram...) } noexcept;
      });
      std::move(e).defuse(*this, pram...);
    };
  }

  MJZ_CX_AL_FN bool defuse_resolution(node_id_t id, bool direction,
                                      state_space_t space) noexcept {

    node_direction_t &node_dependancy = dependancy(id, direction);
    node_direction_t &node_opposite = dependancy(id, !direction);
    for (node_id_t dep_i : node_dependancy.connections)
      mjz_prefetch(dependancy(dep_i, direction));
    for (node_id_t dep_i : node_opposite.connections)
      mjz_prefetch(dependancy(dep_i, direction));
    if (!node_dependancy.is_passively_triggered())
      return false;
    state_space_t old{node_dependancy};
    if (!node_dependancy.defuse(space))
      return false;
    //   Kahn's Algorithm , section of in-dgree re-initilization of node.
    state_space_t fresh{node_dependancy};

    for (node_id_t dep_i : node_dependancy.connections) {
      node_dependancy.refresh_dependancy_defuse(dependancy(dep_i, direction));
    };
    if (node_dependancy.is_incomplete())
      make_resolution_query(id, direction);
    if (is_unrecoverable(id))
      return false;

    //   Kahn's Algorithm , section of in-dgree 0  ( opposite direction  )
    //   dgree propagation.

    for (node_id_t dep_i : node_opposite.connections) {
      node_direction_t &node_dependant = dependancy(dep_i, direction);
      if (!node_dependant.refresh_dependancy_triggers(old, fresh))
        continue;
      make_resolution_query(dep_i, !!direction);
    };
    return !is_unrecoverable(id);
  }
  MJZ_CX_AL_FN bool events_running() const noexcept {
    return event_list.size();
  }
  MJZ_CX_FN bool run_resolution_queries(auto &&...pram) noexcept {
    asserts(!events_running());
    MJZ_RAII_RELEASE { asserts(!events_running()); };
    bool did_somthing = false;

    std::swap(applied_list, apply_list);
    {
      MJZ_RAII_RELEASE { execute_resolution_join(pram...); };
      for (node_id_t j : applied_list) {
        execute_resolution(j, pram...);
        did_somthing = true;
      }
      applied_list.clear();
      execute_resolution_ignite(pram...);
    }

    return did_somthing;
  };

  MJZ_CX_AL_FN success_t make_edge_impl2(bool direction, node_id_t dependancy_i,
                                         node_id_t dependant_i,
                                         uintlen_t extra_later) noexcept {
    if (extra_later &&
        extra_later + dependancy(dependant_i, direction).connections.size() >
            dependancy(dependant_i, direction).connections.capacity()) {
      extra_later = std::max<uintlen_t>(
          extra_later, dependancy(dependant_i, direction).connections.size());

      dependancy(dependant_i, direction)
          .connections.reserve(
              extra_later +
              dependancy(dependant_i, direction).connections.size());
    }
    return dependancy(dependant_i, direction)
        .make_dependancy(dependancy(dependancy_i, direction), dependancy_i);
  }

  MJZ_CX_FN success_t make_edge_impl(bool direction, node_id_t dependancy_i,
                                     node_id_t dependant_i,
                                     uintlen_t extra_later_dependancy,
                                     uintlen_t extra_later_dependant) noexcept {

    auto &a1 = mjz_prefetch(dependancy(dependant_i, true)).connections;
    auto &a2 = mjz_prefetch(dependancy(dependant_i, false)).connections;
    auto &a3 = mjz_prefetch(dependancy(dependancy_i, true)).connections;
    auto &a4 = mjz_prefetch(dependancy(dependancy_i, false)).connections;
    mjz_prefetch_p(a1.data() + a1.size());
    mjz_prefetch_p(a2.data() + a2.size());
    mjz_prefetch_p(a3.data() + a3.size());
    mjz_prefetch_p(a4.data() + a4.size());
    return make_edge_impl2(direction, dependancy_i, dependant_i,
                           extra_later_dependancy) &&
           make_edge_impl2(!direction, dependant_i, dependancy_i,
                           extra_later_dependant);
  }

  MJZ_CX_FN node_id_t make_node_impl(state_space_t forward_space,
                                     state_space_t backward_space,
                                     uintlen_t extra_later) noexcept {
    node_id_t i = node_id_t(nodes.size());
    if (extra_later && extra_later + nodes.size() > nodes.capacity()) {
      extra_later = std::max<uintlen_t>(extra_later, nodes.size());
      nodes.reserve(extra_later + nodes.size());
    }
    auto &node = nodes.emplace_back();
    if (expected_edges_per_node_v > 0) {
      for (auto &dirct : node.directions) {
        dirct.connections.reserve(expected_edges_per_node_v);
      }
    }
    node.directions[0].defuse(forward_space);
    node.directions[1].defuse(backward_space);
    return i;
  }
  MJZ_CX_AL_FN void reserve_impl(uintlen_t node_estimate,
                                 uintlen_t edge_estimate) noexcept {
    nodes.reserve(node_estimate);
    if (node_estimate > 0 && edge_estimate > 0) {
      expected_edges_per_node_v =
          (edge_estimate + node_estimate - 1) / node_estimate;
    }

    applied_list.reserve(node_estimate);
    apply_list.reserve(node_estimate);
    event_list.reserve(node_estimate);
  }

public:
  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate = 0) noexcept {
    return reserve_impl(node_estimate, edge_estimate);
  }
  MJZ_CX_ND_FN optional_ref_t<const dependency_node_t>
  get_node_cref(node_id_t i) const noexcept {
    if (is_unrecoverable(i))
      return {};
    return nodes[i.index()];
  }

  MJZ_CX_ND_FN std::optional<dependency_node_t>
  get_node(node_id_t i) const noexcept {
    if (is_unrecoverable(i))
      return {};
    return nodes[i.index()];
  }

  MJZ_CX_ND_FN mjz::bstr_ns::basic_str_t<version_v>
  format_graph_state() const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v, "{}">,
        std::views::iota(uintlen_t(), uintlen_t(nodes.size())) |
            std::views::transform([this](uintlen_t i) noexcept {
              return format_node_state(node_id_t(i));
            }));
  }

#if MJZ_WITH_iostream
  MJZ_NCX_FN friend std::ostream &
  operator<<(std::ostream &cout_v, const basic_dependency_graph_t &obj) {
    return cout_v << obj.format_graph_state();
  }
#endif

  MJZ_CX_ND_FN success_t
  make_edges(node_id_t id, std::span<const node_id_t> child_is) noexcept {
    uintlen_t extra_later = child_is.size();
    if (is_unrecoverable(id))
      return false;
    for (node_id_t child_i : child_is) {
      if (is_unrecoverable(child_i))
        return false;
      if (!make_edge_impl(true, id, child_i, extra_later, 0))
        return false;
      extra_later = 0;
    }
    return true;
  }
  MJZ_CX_ND_FN success_t make_edges(std::span<const node_id_t> ids,
                                    node_id_t child_i) noexcept {
    if (is_unrecoverable(child_i))
      return false;
    uintlen_t extra_later = ids.size();
    for (node_id_t id : ids) {
      if (is_unrecoverable(id))
        return false;
      if (!make_edge_impl(false, child_i, id, extra_later, 0))
        return false;
      extra_later = 0;
    }
    return true;
  }
  MJZ_CX_ND_FN success_t make_edge(node_id_t id, node_id_t child_i) noexcept {
    if (is_unrecoverable(child_i))
      return false;
    if (is_unrecoverable(id))
      return false;
    if (make_edge_impl(true, id, child_i, 0, 0))
      return true;
    return is_unrecoverable(child_i) || is_unrecoverable(id);
  }
  MJZ_CX_ND_FN node_id_t make_node(state_space_t forward_space,
                                   state_space_t backward_space) noexcept {
    return node_id_t(make_node_impl(forward_space, backward_space, 0));
  }
  MJZ_CX_FN void make_nodes(std::span<node_id_t> out_is,
                            state_space_t forward_space,
                            state_space_t backward_space) noexcept {
    uintlen_t extra_later = out_is.size();
    for (node_id_t &id_ : out_is) {
      id_ = make_node_impl(forward_space, backward_space, extra_later);
      extra_later = 0;
    }
  }
  MJZ_CX_ND_FN success_t query(node_id_t id, bool direction) noexcept {
    if (is_unrecoverable(id))
      return false;
    if (make_resolution_query(id, direction))
      return true;
    return !is_unrecoverable(id);
  }
  MJZ_CX_ND_FN success_t query_forward(node_id_t id) noexcept {
    return query(id, true);
  }
  MJZ_CX_ND_FN success_t query_backward(node_id_t id) noexcept {
    return query(id, false);
  }
  constexpr static inline state_space_t resolved_state_v =
      state_space_t{max_invalid_state_v, max_invalid_state_v};
  MJZ_CX_ND_FN success_t defuse(node_id_t id, state_space_t fresh,
                                bool direction) noexcept {
    if (is_unrecoverable(id))
      return false;
    if (defuse_resolution(id, direction, fresh))
      return true;
    return !is_unrecoverable(id);
  }
  MJZ_CX_ND_FN success_t defuse_forward(
      node_id_t id, state_space_t fresh = resolved_state_v) noexcept {
    return defuse(id, fresh, true);
  }
  MJZ_CX_ND_FN success_t defuse_backward(
      node_id_t id, state_space_t fresh = resolved_state_v) noexcept {
    return defuse(id, fresh, false);
  }

  MJZ_CX_ND_FN success_t query_bidirectional(node_id_t id) noexcept {
    return operator_or(query_forward(id), query_backward(id)) &&
           !is_unrecoverable(id);
  }
  MJZ_CX_ND_FN bool run_one(auto &&...pram) noexcept {
    return run_resolution_queries(pram...);
  }

  MJZ_CX_FN uintlen_t run_all(uintlen_t limit, auto &&...pram) noexcept {
    while (limit && run_resolution_queries(pram...))
      limit--;
    return limit;
  }

  MJZ_CX_ND_FN uintlen_t node_count() const noexcept { return nodes.size(); }
  MJZ_CX_FN tuple_t<uintlen_t, uintlen_t, uintlen_t>
  seed_all_nodes(bool forward_ = true, bool backward_ = true) noexcept {
    uintlen_t backward_seeds = 0;
    uintlen_t forward_seeds = 0;
    uintlen_t invalid_states = 0;
    for (uintlen_t i = 0; i < nodes.size(); ++i) {
      if (backward_) {
        backward_seeds += query_backward(node_id_t(i));
      }
      if (forward_) {
        forward_seeds += query_forward(node_id_t(i));
      }
      invalid_states += is_unrecoverable(node_id_t(i));
    }
    return {forward_seeds, backward_seeds, invalid_states};
  }
  MJZ_CX_ND_FN bool is_unrecoverable(node_id_t i, bool forward_ = true,
                                     bool backward_ = true) const noexcept {
    if (nodes.size() <= i.index())
      return true;
    auto &node = nodes[i.index()];
    return (backward_ && node.directions[1].is_unrecoverable()) ||
           (forward_ && node.directions[0].is_unrecoverable());
  }
  MJZ_CX_ND_FN bool is_unresolved(node_id_t i, bool forward_ = true,
                                  bool backward_ = true) const noexcept {
    if (is_unrecoverable(i, forward_, backward_))
      return true;
    auto &node = nodes[i.index()];
    return (backward_ && node.directions[1].is_incomplete()) ||
           (forward_ && node.directions[0].is_incomplete());
  }
  MJZ_CX_ND_FN auto view_all_fail_ids(bool forward_ = true,
                                      bool backward_ = true) const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(nodes.size())) |
           std::views::transform(
               [](uintlen_t i) noexcept { return node_id_t(i); }) |
           std::views::filter(
               [this, backward_, forward_](node_id_t i) noexcept {
                 return is_unresolved(i, forward_, backward_);
               });
  }

  MJZ_CX_FN auto basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return format_graph_state();
  };
};
}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_base_FILE_