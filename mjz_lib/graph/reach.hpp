#ifndef MJZ_SRC_GRAPH_reach_FILE_
#define MJZ_SRC_GRAPH_reach_FILE_
#include "../unordered_vector.hpp"
#include "algo.hpp"

MJZ_EXPORT
namespace mjz::graph_ns {

template <version_t version_v> struct reachability_checker_t {

  using mjz_uint128_t_ = uintN_t<version_v, 128>;
  recycling_inline_two_leveled_interning_vector_t<version_v, mjz_uint128_t_>
      reachability_cache{};
  std::vector<mjz_uint128_t_> hyper_log_log{};
  std::vector<uintlen_t> dominance_interval_begin{};
  std::vector<uintlen_t> dominance_interval_end{};
  std::vector<uintlen_t> depth_first_interval_begin{};
  std::vector<uintlen_t> depth_first_interval_end{};
  std::vector<intlen_t> wave_seq{};
  std::vector<uintlen_t> begin_temp{};
  std::vector<uintlen_t> end_temp{};
  std::vector<uintlen_t> parent_temp{};
  std::vector<uintlen_t> forward_hyper_log_log_representitive_front{};
  std::vector<uintlen_t> backward_hyper_log_log_representitive_behind{};
  std::vector<bool> traversed_roots{};
  treversal_result_t<version_v> m_node_to_edge_{};
  treversal_result_t<version_v> m_node_to_pred_{};
  disjoint_set_union_t<version_v, std::vector<uintlen_t>> scc_dsu{};
  disjoint_set_union_t<version_v, std::vector<uintlen_t>> undirected_dsu{};

  static inline constexpr mjz_uint128_t_ phi_128 =
      (mjz_uint128_t_(0x9e3779b97f4a7c15ULL) << 64) |
      mjz_uint128_t_(0xf39cc0605cedc835ULL);
  static inline constexpr uint64_t phi_32 = uint32_t(phi_128);

  struct rand_gen_t {
    std::array<uint32_t, 64> rand_generator{};
    MJZ_CX_FN rand_gen_t() noexcept {
      uint32_t seed{0xdeadbeef};
      seed *= phi_32;
      for (uint32_t &u32 : rand_generator) {
        seed ^= (seed >> 16);
        seed *= phi_32;
        u32 = bitswap(seed);
      };
    }
    MJZ_CX_FN rand_gen_t &next() noexcept {
      for (uint32_t &u32 : rand_generator) {
        u32 ^= (u32 >> 16);
        u32 *= phi_32;
      };
      return *this;
    }
  };

  MJZ_CX_FN static uint64_t tighten_max_byte_8x8(uint64_t lhs,
                                                 uint64_t rhs) noexcept {
    constexpr uint64_t byte_lsb_mask = 0x0101010101010101ULL;
    constexpr uint64_t byte_msb_mask = byte_lsb_mask << 7;
    constexpr uint64_t lower_bits_mask = byte_msb_mask - byte_lsb_mask;

    const uint64_t lhs_lower_bits = lhs & lower_bits_mask;
    const uint64_t rhs_lower_bits = rhs & lower_bits_mask;
    const uint64_t lhs_low_bits_greater_or_equal =
        byte_msb_mask + lhs_lower_bits - rhs_lower_bits;
    const uint64_t lhs_low_bits_greater_or_equal_inclusive =
        (~rhs | lhs) & lhs_low_bits_greater_or_equal;

    const uint64_t lhs_low_bits_greater_or_equal_exclusive =
        (~rhs & lhs) | lhs_low_bits_greater_or_equal_inclusive;
    const uint64_t lhs_greater_or_equal_msb =
        lhs_low_bits_greater_or_equal_exclusive & byte_msb_mask;

    const uint64_t lhs_greater_or_equal_lsb = lhs_greater_or_equal_msb >> 7;

    const uint64_t lhs_select_mask =
        (lhs_greater_or_equal_msb - lhs_greater_or_equal_lsb) |
        lhs_greater_or_equal_msb;

    return (lhs & lhs_select_mask) | (rhs & ~lhs_select_mask);
  }

  MJZ_CX_FN static auto representitive_power_impl(mjz_uint128_t_ tightness,
                                                  intlen_t wave,
                                                  intlen_t max_wave) noexcept {
    uint64_t f_hll = uint64_t(tightness);
    uint64_t b_hll = uint64_t(tightness >> 64);

    auto f_bytes = std::bit_cast<std::array<uint8_t, 8>>(f_hll);
    auto b_bytes = std::bit_cast<std::array<uint8_t, 8>>(b_hll);

    uint64_t f_cardinality{};
    uint64_t b_cardinality{};

    for (size_t i = 0; i < 8; ++i) {
      f_cardinality += (uint64_t(1) << std::min<uint8_t>(f_bytes[i], 60));
      b_cardinality += (uint64_t(1) << std::min<uint8_t>(b_bytes[i], 60));
    }

    uint64_t bottleneck_score = std::min(f_cardinality, b_cardinality);

    uint64_t wave_score = 0;
    if (max_wave > 1) {
      intlen_t abs_wave = wave < 0 ? -wave : wave;
      intlen_t dist_from_center = (max_wave / 2) - abs_wave;
      if (dist_from_center < 0)
        dist_from_center = -dist_from_center;
      wave_score = uint64_t(max_wave - dist_from_center);
    }
    mjz_uint128_t_ card =
        mjz_uint128_t_(f_cardinality) * mjz_uint128_t_(b_cardinality);
    mjz_uint128_t_ harmonic_mean_aprox =
        card >>
        (bit_width(mjz_uint128_t_(1) | (mjz_uint128_t_(f_cardinality) +
                                        mjz_uint128_t_(b_cardinality))) -
         1);
    return tuple_t{harmonic_mean_aprox, bottleneck_score, card, wave_score};
  }

  MJZ_CX_FN static auto
  representitive_power_forward(mjz_uint128_t_ tightness, intlen_t wave,
                               intlen_t max_wave) noexcept {
    return representitive_power_impl(tightness, wave, max_wave);
  }

  MJZ_CX_FN static auto
  representitive_power_backward(mjz_uint128_t_ tightness, intlen_t wave,
                                intlen_t max_wave) noexcept {
    return representitive_power_impl(tightness, wave, max_wave);
  }

  MJZ_CX_FN static mjz_uint128_t_ tighten_forward(mjz_uint128_t_ tightest,
                                                  mjz_uint128_t_ rhs) noexcept {
    return (tightest & ~mjz_uint128_t_(uint64_t(-1))) |
           mjz_uint128_t_(
               tighten_max_byte_8x8(uint64_t(rhs), uint64_t(tightest)));
  }

  MJZ_CX_AL_FN static void make_forward(mjz_uint128_t_ &tightest,
                                        const rand_gen_t &rand_gen) noexcept {
    std::array<uint8_t, 8> hll{};
    const auto random_array =
        std::bit_cast<std::array<uintN_t<version_v, 256>, 8>>(
            rand_gen.rand_generator);
    for (uintlen_t i{}; i < 8; i++) {
      hll[i] = uint8_t(countr_zero(random_array[i]));
    }
    tightest |= mjz_uint128_t_(std::bit_cast<uint64_t>(hll));
  }

  MJZ_CX_FN static mjz_uint128_t_
  tighten_backward(mjz_uint128_t_ lhs, mjz_uint128_t_ rhs) noexcept {
    return rotl(tighten_forward(rotr(lhs, 64), rotr(rhs, 64)), 64);
  }

  MJZ_CX_FN static void make_backward(mjz_uint128_t_ &lhs,
                                      const rand_gen_t &rand_gen) noexcept {
    lhs = rotr(lhs, 64);
    make_forward(lhs, rand_gen);
    lhs = rotl(lhs, 64);
  }

  template <class R = std::span<const uintlen_t>,
            index_range_of_c<version_v> RIR_t>
  MJZ_CX_FN explicit reachability_checker_t(const auto &range_of_range,
                                            RIR_t &&entry_node_order) noexcept {

    auto too_much = calculate_too_much_t<version_v>::make_with_order(
        range_of_range, entry_node_order);

    m_node_to_edge_ = std::move(too_much.edge_of_node);
    m_node_to_pred_ = std::move(too_much.pred_of_node);
    auto node_to_edge = m_node_to_edge_.range();
    auto node_to_pred = m_node_to_pred_.range();
    const uintlen_t node_count = std::ranges::size(m_node_to_edge_.range());
    reachability_cache.reserve(m_node_to_edge_.edges.size());

    scc_dsu.parent_then_rank.resize(node_count);
    hyper_log_log.resize(node_count);
    scc_dsu.init(node_count);
    rand_gen_t rand_gen{};

    for (auto &&scc : too_much.strongly_connected_components.range()) {
      if (scc.empty())
        continue;
      uintlen_t root = scc.back();
      mjz_uint128_t_ tightest{};
      make_forward(tightest, rand_gen.next());
      for (uintlen_t node : scc) {
        for (uintlen_t i : node_to_edge[node]) {
          tightest = tighten_forward(
              tightest, hyper_log_log[scc_dsu.find_and_cache_root(i)]);
        }
        scc_dsu.rankless_find_and_unite_root(root, node);
      }
      root = scc_dsu.find_and_cache_root(root);
      hyper_log_log[root] = tightest;
    }

    for (auto &&scc :
         too_much.strongly_connected_components.range() | std::views::reverse) {
      if (scc.empty())
        continue;
      uintlen_t root = scc_dsu.find_and_cache_root(scc.back());
      mjz_uint128_t_ tightest{hyper_log_log[root]};
      make_backward(tightest, rand_gen.next());
      for (uintlen_t node : scc) {
        for (uintlen_t i : node_to_pred[node]) {
          tightest = tighten_backward(
              tightest, hyper_log_log[scc_dsu.find_and_cache_root(i)]);
        }
      }
      hyper_log_log[root] = tightest;
    }

    forward_hyper_log_log_representitive_front.resize(node_count);
    backward_hyper_log_log_representitive_behind.resize(node_count);
    for (uintlen_t i = 0; i < node_count; ++i) {
      uintlen_t root = scc_dsu.find_and_cache_root(i);
      forward_hyper_log_log_representitive_front[i] = root;
      backward_hyper_log_log_representitive_behind[i] = root;
    }

    for (auto &&scc : too_much.strongly_connected_components.range()) {
      if (scc.empty())
        continue;
      uintlen_t root = scc_dsu.find_and_cache_root(scc.back());

      for (uintlen_t node : scc) {
        for (uintlen_t neighbor : node_to_edge[node]) {
          neighbor = scc_dsu.find_and_cache_root(neighbor);

          uintlen_t cand = forward_hyper_log_log_representitive_front[neighbor];
          uintlen_t &best = forward_hyper_log_log_representitive_front[root];

          if (representitive_power_forward(
                  hyper_log_log[best],
                  too_much.sequenced_components.first[best],
                  too_much.sequenced_components.second) <
              representitive_power_forward(
                  hyper_log_log[cand],
                  too_much.sequenced_components.first[cand],
                  too_much.sequenced_components.second)) {
            best = cand;
          }
        }
      }
      uintlen_t final_best = forward_hyper_log_log_representitive_front[root];
      for (uintlen_t node : scc) {
        forward_hyper_log_log_representitive_front[node] = final_best;
      }
    }

    for (auto &&scc :
         too_much.strongly_connected_components.range() | std::views::reverse) {
      if (scc.empty())
        continue;
      uintlen_t root = scc_dsu.find_and_cache_root(scc.back());

      for (uintlen_t node : scc) {
        for (uintlen_t pred : node_to_pred[node]) {
          pred = scc_dsu.find_and_cache_root(pred);

          uintlen_t cand = backward_hyper_log_log_representitive_behind[pred];
          uintlen_t &best = backward_hyper_log_log_representitive_behind[root];

          if (representitive_power_backward(
                  hyper_log_log[best],
                  too_much.sequenced_components.first[best],
                  too_much.sequenced_components.second) <
              representitive_power_backward(
                  hyper_log_log[cand],
                  too_much.sequenced_components.first[cand],
                  too_much.sequenced_components.second)) {
            best = cand;
          }
        }
      }
      uintlen_t final_best = backward_hyper_log_log_representitive_behind[root];
      for (uintlen_t node : scc) {
        backward_hyper_log_log_representitive_behind[node] = final_best;
      }
    }

    undirected_dsu.parent_then_rank.resize(node_count * 2);
    undirected_dsu.init(node_count);
    auto adjs = m_node_to_edge_.range();
    for (uintlen_t u = 0; u < node_count; ++u) {
      for (uintlen_t v : adjs[u]) {
        undirected_dsu.find_and_unite_root(u, v);
      }
    }

    wave_seq = std::move(too_much.sequenced_components.first);
    auto structural_root_view = std::views::iota(uintlen_t(0), node_count) |
                                std::views::filter([&](uintlen_t i) noexcept {
                                  return wave_seq[i] == 1 || wave_seq[i] == -1;
                                });

    auto dom_forest = make_dominator_forest_given_dominators<version_v>(
        too_much.immidiate_dominators);
    auto [dom_begin, dom_end] =
        calculate_dominance_intervals_given_dominators_and_forest<version_v>(
            dom_forest, too_much.immidiate_dominators);
    dominance_interval_begin = std::move(dom_begin);
    dominance_interval_end = std::move(dom_end);

    depth_first_interval_begin.assign(node_count, 0);
    depth_first_interval_end.assign(node_count, 0);
    traversed_roots.assign(node_count, false);

    begin_temp.assign(node_count, 0);
    parent_temp.assign(node_count, 0);
    end_temp.assign(node_count, 0);

    calculate_depth_first_intervals<version_v>(
        depth_first_interval_begin, depth_first_interval_end, parent_temp,
        m_node_to_edge_, structural_root_view);
  }

  template <class R = std::span<const uintlen_t>>
  MJZ_CX_FN explicit reachability_checker_t(const auto &range_of_range) noexcept
      : reachability_checker_t(
            range_of_range,
            std::views::iota(uintlen_t(), std::ranges::size(range_of_range))) {}

  MJZ_CX_FN static uintlen_t
  maximizer_compress_pass_fn(auto &range_, uintlen_t start) noexcept {
    uintlen_t root = start;
    while (true) {
      uintlen_t next = range_[root];
      if (next == root)
        break;
      root = next;
    };
    while (true) {
      uintlen_t next = std::exchange(range_[start], root);
      if (next == start)
        break;
      start = next;
    };
    return root;
  };

  MJZ_CX_FN bool is_dominator_impl_(uintlen_t u, uintlen_t v) noexcept {
    return dominance_interval_begin[u] != 0 &&
           dominance_interval_begin[v] != 0 &&
           dominance_interval_begin[u] <= dominance_interval_begin[v] &&
           dominance_interval_end[u] >= dominance_interval_end[v];
  }

  MJZ_CX_FN may_bool_t structured_may_reach(uintlen_t u, uintlen_t v) noexcept {
    u = scc_dsu.find_and_cache_root(u);
    v = scc_dsu.find_and_cache_root(v);
    if (u == v)
      return may_bool_t::yes;

    if (undirected_dsu.find_and_cache_root(u) !=
        undirected_dsu.find_and_cache_root(v))
      return may_bool_t::no;

    if (std::max(-wave_seq[u], wave_seq[u]) >=
        std::max(wave_seq[v], -wave_seq[v]))
      return may_bool_t::no;

    if (is_dominator_impl_(u, v))
      return may_bool_t::yes;

    if (is_dominator_impl_(v, u))
      return may_bool_t::no;

    const bool df_u_valid = depth_first_interval_begin[u] != 0;
    const bool df_v_valid = depth_first_interval_begin[v] != 0;

    if (df_u_valid && df_v_valid) {
      if (depth_first_interval_begin[u] <= depth_first_interval_begin[v] &&
          depth_first_interval_end[u] >= depth_first_interval_end[v])
        return may_bool_t::yes;

      if (depth_first_interval_begin[v] <= depth_first_interval_begin[u] &&
          depth_first_interval_end[v] >= depth_first_interval_end[u])
        return may_bool_t::no;
    }

    const mjz_uint128_t_ tighest_u = hyper_log_log[u];
    const mjz_uint128_t_ tighest_v = hyper_log_log[v];

    const mjz_uint128_t_ expected_u = tighten_forward(tighest_u, tighest_v);
    const mjz_uint128_t_ expected_v = tighten_backward(tighest_v, tighest_u);
    if (tighest_u != expected_u || tighest_v != expected_v)
      return may_bool_t::no;

    if (reachability_cache.find(bijective_mix(u, v)).has_value())
      return may_bool_t::yes;

    if (reachability_cache.find(bijective_mix(v, u)).has_value())
      return may_bool_t::no;

    if (traversed_roots[u] || traversed_roots[v])
      return may_bool_t::no;

    return may_bool_t::idk;
  }

  MJZ_CX_FN may_bool_t may_reach(uintlen_t u, uintlen_t v) noexcept {
    may_bool_t mb = structured_may_reach(u, v);
    if (mb != may_bool_t::idk)
      return mb;

    const uintlen_t f_u = maximizer_compress_pass_fn(
        forward_hyper_log_log_representitive_front, u);
    const uintlen_t b_u = maximizer_compress_pass_fn(
        backward_hyper_log_log_representitive_behind, u);

    const uintlen_t f_v = maximizer_compress_pass_fn(
        forward_hyper_log_log_representitive_front, v);
    const uintlen_t b_v = maximizer_compress_pass_fn(
        backward_hyper_log_log_representitive_behind, v);

    if (f_u == b_v || f_u == v || b_v == u)
      return may_bool_t::yes;

    if (f_v == b_u || f_v == u || b_u == v)
      return may_bool_t::no;

    may_bool_t smr_v_u = structured_may_reach(v, u);
    if (smr_v_u == may_bool_t::yes)
      return may_bool_t::no;

    const bool dominant_backward_representation_v = is_dominator_impl_(b_v, v);
    const bool dominant_backward_representation_u = is_dominator_impl_(b_u, u);
    const bool dominated_forward_representation_v = is_dominator_impl_(v, f_v);
    const bool dominated_forward_representation_u = is_dominator_impl_(u, f_u);

    may_bool_t smr_fu_bv = structured_may_reach(f_u, b_v);
    may_bool_t smr_fu_v = structured_may_reach(f_u, v);
    may_bool_t smr_u_bv = structured_may_reach(u, b_v);

    may_bool_t smr_fv_bu = structured_may_reach(f_v, b_u);
    may_bool_t smr_fv_u = structured_may_reach(f_v, u);
    may_bool_t smr_v_bu = structured_may_reach(v, b_u);

    may_bool_t smr_bu_v = structured_may_reach(b_u, v);
    may_bool_t smr_bu_fv = structured_may_reach(b_u, f_v);
    may_bool_t smr_bu_bv = structured_may_reach(b_u, b_v);

    may_bool_t smr_bv_u = structured_may_reach(b_v, u);
    may_bool_t smr_bv_fu = structured_may_reach(b_v, f_u);
    may_bool_t smr_bv_bu = structured_may_reach(b_v, b_u);

    may_bool_t smr_u_fv = structured_may_reach(u, f_v);
    may_bool_t smr_v_fu = structured_may_reach(v, f_u);

    auto test_and_set_fn = [](may_bool_t &val, bool res) noexcept -> bool {
      if (val != may_bool_t::idk) {
        asserts((val == may_bool_t::yes) == res);
        return false;
      }
      val = res ? may_bool_t::yes : may_bool_t::no;
      return true;
    };

    while (mb == may_bool_t::idk) {
      if (smr_fu_v == may_bool_t::yes && test_and_set_fn(mb, true))
        continue;

      if (smr_fu_bv == may_bool_t::yes && test_and_set_fn(mb, true))
        continue;

      if (smr_u_bv == may_bool_t::yes && test_and_set_fn(mb, true))
        continue;

      if (dominated_forward_representation_v && smr_u_fv == may_bool_t::yes &&
          smr_v_u == may_bool_t::no && test_and_set_fn(mb, true))
        continue;

      if (smr_fv_u == may_bool_t::yes && test_and_set_fn(smr_v_u, true))
        continue;

      if (smr_fv_bu == may_bool_t::yes && test_and_set_fn(smr_v_u, true))
        continue;

      if (smr_v_bu == may_bool_t::yes && test_and_set_fn(smr_v_u, true))
        continue;

      if (dominated_forward_representation_u && smr_v_fu == may_bool_t::yes &&
          mb == may_bool_t::no && test_and_set_fn(smr_v_u, true))
        continue;

      if (smr_bu_v == may_bool_t::no && test_and_set_fn(mb, false))
        continue;

      if (smr_u_fv == may_bool_t::no && test_and_set_fn(mb, false))
        continue;

      if (smr_bu_fv == may_bool_t::no && test_and_set_fn(mb, false))
        continue;

      if (smr_v_u == may_bool_t::yes && test_and_set_fn(mb, false))
        continue;

      if ((smr_fv_bu == may_bool_t::yes || smr_fv_u == may_bool_t::yes ||
           smr_v_bu == may_bool_t::yes) &&
          test_and_set_fn(mb, false))
        continue;

      if (dominant_backward_representation_v && smr_bv_u == may_bool_t::no &&
          smr_u_bv == may_bool_t::no && test_and_set_fn(mb, false))
        continue;

      if (dominant_backward_representation_v &&
          dominant_backward_representation_u && smr_bv_bu == may_bool_t::no &&
          smr_bu_bv == may_bool_t::no && test_and_set_fn(mb, false))
        continue;

      if (smr_bv_u == may_bool_t::no && test_and_set_fn(smr_v_u, false))
        continue;

      if (smr_v_fu == may_bool_t::no && test_and_set_fn(smr_v_u, false))
        continue;

      if (smr_bv_fu == may_bool_t::no && test_and_set_fn(smr_v_u, false))
        continue;

      if (mb == may_bool_t::yes && test_and_set_fn(smr_v_u, false))
        continue;

      if ((smr_fu_bv == may_bool_t::yes || smr_fu_v == may_bool_t::yes ||
           smr_u_bv == may_bool_t::yes) &&
          test_and_set_fn(smr_v_u, false))
        continue;

      if (dominant_backward_representation_u && smr_bu_v == may_bool_t::no &&
          smr_v_bu == may_bool_t::no && test_and_set_fn(smr_v_u, false))
        continue;

      if (dominant_backward_representation_u &&
          dominant_backward_representation_v && smr_bu_bv == may_bool_t::no &&
          smr_bv_bu == may_bool_t::no && test_and_set_fn(smr_v_u, false))
        continue;

      if (dominant_backward_representation_v && smr_bv_fu == may_bool_t::no &&
          smr_fu_v == may_bool_t::yes && test_and_set_fn(smr_fu_bv, true))
        continue;

      if (dominant_backward_representation_u && smr_bu_fv == may_bool_t::no &&
          smr_fv_u == may_bool_t::yes && test_and_set_fn(smr_fv_bu, true))
        continue;

      if (dominant_backward_representation_v && smr_bv_bu == may_bool_t::no &&
          smr_bu_bv == may_bool_t::no && test_and_set_fn(smr_bu_v, false))
        continue;

      if (dominant_backward_representation_u && smr_bu_fv == may_bool_t::no &&
          smr_fv_u == may_bool_t::yes && test_and_set_fn(smr_fv_bu, true))
        continue;

      if (smr_bu_fv == may_bool_t::no && test_and_set_fn(smr_u_fv, false))
        continue;

      if (smr_bu_fv == may_bool_t::no && test_and_set_fn(smr_bu_v, false))
        continue;

      if (smr_bu_bv == may_bool_t::no && test_and_set_fn(smr_u_bv, false))
        continue;

      if (smr_bv_fu == may_bool_t::no && test_and_set_fn(smr_v_fu, false))
        continue;

      if (smr_bv_fu == may_bool_t::no && test_and_set_fn(smr_bv_u, false))
        continue;

      if (smr_bv_bu == may_bool_t::no && test_and_set_fn(smr_v_bu, false))
        continue;

      if (mb == may_bool_t::yes && dominant_backward_representation_v &&
          smr_bv_u == may_bool_t::no && test_and_set_fn(smr_u_bv, true))
        continue;

      if (smr_v_u == may_bool_t::yes && dominant_backward_representation_u &&
          smr_bu_v == may_bool_t::no && test_and_set_fn(smr_v_bu, true))
        continue;

      break;
    };

    return mb;
  }

  MJZ_CX_FN void traverse_representitives(uintlen_t u) noexcept {
    traverse(maximizer_compress_pass_fn(
        forward_hyper_log_log_representitive_front, u));
    traverse(maximizer_compress_pass_fn(
        backward_hyper_log_log_representitive_behind, u));
  }

  MJZ_CX_FN bool can_reach(uintlen_t u, uintlen_t v,
                           bool extra = true) noexcept {
    u = scc_dsu.find_and_cache_root(u);
    v = scc_dsu.find_and_cache_root(v);
    may_bool_t mb = may_reach(u, v);
    if (mb == may_bool_t::idk) {
      if (extra) {
        traverse_representitives(v);
        traverse_representitives(u);
        mb = may_reach(u, v);
        if (mb != may_bool_t::idk)
          return mb == may_bool_t::yes;
      }
      traverse(u);
      traverse(v);
      mb = may_reach(u, v);
    }
    return mb == may_bool_t::yes;
  }

  MJZ_CX_FN void traverse(uintlen_t root) noexcept {
    root = scc_dsu.find_and_cache_root(root);
    if (traversed_roots[root])
      return;
    const uintlen_t node_count = dominance_interval_end.size();

    begin_temp.assign(node_count, 0);
    end_temp.assign(node_count, 0);
    auto edge_cache_fn = [&](uintlen_t u_node, uintlen_t v_node) noexcept {
      cache_reach(u_node, v_node);
    };

    calculate_depth_first_intervals<version_v>(
        begin_temp, end_temp, parent_temp, m_node_to_edge_, std::array{root},
        [&](uintlen_t, uintlen_t u_node, uintlen_t) noexcept {
          cache_reach(root, u_node);
        },
        edge_cache_fn, edge_cache_fn, edge_cache_fn, edge_cache_fn,
        [&](uintlen_t, uintlen_t u_node, uintlen_t) noexcept {
          cache_reach(root, u_node);
        });

    auto inv_edge_cache_fn = [&](uintlen_t u_node, uintlen_t v_node) noexcept {
      cache_reach(v_node, u_node);
    };
    begin_temp.assign(node_count, 0);
    end_temp.assign(node_count, 0);
    calculate_depth_first_intervals<version_v>(
        begin_temp, end_temp, parent_temp, m_node_to_pred_, std::array{root},
        [&](uintlen_t, uintlen_t u_node, uintlen_t) noexcept {
          cache_reach(u_node, root);
        },
        inv_edge_cache_fn, inv_edge_cache_fn, inv_edge_cache_fn,
        inv_edge_cache_fn,
        [&](uintlen_t, uintlen_t u_node, uintlen_t) noexcept {
          cache_reach(u_node, root);
        });

    traversed_roots[root] = true;
  }

  MJZ_CX_FN void cache_reach(uintlen_t u, uintlen_t v) noexcept {
    u = scc_dsu.find_and_cache_root(u);
    v = scc_dsu.find_and_cache_root(v);
    may_bool_t mb = structured_may_reach(u, v);
    asserts(mb != may_bool_t::no);
    if (mb != may_bool_t::idk)
      return;

    reachability_cache.insert(bijective_mix(u, v));
  }

  MJZ_CX_FN static mjz_uint128_t_ bijective_mix(uintlen_t u,
                                                uintlen_t v) noexcept {
    mjz_uint128_t_ key = (mjz_uint128_t_(u) << 64) | mjz_uint128_t_(v);
    key ^= (key >> 64);
    return key * phi_128;
  }
};

} // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_reach_FILE_