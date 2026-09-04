
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

#ifndef MJZ_SRC_GRAPH_rdeps_FILE_
#define MJZ_SRC_GRAPH_rdeps_FILE_
#include "udeps.hpp"
MJZ_EXPORT
//
namespace mjz::graph_ns {

// unsafe unidirectional DAG variant, it does node recycling
template <version_t version_v>
struct MJZ_maybe_trivially_relocatable
    basic_unsafe_uni_dependency_graph_base_t {
  using node_id_t = base_node_id_t<version_v>;
  using dependency_node_t = uni_dependancy_node_t<version_v>;
  using dependency_base_t = uni_dependancy_node_base_t<version_v>;
  using dependency_edges_t = base_edges_ids_t<version_v>;

  static_assert(std::is_trivially_move_constructible_v<dependency_node_t>);
  static_assert(std::is_trivially_copy_constructible_v<dependency_node_t>);
  static_assert(std::is_trivially_destructible_v<dependency_node_t>);
  struct m_t {
    std::vector<dependency_node_t> m_nodes{};
    std::vector<uintlen_t> m_applied_list{};
    std::vector<uintlen_t> m_apply_list{};
    std::vector<uintlen_t> m_free_list{};
    base_edge_connections_list_t<version_v> m_connections_list{};
  };
  m_t m{};
  MJZ_CX_AL_FN dependency_node_t &dependency(node_id_t me) noexcept {
    return m.m_nodes[me.index()];
  }
  MJZ_CX_AL_FN const dependency_node_t &
  dependency(node_id_t me) const noexcept {
    return m.m_nodes[me.index()];
  }

  MJZ_CX_AL_FN dependency_base_t &dependency_base(node_id_t me) noexcept {
    return m.m_nodes[me.index()];
  }
  MJZ_CX_AL_FN const dependency_base_t &
  dependency_base(node_id_t me) const noexcept {
    return m.m_nodes[me.index()];
  }
  MJZ_CX_AL_FN void prefetch_dependency(node_id_t me) const noexcept {
    if constexpr (!mjz_do_prefetch_v)
      return;
    const dependency_node_t &d = dependency(me);
    const dependency_base_t &b = d;
    mjz_prefetch(b);
    mjz_prefetch(d);
  }
  MJZ_CX_AL_FN void prefetch_dependency_base(node_id_t me) const noexcept {
    if constexpr (!mjz_do_prefetch_v)
      return;
    mjz_prefetch(dependency_base(me));
  }

  MJZ_CX_FN success_t query(node_id_t id) noexcept {
    dependency_node_t &node_dependency = dependency(id);
    if (!node_dependency.can_trigger())
      return false;
    node_dependency.actively_trigger();
    m.m_apply_list.emplace_back(id.index());
    return true;
  }

  MJZ_CX_AL_FN void defuse_resolution(node_id_t id, bool and_free) noexcept {
    const auto [as_complete, query_me, query_deps] =
        and_free ? tuple_t{true, false, true} : tuple_t{false, true, false};
    auto dependency_ptrs = m.m_nodes.data();
    dependency_base_t base_node = dependency_base(id);
    const auto connections_span =
        dependency(id).get_connections(m.m_connections_list);
    uintlen_t *apply_list_ptr{};
    {
      uintlen_t real_size = m.m_apply_list.size();
      m.m_apply_list.resize(
          m.m_apply_list.size() +
          (uintlen_t(query_deps && as_complete) * connections_span.size()) + 1);
      apply_list_ptr = m.m_apply_list.data() + real_size;
    }
    MJZ_RAII_RELEASE {
      uintlen_t real_size = apply_list_ptr - m.m_apply_list.data();
      const bool plese_optimize_ = real_size < m.m_apply_list.size();

      asserts(asserts.assume_rn, plese_optimize_);
      MJZ_JUST_ASSUME_(plese_optimize_);
      if (plese_optimize_) {
        m.m_apply_list.resize(real_size);
      }
    };
    constexpr uintlen_t prefetch_batch_v = 8;
    for (uintlen_t prefetch_count =
                       std::min(connections_span.size(), prefetch_batch_v),
                   i{};
         query_deps && as_complete && i < prefetch_count; i++) {
      if constexpr (mjz_do_prefetch_v) {
        mjz_prefetch_p(static_cast<dependency_base_t *>(dependency_ptrs +
                                                        connections_span[i]));
      }
    }
    {
      MJZ_RAII_RELEASE { dependency_base(id) = base_node; };
      if (!base_node.is_passively_triggered() ||
          !base_node.defuse(as_complete)) {

        asserts(asserts.assume_rn, false);
      }
      *apply_list_ptr = id.index();
      apply_list_ptr += base_node.speculative_actively_trigger(query_me);

      asserts(asserts.assume_rn, !base_node.is_unrecoverable());
    }
    if (!as_complete)
      return;
    for (uintlen_t i{}; i < connections_span.size(); i++) {
      if constexpr (mjz_do_prefetch_v) {
        mjz_prefetch_p(static_cast<dependency_base_t *>(
            dependency_ptrs +
            connections_span[std::min(connections_span.size() - 1,
                                      i + prefetch_batch_v)]));
      }
      node_id_t dep_id{connections_span[i]};

      dependency_base_t &dep_node_ref =
          *static_cast<dependency_base_t *>(dependency_ptrs + dep_id.index());
      dependency_base_t dep_node = dep_node_ref;

      asserts(asserts.assume_rn, !dep_node.is_unrecoverable());
      asserts(asserts.assume_rn, !dep_node.is_sentinel());
      bool exeute = true;
      exeute &= dep_node.speculative_dec_dgree(exeute) == 0;
      if (query_deps) {
        exeute = dep_node.speculative_actively_trigger(exeute);
        *apply_list_ptr = dep_id.index();
        apply_list_ptr += exeute;
      }
      dep_node_ref = dep_node;
    }
    dependency_node_t &node_ = dependency(id);
    dependency_edges_t &edge_ = node_;
    dependency_base_t &base_ = node_;
    base_ = dependency_base_t{};
    base_.as_sentinel();
    m.m_connections_list.edge_list_delete(std::exchange(edge_, {}));
    m.m_free_list.push_back(id.index());
  }

  MJZ_CX_FN node_id_t make_nodes_impl(uintlen_t count_nodes) noexcept {
    uintlen_t i = this->m.m_nodes.size();
    m.m_nodes.resize(i + count_nodes);
    return node_id_t(i);
  }

  MJZ_CX_FN void make_edge_impl(node_id_t dependency_i, node_id_t dependant_i,
                                uintlen_t extra_later_dependant) noexcept {

    asserts(asserts.assume_rn, dependant_i != dependency_i &&
                                   !dependency(dependency_i).is_sentinel() &&
                                   !dependency(dependant_i).is_sentinel());
    dependency(dependant_i).inc_dgree();
    auto dep_node = dependency(dependency_i);
    m.m_connections_list.edge_list_push_back(dep_node, dependant_i.index(),
                                             extra_later_dependant);
    dependency(dependency_i) = dep_node;
  }

  MJZ_CX_FN void edge_reserve_impl(uintlen_t extra_later) noexcept {
    return m.m_connections_list.reserve_edge_list(extra_later);
  }

  MJZ_CX_FN void reserve_impl(uintlen_t node_estimate,
                              uintlen_t edge_estimate) noexcept {
    m.m_nodes.reserve(1 + node_estimate);
    m.m_free_list.reserve(1 + node_estimate);
    m.m_applied_list.reserve(1 + node_estimate);
    m.m_apply_list.reserve(1 + node_estimate);
    m.m_connections_list.reserve_connections_list(node_estimate +
                                                  edge_estimate);
  }

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state(node_id_t me) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v,
                                                          "[id({}),{}]\n">(),
        me.index(),
        dependency(me).format_node_state_direct(m.m_connections_list));
  }

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state_dot(node_id_t me) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v,
                                                          "\"id:{}\n{}\"">(),
        me.index(), dependency(me).format_node_state_direct_dot());
  }
  MJZ_CX_AL_FN bool events_running() const noexcept {
    return !m.m_applied_list.empty();
  }

  template <class execute_resolution_wave_fnt>
  MJZ_CX_FN bool run_resolution_queries(
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    asserts(asserts.assume_rn, !events_running());
    MJZ_RAII_RELEASE {
      m.m_applied_list.clear();
      asserts(asserts.assume_rn, !events_running());
    };
    std::swap(m.m_applied_list, m.m_apply_list);
    for (uintlen_t j : m.m_applied_list) {
      node_id_t id = node_id_t(j);
      dependency(id).passively_trigger();
    }
    auto id_view = m.m_applied_list |
                   std::views::transform([](uintlen_t j) noexcept -> node_id_t {
                     return node_id_t(j);
                   });
    static_assert(requires() {
      {
        std::forward<execute_resolution_wave_fnt>(execute_resolution_wave_fn)(
            std::move(id_view))
      } noexcept;
    });
    std::forward<execute_resolution_wave_fnt>(execute_resolution_wave_fn)(
        std::move(id_view));

    return events_running();
  }

  MJZ_CX_FN void reserve_per_node(uintlen_t edges_per_node_estimate) noexcept {
    this->m.m_connections_list.edge_reserve_all(edges_per_node_estimate);
  }

  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate) noexcept {
    return this->reserve_impl(node_estimate, edge_estimate);
  }

  MJZ_CX_FN dependency_node_t get_node(node_id_t i) const noexcept {
    return this->m.m_nodes[i.index()];
  }

  MJZ_CX_FN uintlen_t node_size_limit() const noexcept {
    return this->m.m_nodes.size();
  }
  MJZ_CX_FN uintlen_t node_count() const noexcept {
    return this->m.m_nodes.size() - this->m.m_free_list.size();
  }

  MJZ_CX_FN const dependency_node_t &get_node_cref(node_id_t i) const noexcept {
    return this->m.m_nodes[i.index()];
  }

  MJZ_CX_FN
  void make_edges(node_id_t dependency_i,
                  std::span<const node_id_t> dependant_ids) noexcept {
    uintlen_t extra_later = dependant_ids.size();
    this->edge_reserve_impl(extra_later);
    for (node_id_t dependant_i : dependant_ids) {
      this->make_edge_impl(dependency_i, dependant_i, extra_later);
      extra_later--;
    }
  }

  MJZ_CX_FN void make_edges(std::span<const node_id_t> dependency_ids,
                            node_id_t dependant_i) noexcept {
    this->edge_reserve_impl(dependency_ids.size());
    for (node_id_t dependency_i : dependency_ids) {
      this->make_edge_impl(dependency_i, dependant_i, 0);
    }
  }

  MJZ_CX_FN void make_edge(node_id_t dependency_i,
                           node_id_t dependant_i) noexcept {
    this->edge_reserve_impl(1);
    return this->make_edge_impl(dependency_i, dependant_i, 0);
  }

  MJZ_CX_FN node_id_t make_node() noexcept {
    if (m.m_free_list.size()) {
      auto ret = node_id_t(m.m_free_list.back());
      m.m_free_list.pop_back();
      return ret;
    }
    return this->make_nodes_impl(1);
  }

  MJZ_CX_FN void make_nodes(std::span<node_id_t> out_is) noexcept {
    uintlen_t alloc_size =
        out_is.size() - std::min(m.m_free_list.size(), out_is.size());
    if (alloc_size) {
      uintlen_t i = this->make_nodes_impl(alloc_size).index();
      for (node_id_t &id_ : out_is.subspan(0, alloc_size))
        id_ = node_id_t(i++);
    }
    for (node_id_t &id_ : out_is.subspan(alloc_size)) {
      id_ = node_id_t(m.m_free_list.back());
      m.m_free_list.pop_back();
      dependency(id_) = dependency_node_t{};
    }
  }

  MJZ_CX_FN void defuse_and_free(node_id_t id) noexcept {
    return this->defuse_resolution(id, true);
  }

  MJZ_CX_FN void defuse_and_yeild(node_id_t id) noexcept {
    return this->defuse_resolution(id, false);
  }

  MJZ_CX_FN uintlen_t seed_all_nodes() noexcept {
    uintlen_t seeds = 0;
    for (uintlen_t i = 0; i < this->m.m_nodes.size(); ++i) {
      seeds += query(node_id_t(i));
    }
    return seeds;
  }

  MJZ_CX_FN bool is_unresolved(node_id_t i) const noexcept {
    return !this->dependency(i).is_complete();
  }

  MJZ_CX_FN bool is_failed(node_id_t i) const noexcept {
    return is_unresolved(i);
  }

  MJZ_CX_FN auto view_all_fail_ids() const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(this->m.m_nodes.size())) |
           std::views::transform(
               [](uintlen_t i) noexcept { return node_id_t(i); }) |
           std::views::filter(
               [this](node_id_t i) noexcept { return is_failed(i); });
  }

  template <class execute_resolution_wave_fnt>
  MJZ_CX_FN bool run_one_callback(
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    return this->run_resolution_queries(
        std::forward<execute_resolution_wave_fnt>(execute_resolution_wave_fn));
  }

  template <class execute_resolution_wave_fnt>
  MJZ_CX_FN uintlen_t run_all_callback(
      uintlen_t limit,
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    while (limit && this->run_resolution_queries(execute_resolution_wave_fn))
      limit--;
    return limit;
  }

  MJZ_CX_FN std::span<const uintlen_t> get_connections_view() const noexcept {
    return this->m.m_connections_list;
  }

  MJZ_CX_FN void clear() noexcept {
    asserts(asserts.assume_rn, !events_running());
    this->m.m_nodes.clear();
    this->m.m_applied_list.clear();
    this->m.m_free_list.clear();
    this->m.m_apply_list.clear();
    this->m.m_connections_list.clear();
  }
};
}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_rdeps_FILE_