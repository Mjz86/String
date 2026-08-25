
#ifndef MJZ_SRC_GRAPH_algo_FILE_
#define MJZ_SRC_GRAPH_algo_FILE_
#include "../releasers.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <typename T>
concept random_access_iterator_c =
    std::random_access_iterator<std::remove_cvref_t<T>>;

template <typename T, version_t version_v>
concept usable_range_exact_c =
    std::ranges::random_access_range<T> && std::ranges::sized_range<T>;
template <typename T, class U, version_t version_v>
concept usable_type_range_exact_c =
    usable_range_exact_c<T, version_v> &&
    std::convertible_to<std::ranges::range_reference_t<T>, U>;
template <typename T, class U, version_t version_v>
concept usable_type_range_c =
    usable_type_range_exact_c<std::remove_cvref_t<T>, U, version_v>;

template <typename T, version_t version_v>
concept index_range_of_c =
    std::ranges::range<T> &&
    std::convertible_to<std::ranges::range_reference_t<T>, uintlen_t>;

template <typename T, version_t version_v>
concept usable_index_range_exact_c =
    usable_type_range_exact_c<T, uintlen_t, version_v>;
template <typename T, version_t version_v>
concept usable_index_range_c =
    usable_index_range_exact_c<std::remove_cvref_t<T>, version_v>;

template <typename T, version_t version_v>
concept mutable_index_range_exact_c =
    usable_type_range_exact_c<T, uintlen_t &, version_v>;
template <typename T, version_t version_v>
concept mutable_index_range_c =
    mutable_index_range_exact_c<std::remove_cvref_t<T>, version_v>;

template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> range_bijective_inv(R &&index_range) noexcept {
  uintlen_t sz = std::ranges::size(index_range);
  std::vector<uintlen_t> inverse_index_range(sz);
  for (uintlen_t i : std::views::iota(uintlen_t(0), sz)) {
    inverse_index_range[index_range[i]] = i;
  }
  return inverse_index_range;
}

template <version_t version_v, usable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN std::vector<std::iter_value_t<it_t>>
make_reorder_bijective_transform_indexies(index_range_t &&map,
                                          it_t &&data) noexcept {
  std::vector<std::iter_value_t<it_t>> ret{};
  auto v =
      map | std::views::transform([&](uintlen_t i) noexcept -> decltype(auto) {
        return *(data + intlen_t(i));
      });
  ret.insert(ret.begin(), v.begin(), v.end());

  return ret;
}

template <version_t version_v, usable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN std::vector<std::iter_value_t<it_t>>
make_reorder_inv_bijective_transform_indexies(index_range_t &&map,
                                              it_t &&data) noexcept {
  return make_reorder_bijective_transform_indexies<version_v>(
      range_bijective_inv<version_v>(map), std::forward<it_t>(data));
}

template <version_t version_v, mutable_index_range_c<version_v> index_range_t,
          random_access_iterator_c it_t>
MJZ_CX_FN void
reorder_range_bijective_transform_indexies(index_range_t &&map,
                                           it_t &&data) noexcept {
  intlen_t sz = intlen_t(std::ranges::distance(map));
  auto mp = std::ranges::begin(map);
  for (intlen_t i{}; i < sz; i++) {
    intlen_t old_i = i;
    intlen_t new_i = intlen_t(*(mp + old_i));
    if (new_i == old_i)
      continue;
    std::remove_cvref_t<decltype(*data)> first = std::move(*(data + i));
    do {
      *(data + old_i) = std::move(*(data + new_i));
      *(mp + old_i) = uintlen_t(old_i);
      old_i = new_i;
      new_i = intlen_t(*(mp + old_i));
    } while (new_i != i);
    *(data + old_i) = std::move(first);
    *(mp + old_i) = uintlen_t(old_i);
  }
}
template <std::ranges::random_access_range it_range_t>
  requires std::random_access_iterator<std::ranges::range_value_t<it_range_t>>
