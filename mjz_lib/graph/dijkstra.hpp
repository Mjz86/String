
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

#ifndef MJZ_SRC_GRAPH_dijkstra_FILE_
#define MJZ_SRC_GRAPH_dijkstra_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, class dist_t>
MJZ_CX_FN dist_t distance_inf() noexcept {
  static_assert(std::floating_point<dist_t> || std::unsigned_integral<dist_t> ||
                requires() {
                  typename dist_t::mjz_uintN_t_id_val_t_2354675648764874753789;
                });
  if constexpr (std::floating_point<dist_t>) {
    static_assert(std::numeric_limits<dist_t>::has_infinity);
    return std::numeric_limits<dist_t>::infinity();
  } else {
    return dist_t(~dist_t());
  }
}

template <version_t version_v, class T>
MJZ_CX_AL_FN void calculate_shortest_positive_path_entry_edge_distance_events(
    usable_range_exact_c<version_v> auto &&inf_initilized_distance_range,
    uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
    auto &&weight_of_edge,

    auto &&node_entry_event, auto &&node_exit_event,

    auto &&edge_drop_event, auto &&edge_select_event,

    auto &&edge_stale_event, auto &&edge_hit_event) noexcept
  requires requires(uintlen_t node_index, uintlen_t edge_index,
                    uintlen_t from_index, uintlen_t to_index, bool early_exit) {
    //
    { weight_of_edge(+edge_index, +from_index, +to_index) } noexcept;
    //
    { early_exit == bool(node_entry_event(+node_index)) } noexcept;
    {
      early_exit == bool(node_exit_event(+node_index, bool(early_exit)))
    } noexcept;
    //
    {
      early_exit == bool(edge_drop_event(+edge_index, +from_index, +to_index))
    } noexcept;
    {
      early_exit == bool(edge_select_event(+edge_index, +from_index, +to_index))
    } noexcept;
    requires(
        requires() {
          { early_exit == bool(edge_stale_event(+edge_index)) } noexcept;
          { early_exit == bool(edge_hit_event(+edge_index)) } noexcept;
        } ||
        requires() {
          {
            early_exit ==
                bool(edge_stale_event(+edge_index, +from_index, +to_index))
          } noexcept;
          {
            early_exit ==
                bool(edge_hit_event(+edge_index, +from_index, +to_index))
          } noexcept;
        });
  }
{
  uintlen_t esz = std::ranges::size(graph.edges);
  uintlen_t nsz = std::ranges::size(graph.nodes_index);
  auto it_edge_node = std::ranges::begin(graph.edges);
  auto edge_indexies = graph.index_range();

  auto edge_node = [&](uintlen_t edge_i) noexcept {
    return *(it_edge_node + intlen_t(edge_i));
  };
  using dist_t = std::remove_cvref_t<decltype(weight_of_edge(
      uintlen_t(), uintlen_t(), uintlen_t()))>;
  auto dv_ = std::views::transform(
      [iidrit = std::ranges::begin(inf_initilized_distance_range)](
          uintlen_t i) noexcept -> decltype(auto) {
        return *(iidrit + intlen_t(i));
      });
  asserts(entry_index < nsz);
  //
  uintlen_t *heap_strotage = std::allocator<uintlen_t>().allocate(esz);
  MJZ_RAII_RELEASE {
    std::allocator<uintlen_t>().deallocate(heap_strotage, esz);
  };
  //

  auto node_distance = std::views::iota(uintlen_t(), nsz) | dv_;
  auto edge_distance = std::views::iota(nsz, esz) | dv_;
  node_distance[entry_index] = dist_t();

  uintlen_t heap_count{};
  auto comp = [&](uintlen_t edge_il, uintlen_t edge_ir) noexcept {
    return +edge_distance[edge_il] > +edge_distance[edge_ir];
  };
  uintlen_t node_i{entry_index};
  bool early_exit{};
  while (true) {
    early_exit = bool(node_entry_event(+node_i));
    if (early_exit)
      break;
    for (uintlen_t edge_i : edge_indexies[node_i]) {
      uintlen_t target_node = edge_node(edge_i);
      const dist_t edge_weight_ =
          weight_of_edge(+edge_i, +node_i, +target_node);
      const dist_t edge_dist = dist_t(edge_weight_ + +node_distance[node_i]);
      edge_distance[edge_i] = edge_dist;
      if constexpr (std::floating_point<dist_t>) {
        asserts(dist_t() <= edge_weight_, "no negative / NAN weight ");
      } else {
        asserts(+node_distance[node_i] <= edge_dist,
                "no overflow , use bigger uints if this happens");
      }
      if (+node_distance[target_node] <= edge_dist) {
        early_exit = bool(edge_drop_event(+edge_i, +node_i, +target_node));
        if (early_exit)
          break;
        continue;
      } else {
        early_exit = bool(edge_select_event(+edge_i, +node_i, +target_node));
        if (early_exit)
          break;
      }
      node_distance[target_node] = edge_dist;
      heap_strotage[heap_count++] = edge_i;
      std::ranges::push_heap(std::span(heap_strotage, heap_count), comp);
    }
    early_exit = bool(node_exit_event(+node_i, bool(early_exit)));
    if (early_exit)
      break;
    if (!heap_count)
      break;

    uintlen_t edge_i{};
    bool stale{};
    do {
      std::ranges::pop_heap(std::span(heap_strotage, heap_count), comp);
      edge_i = heap_strotage[--heap_count];
      node_i = edge_node(edge_i);
      stale = make_bitcast(+edge_distance[edge_i]) !=
              make_bitcast(+node_distance[node_i]);
      if constexpr (requires() {
                      {
                        early_exit == bool(edge_stale_event(+edge_i))
                      } noexcept;
                      { early_exit == bool(edge_hit_event(+edge_i)) } noexcept;
                    }) {
        if (stale) {
          early_exit = bool(edge_stale_event(+edge_i));
        } else {
          early_exit = bool(edge_hit_event(+edge_i));
        }
      } else {
        auto [from_index, to_index] = graph.get_edge_from_to(+edge_i);
        if (stale) {
          early_exit = bool(edge_stale_event(+edge_i, +from_index, +to_index));
        } else {
          early_exit = bool(edge_hit_event(+edge_i, +from_index, +to_index));
        }
      }
      if (early_exit)
        break;
    } while (stale && heap_count);
    if (early_exit)
      break;
    if (stale)
      break;
  }
}

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t>
MJZ_CX_FN
    std::vector<std::remove_cvref_t<std::ranges::range_value_t<edge_wights_t>>>
    calculate_shortest_positive_path_distanced_impl_(
        uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
        edge_wights_t &&weight_of_adjust, auto &&edge_select_event) noexcept {
  auto dummy = [](auto &&...) noexcept { return false; };
  using dist_t = std::remove_cvref_t<std::ranges::range_value_t<edge_wights_t>>;
  std::vector<dist_t> distance_metic_(std::ranges::size(graph.edges) +
                                          std::ranges::size(graph.nodes_index),
                                      distance_inf<version_v, dist_t>());
  calculate_shortest_positive_path_entry_edge_distance_events(
      distance_metic_, entry_index, graph,
      [it_weights = std::ranges::begin(weight_of_adjust)](
          uintlen_t edge_index, uintlen_t, uintlen_t) noexcept {
        return *(it_weights + intlen_t(edge_index));
      },
      dummy, dummy, dummy, edge_select_event, dummy, dummy);

  return distance_metic_;
}

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t>
MJZ_CX_FN
    std::vector<std::remove_cvref_t<std::ranges::range_value_t<edge_wights_t>>>
    calculate_shortest_positive_path_entry_edge_distance(
        uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
        edge_wights_t &&weight_of_adjust) noexcept {
  return calculate_shortest_positive_path_distanced_impl_(
      entry_index, graph, weight_of_adjust,
      [](auto &&...) noexcept { return false; });
}

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t>
MJZ_CX_FN tuple_t<
    std::vector<uintlen_t>,
    std::vector<std::remove_cvref_t<std::ranges::range_value_t<edge_wights_t>>>>
