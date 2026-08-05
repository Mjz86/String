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

#ifndef MJZ_SRC_GRAPH_deps_FILE_
#define MJZ_SRC_GRAPH_deps_FILE_
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

template <class T, version_t version_v>
concept dependency_state_c = requires(std::remove_cvref_t<T> s) {
  requires std::is_enum_v<std::remove_cvref_t<T>>;
  { to_underlying(s) } noexcept -> std::unsigned_integral;
};

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
struct MJZ_trivially_relocatable base_state_space_t {
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
    return operator_and(operator_or(max_invalid_state_v <= current,
                                    max_invalid_state_v < trigger),
                        is_incomplete());
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
  MJZ_CX_FN success_t refresh(base_state_space_t fresh) noexcept {
    base_state_space_t me = *this;
    base_state_space_t cpy = me;
    std::ignore =
        cpy.error(" the 'current' of a consumer must not exceed its provider ");
    cpy = bit_branchless_teranary(fresh.current < me.current, cpy, fresh);
    *this = bit_branchless_teranary(me.is_unrecoverable(), me, cpy);
    return !is_unrecoverable();
  }
  MJZ_CX_ND_FN bool error(auto &&) noexcept {
    //  A then B is solved , but we suddenly changed A under B's feet to
    //  be for example C then A , but we have sent a signal already that A
    //  is ready
    trigger = std::min(
        std::min(trigger, states_e(to_underlying(max_invalid_state_v) - 1)),
        current);
    current = max_invalid_state_v;
    return false;
  }
  MJZ_CX_FN decltype(auto)
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return state_to_str_impl_<version_v>(current);
  };
};

template <version_t version_v,
          dependency_state_c<version_v> auto max_invalid_state_v>