MJZ_CX_FN void reorder_range_bijective_transform_iterators(
    it_range_t &&map,
    std::ranges::range_value_t<it_range_t> begining) noexcept {
  auto en = std::ranges::end(map);
  auto bg = std::ranges::begin(map);
  for (auto it = bg; it != en; ++it) {
    auto i = it - bg;
    auto old_i = i;
    auto new_i = *(bg + old_i) - begining;
    if (new_i == old_i)
      continue;
    auto first = std::move(*(begining + i));
    do {
      *(begining + old_i) = std::move(*(begining + new_i));
      *(bg + old_i) = old_i + begining;
      old_i = new_i;
      new_i = *(bg + old_i) - begining;
    } while (new_i != i);
    *(begining + old_i) = std::move(first);
    *(bg + old_i) = old_i + begining;
  }
}

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

template <version_t version_v, mutable_index_range_c<version_v> range_t>
struct disjoint_set_union_t {

  // 2x the size of this bad boy and it will give you ranking
  range_t parent_then_rank{};
  intlen_t m_range_size{};

  MJZ_CX_FN uintlen_t get_range_size() const noexcept {
    return uintlen_t(std::max(m_range_size, -m_range_size));
  }

  MJZ_CX_FN bool has_ranks() const noexcept { return 0 < m_range_size; }
  MJZ_CX_FN uintlen_t get_impl(uintlen_t i) const noexcept {
    return *(std::ranges::begin(parent_then_rank) + intlen_t(i));
  }
  MJZ_CX_FN void set_impl(uintlen_t i, uintlen_t v) noexcept {
    *(std::ranges::begin(parent_then_rank) + intlen_t(i)) = v;
  }
  MJZ_CX_FN uintlen_t get_parent(uintlen_t i) const noexcept {
    return get_impl(i);
  }
  MJZ_CX_FN uintlen_t get_rank(uintlen_t i) const noexcept {
    if (!has_ranks())
      return 0;
    return get_impl(get_range_size() + i);
  }
  MJZ_CX_FN void set_parent(uintlen_t i, uintlen_t parent) noexcept {
    set_impl(i, parent);
  }

  MJZ_CX_FN void set_rank(uintlen_t i, uintlen_t rank) noexcept {
    if (!has_ranks())
      return;
    set_impl(get_range_size() + i, rank);
  }
  MJZ_CX_FN void init(uintlen_t range_size_) noexcept {
    uintlen_t sz = std::ranges::size(parent_then_rank);
    asserts(range_size_ <= sz && range_size_ < (uintlen_t(-1) >> 1));
    m_range_size = range_size_ <= (sz >> 1) ? intlen_t(range_size_)
                                            : -intlen_t(range_size_);
    for (uintlen_t i : std::views::iota(uintlen_t(0), range_size_)) {
      set_parent(i, i);
      set_rank(i, 0);
    }
  }

  MJZ_CX_FN uintlen_t find_no_cache_root(uintlen_t child) const noexcept {
    uintlen_t root_node = child;
    while (true) {
      uintlen_t parent = get_parent(root_node);
      if (parent == std::exchange(root_node, parent))
        break;
    }
    return root_node;
  }
  MJZ_CX_FN bool is_cache_root_of_found_valid(uintlen_t root_node,
                                              uintlen_t child) const noexcept {
    return root_node == get_parent(child) && get_parent(root_node) == root_node;
  }
  MJZ_CX_FN uintlen_t cache_root_of_found(uintlen_t root_node,
                                          uintlen_t child) noexcept {
    while (true) {
      uintlen_t parent = get_parent(child);
      set_parent(child, root_node);
      if (parent == std::exchange(child, parent))
        break;
    };
    return root_node;
  }
  MJZ_CX_FN uintlen_t find_and_cache_root(uintlen_t child) noexcept {
    return cache_root_of_found(find_no_cache_root(child), child);
  }

