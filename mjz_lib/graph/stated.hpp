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

#ifndef MJZ_SRC_GRAPH_stated_FILE_
#define MJZ_SRC_GRAPH_stated_FILE_
#include "deps.hpp"
#include "udeps.hpp"
namespace mjz::graph_ns {
/////////////////////////////// from my compiler project

/*
context:
this is the engine that i plan to use for my compiler SEMA ,
it is prremoted to my core library utilities beacuse in my view its a useful
data structure, i recommened using 'basic_dependency_graph_t' instead of
'stated_dependency_graph_t' if its outside my mcc compiler.


*/

/*
 multithread constraints:
 if we want to multithread all work must be multithread done
 between ignite begin and join end. any write to *this (the graph) must be
 done exclusively, however if its read-only we can do that on all threads.
 the time frame from   create begin to create end  MUST NOT be multithreaded.
 also do NOT call into the run_resolution_queries! inside these.


 almost always we want to defuse the node to trigger the next events ,
 defuse is a graph mutating operation,
 to have a syncronous after the last node ends (without writing destructor
 boiler-plate) one can use defuse and call graph handle on it.

 note:
 btw theres a little qurirk in the engine and i love it :
 trigger freedom , assuming we dont go backward, defuse
 can launch a node from ast into code gen in 1 event, or make it stay in ast
 , if it doesnt chage the current and puts trigger as next state of current ,
 it will automatically retrigger itself in the next event ( becuze it
 already was ready ) lol.
 it will just spin us until were bored , but notice , this is as if were
 waiting for an infinitley far behind dependency to catch us , so the
 dependats are ok


 note:
 to effectively use this for parralel work,
 we can do read-only work inside ignite to join in multithreaded batches ,
 and flush the modification buffer of each batch in defuse,
 this way no lock is introduced,
 also , if any worker event does heavy work , it can yeild via my quirk of
 trigger, this way each wave is bounded to end pridictably and the thread
 pool will not be waiting that much. however this graph structure is thread
 agnostic.

 note:
 each defuse costs us O(indeg(v)+outdeg(v)) , v being our node, so ,
 each ignite to join should also cost at most O(deg(v)) , if it surpasses it
 must yeild , this way the cost of each yeild is never more than the cost of
 our work, and we can say , let K be the number of events in the wave, this
 way the time complexity of each wave is O(K+E_K), where E_K is all edges of
 the nodes in event K. while one might expect the cost of the defuse
 micro-tick to dominate, i theorize that a lock-free/locked implementation of
 this micro-tick will be slower than using a flush buffer, however this is
 all just my view, i may be wrong.


 important note :

  - AND parallezim:
    MCC's scheduler loves this , its built for it .

  - OR parallezim:
    for the love all all determinism ,
    ban anything atomic related other than refrence counting and or
 deterministic OS-independant atomic operations ( A and B share a wave ,   A
 happens before B, vs B happens before A, i.e aquire relese  , it must be
 equivlent. imagine somthing like , f(A,B) == f(B,A) ,  f(f(A,B),C) ==
 f(A,f(B,C)) , and so on , the order of execution shall not matter in a wave,
     even the sequencial defuse order is icky(tho technically OS-independant )
 to rely on  ).

    instead , use the yeild mechanism , while a tiny change in the algo can
 cause order divergance , its far more deterministic and OS independant, what i
 mean is : we need OR ( A1,A2,...) we do and (ASP1,ASP2,...) where ASP is a
 minimal suspention point ( a yeild/defuse at a node is a suspention point of
 that node  ), during defuse , if sibling tasks got done , we know at suspention
 that were unneded , and tell childs to cancel as well, while this needs every
 node to be chopped up into fine grained suspention cancel checks , it is the
 way i found to manage OR parrallel semi-determinism ( OS independant ).


     - inner note :
      explicit cancel is logic driven , for example the OR thing from above,
      however, for a node U to be considered transitivly cancelled ,
      all of the nodes that depend on U must be cancelled,
      this can be enforced by local cancel counter and local propagation ,
 similar to the active counter. however becuz canclation is heavily state
 machine dependant, i will not make it a native node construct. also, a node
 that has all of its dependancies complete-canceled can get complete-canceled ,
      complete-canclation will make it complete , so no one will be able depend
 on it anymore.



 */

template <version_t version_v, class event_t>
MJZ_CX_FN void ignite_join_defuse_all_impl(auto &self_, std::span<event_t> ev,
                                           auto &...pram) noexcept {
  if constexpr (requires() {
                  {
                    event_t::ignite_join_defuse_all(self_, ev, pram...)
                  } noexcept;
                }) {
    event_t::ignite_join_defuse_all(self_, ev, pram...);
  } else {

    if constexpr (requires() {
                    { event_t::ignite_all(ev, pram...) } noexcept;
                  }) {
      event_t::ignite_all(ev, pram...);
    } else {
      for (event_t &e : ev) {
        static_assert(requires() {
          { e.ignite(pram...) } noexcept;
        });
        e.ignite(pram...);
      };
    }
    if constexpr (requires() {
                    { event_t::join_all(ev, pram...) } noexcept;
                  }) {
      event_t::join_all(ev, pram...);
    } else {
      for (event_t &e : ev | std::views::reverse) {
        static_assert(requires() {
          { e.join(pram...) } noexcept;
        });
        e.join(pram...);
      };
    }
    if constexpr (requires() {
                    { event_t::defuse_all(self_, ev, pram...) } noexcept;
                  }) {
      event_t::defuse_all(self_, ev, pram...);
    } else {
      for (event_t &e : ev | std::views::reverse) {
        static_assert(requires() {
          { std::move(e).defuse(self_, pram...) } noexcept;
        });
        std::move(e).defuse(self_, pram...);
      };
    }
  }
}

template <version_t version_v, class event_t, auto max_invalid_state_v>
struct basic_dependency_graph_t
    : basic_dependency_graph_base_t<version_v, max_invalid_state_v> {
  using base = basic_dependency_graph_base_t<version_v, max_invalid_state_v>;

protected:
  std::vector<event_t> event_list{};

  MJZ_CX_FN bool execute_resolution_wave(auto &...pram) noexcept {
    return base::run_one_callback([&](auto &&id_range) noexcept {
      MJZ_RAII_RELEASE { event_list.clear(); };
      asserts(!event_list.size());
      event_list.reserve(std::ranges::size(id_range));
      for (auto id : id_range) {
        previous_states_t<version_v, max_invalid_state_v> signal{
            base::passive_trigger(id)};
        const bool plese_optimize_emplaceback =
            event_list.capacity() != event_list.size();

        asserts(plese_optimize_emplaceback);
        MJZ_JUST_ASSUME_(plese_optimize_emplaceback);
        if (plese_optimize_emplaceback) {
          if constexpr (requires() { event_t(std::move(signal), pram...); }) {
            event_list.emplace_back(std::move(signal), pram...);
          } else {
            event_list.emplace_back(std::move(signal));
          }
        }
      }
      ignite_join_defuse_all_impl<version_v, event_t>(*this, event_list,
                                                      pram...);
    });
  }

public:
  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate) noexcept {
    base::reserve(node_estimate, edge_estimate);
    event_list.reserve(node_estimate);
  }
  MJZ_CX_ND_FN bool run_one(auto &&...pram) noexcept {
    return execute_resolution_wave(pram...);
  }

