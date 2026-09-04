

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

#ifndef MJZ_SRC_GRAPH_bdeps_FILE_
#define MJZ_SRC_GRAPH_bdeps_FILE_
#include "../byte_str/formatting/basic_formatters.hpp"
#include "../byte_str/formatting/format.hpp"
MJZ_EXPORT
//
namespace mjz::graph_ns {
template <version_t version_v> struct MJZ_trivially_relocatable base_node_id_t {

  constexpr static inline uintlen_t sentinel_v = uintlen_t(-1);

private:
  uintlen_t value = sentinel_v;

public:
  MJZ_CX_FN explicit operator bool() const noexcept {
    return value != sentinel_v;
  }
  MJZ_CX_FN bool operator!() const noexcept { return value == sentinel_v; }
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

template <version_t version_v>
struct MJZ_trivially_relocatable base_edges_ids_t {
  // it cant be a "Compressed Sparse Row", my graph evolves dynamically.
  intlen_t m_connections_begin_index{};
  intlen_t m_connections_length{};
  //

  MJZ_CX_AL_FN uintlen_t connection_count() const noexcept {
    return uintlen_t(std::max(m_connections_length, -m_connections_length));
  }
  MJZ_CX_AL_FN basic_index_range_t<version_v>
  connection_index_ids() const noexcept {
    return {.i = uintlen_t(std::max(m_connections_begin_index,
                                    -m_connections_begin_index)),
            .n = connection_count()};
  }
  MJZ_CX_AL_FN uintlen_t expand_valid_impl(uintlen_t count) noexcept {
    uintlen_t i = connection_index_ids().ending();
    m_connections_length = bit_branchless_teranary<intlen_t>(
        m_connections_length < 0, m_connections_length - intlen_t(count),
        m_connections_length + intlen_t(count));
    return i;
  }

  MJZ_CX_AL_FN uintlen_t emplace_back_valid_impl() noexcept {
    return expand_valid_impl(1);
  }

  MJZ_CX_AL_FN bool has_capacity_field() const noexcept {
    return m_connections_begin_index < 0;
  }
  MJZ_CX_AL_FN uintlen_t capacity_field_index() const noexcept {
    return uintlen_t(~m_connections_begin_index);
  }
  MJZ_CX_AL_FN bool has_implicit_capacity() const noexcept {
    return m_connections_length <= 0;
  }

  MJZ_CX_AL_FN uintlen_t implicit_capacity() const noexcept {
    return bit_branchless_teranary(
        !!m_connections_length,
        uintlen_t(std::bit_ceil(uintlen_t(-m_connections_length))),
        uintlen_t());
  }

  MJZ_CX_AL_FN std::span<const uintlen_t>
  get_connections(std::span<const uintlen_t> connections_list) const noexcept {
    auto cir_ = connection_index_ids();
    if (!basic_index_range_t<version_v>{.i = 0, .n = connections_list.size()}
             .has_inside(cir_))
      return {};
    return connections_list.subspan(cir_.i, cir_.n);
  }

  MJZ_CX_AL_FN uintlen_t
  get_capacity(std::span<const uintlen_t> connections_list) const noexcept {
    if (has_implicit_capacity())
      return implicit_capacity();
    uintlen_t capacity_field_index_ = capacity_field_index();
    if (capacity_field_index_ < connections_list.size())
      return connections_list[capacity_field_index_];
    return connection_count();
  }
};

template <version_t version_v>
struct MJZ_maybe_trivially_relocatable base_edge_connections_list_t {
  using edges_ids_t = base_edges_ids_t<version_v>;
  std::vector<uintlen_t> connections_list{};
  uintlen_t connections_free_list_mask{};
  uintlen_t expected_edges_per_node_v{0};
  std::array<intlen_t, std::bit_width(uintlen_t(-1))>
      connections_free_list_heads{};
  MJZ_CX_AL_FN intlen_t
  connections_free_list_pop_impl(uintlen_t free_index) noexcept {
    intlen_t index = connections_free_list_heads[free_index];
    asserts(asserts.assume_rn, !!index);
    if (0 < index)
      index--;
    intlen_t old =
        intlen_t(connections_list[uintlen_t(std::max(index, -index))]);
    connections_free_list_heads[free_index] = old;
    connections_free_list_mask &=
        old ? connections_free_list_mask : ~(uintlen_t(1) << free_index);
    return index;
  }
  MJZ_CX_AL_FN void connections_free_list_push_impl(uintlen_t free_index,
                                                    intlen_t index) noexcept {
    intlen_t old = connections_free_list_heads[free_index];
    connections_list[uintlen_t(std::max(index, -index))] = uintlen_t(old);
    connections_free_list_heads[free_index] = index < 0 ? index : index + 1;
    connections_free_list_mask |= uintlen_t(1) << free_index;
  }
  MJZ_CX_AL_FN void deallocate_connections(intlen_t index,
                                           uintlen_t capacity) noexcept {
    if (index < 0) {
      index = ~index;
      capacity++;
    }
    if (!capacity)
      return;
    uintlen_t cap_floor = std::bit_floor(capacity);
    uintlen_t free_index = uintlen_t(std::bit_width(cap_floor) - 1);

    if (cap_floor != capacity) {
      connections_list[uintlen_t(index)] = --capacity;
      index = ~index;
    }
    connections_free_list_push_impl(free_index, index);
  }

  MJZ_CX_AL_FN intlen_t allocate_connections(uintlen_t &capacity,
                                             bool implicit_field) noexcept {
    asserts(asserts.assume_rn, !!capacity);
    uintlen_t exact_cap = capacity;
    if (!implicit_field)
      exact_cap++;
    uintlen_t cap_ceil = std::bit_ceil(exact_cap);
    intlen_t index{};
    if (connections_free_list_mask < cap_ceil) {
      uintlen_t uindex = connections_list.size();
      connections_list.resize(uindex + exact_cap);
      asserts(asserts.assume_rn,
              0 < intlen_t(uintlen_t(connections_list.size() + 1)));
      index = intlen_t(uindex);
      if (!implicit_field) {
        connections_list[uintlen_t(index)] = capacity;
        index = ~index;
        return index;
      }
    } else {
      uintlen_t free_index = uintlen_t(std::countr_zero(
          connections_free_list_mask & ~uintlen_t(cap_ceil - 1)));
      index = connections_free_list_pop_impl(free_index);
      if (index < 0) {
        capacity = connections_list[uintlen_t(~index)];
        return index;
      }
      capacity = uintlen_t(1) << free_index;
    }
    if (implicit_field && exact_cap == capacity)
      return index;
    connections_list[uintlen_t(index)] = --capacity;
    return ~index;
  }

  MJZ_CX_FN void reserve_edge_list(uintlen_t extra_later) noexcept {
    extra_later = extra_later * 2 + 2;
    if (connections_list.size() + extra_later < connections_list.capacity())
      return;
    connections_list.reserve(
        connections_list.size() +
        std::max(connections_list.capacity(), extra_later));
  }
  MJZ_CX_FN void reserve_connections_list(uintlen_t reserve_val) noexcept {
    connections_list.reserve(32 + reserve_val);
  }
  MJZ_CX_FN void edge_reserve_all(uintlen_t v) noexcept {
    expected_edges_per_node_v = v;
  }
  MJZ_CX_FN uintlen_t get_expected_edges_per_node() const noexcept {
    return expected_edges_per_node_v;
  }

  MJZ_CX_AL_FN void reserve_edge_list_impl(edges_ids_t &new_direct,
                                           uintlen_t new_capacity,
                                           bool &implicit_field) noexcept {
    edges_ids_t old_direct = new_direct;
    uintlen_t old_capacity = old_direct.get_capacity(connections_list);

    if (new_capacity <= old_capacity)
      return;
    uintlen_t prefer_cap = new_capacity;
    intlen_t new_connections_begin_index =
        allocate_connections(new_capacity, implicit_field);
    new_direct.m_connections_begin_index = new_connections_begin_index;
    new_direct.m_connections_length = 0;
    for (uintlen_t i : old_direct.connection_index_ids().iota()) {
      connections_list[new_direct.emplace_back_valid_impl()] =
          connections_list[i];
    }
    implicit_field &= prefer_cap == new_capacity;
    deallocate_connections(old_direct.m_connections_begin_index, old_capacity);
  }
  MJZ_CX_FN uintlen_t edge_list_expand(edges_ids_t &new_direct,
                                       uintlen_t add_count,
                                       bool force_realloc = false) noexcept {
    uintlen_t count_sz = new_direct.connection_count();
    uintlen_t cap = new_direct.get_capacity(connections_list);
    if (cap >= count_sz + add_count && !force_realloc)
      return new_direct.expand_valid_impl(add_count);
    uintlen_t geo_cap = std::bit_ceil(count_sz + add_count);
    uintlen_t extra_later = std::max(expected_edges_per_node_v, geo_cap);
    bool implicit_field = geo_cap == extra_later;
    reserve_edge_list_impl(new_direct, extra_later, implicit_field);
    uintlen_t r = new_direct.expand_valid_impl(add_count);
    if (implicit_field)
      new_direct.m_connections_length = -new_direct.m_connections_length;
    return r;
  }

  MJZ_CX_FN void edge_list_push_back(edges_ids_t &new_direct,
                                     uintlen_t push_val,
                                     bool force_realloc = false) noexcept {
    uintlen_t push_i{};
    uintlen_t count_sz = new_direct.connection_count();
    if (!new_direct.has_implicit_capacity() &&
        new_direct.get_capacity(connections_list) == count_sz &&
        std::has_single_bit(count_sz + 1) && !force_realloc) {
      new_direct.m_connections_begin_index =
          ~new_direct.m_connections_begin_index;
      new_direct.emplace_back_valid_impl();
      new_direct.m_connections_length = -new_direct.m_connections_length;
      push_i = new_direct.connection_index_ids().begining();
    } else {
      push_i = edge_list_expand(new_direct, 1, force_realloc);
    }
    connections_list[push_i] = push_val;
  } 
  MJZ_CX_FN void edge_list_delete(edges_ids_t old_direct) noexcept {
    deallocate_connections(old_direct.m_connections_begin_index,
                           old_direct.get_capacity(connections_list));
  }
  
  MJZ_CX_FN void edge_list_push_back(edges_ids_t &new_direct,
                                     uintlen_t push_val,
                                     uintlen_t extra_later) noexcept {

    bool force_realloc = new_direct.get_capacity(connections_list) -
                                 new_direct.connection_count() <
                             extra_later &&
                         2 < extra_later;
    extra_later =
        std::exchange(expected_edges_per_node_v,
                      std::max(expected_edges_per_node_v,
                               new_direct.connection_count() + extra_later));
    force_realloc &= extra_later;
    edge_list_push_back(new_direct, push_val, force_realloc);
    expected_edges_per_node_v = extra_later;
  }
  MJZ_CX_FN void clear() noexcept {
    connections_list.clear();
    connections_free_list_mask = 0;
    expected_edges_per_node_v = 0;
    for (auto &e : connections_free_list_heads)
      e = 0;
  }
  MJZ_CX_FN operator std::span<const uintlen_t>() const noexcept {
    return connections_list;
  }
};
}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_bdeps_FILE_