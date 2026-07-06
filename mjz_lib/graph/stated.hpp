#ifndef MJZ_SRC_GRAPH_stated_FILE_
#define MJZ_SRC_GRAPH_stated_FILE_
#include "base.hpp"
namespace mjz::graph_ns {
/////////////////////////////// from my compiler project

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