  MJZ_CX_FN bool find_and_unite_root(uintlen_t i, uintlen_t j) noexcept {
    i = find_and_cache_root(i);
    j = find_and_cache_root(j);
    if (i == j) {
      return false;
    }

    uintlen_t rank_i = get_rank(i);
    uintlen_t rank_j = get_rank(j);

    bool i_child = rank_i < rank_j;
    bool tie = (rank_i == rank_j);

    uintlen_t child = i_child ? i : j;
    uintlen_t parent = i_child ? j : i;

    set_parent(child, parent);

    if (tie) {
      set_rank(parent, rank_i + 1);
    }

    return true;
  }

  MJZ_CX_FN bool rankless_find_and_unite_root(uintlen_t parent,
                                              uintlen_t child) noexcept {
    asserts(!has_ranks());
    parent = find_and_cache_root(parent);
    child = find_and_cache_root(child);
    if (parent == child) {
      return false;
    }
    set_parent(child, parent);
    return true;
  }

  MJZ_CX_FN bool is_united(uintlen_t parent, uintlen_t child) noexcept {
    return find_and_cache_root(parent) == find_and_cache_root(child);
  }
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

template <version_t version_v> struct basic_iota_forest_t {
  basic_index_range_t<version_v> edges_index{};
  basic_index_range_t<version_v> nodes_index_index{};
};
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
MJZ_CX_FN uintlen_t count_unreducibles_given_scc_and_dominance_intervals(
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

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_graph_sequenced_components(
    const basic_forest_t<version_v, R1> &scc_forest_,
    const basic_forest_t<version_v, R2> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  auto scc_forest = scc_forest_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  uintlen_t total_scc_count = std::ranges::size(scc_forest);
  std::vector<intlen_t> sequence_number(total_node_count * 2 +
                                        total_scc_count * 3);
  auto scc_indexies_compl =
      std::span(sequence_number).subspan(total_node_count, total_node_count);
  auto wave_stack =
      std::span(sequence_number).subspan(total_node_count * 2, total_scc_count);
  auto next_wave_stack =
      std::span(sequence_number)
          .subspan(total_node_count * 2 + total_scc_count, total_scc_count);
  auto in_degree =
      std::span(sequence_number)
          .subspan(total_node_count * 2 + total_scc_count * 2, total_scc_count);
  uintlen_t next_wave_stack_ptr{};
  uintlen_t wave_stack_ptr{};

  uintlen_t monotonic_id_counter{};
  for (auto &&scc : scc_forest) {
    for (uintlen_t node : scc) {
      scc_indexies_compl[node] = intlen_t(~monotonic_id_counter);
    }
    monotonic_id_counter++;
  }

  monotonic_id_counter = 0;
  for (auto &&nodes : edge_of_node) {
    uintlen_t src_scc = ~uintlen_t(scc_indexies_compl[monotonic_id_counter++]);
    if (src_scc == uintlen_t(-1)) {
      continue;
    }
    for (uintlen_t next : nodes) {
      uintlen_t dst_scc = ~uintlen_t(scc_indexies_compl[next]);
      if (dst_scc == uintlen_t(-1) || src_scc == dst_scc) {
        continue;
      }
      in_degree[dst_scc]++;
    }
  }

  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (!in_degree[i])
      next_wave_stack[next_wave_stack_ptr++] = intlen_t(i);
  }

  intlen_t wave_index_sequence{};
  while (next_wave_stack_ptr) {
    wave_index_sequence++;
    wave_stack_ptr = 0;
    std::swap(wave_stack_ptr, next_wave_stack_ptr);
    std::swap(next_wave_stack, wave_stack);
    for (intlen_t si : wave_stack.subspan(uintlen_t(), wave_stack_ptr)) {
      uintlen_t i = uintlen_t(si);
      bool was_self_sequencal = std::ranges::size(scc_forest[i]) > 1;
      for (uintlen_t node : scc_forest[i]) {
        for (uintlen_t next : edge_of_node[node]) {
          uintlen_t dst_scc = ~uintlen_t(scc_indexies_compl[next]);
          if (dst_scc == uintlen_t(-1)) {
            continue;
          }
          if (i == dst_scc) {
            was_self_sequencal = true;
            continue;
          }
          if (--in_degree[dst_scc])
            continue;
          next_wave_stack[next_wave_stack_ptr++] = intlen_t(dst_scc);
        }
        sequence_number[node] =
            was_self_sequencal ? -wave_index_sequence : wave_index_sequence;
      }
    }
  }
  sequence_number.resize(total_node_count);
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_acyclic_graph_sequenced_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<intlen_t> sequence_number(total_node_count * 4);
  auto in_degree =
      std::span(sequence_number).subspan(total_node_count, total_node_count);
  auto wave_stack = std::span(sequence_number)
                        .subspan(total_node_count * 2, total_node_count);
  auto next_wave_stack = std::span(sequence_number)
                             .subspan(total_node_count * 3, total_node_count);
  uintlen_t next_wave_stack_ptr{};
  uintlen_t wave_stack_ptr{};

  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      asserts(next < total_node_count);
      in_degree[next]++;
    }
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (in_degree[i])
      continue;
    next_wave_stack[next_wave_stack_ptr++] = intlen_t(i);
  }
  intlen_t wave_index_sequence{};
  while (next_wave_stack_ptr) {
    wave_index_sequence++;
    wave_stack_ptr = 0;
    std::swap(wave_stack_ptr, next_wave_stack_ptr);
    std::swap(next_wave_stack, wave_stack);
    for (intlen_t si : wave_stack.subspan(uintlen_t(), wave_stack_ptr)) {
      uintlen_t i = uintlen_t(si);
      for (uintlen_t next : edge_of_node[i]) {
        if (--in_degree[next])
          continue;
        next_wave_stack[next_wave_stack_ptr++] = intlen_t(next);
      }
      sequence_number[i] = wave_index_sequence;
    }
  }
  sequence_number.resize(total_node_count);
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
calculate_acyclic_graph_topological_sort_sequenced_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<uintlen_t> sequence_number(total_node_count * 3 + 1);
  auto wave_que = std::span(sequence_number)
                      .subspan(total_node_count, total_node_count + 1);
  auto in_degree = std::span(sequence_number)
                       .subspan(total_node_count * 2 + 1, total_node_count);
  uintlen_t tail{};
  uintlen_t head{};
  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      asserts(next < total_node_count);
      in_degree[next]++;
    }
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    wave_que[head] = i;
    head += !in_degree[i];
  }
  uintlen_t wave_index_sequence{};
  while (head != tail) {
    wave_index_sequence++;
    auto sp = wave_que.subspan(tail, head - tail);
    tail = head;
    for (uintlen_t i : sp) {
      for (uintlen_t next : edge_of_node[i]) {
        wave_que[head] = next;
        head += !--in_degree[next];
      }
      sequence_number[i] = wave_index_sequence;
    }
  }
  std::ranges::fill(wave_que.subspan(tail, total_node_count - tail),
                    uintlen_t(-1));
  sequence_number.resize(total_node_count * 2);
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
calculate_acyclic_graph_topological_sort_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<uintlen_t> wave_que(total_node_count * 2 + 1);
  auto in_degree =
      std::span(wave_que).subspan(total_node_count + 1, total_node_count);
  uintlen_t tail{};
  uintlen_t head{};
  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      asserts(next < total_node_count);
      in_degree[next]++;
    }
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    wave_que[head] = i;
    head += !in_degree[i];
  }
  uintlen_t wave_index_sequence{};
  while (head != tail) {
    wave_index_sequence++;
    auto sp = std::span(wave_que).subspan(tail, head - tail);
    tail = head;
    for (uintlen_t i : sp) {
      for (uintlen_t next : edge_of_node[i]) {
        wave_que[head] = next;
        head += !--in_degree[next];
      }
    }
  }
  std::ranges::fill(std::span(wave_que).subspan(tail, total_node_count - tail),
                    uintlen_t(-1));
  wave_que.resize(total_node_count);
  return {std::move(wave_que), wave_index_sequence};
}

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
  make_with_order(const auto &range_of_range,
                  RIR_t &&entry_node_order) noexcept {
    calculate_too_much_t ret{};
    ret.edge_of_node = make_basic_forest<version_v>(range_of_range);
    ret.pred_of_node =
        make_basic_inv_forest<version_v>(ret.edge_of_node.range());
    ret.strongly_connected_components =
        calculate_strongly_connected_components_with_order(
            ret.edge_of_node, std::forward<RIR_t>(entry_node_order));
    ret.sequenced_components = calculate_graph_sequenced_components(
        ret.strongly_connected_components, ret.edge_of_node);
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
  template <class R = std::span<const uintlen_t>>
  MJZ_CX_FN static calculate_too_much_t
  make(const auto &range_of_range) noexcept {
    return make_with_order(
        range_of_range,
        std::views::iota(uintlen_t(), std::ranges::size(range_of_range)));
  }
};
template <version_t version_v, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> make_view_vector_range(R &&range_) noexcept {
  return std::vector<uintlen_t>(std::ranges::begin(range_),
                                std::ranges::end(range_));
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
  for (uintlen_t i : std::forward<R>(order)) {
    uintlen_t selected_color{color_count};
    for (uintlen_t j : adjs[i]) {
      if ((graph_color[j] | 1) == uintlen_t(-1))
        continue;
      graph_color[graph_color[j] >> 1] &= ~uintlen_t(1);
    }

    for (uintlen_t j{}; j < color_count; j++) {
      if (!(graph_color[j] & 1))
        continue;
      selected_color = j;
      break;
    }
    for (uintlen_t j : adjs[i]) {
      if ((graph_color[j] | 1) == uintlen_t(-1))
        continue;
      graph_color[graph_color[j] >> 1] |= uintlen_t(1);
    }
    graph_color[i] = (selected_color << 1) | 1;
    color_count = std::max(color_count, selected_color + 1);
  }
  for (uintlen_t &c : graph_color)
    c = uintlen_t(intlen_t(c) >> 1);
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
  // empty timelines get colored with  uintlen_t(-1)
  std::vector<uintlen_t> ret(timelines.size() * 4, uintlen_t(-1));
  uintlen_t bumbp_alloc_ptr{timelines.size()};
  auto alloc_ptr_fn_ = [&](uintlen_t sz) noexcept {
    bumbp_alloc_ptr += sz;
    asserts(bumbp_alloc_ptr <= ret.size());
    return std::span(ret).subspan(bumbp_alloc_ptr - sz, sz);
  };
  auto free_list = alloc_ptr_fn_(timelines.size());
  auto intersected_time = alloc_ptr_fn_(timelines.size() * 2);
  uintlen_t color_count{};
  uintlen_t free_list_ptr{};
  uintlen_t intersected_time_ptr{};

  for (uintlen_t i{}; i < timelines.size(); i++) {
    if (!timelines[i].n)
      continue;
    intersected_time[intersected_time_ptr++] = (i << 1) | 1;
    intersected_time[intersected_time_ptr++] = (i << 1);
  }

  auto intersected_time_slice =
      intersected_time.subspan(uintlen_t(), intersected_time_ptr);
  std::ranges::sort(
      intersected_time_slice, std::ranges::less{}, [&](uintlen_t i) noexcept {
        auto [begi, endi] = timelines[i >> 1].bounds();
        return (i & 1) ? pair_t{begi * 2 + 1, i} : pair_t{endi * 2, i};
      });
  for (uintlen_t index_and_bit : intersected_time_slice) {
    uintlen_t index = index_and_bit >> 1;
    if (!(index_and_bit & 1)) {
      free_list[free_list_ptr++] = ret[index];
      continue;
    }
    free_list[free_list_ptr] = color_count;
    color_count += !free_list_ptr;
    free_list_ptr += !free_list_ptr;
    ret[index] = free_list[--free_list_ptr];
  }
  ret.resize(timelines.size());
  return {std::move(ret), color_count};
}
template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_cardinality_search_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  auto adjs = suboptimal_indexed_graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> ret(sz * 4, uintlen_t(-1));
  if (!sz)
    return ret;
  auto priority_list_next = std::span<uintlen_t>(ret).subspan(sz, sz);
  auto priority_list_head = std::span<uintlen_t>(ret).subspan(sz * 2, sz);
  auto priority_of_node_compl = std::span<uintlen_t>(ret).subspan(sz * 3, sz);

  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    priority_list_next[i] = std::exchange(priority_list_head[0], i);
  }
  uintlen_t highest_priority{};
  uintlen_t position_node_index{};
  MJZ_RAII_RELEASE { ret.resize(position_node_index); };
  while (position_node_index != sz) {
    while (priority_list_head[highest_priority] == uintlen_t(-1)) {
      if (!highest_priority)
        return (ret);
      --highest_priority;
    }
    uintlen_t candidate_node_index = priority_list_head[highest_priority];
    priority_list_head[highest_priority] =
        priority_list_next[candidate_node_index];
    if (!priority_of_node_compl[candidate_node_index])
      continue;
    priority_of_node_compl[candidate_node_index] = 0;
    ret[position_node_index++] = candidate_node_index;
    for (uintlen_t neighbor :
         adjs[candidate_node_index] | std::views::reverse) {
      if (!priority_of_node_compl[neighbor])
        continue;
      uintlen_t prio_index = ~--priority_of_node_compl[neighbor];
      highest_priority = std::max(highest_priority, prio_index);
      priority_list_next[neighbor] =
          std::exchange(priority_list_head[prio_index], neighbor);
    }
  }
  return (ret);
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_bucket_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph, R &&order,
    auto &&bucket_fn, uintlen_t bucket_max) noexcept {
  auto adjs = suboptimal_indexed_graph.range();
  const uintlen_t sz = std::ranges::size(adjs);
  std::vector<uintlen_t> ret(sz * 2 + 1 + bucket_max, uintlen_t(-1));
  if (!sz)
    return ret;
  uintlen_t position_node_index{};
  MJZ_RAII_RELEASE { ret.resize(position_node_index); };
  auto priority_list_next = std::span<uintlen_t>(ret).subspan(sz, sz);
  auto priority_list_head = std::span<uintlen_t>(ret).subspan(sz * 2);

  uintlen_t max_deg{};
  for (uintlen_t i : std::forward<R>(order) | std::views::reverse) {
    uintlen_t deg = bucket_fn(i);
    asserts(deg <= bucket_max);
    max_deg = std::max(deg, max_deg);
    priority_list_next[i] = std::exchange(priority_list_head[deg], i);
  }
  uintlen_t highest_priority{max_deg};
  while (position_node_index != sz) {
    while (priority_list_head[highest_priority] == uintlen_t(-1)) {
      if (!highest_priority)
        return ret;
      --highest_priority;
    }
    uintlen_t candidate_node_index = priority_list_head[highest_priority];
    priority_list_head[highest_priority] =
        priority_list_next[candidate_node_index];
    ret[position_node_index++] = candidate_node_index;
  }
  return ret;
}

