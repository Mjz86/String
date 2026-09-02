

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

#ifndef MJZ_SRC_GRAPH_sequence_FILE_
#define MJZ_SRC_GRAPH_sequence_FILE_
#include "csr.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_AL_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_graph_sequenced_components_events(
    const basic_forest_t<version_v, R1> &scc_forest_,
    const basic_forest_t<version_v, R2> &edge_of_node_, auto &&wave_exit_event,
    auto &&wave_entry_event, auto &&scc_found_event, auto &&node_exit_event,
    auto &&node_entry_event, auto &&scc_entry_event, auto &&scc_exit_event,
    auto &&edge_remove_event, auto &&same_scc_edge_remove_event,
    auto &&null_scc_edge_remove_event, auto &&edge_found_event,
    auto &&same_scc_edge_found_event, auto &&null_scc_edge_found_event,
    auto &&null_scc_node_found_event, auto &&scc_node_found_entry_event,
    auto &&scc_node_found_exit_event) noexcept
  requires requires(uintlen_t node_index, uintlen_t edge_index,
                    uintlen_t from_node_index, uintlen_t to_node_index,
                    uintlen_t from_scc_index, uintlen_t to_scc_index,
                    intlen_t degree_index, bool early_exit,
                    std::span<const intlen_t> scc_wave_que,
                    intlen_t wave_index_sequence, bool was_self_sequencal) {
    //
    {
      early_exit ==
          bool(wave_exit_event(+wave_index_sequence, std::span(scc_wave_que),
                               bool(early_exit)))
    } noexcept;
    {
      early_exit ==
          bool(wave_entry_event(+wave_index_sequence, std::span(scc_wave_que)))
    } noexcept;

    //
    {
      early_exit == bool(scc_found_event(+from_scc_index, +degree_index))
    } noexcept;
    //
    {
      early_exit ==
          bool(node_exit_event(+from_scc_index, +from_node_index,
                               bool(early_exit), bool(was_self_sequencal)))
    } noexcept;

    {
      early_exit == bool(node_entry_event(+from_scc_index, +from_node_index))
    } noexcept;
    //
    { early_exit == bool(scc_entry_event(+from_scc_index)) } noexcept;
    {
      early_exit == bool(scc_exit_event(+from_scc_index, bool(early_exit)))
    } noexcept;
    //
    {
      early_exit ==
          bool(edge_remove_event(+edge_index, +from_node_index, +from_scc_index,
                                 +to_node_index, +to_scc_index, +degree_index))
    } noexcept;
    {
      early_exit ==
          bool(same_scc_edge_remove_event(+edge_index, +from_node_index,
                                          +from_scc_index, +to_node_index))
    } noexcept;
    {
      early_exit ==
          bool(null_scc_edge_remove_event(+edge_index, +from_node_index,
                                          +from_scc_index, +to_node_index))
    } noexcept;
    //

    {
      early_exit ==
          bool(edge_found_event(+edge_index, +from_node_index, +from_scc_index,
                                +to_node_index, +to_scc_index, +degree_index))
    } noexcept;
    {
      early_exit ==
          bool(same_scc_edge_found_event(+edge_index, +from_node_index,
                                         +from_scc_index, +to_node_index))
    } noexcept;
    {
      early_exit ==
          bool(null_scc_edge_found_event(+edge_index, +from_node_index,
                                         +from_scc_index, +to_node_index))
    } noexcept;
    //
    {
      early_exit == bool(null_scc_node_found_event(+from_node_index))
    } noexcept;
    //
    {
      early_exit ==
          bool(scc_node_found_entry_event(+from_node_index, +from_scc_index))
    } noexcept;
    {
      early_exit == bool(scc_node_found_exit_event(
                        +from_node_index, +from_scc_index, bool(early_exit)))
    } noexcept;
  }
{
  auto edge_of_node = edge_of_node_.index_range();
  auto scc_forest = scc_forest_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  uintlen_t total_scc_count = std::ranges::size(scc_forest);
  std::vector<intlen_t> sequence_number((total_node_count + total_scc_count) *
                                        2);
  auto scc_indexies_compl =
      std::span(sequence_number).subspan(total_node_count, total_node_count);
  auto wave_que =
      std::span(sequence_number).subspan(total_node_count * 2, total_scc_count);
  auto in_degree =
      std::span(sequence_number)
          .subspan(total_node_count * 2 + total_scc_count, total_scc_count);
  uintlen_t tail{};
  uintlen_t head{};
  bool early_exit{};
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
      early_exit = bool(null_scc_node_found_event(+monotonic_id_counter));
      if (early_exit)
        break;
      continue;
    }
    early_exit =
        bool(scc_node_found_entry_event(+monotonic_id_counter, +src_scc));

    if (early_exit)
      break;

    for (uintlen_t edge_index : nodes) {
      uintlen_t to_index{edge_of_node_.get_edge_to(edge_index)};
      uintlen_t dst_scc = ~uintlen_t(scc_indexies_compl[to_index]);
      if (dst_scc == uintlen_t(-1)) {
        early_exit = bool(null_scc_edge_found_event(
            +edge_index, +monotonic_id_counter, +src_scc, +to_index));
        if (early_exit)
          break;
        continue;
      }
      if (src_scc == dst_scc) {
        early_exit = bool(same_scc_edge_found_event(
            +edge_index, +monotonic_id_counter, +src_scc, +to_index));
        if (early_exit)
          break;
        continue;
      }
      early_exit =
          bool(edge_found_event(+edge_index, +monotonic_id_counter, +src_scc,
                                +to_index, +dst_scc, +(in_degree[dst_scc]++)));
      if (early_exit)
        break;
    }
    early_exit = bool(scc_node_found_exit_event(+monotonic_id_counter, +src_scc,
                                                bool(early_exit)));
    if (early_exit)
      break;
  }

  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (!in_degree[i])
      wave_que[head++] = intlen_t(i);
    early_exit = bool(scc_found_event(+i, +in_degree[i]));
    if (early_exit)
      break;
  }

  intlen_t wave_index_sequence{};
  while (!early_exit && head != tail) {
    wave_index_sequence++;
    auto sp = wave_que.subspan(tail, head - tail);

    early_exit = bool(wave_entry_event(+wave_index_sequence, std::span(sp)));
    if (early_exit)
      break;

    tail = head;
    for (intlen_t si : sp) {

      uintlen_t i = uintlen_t(si);
      early_exit = bool(scc_entry_event(+i));
      if (early_exit)
        break;

      bool was_self_sequencal = std::ranges::size(scc_forest[i]) > 1;
      for (uintlen_t node : scc_forest[i]) {
        early_exit = bool(node_entry_event(+node, +i));
        if (early_exit)
          break;

        for (uintlen_t edge_index : edge_of_node[node]) {
          uintlen_t next{edge_of_node_.get_edge_to(edge_index)};
          uintlen_t dst_scc = ~uintlen_t(scc_indexies_compl[next]);
          if (dst_scc == uintlen_t(-1)) {
            early_exit =
                bool(null_scc_edge_remove_event(+edge_index, +node, +i, +next));
            if (early_exit)
              break;
            continue;
          }
          if (i == dst_scc) {
            was_self_sequencal = true;
            early_exit =
                bool(same_scc_edge_remove_event(+edge_index, +node, +i, +next));
            if (early_exit)
              break;
            continue;
          }
          intlen_t deg = --in_degree[dst_scc];
          if (!deg)
            wave_que[head++] = intlen_t(dst_scc);

          early_exit = bool(
              edge_remove_event(+edge_index, +node, +i, +next, +dst_scc, +deg));
          if (early_exit)
            break;
        }
        sequence_number[node] =
            was_self_sequencal ? -wave_index_sequence : wave_index_sequence;
        early_exit = bool(node_exit_event(+node, +i, bool(early_exit),
                                          bool(was_self_sequencal)));
        if (early_exit)
          break;
      }

      early_exit = bool(scc_exit_event(+i, bool(early_exit)));
      if (early_exit)
        break;
    }
    early_exit = bool(
        wave_exit_event(+wave_index_sequence, std::span(sp), bool(early_exit)));
    if (early_exit)
      break;
  }

  std::ranges::fill(wave_que.subspan(tail, total_node_count - tail),
                    uintlen_t(-1));
  sequence_number.resize(total_node_count * 2 + total_scc_count);
  return {std::move(sequence_number), wave_index_sequence};
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_graph_sequenced_components(
    const basic_forest_t<version_v, R1> &scc_forest_,
    const basic_forest_t<version_v, R2> &edge_of_node_) noexcept {
  auto dumy = [](auto &&...) noexcept -> bool { return false; };
  return calculate_graph_sequenced_components_events(
      scc_forest_, edge_of_node_, dumy, dumy, dumy, dumy, dumy, dumy, dumy,
      dumy, dumy, dumy, dumy, dumy, dumy, dumy, dumy, dumy);
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<intlen_t>, intlen_t>
calculate_acyclic_graph_sequenced_components(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  std::vector<intlen_t> sequence_number(total_node_count * 3);
  auto wave_que =
      std::span(sequence_number).subspan(total_node_count, total_node_count);
  auto in_degree = std::span(sequence_number)
                       .subspan(total_node_count * 2, total_node_count);

  uintlen_t tail{};
  uintlen_t head{};

  for (auto &&nodes : edge_of_node) {
    for (uintlen_t next : nodes) {
      asserts(next < total_node_count);
      in_degree[next]++;
    }
  }
  for (uintlen_t i{}; i < in_degree.size(); i++) {
    if (in_degree[i])
      continue;
    wave_que[head++] = intlen_t(i);
  }
  intlen_t wave_index_sequence{};
  while (head != tail) {
    wave_index_sequence++;
    auto sp = wave_que.subspan(tail, head - tail);
    tail = head;
    for (intlen_t si : sp) {
      uintlen_t i = uintlen_t(si);
      for (uintlen_t next : edge_of_node[i]) {
        if (--in_degree[next])
          continue;
        wave_que[head++] = intlen_t(next);
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

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN pair_t<std::vector<uintlen_t>, uintlen_t>
calculate_acyclic_graph_topological_sort_events(
    const basic_forest_t<version_v, R> &edge_of_node_,

    auto &&edge_found_event, auto &&node_found_event,

    auto &&node_entry_event, auto &&node_exit_event,

    auto &&edge_remove_event,

    auto &&wave_entry_event, auto &&wave_exit_event

    ) noexcept
  requires requires(uintlen_t node_index, uintlen_t edge_index,
                    uintlen_t from_index, uintlen_t to_index,
                    uintlen_t degree_index, bool early_exit,
                    std::span<const uintlen_t> wave_que,
                    uintlen_t wave_index_sequence) {
    { early_exit == bool(node_entry_event(+node_index)) } noexcept;
    {
      early_exit == bool(node_exit_event(+node_index, bool(early_exit)))
    } noexcept;
    {
      early_exit == bool(node_found_event(+node_index, +degree_index))
    } noexcept;

    //

    {
      early_exit == bool(edge_remove_event(+edge_index, +from_index, +to_index,
                                           +degree_index))
    } noexcept;

    {
      early_exit == bool(edge_found_event(+edge_index, +from_index, +to_index,
                                          +degree_index))
    } noexcept;

    //
    {
      early_exit == bool(wave_entry_event(+wave_index_sequence, wave_que))
    } noexcept;
    {
      early_exit == bool(wave_exit_event(+wave_index_sequence, wave_que,
                                         bool(early_exit)))
    } noexcept;
  }
{
  auto edge_of_node_index = edge_of_node_.index_range();
  uintlen_t total_node_count = std::ranges::size(edge_of_node_index);
  std::vector<uintlen_t> wave_que(total_node_count * 2 + 1);
  auto in_degree =
      std::span(wave_que).subspan(total_node_count + 1, total_node_count);
  uintlen_t tail{};
  uintlen_t head{};
  bool early_exit{};

  for (uintlen_t node_index{}; node_index < total_node_count; node_index++) {
    for (uintlen_t edge_index : edge_of_node_index[node_index]) {
      uintlen_t to_index{edge_of_node_.get_edge_to(edge_index)};
      asserts(to_index < total_node_count);
      early_exit = bool(edge_found_event(+edge_index, +node_index, +to_index,
                                         +(in_degree[to_index]++)));
      if (early_exit)
        break;
    }
    if (early_exit)
      break;
  }
  for (uintlen_t node_index{}; !early_exit && node_index < in_degree.size();
       node_index++) {
    wave_que[head] = node_index;
    uintlen_t degree_index = in_degree[node_index];
    head += !degree_index;
    early_exit = bool(node_found_event(+node_index, +degree_index));
    if (early_exit)
      break;
  }
  uintlen_t wave_index_sequence{};
  while (!early_exit && head != tail) {
    wave_index_sequence++;
    auto sp = std::span<const uintlen_t>(wave_que).subspan(tail, head - tail);
    early_exit = bool(
        wave_entry_event(+wave_index_sequence, std::span<const uintlen_t>(sp)));
    if (early_exit)
      break;
    tail = head;

    for (uintlen_t from_index : sp) {
      early_exit = bool(node_entry_event(+from_index));
      if (early_exit)
        break;
      for (uintlen_t edge_index : edge_of_node_index[from_index]) {
        uintlen_t to_index{edge_of_node_.get_edge_to(edge_index)};
        wave_que[head] = to_index;
        uintlen_t degree_edge = --in_degree[to_index];
        bool hits = !degree_edge;
        head += hits;
        early_exit = bool(edge_remove_event(+edge_index, +from_index, +to_index,
                                            +degree_edge));
        if (early_exit)
          break;
      }
      early_exit = bool(node_exit_event(+from_index, bool(early_exit)));
      if (early_exit)
        break;
    }
    early_exit =
        bool(wave_exit_event(+wave_index_sequence,
                             std::span<const uintlen_t>(sp), bool(early_exit)));
    if (early_exit)
      break;
  }
  std::ranges::fill(std::span(wave_que).subspan(tail, total_node_count - tail),
                    uintlen_t(-1));
  wave_que.resize(total_node_count);
  return {std::move(wave_que), wave_index_sequence};
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN
    tuple_t<treversal_result_t<version_v>, std::vector<intlen_t>, intlen_t>
    calculate_graph_topo_sequenced_components(
        const basic_forest_t<version_v, R1> &scc_forest_,
        const basic_forest_t<version_v, R2> &edge_of_node_) noexcept {
  uintlen_t total_node_count = std::ranges::size(edge_of_node_);
  uintlen_t total_scc_count = std::ranges::size(scc_forest_);
  auto [sequence_number, wave_count] =
      calculate_graph_sequenced_components(scc_forest_, edge_of_node_);
  auto scc_indexies_compl =
      std::span(sequence_number).subspan(total_node_count, total_node_count);
  auto scc_order =
      std::span(sequence_number).subspan(total_node_count * 2, total_scc_count);
  auto ordered = scc_forest_.transform(
      scc_order |
      std::views::transform([](intlen_t i) noexcept { return uintlen_t(i); }));
  sequence_number.resize(total_node_count);
  return {std::move(ordered), std::move(sequence_number), wave_count};
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          class R2 = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v> calculate_graph_topo_scc_order(
    const basic_forest_t<version_v, R1> &scc_forest_,
    const basic_forest_t<version_v, R2> &edge_of_node_) noexcept {
  auto [scc_ordered, _0, _1] = calculate_graph_toposorted_sequenced_components(
      scc_forest_, edge_of_node_);
  return std::move(scc_ordered);
}

template <version_t version_v, class R1 = std::span<const uintlen_t>,
          mutable_index_range_c<version_v> uninit_index_range_t>
MJZ_CX_FN void
node_to_scc_map(uninit_index_range_t &&uninit_scc_indexies,
                const basic_forest_t<version_v, R1> &scc_forest_) noexcept {
  uintlen_t scc_index{};
  auto uninit_scc_indexies_it = std::ranges::begin(uninit_scc_indexies);
  for (auto &&scc : scc_forest_.range()) {
    for (uintlen_t node : scc) {
      *(uninit_scc_indexies_it + intlen_t(node)) = scc_index;
    }
    scc_index++;
  }
}

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_FN treversal_result_t<version_v>
calculate_acyclic_graph_topological_sort_waves(
    const basic_forest_t<version_v, R> &edge_of_node_) noexcept {
  auto edge_of_node = edge_of_node_.range();
  treversal_result_t<version_v> ret{};
  uintlen_t total_node_count = std::ranges::size(edge_of_node);
  ret.edges.resize(total_node_count * 2 + 1, uintlen_t(-1));
  ret.nodes_index.reserve(total_node_count);
  auto wave_que =
      std::span(ret.edges).subspan(uintlen_t(), total_node_count + 1);
  auto in_degree =
      std::span(ret.edges).subspan(total_node_count + 1, total_node_count);
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
  while (head != tail) {
    ret.nodes_index.push_back(tail);
    auto sp = wave_que.subspan(tail, head - tail);
    tail = head;
    for (uintlen_t i : sp) {
      for (uintlen_t next : edge_of_node[i]) {
        wave_que[head] = next;
        head += !--in_degree[next];
      }
    }
  }
  ret.edges.resize(tail);
  return ret;
}

} // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_sequence_FILE_