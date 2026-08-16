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

#ifndef MJZ_SRC_GRAPH_dot_FILE_
#define MJZ_SRC_GRAPH_dot_FILE_
#include "../byte_str/formatting/basic_formatters.hpp"
#include "../byte_str/formatting/format.hpp"
#include "algo.hpp"
MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v>
constexpr inline static mjz::bstr_ns::basic_string_view_t<version_v>
    directed_graph_to_dot_style_v_ =
        mjz::bstr_ns::static_string_view_t<version_v>(
            "node [shape=circle, style=filled, "
            "fillcolor=lightblue,fontname=\"Helvetica\"];edge "
            "[color=gray40];");

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_ND_FN mjz::bstr_ns::basic_str_t<version_v> directed_graph_to_dot_styled(
    const basic_forest_t<version_v, R> &edge_of_node_, auto &&edge_name_array,
    mjz::bstr_ns::basic_string_view_t<version_v> name =
        mjz::bstr_ns::static_string_view_t<version_v>("G"),
    mjz::bstr_ns::basic_string_view_t<version_v> style =
        directed_graph_to_dot_style_v_<version_v>) noexcept {
  auto edge_of_node = edge_of_node_.range();
  auto iv =
      std::views::iota(uintlen_t(), uintlen_t(std::ranges::size(edge_of_node)));
  return mjz::bstr_ns::format_ns::format(
      bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<version_v,
                                                        R"RAW(digraph {} {{
            {}
            {:s:s}
            {:s:s:s}
                                                                }};)RAW">,
      name, style, iv | std::views::transform([&](uintlen_t i) noexcept {
                     return tuple_t(edge_name_array[+i], ';');
                   }),
      iv | std::views::transform([&](uintlen_t i) noexcept {
        return edge_of_node[i] |
               std::views::transform([&, i](uintlen_t child_i) noexcept {
                 return tuple_t(
                     edge_name_array[+i],
                     bstr_ns::format_ns::fmt_litteral_ns::operator_fmt<
                         version_v, " -> ">(),
                     edge_name_array[+child_i], ';');
               });
      }));
};

template <version_t version_v, class R = std::span<const uintlen_t>>
MJZ_CX_ND_FN mjz::bstr_ns::basic_str_t<version_v>
directed_graph_to_dot(const basic_forest_t<version_v, R> &edge_of_node_,
                      mjz::bstr_ns::basic_string_view_t<version_v> name =
                          mjz::bstr_ns::static_string_view_t<version_v>("G"),
                      mjz::bstr_ns::basic_string_view_t<version_v> style =
                          directed_graph_to_dot_style_v_<version_v>) noexcept {
  return directed_graph_to_dot_styled<version_v>(
      edge_of_node_, std::views::iota(uintlen_t()), name, style);
}

}; // namespace mjz::graph_ns

#endif // MJZ_SRC_GRAPH_dot_FILE_