template <version_t version_v, class T>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
color_chordal_graph(const basic_forest_t<version_v, T> &graph) noexcept {
  return color_graph_greedy_by_order(
      graph, optimize_maximum_cardinality_search_ordering(graph));
}

template <version_t version_v, class T, usable_index_range_c<version_v> R>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_degree_ordering(
    const basic_forest_t<version_v, T> &suboptimal_indexed_graph,
    R &&order) noexcept {
  return optimize_maximum_bucket_ordering(
      suboptimal_indexed_graph, order,
      [adjs = suboptimal_indexed_graph.range()](uintlen_t i) noexcept
          -> uintlen_t { return std::ranges::size(adjs[i]); },
      std::ranges::size(suboptimal_indexed_graph.edges));
}

template <version_t version_v, class T>
MJZ_CX_FN std::vector<uintlen_t> optimize_maximum_degree_ordering(
    const basic_forest_t<version_v, T> &graph) noexcept {
  return optimize_maximum_degree_ordering(
      graph, std::views::iota(uintlen_t(),
                              uintlen_t(std::ranges::size(graph.range()))));
}

///////////

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t>
MJZ_CX_FN auto calculate_shortest_positive_path_distanced(
    uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
    edge_wights_t &&weight_of_adjust) noexcept {
  uintlen_t esz = std::ranges::size(graph.edges);
  uintlen_t nsz = std::ranges::size(graph.nodes_index);
  auto it_edge_node = std::ranges::begin(graph.edges);
  auto it_weights = std::ranges::begin(weight_of_adjust);
  auto edge_indexies = graph.index_range();
  auto edge_weight = [&](uintlen_t edge_i) noexcept -> decltype(auto) {
    return *(it_weights + intlen_t(edge_i));
  };
  auto edge_node = [&](uintlen_t edge_i) noexcept {
    return *(it_edge_node + intlen_t(edge_i));
  };
  using dist_t = std::remove_cvref_t<decltype(edge_weight(0))>;

  static_assert(std::floating_point<dist_t> || std::unsigned_integral<dist_t> ||
                requires() {
                  typename dist_t::mjz_uintN_t_id_val_t_2354675648764874753789;
                });
  asserts(esz == std::ranges::size(weight_of_adjust) && entry_index < nsz);
  auto ret = make_index_vector_range<version_v>(nsz + esz);

  dist_t max_value{};
  if constexpr (std::floating_point<dist_t>) {
    max_value = std::numeric_limits<dist_t>::infinity();
    static_assert(std::numeric_limits<dist_t>::has_infinity);
  } else {
    max_value = dist_t(~dist_t());
  }

  std::vector<dist_t> distance_metic_(nsz + esz, max_value);
  auto node_distance = std::span<dist_t>(distance_metic_).subspan(0, nsz);
  auto edge_distance = std::span<dist_t>(distance_metic_).subspan(nsz, esz);
  node_distance[entry_index] = dist_t();
  uintlen_t heap_count{};
  std::span<uintlen_t> parents = std::span(ret).subspan(0, nsz);
  std::span<uintlen_t> heap = std::span(ret).subspan(nsz, esz);
  auto comp = [&](uintlen_t edge_il, uintlen_t edge_ir) noexcept {
    return edge_distance[edge_il] > edge_distance[edge_ir];
  };
  uintlen_t node_i{entry_index};
  while (true) {
    for (uintlen_t edge_i : edge_indexies[node_i]) {
      const dist_t edge_weight_ = edge_weight(edge_i);
      const dist_t edge_dist = edge_distance[edge_i] =
          dist_t(edge_weight_ + node_distance[node_i]);
      if constexpr (std::floating_point<dist_t>) {
        asserts(dist_t() <= edge_weight_, "no negative / NAN weight ");
      } else {
        asserts(node_distance[node_i] <= edge_dist,
                "no overflow , use bigger uints if this happens");
      }
      uintlen_t target_node = edge_node(edge_i);
      if (node_distance[target_node] <= edge_dist)
        continue;
      parents[target_node] = node_i;
      node_distance[target_node] = edge_dist;
      heap[heap_count++] = edge_i;
      std::ranges::push_heap(heap.subspan(0, heap_count), comp);
    }
    uintlen_t edge_i{};
    if (!heap_count)
      break;

    bool stale{};
    do {
      std::ranges::pop_heap(heap.subspan(0, heap_count), comp);
      edge_i = heap[--heap_count];
      node_i = edge_node(edge_i);
      stale = edge_distance[edge_i] != node_distance[node_i];
    } while (stale && heap_count);
    if (stale)
      break;
  }
  ret.resize(nsz);
  return tuple_t{std::move(ret), std::move(distance_metic_)};
}

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t>
MJZ_CX_FN std::vector<uintlen_t>
calculate_shortest_positive_path(uintlen_t entry_index,
                                 const basic_forest_t<version_v, T> &graph,
                                 edge_wights_t &&edge_wight_) noexcept {
  auto [ret, dist] = calculate_shortest_positive_path_distanced(
      entry_index, graph, edge_wight_);
  return (std::move(ret));
}

}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_algo_FILE_
