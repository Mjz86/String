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

#ifndef MJZ_SRC_GRAPH_udeps_FILE_
#define MJZ_SRC_GRAPH_udeps_FILE_
#include "bdeps.hpp"
MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v>
struct MJZ_trivially_relocatable uni_dependancy_node_base_t {
  enum state_mask_e : uintlen_t {
    sentinel_v = 0b00,
    ready_v = 0b01,
    active_v = 0b10,
    passive_v = 0b11,
    mask_v = 0b11,
    mask_width_v = 2,
  };

  uintlen_t node_in_dgree_and_mask{ready_v};

  MJZ_CX_FN uintlen_t in_dgree() const noexcept {
    return node_in_dgree_and_mask >> mask_width_v;
  }
  MJZ_CX_AL_FN uintlen_t inc_dgree() noexcept {
    asserts(asserts.assume_rn, !is_sentinel());
    node_in_dgree_and_mask += mask_v + 1;
    return in_dgree();
  }
  MJZ_CX_AL_FN uintlen_t dec_dgree() noexcept {
    asserts(asserts.assume_rn, !is_sentinel());
    node_in_dgree_and_mask -= mask_v + 1;
    return in_dgree();
  }
  MJZ_CX_FN state_mask_e state() const noexcept {
    return state_mask_e(node_in_dgree_and_mask & mask_v);
  }
  MJZ_CX_FN void set_state(state_mask_e s) noexcept {
    node_in_dgree_and_mask &= ~mask_v;
    node_in_dgree_and_mask |= uintlen_t(s);
  }

  MJZ_CX_FN bool is_sentinel() const noexcept { return state() == sentinel_v; }
  MJZ_CX_FN bool is_ready() const noexcept { return state() == ready_v; }
  MJZ_CX_FN bool is_passively_triggered() const noexcept {
    return state() == passive_v;
  }
  MJZ_CX_FN bool is_actively_triggered() const noexcept {
    return state() == active_v;
  }
  MJZ_CX_FN bool can_trigger() const noexcept {
    return operator_and(!in_dgree(), is_ready());
  }
  MJZ_CX_FN bool is_complete() const noexcept {
    return operator_and(!in_dgree(), is_sentinel());
  }
  MJZ_CX_FN bool is_unrecoverable() const noexcept {
    return operator_and(in_dgree(), is_sentinel());
  }

  MJZ_CX_AL_FN void actively_trigger() noexcept {
    asserts(asserts.assume_rn, can_trigger());
    set_state(active_v);
    asserts(asserts.assume_rn, is_actively_triggered());
  }
  MJZ_CX_AL_FN uintlen_t speculative_dec_dgree(bool &exeute) noexcept {
    exeute &= !is_sentinel();
    node_in_dgree_and_mask -=
        bit_branchless_teranary(exeute, mask_v + 1, uintlen_t());
    return in_dgree();
  }

  MJZ_CX_AL_FN bool speculative_actively_trigger(bool exeute) noexcept {
    exeute &= can_trigger();
    set_state(bit_branchless_teranary(exeute, active_v, state()));
    return exeute;
  }
  MJZ_CX_FN bool speculative_error(bool exeute, auto &&) noexcept {
    node_in_dgree_and_mask += bit_branchless_teranary(
        operator_and(exeute, !in_dgree()), mask_v + 1, uintlen_t());
    set_state(bit_branchless_teranary(exeute, sentinel_v, state()));
    return !exeute;
  }

  MJZ_CX_AL_FN void passively_trigger() noexcept {
    asserts(asserts.assume_rn, is_actively_triggered());
    set_state(passive_v);
    asserts(asserts.assume_rn, is_passively_triggered());
  }
  MJZ_CX_AL_FN void as_sentinel() noexcept {
    asserts(asserts.assume_rn, !is_unrecoverable());
    set_state(sentinel_v);
  }
  MJZ_CX_FN bool error(auto &&) noexcept {
    if (!in_dgree()) {
      node_in_dgree_and_mask += mask_v + 1;
    }
    set_state(sentinel_v);
    return false;
  }
  MJZ_CX_FN bool defuse(bool is_completed) noexcept {
    asserts(asserts.assume_rn, is_passively_triggered());
    set_state(is_completed ? sentinel_v : ready_v);
    return !is_unrecoverable();
  }
};