calculate_shortest_positive_path_distanced(
    uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
    edge_wights_t &&weight_of_adjust) noexcept {
  auto node_parents =
      std::vector<uintlen_t>(std::ranges::size(graph.range()), uintlen_t(-1));
  auto distance_metic_ = calculate_shortest_positive_path_distanced_impl_(
      entry_index, graph, weight_of_adjust,
      [&](uintlen_t, uintlen_t from_index, uintlen_t to_index) noexcept {
        node_parents[to_index] = from_index;
        return false;
      });
  return {std::move(node_parents), std::move(distance_metic_)};
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

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t,
          usable_range_exact_c<version_v> node_potential_t>
MJZ_CX_FN void calculate_shortest_positive_path_entry_edge_distance_reweight(
    usable_range_exact_c<version_v> auto &&inf_initilized_distance_range,
    uintlen_t entry_index, const basic_forest_t<version_v, T> &graph,
    edge_wights_t &&weight_of_adjust, node_potential_t &&potential_nodes,

    auto &&node_entry_event, auto &&node_exit_event,

    auto &&edge_drop_event, auto &&edge_select_event,

    auto &&edge_stale_event, auto &&edge_hit_event) noexcept {
  auto it_weights = std::ranges::begin(weight_of_adjust);
  auto it_potential = std::ranges::begin(potential_nodes);
  return calculate_shortest_positive_path_entry_edge_distance_events(
      inf_initilized_distance_range, entry_index, graph,
      [&](uintlen_t edge_index, uintlen_t from_index,
          uintlen_t to_index) noexcept {
        return *(it_weights + intlen_t(edge_index)) +
               *(it_potential + intlen_t(to_index)) -
               *(it_potential + intlen_t(from_index));
      },
      node_entry_event, node_exit_event, edge_drop_event, edge_select_event,
      edge_stale_event, edge_hit_event);
}

template <version_t version_v, class T,
          usable_range_exact_c<version_v> edge_wights_t,
          usable_range_exact_c<version_v> node_potential_t>
MJZ_CX_FN std::vector<uintlen_t>
calculate_A_start_path(uintlen_t entry_index, uintlen_t exit_index,
                       const basic_forest_t<version_v, T> &graph,
                       edge_wights_t &&weight_of_adjust,
                       node_potential_t &&potential_nodes) noexcept {
  if (entry_index == exit_index)
    return {};
  auto it_weights = std::ranges::begin(weight_of_adjust);
  auto it_potential = std::ranges::begin(potential_nodes);

  using dist_t = std::remove_cvref_t<std::ranges::range_value_t<edge_wights_t>>;
  uintlen_t node_sz = uintlen_t(std::ranges::size(graph.nodes_index));
  uintlen_t distance_count =
      uintlen_t(std::ranges::size(graph.edges)) + node_sz;
  std::byte *calloc_ptr{};
  dist_t *ptr{};

  using pair_path_t = pair_t<uintlen_t, uintlen_t>;
  pair_path_t *path_ptr{};
  MJZ_IF_CONSTEVAL {
    dist_t zero_init =
        std::bit_cast<dist_t>(std::array<char, sizeof(dist_t)>{});

    path_ptr = std::allocator<pair_path_t>().allocate(node_sz);
    ptr = std::allocator<dist_t>().allocate(distance_count);
    for (uintlen_t i : std::views::iota(uintlen_t(), distance_count))
      std::construct_at(ptr + i, zero_init);
    ptr = std::launder(ptr);
  }
  else {
    size_t alignm = std::max(alignof(dist_t), alignof(pair_path_t));
    size_t dist_sz = size_t(distance_count) * sizeof(dist_t);
    size_t path_sz = node_sz * sizeof(pair_path_t);
    size_t sz = dist_sz + path_sz;
    size_t asz = sz + alignm;
    void *calloc_ptr_ = std::calloc(asz, 1);
    if (!std::align(alignm, sz, calloc_ptr_, asz))
      asserts(false);
    calloc_ptr = reinterpret_cast<std::byte *>(calloc_ptr_);
    auto dist_p = calloc_ptr;
    auto path_p = calloc_ptr;
    if constexpr (alignof(dist_t) < alignof(pair_path_t)) {
      dist_p += path_sz;
    } else {
      path_p += dist_sz;
    }

    ptr = reinterpret_cast<dist_t *>(dist_p);
    path_ptr = reinterpret_cast<pair_path_t *>(path_p);
  }
  MJZ_RAII_RELEASE {
    static_assert(std::is_trivially_destructible_v<dist_t> &&
                  std::is_trivially_copy_constructible_v<dist_t>);
    static_assert(std::is_trivially_destructible_v<pair_path_t> &&
                  std::is_trivially_copy_constructible_v<pair_path_t>);
    MJZ_IF_CONSTEVAL {
      std::allocator<dist_t>().deallocate(ptr, distance_count);
      std::allocator<pair_path_t>().deallocate(path_ptr, node_sz);
    }
    else {
      std::free(calloc_ptr);
    }
  };

  auto inf_initilized_distance_range =
      std::span(ptr, distance_count) |
      std::views::transform([](dist_t &ref) noexcept {
        struct proxy {
          dist_t *p;
          MJZ_NO_MV_NO_CPY(proxy);
          MJZ_CX_FN proxy(dist_t &r) noexcept : p{&r} {}
          MJZ_CX_FN static dist_t xor_inf(dist_t d) noexcept {
            auto d_bytes = make_bitcast(d);
            uintlen_t i{};
            for (char inf_byte :
                 make_bitcast(distance_inf<version_v, dist_t>()))
              d_bytes[i++] ^= inf_byte;
            return std::bit_cast<dist_t>(d_bytes);
          }
          MJZ_CX_FN proxy &operator=(const dist_t &d) noexcept {
            *p = xor_inf(d);
            return *this;
          }
          MJZ_CX_FN dist_t operator+() const noexcept { return xor_inf(*p); }
        };
        return proxy{ref};
      });

  bool hit_exit{};

  auto node_entry_event = [&](uintlen_t i) noexcept {
    hit_exit |= i == exit_index;
    return hit_exit;
  };
  auto edge_select_event = [&](uintlen_t edge_index, uintlen_t from_index,
                               uintlen_t to_index) noexcept {
    std::construct_at(path_ptr + to_index, edge_index, from_index);
    return false;
  };
  auto dum = [](auto &&...) noexcept { return false; };

  calculate_shortest_positive_path_entry_edge_distance_events(
      inf_initilized_distance_range, entry_index, graph,
      [&](uintlen_t edge_index, uintlen_t from_index,
          uintlen_t to_index) noexcept {
        return *(it_weights + intlen_t(edge_index)) +
               *(it_potential + intlen_t(to_index)) -
               *(it_potential + intlen_t(from_index));
      },
      node_entry_event, dum, dum, edge_select_event, dum, dum);
  if (!hit_exit)
    return {};

  std::vector<uintlen_t> path_vec{};
  uintlen_t index_node{exit_index};
  while (index_node != entry_index) {
    auto &&[edge_index, from_index] = path_ptr[index_node];
    index_node = from_index;
    path_vec.push_back(edge_index);
  };
  std::ranges::reverse(path_vec);
  return path_vec;
}
}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_dijkstra_FILE_