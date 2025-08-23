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

#include "asserts.hpp"
#ifndef MJZ_OUTPUTS_LIB_HPP_FILE_
#define MJZ_OUTPUTS_LIB_HPP_FILE_

namespace mjz {
namespace defualt_mjz_cout_namespace_stuff {
struct MY_cout_t {
  MJZ_MCONSTANT(bool) log { MJZ_DO_DEBUG_COUT };
  template <typename T>
#if MJZ_WITH_iostream
    requires requires(T &&arg) { std::cout << std::forward<T>(arg); }
#endif  // MJZ_WITH_iostream
  MJZ_CX_FN auto &&operator<<(MJZ_MAYBE_UNUSED T &&arg) const {
#if MJZ_WITH_iostream
    MJZ_IFN_CONSTEVAL {
      if constexpr (log) {
        std::cout << std::forward<T>(arg);
      }
    }
#endif  // MJZ_WITH_iostream
    return *this;
  }
};
};  // namespace defualt_mjz_cout_namespace_stuff
template <auto Cout_get = []() -> defualt_mjz_cout_namespace_stuff::MY_cout_t {
  return {};
}>
struct mjz_cout_t_ {
  template <typename... Ts>
  MJZ_CX_FN static void print(Ts &&...args) {
    // we use operator,(r,l)->l
    MJZ_UNUSED totally_empty_type_t a[]{
        (Cout_get() << std::forward<Ts>(args),
         totally_empty_type)...};  //-V3519 //-V2528
  }
  template <class L_t>
  MJZ_CX_FN static void doln_FE(L_t &&lam) {
    // we use operator,(r,l)->l
    auto temp = lam();
  }
  template <class L_t>
  MJZ_UNUSED MJZ_CX_FN void do_FE(L_t &&) noexcept {}
  template <class L_t, typename T1, typename... Ts>
  static inline void do_FE(L_t &&lam, T1 &&arg1, Ts &&...args) {  //-V2565
    // we use operator,(r,l)->l
    MJZ_UNUSED auto temp = lam(std::forward<T1>(
        arg1));  // for users to be able to do something on ~temp
    do_FE(std::forward<L_t>(lam), std::forward<Ts>(args)...);
  }
  template <class L_t, typename T1, typename... Ts>
  MJZ_CX_FN static void doln_FE(L_t &&lam, T1 &&arg1,
                                Ts &&...args) {  //-V2565
    // we use operator,(r,l)->l
    auto temp = lam(std::forward<T1>(arg1));
    doln_FE(std::forward<L_t>(lam), std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static void println(Ts &&...args) {
    print(std::forward<Ts>(args)..., '\n');
  }
  template <typename... Ts>
  MJZ_CX_FN static void println_FE(Ts &&...args) {
    MJZ_UNUSED char a[sizeof...(Ts)]{
        //-V2571
        //-V3502 //-V2504 //-V3546 //-V2571
        ([]<typename T>(T &&arg) {  //-V3546 //-V2571 //-V2504 //-V3502
          Cout_get() << std::forward<T>(arg) << '\n';
          return '\0';
        }(std::forward<Ts>(args)))...};
  }
  MJZ_MCONSTANT(auto)
  geter = Cout_get;
};
using mjz_debug_cout = mjz_cout_t_<>;

}  // namespace mjz
#endif  // MJZ_OUTPUTS_LIB_HPP_FILE_