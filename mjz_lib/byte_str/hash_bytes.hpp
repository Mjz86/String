
/*
 * i used the libc++ hash algorithim
 */
// Definition of _Hash_bytes. -*- C++ -*-

// Copyright (C) 2010-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

// This file defines Hash_bytes, a primitive used for defining hash
// functions. Based on public domain MurmurHashUnaligned2, by Austin
// Appleby.  http://murmurhash.googlepages.com/
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

#include "../versions.hpp"
#ifndef MJZ_STR_HASH_BYTES_LIB_HPP_FILE_
#define MJZ_STR_HASH_BYTES_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz {
  template <version_t> struct hash_bytes_t {
  private:
    MJZ_IL_CX_FN static uintlen_t
    load_bytes(const char *p, uint8_t len = sizeof(uintlen_t)) noexcept {
      asserts(asserts.assume_rn, len <= sizeof(uintlen_t));
      std::array<char, sizeof(uintlen_t)> buf{};
      for (uintlen_t i{}; i < len; i++)
        buf[(size_t)i] = p[(size_t)i];
      return std::bit_cast<uintlen_t>(buf);
    }

  public:
    uintlen_t val{};

  public:
    MJZ_CX_FN static uintlen_t
    hash_bytes(const char *ptr, uintlen_t len,
               uintlen_t seed = uintlen_t(0xc70f6907UL)) noexcept {
      if constexpr (sizeof(uintlen_t) == 8) {
        auto shift_mix = [](uint64_t v) noexcept -> uint64_t {
          return v ^ (v >> 47);
        };
        const uint64_t mul =
            (((uint64_t)0xc6a4a793UL) << 32UL) + (uint64_t)0x5bd1e995UL;
        const char *const buf = static_cast<const char *>(ptr);

        // Remove the bytes not divisible by the sizeof(uint64_t).  This
        // allows the main loop to process the data as 64-bit integers.
        const uint64_t len_aligned = len & ~(uint64_t)0x7;
        const char *const end = buf + len_aligned;
        uint64_t hash = seed ^ (len * mul);
        for (const char *p = buf; p != end; p += 8) {
          const uint64_t data = shift_mix(load_bytes(p) * mul) * mul;
          hash ^= data;
          hash *= mul;
        }
        if ((len & 0x7) != 0) {
          const uint64_t data = load_bytes(end, len & 0x7);
          hash ^= data;
          hash *= mul;
        }
        hash = shift_mix(hash) * mul;
        hash = shift_mix(hash);
        return hash;
      } else if constexpr (sizeof(uintlen_t) == 4) {
        const uint32_t m = 0x5bd1e995;
        uint32_t hash = seed ^ len;
        const char *buf = static_cast<const char *>(ptr);

        // Mix 4 bytes at a time into the hash.
        while (len >= 4) {
          uint32_t k = load_bytes(buf);
          k *= m;
          k ^= k >> 24;
          k *= m;
          hash *= m;
          hash ^= k;
          buf += 4;
          len -= 4;
        }

        uint32_t k{};
        // Handle the last few bytes of the input array.
        switch (len) {
        case 3:
          k = static_cast<uint8_t>(buf[2]);
          hash ^= k << 16;
          MJZ_FALLTHROUGH;
        case 2:
          k = static_cast<uint8_t>(buf[1]);
          hash ^= k << 8;
          MJZ_FALLTHROUGH;
        case 1:
          k = static_cast<uint8_t>(buf[0]);
          hash ^= k;
          hash *= m;
        };

        // Do a few final mixes of the hash.
        hash ^= hash >> 13;
        hash *= m;
        hash ^= hash >> 15;
        return hash;
      }

      else {
        static_assert(4 <= sizeof(uintlen_t));
        return 0;
      }
    }

    MJZ_CX_FN hash_bytes_t(const char *ptr, uintlen_t len) noexcept
        : val(hash_bytes(ptr, len)) {}
    template <size_t N>
    MJZ_CE_FN hash_bytes_t(const char (&a)[N]) noexcept
        : val(hash_bytes(a, N - 1)) {}

    MJZ_CX_FN explicit hash_bytes_t(uintlen_t hash) noexcept : val(hash) {}
    MJZ_CX_FN hash_bytes_t(nullptr_t = nullptr) noexcept {}

    MJZ_CX_FN explicit operator uintlen_t() const noexcept { return val; }

  public:
    MJZ_CX_FN std::strong_ordering
    operator<=>(hash_bytes_t rhs) const noexcept {
      return val <=> rhs.val;
    }
    MJZ_CX_FN bool operator==(hash_bytes_t rhs) const noexcept {
      return val == rhs.val;
    }
  };

} // namespace mjz
#endif // MJZ_STR_HASH_BYTES_LIB_HPP_FILE_