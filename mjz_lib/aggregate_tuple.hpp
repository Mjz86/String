/*MIT License

Copyright (c) 2025 Mjz86

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
#include "memories.hpp"
#ifndef MJZ_aggregate_tuple_LIB_HPP_FILE_
#define MJZ_aggregate_tuple_LIB_HPP_FILE_

MJZ_EXPORT namespace mjz {
  template <typename T>
  concept std_tuple_like_c = requires() {
    typename std::tuple_size<std::remove_cvref_t<T>>;
    std::tuple_size<std::remove_cvref_t<T>>::value;
  };
#if MJZ_aggregate_tuple_workaround
  namespace aggregate_ns_ {
  struct filler_t {
    template <typename type> MJZ_CX_FN operator type &&();
  };
  /*we sadly cant do a generalized one in c++20
   * https://en.cppreference.com/w/cpp/language/structured_binding
   *auto [...args] = E{};
   */
  MJZ_COMMENT(R"RAW(
    vprint(" if constexpr (false){{return;}}"_view);
    for (uintlen_t i{1}; i <= 32; i++) {
      auto stuff = std::views::iota(uintlen_t{1}, i);

      auto bind = "MJZ_MAYBE_UNUSED auto&&[p0"_str;
      for (auto j : stuff) {
        vformat_to(bind, ",p{:x}"_view, j);
      }
      bind += "]= "_str;
      vprint("else if constexpr (n=={:#x}){{"_view,   i);
      vprint("{}std::forward<type>(value);if constexpr (i == 0) {{return(p0);}}"_view, bind);
      for (auto j : stuff) {
        vprint("else if constexpr (i =={0:#x}){{ return (p{0:x}); }}"_view, j);
      }
      vprint("else {{return ;}}}}"_view);
    }
    vprintln("else {{return ;}}"_view); )RAW");

  template <std::size_t i, size_t n, typename type>
  constexpr decltype(auto) get_sized(MJZ_MAYBE_UNUSED type &&value) noexcept {
    if constexpr (false) {
      return;
    } else if constexpr (n == 0x1) {
      MJZ_MAYBE_UNUSED auto &&[p0] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else {
        return;
      }
    } else if constexpr (n == 0x2) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else {
        return;
      }
    } else if constexpr (n == 0x3) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else {
        return;
      }
    } else if constexpr (n == 0x4) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else {
        return;
      }
    } else if constexpr (n == 0x5) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else {
        return;
      }
    } else if constexpr (n == 0x6) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else {
        return;
      }
    } else if constexpr (n == 0x7) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else {
        return;
      }
    } else if constexpr (n == 0x8) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else {
        return;
      }
    } else if constexpr (n == 0x9) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else {
        return;
      }
    } else if constexpr (n == 0xa) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else {
        return;
      }
    } else if constexpr (n == 0xb) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else {
        return;
      }
    } else if constexpr (n == 0xc) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else {
        return;
      }
    } else if constexpr (n == 0xd) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else {
        return;
      }
    } else if constexpr (n == 0xe) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else {
        return;
      }
    } else if constexpr (n == 0xf) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else {
        return;
      }
    } else if constexpr (n == 0x10) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else {
        return;
      }
    } else if constexpr (n == 0x11) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else {
        return;
      }
    } else if constexpr (n == 0x12) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else {
        return;
      }
    } else if constexpr (n == 0x13) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else {
        return;
      }
    } else if constexpr (n == 0x14) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else {
        return;
      }
    } else if constexpr (n == 0x15) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else {
        return;
      }
    } else if constexpr (n == 0x16) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else {
        return;
      }
    } else if constexpr (n == 0x17) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else {
        return;
      }
    } else if constexpr (n == 0x18) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else {
        return;
      }
    } else if constexpr (n == 0x19) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else {
        return;
      }
    } else if constexpr (n == 0x1a) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else {
        return;
      }
    } else if constexpr (n == 0x1b) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else {
        return;
      }
    } else if constexpr (n == 0x1c) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a, p1b] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else if constexpr (i == 0x1b) {
        return (p1b);
      } else {
        return;
      }
    } else if constexpr (n == 0x1d) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a, p1b, p1c] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else if constexpr (i == 0x1b) {
        return (p1b);
      } else if constexpr (i == 0x1c) {
        return (p1c);
      } else {
        return;
      }
    } else if constexpr (n == 0x1e) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a, p1b, p1c, p1d] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else if constexpr (i == 0x1b) {
        return (p1b);
      } else if constexpr (i == 0x1c) {
        return (p1c);
      } else if constexpr (i == 0x1d) {
        return (p1d);
      } else {
        return;
      }
    } else if constexpr (n == 0x1f) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a, p1b, p1c, p1d, p1e] =
          std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else if constexpr (i == 0x1b) {
        return (p1b);
      } else if constexpr (i == 0x1c) {
        return (p1c);
      } else if constexpr (i == 0x1d) {
        return (p1d);
      } else if constexpr (i == 0x1e) {
        return (p1e);
      } else {
        return;
      }
    } else if constexpr (n == 0x20) {
      MJZ_MAYBE_UNUSED auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb,
                               pc, pd, pe, pf, p10, p11, p12, p13, p14, p15,
                               p16, p17, p18, p19, p1a, p1b, p1c, p1d, p1e,
                               p1f] = std::forward<type>(value);
      if constexpr (i == 0) {
        return (p0);
      } else if constexpr (i == 0x1) {
        return (p1);
      } else if constexpr (i == 0x2) {
        return (p2);
      } else if constexpr (i == 0x3) {
        return (p3);
      } else if constexpr (i == 0x4) {
        return (p4);
      } else if constexpr (i == 0x5) {
        return (p5);
      } else if constexpr (i == 0x6) {
        return (p6);
      } else if constexpr (i == 0x7) {
        return (p7);
      } else if constexpr (i == 0x8) {
        return (p8);
      } else if constexpr (i == 0x9) {
        return (p9);
      } else if constexpr (i == 0xa) {
        return (pa);
      } else if constexpr (i == 0xb) {
        return (pb);
      } else if constexpr (i == 0xc) {
        return (pc);
      } else if constexpr (i == 0xd) {
        return (pd);
      } else if constexpr (i == 0xe) {
        return (pe);
      } else if constexpr (i == 0xf) {
        return (pf);
      } else if constexpr (i == 0x10) {
        return (p10);
      } else if constexpr (i == 0x11) {
        return (p11);
      } else if constexpr (i == 0x12) {
        return (p12);
      } else if constexpr (i == 0x13) {
        return (p13);
      } else if constexpr (i == 0x14) {
        return (p14);
      } else if constexpr (i == 0x15) {
        return (p15);
      } else if constexpr (i == 0x16) {
        return (p16);
      } else if constexpr (i == 0x17) {
        return (p17);
      } else if constexpr (i == 0x18) {
        return (p18);
      } else if constexpr (i == 0x19) {
        return (p19);
      } else if constexpr (i == 0x1a) {
        return (p1a);
      } else if constexpr (i == 0x1b) {
        return (p1b);
      } else if constexpr (i == 0x1c) {
        return (p1c);
      } else if constexpr (i == 0x1d) {
        return (p1d);
      } else if constexpr (i == 0x1e) {
        return (p1e);
      } else if constexpr (i == 0x1f) {
        return (p1f);
      } else {
        return;
      }
    } else {
      return;
    }
  }

  MJZ_COMMENT(R"RAW(
  vprint(" if constexpr (false){{return false;}}"_view);
  for (uintlen_t i{1}; i <= 32; i++) {
    auto stuff = std::views::iota(uintlen_t{1}, i);

    auto bind = "auto&&[p0"_str;
    for (auto j : stuff) {
      vformat_to(bind, ",p{:x}"_view, j);
    }
    bind += "]= get_invalid_T_obj<type>();"_str;
    bind = "{[](){"_str + bind + "};};"_str;
    vprint("else if constexpr (n=={:#x}){{return requires(){};}}"_view, i,
           bind);
  }
  vprintln("else {{return false;}}"_view);)RAW");

  MJZ_DISABLE_ALL_WANINGS_START_;
  template <size_t n, typename type>
  concept get_sized_helper_c = requires() {
    requires[]() {
      if constexpr (false) {
        return false;
      } else if constexpr (n == 0x1) {
        return requires() {
          []() { auto &&[p0] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x2) {
        return requires() {
          []() { auto &&[p0, p1] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x3) {
        return requires() {
          []() { auto &&[p0, p1, p2] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x4) {
        return requires() {
          []() { auto &&[p0, p1, p2, p3] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x5) {
        return requires() {
          []() { auto &&[p0, p1, p2, p3, p4] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x6) {
        return requires() {
          []() { auto &&[p0, p1, p2, p3, p4, p5] = get_invalid_T_obj<type>(); };
        };
        ;
      } else if constexpr (n == 0x7) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x8) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x9) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xa) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xb) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xc) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xd) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xe) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0xf) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd,
                    pe] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x10) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x11) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x12) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x13) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x14) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x15) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x16) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x17) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x18) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x19) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1a) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1b) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a] =
                get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1c) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a,
                    p1b] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1d) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a,
                    p1b, p1c] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1e) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a,
                    p1b, p1c, p1d] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x1f) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a,
                    p1b, p1c, p1d, p1e] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else if constexpr (n == 0x20) {
        return requires() {
          []() {
            auto &&[p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, pa, pb, pc, pd, pe,
                    pf, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p1a,
                    p1b, p1c, p1d, p1e, p1f] = get_invalid_T_obj<type>();
          };
        };
        ;
      } else {
        return false;
      }
    }
    ();
  };
  template <std::size_t i, size_t n, typename type>
  concept get_sized_c = get_sized_helper_c<n, type>;
  MJZ_DISABLE_ALL_WANINGS_END_;
  MJZ_COMMENT(R"RAW(
      vprint(" if constexpr (false){{return;}}"_view);
  for (uintlen_t i{1}; i <= 32; i++) {
    vprint(
        "else if constexpr (get_sized_c<i,{0},type>){{return get_sized<i,{0}>(std::forward<type>(value)); }}"_view,
        i);
  }
  vprintln("else {{return ;}}"_view); )RAW");

  template <std::size_t i, typename type>
  constexpr decltype(auto) get(MJZ_MAYBE_UNUSED type &&value) noexcept {
    if constexpr (false) {
      return;
    } else if constexpr (get_sized_c<i, 1, type>) {
      return get_sized<i, 1>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 2, type>) {
      return get_sized<i, 2>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 3, type>) {
      return get_sized<i, 3>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 4, type>) {
      return get_sized<i, 4>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 5, type>) {
      return get_sized<i, 5>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 6, type>) {
      return get_sized<i, 6>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 7, type>) {
      return get_sized<i, 7>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 8, type>) {
      return get_sized<i, 8>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 9, type>) {
      return get_sized<i, 9>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 10, type>) {
      return get_sized<i, 10>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 11, type>) {
      return get_sized<i, 11>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 12, type>) {
      return get_sized<i, 12>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 13, type>) {
      return get_sized<i, 13>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 14, type>) {
      return get_sized<i, 14>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 15, type>) {
      return get_sized<i, 15>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 16, type>) {
      return get_sized<i, 16>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 17, type>) {
      return get_sized<i, 17>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 18, type>) {
      return get_sized<i, 18>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 19, type>) {
      return get_sized<i, 19>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 20, type>) {
      return get_sized<i, 20>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 21, type>) {
      return get_sized<i, 21>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 22, type>) {
      return get_sized<i, 22>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 23, type>) {
      return get_sized<i, 23>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 24, type>) {
      return get_sized<i, 24>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 25, type>) {
      return get_sized<i, 25>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 26, type>) {
      return get_sized<i, 26>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 27, type>) {
      return get_sized<i, 27>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 28, type>) {
      return get_sized<i, 28>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 29, type>) {
      return get_sized<i, 29>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 30, type>) {
      return get_sized<i, 30>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 31, type>) {
      return get_sized<i, 31>(std::forward<type>(value));
    } else if constexpr (get_sized_c<i, 32, type>) {
      return get_sized<i, 32>(std::forward<type>(value));
    } else {
      return;
    }
  }
  template <typename aggregate_t, std::size_t... indices>
  concept aggregate_cs =
      !((std::same_as<decltype(get<indices>(get_invalid_T_obj<aggregate_t>())),
                      void>) ||
        ...);

  template <typename type, size_t... i>
    requires aggregate_cs<type, i...>
  void aggregate_fn(type &&, std::index_sequence<i...>) {};
  template <typename aggregate_t, std::size_t count>
  concept aggregate_c = requires() {
    aggregate_fn(get_invalid_T_obj<aggregate_t>(),
                 std::make_index_sequence<count>{});
  };
  template <typename aggregate_t, size_t size_v = 32> struct aggregate_size_t {
    MJZ_CX_FN size_t static size() noexcept
      requires aggregate_c<aggregate_t, size_v>
    {
      return size_v;
    }
    MJZ_CX_FN size_t static size() noexcept
      requires(!aggregate_c<aggregate_t, size_v>)
    {
      return aggregate_size_t<aggregate_t, size_v - 1>::size();
    }
  };

  template <typename aggregate_t> struct aggregate_size_t<aggregate_t, 0> {
    MJZ_CX_FN size_t static size() noexcept { return 0; }
  };

  template <typename type, std::size_t i>
  concept test = requires(type value) {
    { aggregate_ns_::get<i, type>(std::forward<type>(value)) };
    requires !std::same_as<void, decltype(aggregate_ns_::get<i, type>(
                                     std::forward<type>(value)))>;
  };
  template <typename type, size_t... i>
  concept tests = (test<type, i> && ...);
  template <typename type, size_t... i>
    requires tests<type, i...>
  void tests_fn(std::index_sequence<i...>){};

  } // namespace aggregate_ns_

  template <typename T>
  concept is_small_aggregate_c = requires(T value) {
    requires !std_tuple_like_c<T>;
    requires std::is_aggregate_v<std::remove_cvref_t<T>>;
    requires !!aggregate_ns_::aggregate_size_t<std::remove_cv_t<T>>::size();
    aggregate_ns_::tests_fn<
        T>(std::make_index_sequence<
           aggregate_ns_::aggregate_size_t<std::remove_cv_t<T>>::size() - 1>{});
  };
