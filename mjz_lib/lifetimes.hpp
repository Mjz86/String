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

#ifndef MJZ_TEST_lifetimes_HPP_FILE_
#define MJZ_TEST_lifetimes_HPP_FILE_
#include "all_macors.hpp"

MJZ_EXPORT namespace mjz {
  struct lifetimes_indicator_t {
#if MJZ_DO_DEBUG_COUT && MJZ_WITH_iostream
    MJZ_CX_FN lifetimes_indicator_t() noexcept {
      MJZ_IF_CONSTEVAL { return; }
      std::cout << "lifetimes_indicator_t()->{}"
                << static_cast<const void *>(this);
    }
    MJZ_CX_FN ~lifetimes_indicator_t() noexcept {
      MJZ_IF_CONSTEVAL { return; }
      std::cout << "~lifetimes_indicator_t()->"
                << static_cast<const void *>(this);
    }
    MJZ_CX_FN
    lifetimes_indicator_t(const lifetimes_indicator_t &that) noexcept {
      MJZ_IF_CONSTEVAL { return; }
      std::cout << "lifetimes_indicator_t[copy]("
                << static_cast<const void *>(&that) << ")->"
                << static_cast<const void *>(this);
    }
    MJZ_CX_FN lifetimes_indicator_t &
    operator=(const lifetimes_indicator_t &that) noexcept {
      MJZ_IF_CONSTEVAL { return *this; }
      std::cout << "lifetimes_indicator_t[copy]="
                << static_cast<const void *>(&that) << "->"
                << static_cast<const void *>(this);
      return *this;
    }
    MJZ_CX_FN lifetimes_indicator_t(lifetimes_indicator_t &&that) noexcept {
      MJZ_IF_CONSTEVAL { return; }
      std::cout << "lifetimes_indicator_t[move]("
                << static_cast<const void *>(&that) << ")->"
                << static_cast<const void *>(this);
    }
    MJZ_CX_FN lifetimes_indicator_t &
    operator=(lifetimes_indicator_t &&that) noexcept {
      MJZ_IF_CONSTEVAL { return *this; }
      std::cout << "lifetimes_indicator_t[move]="
                << static_cast<const void *>(&that) << "->"
                << static_cast<const void *>(this);
      return *this;
    }

#endif
  };
}; // namespace mjz
#endif // MJZ_TEST_lifetimes_HPP_FILE_