struct MJZ_trivially_relocatable directed_state_space_t
    : base_state_space_t<version_v, max_invalid_state_v> {
  using base = base_state_space_t<version_v, max_invalid_state_v>;
  using self_t = directed_state_space_t;
  using states_e = typename base::states_e;
  using node_id_t = base_node_id_t<version_v>;

  constexpr static inline uintlen_t active_trigger_tag_v = uintlen_t(-1);
  constexpr static inline uintlen_t passive_trigger_tag_v = uintlen_t(-2);
  // it cant be a "Compressed Sparse Row", my graph evolves dynamically.
  intlen_t m_connections_begin_index{};
  intlen_t m_connections_length{};
  //
  // active_connections is latched to one of -2 or -1  when is_triggered,
  // no amount of connection_count()  will go up to uintlen_t(-3).
  // active_connections grows until reinitilized.
  uintlen_t active_connections{};

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
  MJZ_CX_AL_FN uintlen_t emplace_back_valid_impl() noexcept {
    uintlen_t i = connection_index_ids().ending();
    m_connections_length = bit_branchless_teranary<intlen_t>(
        m_connections_length < 0, m_connections_length - 1,
        m_connections_length + 1);

    return i;
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

  MJZ_CX_AL_FN bool refresh_dependency_triggers(base old, base fresh) noexcept {
    base t = old;
    bool bad = !t.refresh(fresh);
    const self_t cme = *this;
    self_t me = cme;
    bad |= (fresh.current < me.trigger || me.trigger <= old.current);
    bool good = me.dec_zero_in_dgree();
    *this = bit_branchless_teranary(bad, cme, me);
    return !bad && good;
  }
  MJZ_CX_AL_FN bool can_trigger() const noexcept {
    return operator_and(
        operator_and(!this->is_unrecoverable(), !this->is_triggered()),
        operator_and(active_connections == connection_count(),
                     this->is_incomplete()));
  }

  MJZ_CX_AL_FN success_t refresh_dependency_defuse(base fresh) noexcept {
    const self_t cme = *this;
    self_t me = cme;
    bool bad = fresh.is_unrecoverable();
    self_t lessme = me;
    lessme.dec_zero_in_dgree();
    self_t badme = me;
    bool baded = fresh.current < me.current;
    std::ignore = badme.error(
        " the 'current' of a consumer must not exceed its provider ");
    me = bit_branchless_teranary(fresh.current < me.trigger, me, lessme);
    me = bit_branchless_teranary(baded, badme, me);
    me = bit_branchless_teranary(fresh.is_incomplete(), me, lessme);
    *this = bit_branchless_teranary(bad, cme, me);
    return !(bad || baded);
  }

  MJZ_CX_AL_FN success_t make_dependency(base fresh) noexcept {
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
  MJZ_CX_ND_FN bool is_actively_triggered() const noexcept {
    return active_connections == active_trigger_tag_v &&
           (asserts(this->is_triggered()), true);
  }
  MJZ_CX_ND_FN bool is_passively_triggered() const noexcept {
    return active_connections == passive_trigger_tag_v &&
           (asserts(this->is_triggered()), true);
  }
  MJZ_CX_AL_FN void actively_trigger() noexcept {
    active_connections = active_trigger_tag_v;
    this->trigger = this->current;
  }

  MJZ_CX_ND_FN bool passively_trigger() noexcept {
    bool can_do = is_actively_triggered();
    active_connections = bit_branchless_teranary(can_do, passive_trigger_tag_v,
                                                 active_connections);
    return can_do;
  }

  MJZ_CX_ND_FN std::optional<states_e> get_passive_trigger() const noexcept {
    return bit_branchless_teranary<std::optional<states_e>>(
        is_passively_triggered(), this->trigger, std::nullopt);
  }

  MJZ_CX_AL_FN success_t defuse(base fresh) noexcept {
    self_t me = *this;
    self_t mer = me;
    bool bad = fresh.is_triggered();
    std::ignore =
        mer.error(" any ignite must be eventually call defuse before the node "
                  "or its dependancies can update , calling defuse with "
                  "exepectation of an ignite is not allowed ");

    me.active_connections = 0;
    bool good = me.refresh(fresh);
    *this = bit_branchless_teranary(bad, mer, me);
    return !bad && good;
  }
  MJZ_CX_AL_FN bool dec_zero_in_dgree() noexcept {
    const self_t cme = *this;
    self_t me = cme;
    bool bad = me.is_triggered();
    bool good = ++me.active_connections == me.connection_count();
    *this = bit_branchless_teranary(bad, cme, me);
    return !bad && good;
  }
  MJZ_CX_AL_FN mjz::bstr_ns::basic_str_t<version_v> format_node_state_direct(
      std::span<const uintlen_t> connections_list) const noexcept {
    base direc = *this;
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "current({}),trigger({}),active({}),needed({}),"
                       "connections({})">(),
        state_to_str_impl_<version_v>(direc.current),
        state_to_str_impl_<version_v>(direc.trigger),
        intlen_t(active_connections), connection_count(),
        get_connections(connections_list));
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

  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return format_node_state_direct({});
  };
};

enum class basic_dependency_graph_flags_e : uint8_t {
  none = 0,
  forward_bit = 0b1,
  backward_bit = 0b10,
  query_me_bit = 0b100,
  query_deps_bit = 0b1000,
  unresolved_fail_bit = 0b10000,
  flag_all = query_me_bit | query_deps_bit | unresolved_fail_bit,
  forward = flag_all | forward_bit,
  backward = flag_all | backward_bit,
  bidirectional = backward | forward,
};
MJZ_ENUM_MATH_FOR(basic_dependency_graph_flags_e);

template <version_t version_v, auto max_invalid_state_v>
struct MJZ_maybe_trivially_relocatable basic_dependency_graph_base_t {
public:
  using flags_e = basic_dependency_graph_flags_e;
  using state_space_t = base_state_space_t<version_v, max_invalid_state_v>;
  // if max_invalid_state_v is in node_state , the node had a fatal error
  using states_e = state_space_t::states_e;

  using node_id_t = base_node_id_t<version_v>;
  ///////////////////////////////

  using node_direction_t =
      directed_state_space_t<version_v, max_invalid_state_v>;

  using dependency_node_data_t = std::array<node_direction_t, 2>;

  struct dependency_node_t {

    alignas(std::bit_floor(sizeof(dependency_node_data_t)))
        dependency_node_data_t directions;
  };
  static_assert(std::is_trivially_move_constructible_v<dependency_node_t>);
  static_assert(std::is_trivially_copy_constructible_v<dependency_node_t>);
  static_assert(std::is_trivially_destructible_v<dependency_node_t>);

protected:
  std::array<intlen_t, std::bit_width(uintlen_t(-1))>
      connections_free_list_heads{};
  uintlen_t connections_free_list_mask{};
  uintlen_t expected_edges_per_node_v{0};
  std::vector<dependency_node_t> nodes{};
  std::vector<uintlen_t> applied_list{};
  std::vector<uintlen_t> apply_list{};
  std::vector<uintlen_t> connections_list{};
  MJZ_CX_FN mjz::bstr_ns::basic_str_t<version_v>
  format_node_state(node_id_t me) const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
            version_v, "[id({}),forward:({}),backward:({})]\n">(),
        me.index(),
        dependency(me, true).format_node_state_direct(connections_list),
        dependency(me, false).format_node_state_direct(connections_list));
  }
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
  MJZ_CX_AL_FN dependency_node_data_t &dependency_node(node_id_t me) noexcept {
    return assume_aligned<std::bit_floor(sizeof(dependency_node_data_t))>(
               &nodes[me.index()])
        ->directions;
  }
  MJZ_CX_AL_FN const dependency_node_data_t &
  dependency_node(node_id_t me) const noexcept {
    return assume_aligned<std::bit_floor(sizeof(dependency_node_data_t))>(
               &nodes[me.index()])
        ->directions;
  }
  MJZ_CX_AL_FN node_direction_t &dependency(node_id_t me,
                                            bool direction) noexcept {
    return dependency_node(me)[!direction];
  }
  MJZ_CX_AL_FN const node_direction_t &
  dependency(node_id_t me, bool direction) const noexcept {
    return dependency_node(me)[!direction];
  }
  MJZ_CX_AL_FN bool
  make_resolution_query_impl_(bool &can_trigger_,
                              node_direction_t &node_dependency_,
                              const node_direction_t &node_opposite_) noexcept {
    node_direction_t node_dependency = node_dependency_;
    node_direction_t node_opposite = node_opposite_;
    can_trigger_ &= node_dependency.can_trigger();
    node_dependency.actively_trigger();
    bool duplicate_event_request = node_opposite.is_actively_triggered();
    node_dependency_ = bit_branchless_teranary(can_trigger_, node_dependency,
                                               node_dependency_);
    return can_trigger_ && !duplicate_event_request;
  }

  MJZ_CX_AL_FN bool make_resolution_query(node_id_t id,
                                          bool direction) noexcept {
    bool can_trigger_{true};
    node_direction_t &node_dependency = dependency(id, direction);
    node_direction_t &node_opposite = dependency(id, !direction);
    if (make_resolution_query_impl_(can_trigger_, node_dependency,
                                    node_opposite))
      apply_list.emplace_back(id.index());
    return can_trigger_;
  }

  MJZ_CX_FN bool defuse_resolution(node_id_t id,
                                   tuple_t<bool, bool, bool> flag_args,
                                   state_space_t space) noexcept {
    const auto [direction, query_me, query_deps] = flag_args;
    node_direction_t node_dependency = dependency(id, direction);
    const node_direction_t node_opposite = dependency(id, !direction);
    const auto connections_span =
        node_dependency.get_connections(connections_list);
    const auto opposite_connections_span =
        node_opposite.get_connections(connections_list);
    auto prefetch_elem_at_fn =
        [&, my_i = uintlen_t(id.index())](std::span<const uintlen_t> range,
                                          uintlen_t i) noexcept {
          if constexpr (mjz_do_prefetch_v) {
            bool in_bounds = i < range.size();
            i = bit_branchless_teranary(in_bounds, i, i ^ i);
            i = bit_branchless_teranary(in_bounds, range.data(), &my_i)[i];
            mjz_prefetch(dependency(node_id_t(i), direction));
          }
        };
    MJZ_RAII_RELEASE { dependency(id, direction) = node_dependency; };
    constexpr uintlen_t prefetch_batch_v = 4;

    for (uintlen_t i{}; i < prefetch_batch_v; i++) {
      prefetch_elem_at_fn(connections_span, i);
      prefetch_elem_at_fn(opposite_connections_span, i);
    }

    const state_space_t old{node_dependency};
    if (!node_dependency.is_passively_triggered() ||
        !node_dependency.defuse(space))
      MJZ_MAYBE_UNLIKELY { return false; }

    //   Kahn's Algorithm , section of in-dgree re-initilization of node.
    const state_space_t fresh{node_dependency};
    for (uintlen_t i{}; i < connections_span.size(); i++) {
      prefetch_elem_at_fn(connections_span, i + prefetch_batch_v);

      auto dep_i = node_id_t(connections_span[i]);
      node_dependency.refresh_dependency_defuse(dependency(dep_i, direction));
    };
    uintlen_t size_real_apply_list_size{apply_list.size()};
    uintlen_t size_reserve_apply_list{
        uintlen_t(query_me) +
        uintlen_t(query_deps) * opposite_connections_span.size() + 1};
    apply_list.resize(apply_list.size() + size_reserve_apply_list);

    MJZ_RAII_RELEASE {
      const bool plese_optimize_ =
          size_real_apply_list_size < apply_list.size();
      asserts(plese_optimize_);
      MJZ_JUST_ASSUME_(plese_optimize_);
      if (plese_optimize_) {
        apply_list.resize(size_real_apply_list_size);
      }
    };
    auto speculitive_apply_list = std::span(apply_list);

    bool can_trigger_{query_me};
    can_trigger_ = make_resolution_query_impl_(can_trigger_, node_dependency,
                                               node_opposite);
    speculitive_apply_list[size_real_apply_list_size] = id.index();
    size_real_apply_list_size += can_trigger_;

    if (node_dependency.is_unrecoverable())
      MJZ_MAYBE_UNLIKELY { return false; }

    //   Kahn's Algorithm , section of in-dgree 0  ( opposite direction  )
    //   dgree propagation.

    for (uintlen_t i{}; i < opposite_connections_span.size(); i++) {
      prefetch_elem_at_fn(opposite_connections_span, i + prefetch_batch_v);
      uintlen_t dep_i = opposite_connections_span[i];
      node_id_t dp_id = node_id_t(dep_i);
      node_direction_t &node_dependant_ = dependency(dp_id, direction);
      const node_direction_t &node_dependant_opposite =
          dependency(dp_id, !direction);
      node_direction_t node_dependant = node_dependant_;
      can_trigger_ = node_dependant.refresh_dependency_triggers(old, fresh);
      can_trigger_ &= query_deps;
      can_trigger_ = make_resolution_query_impl_(can_trigger_, node_dependant,
                                                 node_dependant_opposite);
      node_dependant_ = node_dependant;
      speculitive_apply_list[size_real_apply_list_size] = dep_i;
      size_real_apply_list_size += can_trigger_;
    };
    return !node_dependency.is_unrecoverable();
  }
  MJZ_CX_AL_FN bool events_running() const noexcept {
    return !!applied_list.size();
  }
  template <class execute_resolution_wave_fnt>
  MJZ_CX_FN bool run_resolution_queries(
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    asserts(asserts.assume_rn, !events_running());
    MJZ_RAII_RELEASE {
      applied_list.clear();
      asserts(asserts.assume_rn, !events_running());
    };
    std::swap(applied_list, apply_list);
    for (uintlen_t j : applied_list) {
      node_id_t id = node_id_t(j);
      bool forward_active = dependency(id, true).passively_trigger();
      bool backward_active = dependency(id, false).passively_trigger();
      asserts(asserts.assume_rn, forward_active || backward_active);
    }
    auto id_view = applied_list |
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
  };
  MJZ_CX_AL_FN void
  reserve_edge_connection_impl(bool direction, node_id_t id,
                               uintlen_t new_capacity,
                               bool &implicit_field) noexcept {
    node_direction_t &new_direct = dependency(id, direction);
    node_direction_t old_direct = new_direct;
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
  MJZ_CX_FN success_t make_edge_impl2(bool direction, node_id_t dependency_i,
                                      node_id_t dependant_i,
                                      uintlen_t extra_later) noexcept {
    if (!dependency(dependant_i, direction)
             .make_dependency(dependency(dependency_i, direction)))
      return false;

    uintlen_t count_sz = dependency(dependant_i, direction).connection_count();

    uintlen_t cap =
        dependency(dependant_i, direction).get_capacity(connections_list);
    if (count_sz < cap) {
      connections_list[dependency(dependant_i, direction)
                           .emplace_back_valid_impl()] = dependency_i.index();
      return true;
    }

    uintlen_t geo_cap = std::bit_ceil(count_sz + 1);
    extra_later = std::max(std::max(expected_edges_per_node_v, geo_cap),
                           count_sz + std::max<uintlen_t>(extra_later, 1));
    bool implicit_field = geo_cap == extra_later;
    reserve_edge_connection_impl(direction, dependant_i, extra_later,
                                 implicit_field);
    connections_list[dependency(dependant_i, direction)
                         .emplace_back_valid_impl()] = dependency_i.index();

    if (implicit_field)
      dependency(dependant_i, direction).m_connections_length =
          -dependency(dependant_i, direction).m_connections_length;
    return true;
  }

  MJZ_CX_FN success_t make_edge_impl(bool direction, node_id_t dependency_i,
                                     node_id_t dependant_i,
                                     uintlen_t extra_later_dependency,
                                     uintlen_t extra_later_dependant) noexcept {
    if (dependant_i == dependency_i)
      return false;

    if constexpr (mjz_do_prefetch_v) {
      auto &&a1 = mjz_prefetch(dependency(dependant_i, true))
                      .get_connections(connections_list);
      auto &&a2 = mjz_prefetch(dependency(dependant_i, false))
                      .get_connections(connections_list);
      auto &&a3 = mjz_prefetch(dependency(dependency_i, true))
                      .get_connections(connections_list);
      auto &&a4 = mjz_prefetch(dependency(dependency_i, false))
                      .get_connections(connections_list);
      mjz_prefetch_p(a1.data() + a1.size());
      mjz_prefetch_p(a2.data() + a2.size());
      mjz_prefetch_p(a3.data() + a3.size());
      mjz_prefetch_p(a4.data() + a4.size());
    }
    return make_edge_impl2(direction, dependency_i, dependant_i,
                           extra_later_dependant) &&
           make_edge_impl2(!direction, dependant_i, dependency_i,
                           extra_later_dependency);
  }

  MJZ_CX_FN void edge_reserve_impl(uintlen_t extra_later) noexcept {
    extra_later = extra_later * 2 + 2;
    if (connections_list.size() + extra_later < connections_list.capacity())
      return;
    connections_list.reserve(
        connections_list.size() +
        std::max(connections_list.capacity(), extra_later));
  }

  MJZ_CX_FN dependency_node_t make_node_temp_impl(
      state_space_t forward_space, state_space_t backward_space) noexcept {
    dependency_node_data_t node{};
    node[0].defuse(forward_space);
    node[1].defuse(backward_space);
    return {.directions = node};
  }

  MJZ_CX_ND_FN node_id_t
  make_nodes_impl(uintlen_t count_nodes, state_space_t forward_space,
                  state_space_t backward_space) noexcept {
    uintlen_t i = node_count();
    nodes.resize(i + count_nodes,
                 make_node_temp_impl(forward_space, backward_space));
    return node_id_t(i);
  }

  MJZ_CX_FN void reserve_impl(uintlen_t node_estimate,
                              uintlen_t edge_estimate) noexcept {

    nodes.reserve(1 + node_estimate);
    applied_list.reserve(1 + node_estimate);
    apply_list.reserve(1 + node_estimate);
    connections_list.reserve((16 + node_estimate + edge_estimate) * 2);
  }

  MJZ_CX_ND_FN success_t query_bimpl(node_id_t id, bool direction) noexcept {
    return make_resolution_query(id, direction);
  }

  MJZ_CX_ND_FN success_t defuse_bimpl(node_id_t id, state_space_t fresh,
                                      bool direction, bool query_me,
                                      bool query_deps) noexcept {
    return defuse_resolution(id, tuple_t{direction, query_me, query_deps},
                             fresh);
  }
  MJZ_CX_FN tuple_t<uintlen_t, uintlen_t, uintlen_t>
  seed_all_nodes_bimpl(bool forward_, bool backward_) noexcept {
    uintlen_t backward_seeds = 0;
    uintlen_t forward_seeds = 0;
    uintlen_t invalid_states = 0;
    for (uintlen_t i = 0; i < node_count(); ++i) {
      if (backward_) {
        backward_seeds += query_backward(node_id_t(i));
      }
      if (forward_) {
        forward_seeds += query_forward(node_id_t(i));
      }
      invalid_states +=
          is_unrecoverable_bimpl(node_id_t(i), forward_, backward_);
    }
    return {forward_seeds, backward_seeds, invalid_states};
  }

  MJZ_CX_ND_FN bool is_unrecoverable_bimpl(node_id_t i, bool forward_,
                                           bool backward_) const noexcept {

    return (backward_ && dependency_node(i)[1].is_unrecoverable()) ||
           (forward_ && dependency_node(i)[0].is_unrecoverable());
  }
  MJZ_CX_ND_FN bool is_unresolved_bimpl(node_id_t i, bool forward_,
                                        bool backward_) const noexcept {
    auto &node = nodes[i.index()];
    return (backward_ && dependency_node(i)[1].is_incomplete()) ||
           (forward_ && dependency_node(i)[0].is_incomplete());
  }
  MJZ_CX_ND_FN bool is_failed_bimpl(node_id_t i, bool forward_, bool backward_,
                                    bool unresolved_fail_) const noexcept {
    return is_unrecoverable_bimpl(i, forward_, backward_) ||
           (unresolved_fail_ && is_unresolved_bimpl(i, forward_, backward_));
  }
  MJZ_CX_ND_FN auto
  view_all_fail_ids_bimpl(bool forward_, bool backward_,
                          bool unresolved_fail_) const noexcept {
    return std::views::iota(uintlen_t(), uintlen_t(node_count())) |
           std::views::transform(
               [](uintlen_t i) noexcept { return node_id_t(i); }) |
           std::views::filter([this, backward_, forward_,
                               unresolved_fail_](node_id_t i) noexcept {
             return is_failed_bimpl(i, forward_, backward_, unresolved_fail_);
           });
  }

public:
  MJZ_CX_FN void reserve_per_node(uintlen_t edges_per_node_estimate) noexcept {
    expected_edges_per_node_v = edges_per_node_estimate;
  }
  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate) noexcept {
    return reserve_impl(node_estimate, edge_estimate);
  }

  MJZ_CX_ND_FN std::optional<dependency_node_t>
  get_node(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return {};
    return nodes[i.index()];
  }
  MJZ_CX_ND_FN optional_ref_t<const dependency_node_t>
  get_node_cref(node_id_t i) const noexcept {
    if (!is_inbounds(i))
      return {};
    return nodes[i.index()];
  }

  MJZ_CX_ND_FN mjz::bstr_ns::basic_str_t<version_v>
  format_graph_state() const noexcept {
    return mjz::bstr_ns::format_ns::format(
        bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v, "{}">,
        std::views::iota(uintlen_t(), uintlen_t(node_count())) |
            std::views::transform([this](uintlen_t i) noexcept {
              return format_node_state(node_id_t(i));
            }));
  }

