#include "main.hpp"
/*
 *paper is at https://github.com/Mjz86/String_description/tree/main
 */
int main() {
#if MJZ_WITH_iostream
  MJZ_RELEASE { std::cin.get(); };
#endif  // MJZ_WITH_iostream
        /*run both in compiler and runtime to check UB*/
  constexpr used_mjz_ns::main_t code{};
  code.run();
}