  MJZ_CX_FN uintlen_t run_all(uintlen_t limit, auto &&...pram) noexcept {
    while (limit && execute_resolution_wave(pram...))
      limit--;
    return limit;
  }
};

template <version_t version_v, auto max_invalid_state_v>
struct signal_dependency_graph_t
    : basic_dependency_graph_base_t<version_v, max_invalid_state_v> {
  using base = basic_dependency_graph_base_t<version_v, max_invalid_state_v>;

protected:
  MJZ_CX_FN bool execute_resolution_wave(auto &resolve_one_callback) noexcept {
    static_assert(
        requires(previous_states_t<version_v, max_invalid_state_v> signal) {
          { (void)resolve_one_callback(*this, std::move(signal)) } noexcept;
        });
    return base::run_one_callback([&](auto &&id_range) noexcept {
      for (auto id : id_range) {
        (void)resolve_one_callback(*this, base::passive_trigger(id));
      }
    });
  }

public:
  MJZ_CX_ND_FN bool run_one(auto &&resolve_one_callback) noexcept {
    return execute_resolution_wave(resolve_one_callback);
  }

  MJZ_CX_FN uintlen_t run_all(uintlen_t limit,
                              auto &&resolve_one_callback) noexcept {
    while (limit && execute_resolution_wave(resolve_one_callback))
      limit--;
    return limit;
  }
};