#if MJZ_WITH_iostream
  MJZ_NCX_FN friend std::ostream &
  operator<<(std::ostream &cout_v, const basic_dependency_graph_base_t &obj) {
    return cout_v << obj.format_graph_state();
  }
#endif

  MJZ_CX_ND_FN success_t
  make_edges(node_id_t id, std::span<const node_id_t> child_is) noexcept {
    uintlen_t extra_later = child_is.size();
    edge_reserve_impl(extra_later);
    if (!is_inbounds(id))
      return false;
    for (node_id_t child_i : child_is) {
      if (!is_inbounds(child_i))
        return false;
      if (!make_edge_impl(true, id, child_i, extra_later, 0))
        return false;
      extra_later = 0;
    }
    return true;
  }
  MJZ_CX_ND_FN success_t make_edges(std::span<const node_id_t> ids,
                                    node_id_t child_i) noexcept {
    if (!is_inbounds(child_i))
      return false;
    uintlen_t extra_later = ids.size();
    edge_reserve_impl(extra_later);
    for (node_id_t id : ids) {
      if (!is_inbounds(id))
        return false;
      if (!make_edge_impl(false, child_i, id, extra_later, 0))
        return false;
      extra_later = 0;
    }
    return true;
  }
  MJZ_CX_ND_FN success_t make_edge(node_id_t id, node_id_t child_i) noexcept {
    if (!is_inbounds(child_i))
      return false;
    if (!is_inbounds(id))
      return false;
    edge_reserve_impl(1);
    return make_edge_impl(true, id, child_i, 0, 0);
  }
  MJZ_CX_ND_FN node_id_t make_node(state_space_t forward_space,
                                   state_space_t backward_space) noexcept {
    return make_nodes_impl(1, forward_space, backward_space);
  }

  MJZ_CX_ND_FN node_id_t
  make_nodes_get_first(uintlen_t count, state_space_t forward_space,
                       state_space_t backward_space) noexcept {
    return make_nodes_impl(count, forward_space, backward_space);
  }
  MJZ_CX_FN void make_nodes(std::span<node_id_t> out_is,
                            state_space_t forward_space,
                            state_space_t backward_space) noexcept {
    uintlen_t i =
        make_nodes_impl(out_is.size(), forward_space, backward_space).index();
    for (node_id_t &id_ : out_is)
      id_ = node_id_t(i++);
  }

  MJZ_CX_AL_FN uintlen_t node_count() const noexcept { return nodes.size(); }
  MJZ_CX_AL_FN bool is_inbounds(node_id_t id) const noexcept {
    return id.index() < node_count();
  }
  MJZ_CX_ND_FN success_t query(node_id_t id, flags_e direction) noexcept {
    if (!is_inbounds(id))
      return false;
    bool good = true;
    if (+(flags_e::forward_bit & direction)) {
      if (!query_bimpl(id, true))
        good = false;
    }
    if (+(flags_e::backward_bit & direction)) {
      if (!query_bimpl(id, false))
        good = false;
    }
    return good;
  }

  MJZ_CX_ND_FN success_t defuse(node_id_t id, state_space_t fresh,
                                flags_e direction) noexcept {
    if (!is_inbounds(id))
      return false;
    bool good = true;
    bool query_me = !!(flags_e::query_me_bit & direction);
    bool query_deps = !!(flags_e::query_deps_bit & direction);
    if (+(flags_e::forward_bit & direction)) {
      if (!defuse_bimpl(id, fresh, true, query_me, query_deps))
        good = false;
    }
    if (+(flags_e::backward_bit & direction)) {
      if (!defuse_bimpl(id, fresh, false, query_me, query_deps))
        good = false;
    }
    return good;
  }
  MJZ_CX_ND_FN success_t query_forward(node_id_t id) noexcept {
    return query(id, flags_e::forward);
  }
  MJZ_CX_ND_FN success_t query_backward(node_id_t id) noexcept {
    return query(id, flags_e::backward);
  }
  MJZ_CX_ND_FN success_t query_bidirectional(node_id_t id) noexcept {
    return query(id, flags_e::bidirectional);
  }
  constexpr static inline state_space_t resolved_state_v =
      state_space_t{max_invalid_state_v, max_invalid_state_v};

  MJZ_CX_ND_FN success_t defuse_forward(
      node_id_t id, state_space_t fresh = resolved_state_v) noexcept {
    return defuse(id, fresh, flags_e::forward);
  }
  MJZ_CX_ND_FN success_t defuse_backward(
      node_id_t id, state_space_t fresh = resolved_state_v) noexcept {
    return defuse(id, fresh, flags_e::backward);
  }

  MJZ_CX_ND_FN success_t
  defuse_bidirectional(node_id_t id, state_space_t fresh_forward,
                       state_space_t fresh_backward) noexcept {
    return operator_or(defuse_forward(id, fresh_forward),
                       defuse_backward(id, fresh_backward));
  }

  MJZ_CX_FN tuple_t<uintlen_t, uintlen_t, uintlen_t>
  seed_all_nodes(flags_e direction = flags_e::bidirectional) noexcept {
    return seed_all_nodes_bimpl(!!(flags_e::forward_bit & direction),
                                !!(flags_e::backward_bit & direction));
  }

  MJZ_CX_ND_FN bool
  is_unrecoverable(node_id_t i,
                   flags_e direction = flags_e::bidirectional) const noexcept {
    if (!is_inbounds(i))
      return true;
    return is_unrecoverable_bimpl(i, !!(flags_e::forward_bit & direction),
                                  !!(flags_e::backward_bit & direction));
  }

  MJZ_CX_ND_FN bool
  is_failed(node_id_t i,
            flags_e direction = flags_e::bidirectional) const noexcept {
    if (!is_inbounds(i))
      return true;
    return is_failed_bimpl(i, !!(flags_e::forward_bit & direction),
                           !!(flags_e::backward_bit & direction),
                           !!(flags_e::unresolved_fail_bit & direction));
  }

  MJZ_CX_ND_FN bool
  is_unresolved(node_id_t i,
                flags_e direction = flags_e::bidirectional) const noexcept {
    if (!is_inbounds(i))
      return true;
    return is_unresolved_bimpl(i, !!(flags_e::forward_bit & direction),
                               !!(flags_e::backward_bit & direction));
  }

  MJZ_CX_ND_FN auto
  view_all_fail_ids(flags_e direction = flags_e::bidirectional) const noexcept {
    return view_all_fail_ids_bimpl(
        !!(flags_e::forward_bit & direction),
        !!(flags_e::backward_bit & direction),
        !!(flags_e::unresolved_fail_bit & direction));
  }

  template <class execute_resolution_wave_fnt>
  MJZ_CX_ND_FN bool run_one_callback(
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    return run_resolution_queries(
        std::forward<execute_resolution_wave_fnt>(execute_resolution_wave_fn));
  }

  template <class execute_resolution_wave_fnt>
  MJZ_CX_FN uintlen_t run_all_callback(
      uintlen_t limit,
      execute_resolution_wave_fnt &&execute_resolution_wave_fn) noexcept {
    while (limit && run_resolution_queries(execute_resolution_wave_fn))
      limit--;
    return limit;
  }

  MJZ_CX_FN auto basic_format_specs_formatted_pv_fn_(auto &&) const noexcept {
    return format_graph_state();
  };
  MJZ_CX_FN void clear() noexcept {
    asserts(asserts.assume_rn, !events_running());
    nodes.clear();
    expected_edges_per_node_v = 0;
    applied_list.clear();
    apply_list.clear();
    connections_list.clear();
    connections_free_list_mask = 0;
    for (auto &e : connections_free_list_heads)
      e = 0;
  }
};
}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_deps_FILE_