template <version_t version_v>
struct MJZ_trivially_relocatable uni_dependancy_node_t
    : base_edges_ids_t<version_v>,
      uni_dependancy_node_base_t<version_v> {
  MJZ_CX_AL_FN mjz::bstr_ns::basic_str_t<version_v> format_node_state_direct(
      std::span<const uintlen_t> connections_list_) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "state({}),in_degree({}),connections({})">(),
        intlen_t(this->state()), this->in_dgree(),
        this->get_connections(connections_list_));
  }
  MJZ_CX_AL_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state_direct_dot() const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "state:{}\nin_degree:{}">(),
        intlen_t(this->state()), this->in_dgree());
  }
  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return this->format_node_state_direct({});
  }
};
 

template <version_t version_v>
struct MJZ_maybe_trivially_relocatable basic_uni_dependency_graph_base_t {
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

  MJZ_CX_AL_FN bool make_resolution_query(node_id_t id) noexcept {
    dependency_node_t &node_dependency = dependency(id);
    if (!node_dependency.can_trigger())
      return false;
    node_dependency.actively_trigger();
    m.m_apply_list.emplace_back(id.index());
    return true;
  }

  MJZ_CX_FN bool
  defuse_resolution(node_id_t id,
                    tuple_t<bool, bool, bool> flag_args) noexcept {
    const auto [as_complete, query_me, query_deps] = flag_args;
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
      asserts(plese_optimize_);
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
      if (!base_node.is_passively_triggered() || !base_node.defuse(as_complete))
        MJZ_IS_UNLIKELY { return false; }
      *apply_list_ptr = id.index();
      apply_list_ptr += base_node.speculative_actively_trigger(query_me);
      if (base_node.is_unrecoverable())
        MJZ_IS_UNLIKELY { return false; }
    }
    if (!as_complete)
      return true;
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
      bool exeute = dep_node.speculative_error(dep_node.is_sentinel(),
                                               "premature completion");
      exeute &= dep_node.speculative_dec_dgree(exeute) == 0;
      if (query_deps) {
        exeute = dep_node.speculative_actively_trigger(exeute);
        *apply_list_ptr = dep_id.index();
        apply_list_ptr += exeute;
      }
      dep_node_ref = dep_node;
    }
    return true;
  }

  MJZ_CX_FN dependency_node_t make_node_temp_impl(bool complete) noexcept {
    dependency_node_t node{};
    if (complete)
      node.as_sentinel();
    return node;
  }

  MJZ_CX_AL_FN uintlen_t node_count() const noexcept {
    return this->m.m_nodes.size();
  }
  MJZ_CX_FN node_id_t make_nodes_impl(uintlen_t count_nodes,
                                      bool complete) noexcept {
    uintlen_t i = node_count();
    m.m_nodes.resize(i + count_nodes, make_node_temp_impl(complete));
    return node_id_t(i);
  }

  MJZ_CX_FN success_t
  make_edge_impl2(bool ret, node_id_t dependency_i, node_id_t dependant_i,
                  uintlen_t extra_later_dependant) noexcept {
    auto dep_node = dependency(dependency_i);
    m.m_connections_list.edge_list_push_back(dep_node, dependant_i.index(),
                                             extra_later_dependant);
    dependency(dependency_i) = dep_node;
    return ret;
  }
  MJZ_CX_FN success_t make_edge_impl(node_id_t dependency_i,
                                     node_id_t dependant_i,
                                     uintlen_t extra_later_dependant) noexcept {
    if (dependant_i == dependency_i)
      return false;
    if (dependency(dependency_i).is_sentinel()) {
      return make_edge_impl2(true, dependency_i, dependant_i,
                             extra_later_dependant);
    }
    if (!dependency(dependant_i).is_sentinel()) {
      dependency(dependant_i).inc_dgree();
      return make_edge_impl2(true, dependency_i, dependant_i,
                             extra_later_dependant);
    }
    return make_edge_impl2(
        dependency(dependant_i).error("premature completion"), dependency_i,
        dependant_i, extra_later_dependant);
  }

  MJZ_CX_FN void edge_reserve_impl(uintlen_t extra_later) noexcept {
    return m.m_connections_list.reserve_edge_list(extra_later);
  }

  MJZ_CX_FN void reserve_impl(uintlen_t node_estimate,
                              uintlen_t edge_estimate) noexcept {
    m.m_nodes.reserve(1 + node_estimate);
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
#if MJZ_WITH_iostream
  MJZ_NCX_FN friend std::ostream &
  operator<<(std::ostream &cout_v,
             const basic_uni_dependency_graph_base_t &obj) {
    return cout_v << obj.format_graph_dot();
  }
#endif

  MJZ_CX_FN void reserve_per_node(uintlen_t edges_per_node_estimate) noexcept {
    this->m.m_connections_list.edge_reserve_all(edges_per_node_estimate);
  }

  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate) noexcept {
    return this->reserve_impl(node_estimate, edge_estimate);
  }

  MJZ_CX_FN std::optional<dependency_node_t>
  get_node(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return {};
    return this->m.m_nodes[i.index()];
  }

  MJZ_CX_FN optional_ref_t<const dependency_node_t>
  get_node_cref(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return {};
    return this->m.m_nodes[i.index()];
  }

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  format_graph_state() const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v, "{}">,
        std::views::iota(uintlen_t(), uintlen_t(node_count())) |
            std::views::transform([this](uintlen_t i) noexcept {
              return this->format_node_state(node_id_t(i));
            }));
  }

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v> format_graph_dot(
      mjz::bstr_ns::basic_string_view_t<version_v> name =
          mjz::bstr_ns::static_string_view_t<version_v>("G")) const noexcept {
    return format_graph_dot(name, mjz::bstr_ns::static_string_view_t<version_v>(
                                      "node [shape=circle, style=filled, "
                                      "fillcolor=lightblue,fontname="
                                      "\"Helvetica\"];edge [color=gray40];"));
  }

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v> format_graph_dot(
      mjz::bstr_ns::basic_string_view_t<version_v> name,
      mjz::bstr_ns::basic_string_view_t<version_v> style) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v,
                                                          R"RAW(digraph {} {{
            {}
            {:s}
                                                                }};)RAW">,
        name, style,
        std::views::iota(uintlen_t(), uintlen_t(node_count())) |
            std::views::transform([this](uintlen_t i) noexcept {
              auto itxt = this->format_node_state_dot(node_id_t(i));
              return mjz::bstr_ns::format_ns::format(
                  bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
                      version_v, "{}; {:s:s}\n">,
                  itxt,
                  this->dependency(node_id_t(i))
                          .get_connections(get_connections_view()) |
                      std::views::transform(
                          [&](uintlen_t dependant_index) noexcept {
                            return tuple_t(
                                itxt,
                                bstr_ns::format_ns::fmt_litteral_ns::
                                    operator_fmt<version_v, " -> ">(),
                                this->format_node_state_dot(
                                    node_id_t(dependant_index)),
                                ';');
                          }));
            }));
  }

  MJZ_CX_FN
  success_t make_edges(node_id_t dependency_i,
                       std::span<const node_id_t> dependant_ids) noexcept {
    if (!is_inbounds(dependency_i))
      return false;
    uintlen_t extra_later = dependant_ids.size();
    this->edge_reserve_impl(extra_later);
    for (node_id_t dependant_i : dependant_ids) {
      if (!is_inbounds(dependant_i))
        return false;
      if (!this->make_edge_impl(dependency_i, dependant_i, extra_later))
        return false;
      extra_later--;
    }
    return true;
  }

  MJZ_CX_FN success_t make_edges(std::span<const node_id_t> dependency_ids,
                                 node_id_t dependant_i) noexcept {
    if (!is_inbounds(dependant_i))
      return false;
    this->edge_reserve_impl(dependency_ids.size());
    for (node_id_t dependency_i : dependency_ids) {
      if (!is_inbounds(dependency_i))
        return false;
      if (!this->make_edge_impl(dependency_i, dependant_i, 0))
        return false;
    }
    return true;
  }

  MJZ_CX_FN success_t make_edge(node_id_t dependency_i,
                                node_id_t dependant_i) noexcept {
    if (!is_inbounds(dependency_i) || !is_inbounds(dependant_i))
      return false;
    this->edge_reserve_impl(1);
    return this->make_edge_impl(dependency_i, dependant_i, 0);
  }

  MJZ_CX_FN node_id_t make_node(bool complete = false) noexcept {
    return this->make_nodes_impl(1, complete);
  }

  MJZ_CX_FN node_id_t make_nodes_get_first(uintlen_t count,
                                           bool complete = false) noexcept {
    return this->make_nodes_impl(count, complete);
  }

  MJZ_CX_FN void make_nodes(std::span<node_id_t> out_is,
                            bool complete = false) noexcept {
    uintlen_t i = this->make_nodes_impl(out_is.size(), complete).index();
    for (node_id_t &id_ : out_is)
      id_ = node_id_t(i++);
  }

  MJZ_CX_AL_FN bool is_inbounds(node_id_t id) const noexcept {
    return id.index() < node_count();
  }

  MJZ_CX_FN success_t query(node_id_t id) noexcept {
    if (!is_inbounds(id))
      return false;
    return this->make_resolution_query(id);
  }

  MJZ_CX_FN success_t defuse(node_id_t id, bool as_complete = true,
                             bool query_me = false,
                             bool query_deps = true) noexcept {
    if (!is_inbounds(id))
      return false;
    return this->defuse_resolution(id,
                                   tuple_t{as_complete, query_me, query_deps});
  }

  MJZ_CX_FN uintlen_t seed_all_nodes() noexcept {
    uintlen_t seeds = 0;
    for (uintlen_t i = 0; i < node_count(); ++i) {
      seeds += query(node_id_t(i));
    }
    return seeds;
  }

  MJZ_CX_FN bool is_unrecoverable(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return true;
    return this->dependency(i).is_unrecoverable();
  }

  MJZ_CX_FN bool is_complete(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return false;
    return this->dependency(i).is_complete();
  }

  MJZ_CX_FN bool is_unresolved(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return true;
    return !this->dependency(i).is_complete();
  }

  MJZ_CX_FN bool is_failed(node_id_t i,
                           bool unresolved_fail = false) const noexcept {
    return is_unrecoverable(i) || (unresolved_fail && is_unresolved(i));
  }

  MJZ_CX_FN auto
  view_all_fail_ids(bool unresolved_fail = false) const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(node_count())) |
           std::views::transform(
               [](uintlen_t i) noexcept { return node_id_t(i); }) |
           std::views::filter([this, unresolved_fail](node_id_t i) noexcept {
             return is_failed(i, unresolved_fail);
           });
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
  MJZ_CX_FN auto get_range_index_view() const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(node_count())) |
           std::views::transform(
               [this](uintlen_t i) noexcept -> std::span<const uintlen_t> {
                 return this->dependency(node_id_t(i))
                     .get_connections(get_connections_view());
               });
  }

  MJZ_CX_FN auto get_range_id_view() const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(node_count())) |
           std::views::transform([this](uintlen_t i) noexcept {
             return this->dependency(node_id_t(i))
                        .get_connections(get_connections_view()) |
                    std::views::transform(
                        [](uintlen_t j) noexcept { return node_id_t(j); });
           });
  }
  MJZ_CX_FN auto basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return format_graph_state();
  }

  MJZ_CX_FN std::span<const uintlen_t> get_connections_view() const noexcept {
    return this->m.m_connections_list;
  }

  MJZ_CX_FN void clear() noexcept {
    asserts(asserts.assume_rn, !events_running());
    this->m.m_nodes.clear();
    this->m.m_applied_list.clear();
    this->m.m_apply_list.clear();
    this->m.m_connections_list.clear();
  }
};

} // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_udeps_FILE_