template <version_t version_v, class event_t>
struct basic_uni_dependency_graph_t
    : basic_uni_dependency_graph_base_t<version_v> {
  using base = basic_uni_dependency_graph_base_t<version_v>;

protected:
  std::vector<event_t> event_list{};

  MJZ_CX_FN bool execute_resolution_wave(auto &...pram) noexcept {
    return base::run_one_callback([&](auto &&id_range) noexcept {
      MJZ_RAII_RELEASE { event_list.clear(); };
      asserts(!event_list.size());
      event_list.reserve(std::ranges::size(id_range));
      for (auto id : id_range) {
        const bool plese_optimize_emplaceback =
            event_list.capacity() != event_list.size();
        asserts(plese_optimize_emplaceback);
        MJZ_JUST_ASSUME_(plese_optimize_emplaceback);
        if (plese_optimize_emplaceback) {
          if constexpr (requires() {
                          event_t(base_node_id_t<version_v>(id), pram...);
                        }) {
            event_list.emplace_back(base_node_id_t<version_v>(id), pram...);
          } else {
            event_list.emplace_back(base_node_id_t<version_v>(id));
          }
        }
      }
      ignite_join_defuse_all_impl<version_v, event_t>(*this, event_list,
                                                      pram...);
    });
  }

public:
  MJZ_CX_FN void reserve(uintlen_t node_estimate,
                         uintlen_t edge_estimate) noexcept {
    base::reserve(node_estimate, edge_estimate);
    event_list.reserve(node_estimate);
  }
  MJZ_CX_ND_FN bool run_one(auto &&...pram) noexcept {
    return execute_resolution_wave(pram...);
  }

  MJZ_CX_FN uintlen_t run_all(uintlen_t limit, auto &&...pram) noexcept {
    while (limit && execute_resolution_wave(pram...))
      limit--;
    return limit;
  }
};

template <version_t version_v>
struct signal_uni_dependency_graph_t
    : basic_uni_dependency_graph_base_t<version_v> {
  using base = basic_uni_dependency_graph_base_t<version_v>;

protected:
  MJZ_CX_FN bool execute_resolution_wave(auto &resolve_one_callback) noexcept {
    static_assert(requires() {
      {
        (void)resolve_one_callback(*this, base_node_id_t<version_v>())
      } noexcept;
    });
    return base::run_one_callback([&](auto &&id_range) noexcept {
      for (auto id : id_range) {
        (void)resolve_one_callback(*this, base_node_id_t<version_v>(id));
      }
    });
  }

public:
  MJZ_CX_ND_FN bool run_one(auto &&resolve_one_callback) noexcept {
    return execute_resolution_wave(resolve_one_callback);
  }

  MJZ_CX_FN uintlen_t run_all(uintlen_t limit,
                              auto &&resolve_one_callback) noexcept {
    while (limit && execute_resolution_wave(resolve_one_callback))
      limit--;
    return limit;
  }
};

namespace states_ns {

#define MJZ_MCC_SRC_DAG_STATES_ALL_                                            \
  MJZ_MCC_SRC_DAG_STATES_ONE_(none)                                            \
  MJZ_MCC_SRC_DAG_STATES_ONE_(AST_v)                                           \
  MJZ_MCC_SRC_DAG_STATES_ONE_(type_check_v)                                    \
  MJZ_MCC_SRC_DAG_STATES_ONE_(codegen_v)                                       \
  MJZ_MCC_SRC_DAG_STATES_ONE_(eval_v)

///////////////////
#define MJZ_MCC_SRC_DAG_STATES_ONE_(X) X,
enum class states_e_impl_ : uint8_t {
  MJZ_MCC_SRC_DAG_STATES_ALL_ max_invalid_v
};
#undef MJZ_MCC_SRC_DAG_STATES_ONE_
using states_e = states_e_impl_;

enum class inf_states_e : uint32_t { invalid = uint32_t(-1) };
}; // namespace states_ns
///////////////////////////////
template <version_t version_v = version_t{}>
MJZ_CX_FN mjz::bstr_ns::static_string_view_t<version_v>
    state_to_str(states_ns::states_e state) noexcept {
#define MJZ_MCC_SRC_DAG_STATES_ONE_(X)                                         \
  case states_ns::states_e::X:                                                 \
    return #X;
  switch (state) {
    MJZ_MCC_SRC_DAG_STATES_ALL_
  default:
    return "unknown";
  };
#undef MJZ_MCC_SRC_DAG_STATES_ONE_
}

MJZ_CX_FN int32_t state_to_str(states_ns::inf_states_e state) noexcept {
  return int32_t(state);
}
namespace states_ns {

template <version_t version_v, class event_t>
using stated_dependency_graph_t =
    basic_dependency_graph_t<version_v, event_t, states_e::max_invalid_v>;

template <version_t version_v, class event_t>
using infinite_dependency_graph_t =
    basic_dependency_graph_t<version_v, event_t, inf_states_e::invalid>;
}; // namespace states_ns
}; // namespace  mjz::graph_ns

#endif