#else  // MJZ_aggregate_tuple_workaround
  template <typename T>
  concept is_small_aggregate_c = false;
#endif // MJZ_aggregate_tuple_workaround
  template <size_t I, typename T> struct tuple_element {};
  template <size_t I, std_tuple_like_c T>
  struct tuple_element<I, T> : std::tuple_element<I, T> {};
  template <typename T> struct tuple_size {};
  template <std_tuple_like_c T> struct tuple_size<T> {
    MJZ_MCONSTANT(size_t)
    value = std::tuple_size<std::remove_cvref_t<T>>::value;
  };
  template <size_t I, class T>
  using tuple_element_t = typename tuple_element<I, T>::type;
  template <class T> MJZ_FCONSTANT(size_t) tuple_size_v = tuple_size<T>::value;

  template <std::size_t Index, std_tuple_like_c T>
  MJZ_CX_FN decltype(auto) get(T && obj) {
    return std::get<Index>(std::forward<T>(obj));
  }

#if MJZ_aggregate_tuple_workaround
  template <size_t I, is_small_aggregate_c T> struct tuple_element<I, T> {
    using type = decltype(aggregate_ns_::get<I, T>(get_invalid_T_obj<T>()));
  };
  template <is_small_aggregate_c T>
  struct tuple_size<T>
      : std::integral_constant<size_t,
                               aggregate_ns_::aggregate_size_t<T>::size()> {};
  template <std::size_t Index, is_small_aggregate_c T>
  MJZ_CX_FN decltype(auto) get(T && obj) noexcept {
    return aggregate_ns_::get<Index, T>(std::forward<T>(obj));
  }
#endif // MJZ_aggregate_tuple_workaround

} // namespace mjz
#endif // MJZ_aggregate_tuple_LIB_HPP_FILE_