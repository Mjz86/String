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

#include "../optional_ref.hpp"
#include "../restricted_arguments.hpp"
#include "base.hpp"
#include "hash_bytes.hpp"
#include "iterator.hpp"
#if MJZ_WITH_iostream
#endif  // MJZ_WITH_iostream
#ifndef MJZ_BYTE_STRING_views_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_views_LIB_HPP_FILE_
namespace mjz::bstr_ns {
template <version_t version_v>
struct basic_string_view_t : private base_string_view_t<version_v> {
  MJZ_MCONSTANT(version_t)
  Version_v{version_v};
  using base_t = base_string_view_t<version_v>;
  using self_t = basic_string_view_t<version_v>;
  using traits_type = byte_traits_t<version_v>;
  using hash_t = hash_bytes_t<version_v>;
  using value_type = char;
  using pointer = const char *;
  using const_pointer = const char *;
  using reference = const char &;
  using const_reference = const char &;
  using const_iterator = iterator_t<const self_t>;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = uintlen_t;
  using difference_type = intlen_t;

 private:
  using dont_mess_up_t = unsafe_ns::i_know_what_im_doing_t;

  template <class>
  friend class mjz_private_accessed_t;
  template <version_t, bool>
  friend struct generic_string_view_t;

 public:
  MJZ_CX_FN base_string_view_t<version_v> to_base_view_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return unsafe_handle();
  }
  using base_string_view_t<version_v>::to_base_lazy_pv_fn_;
  /*
   *gets the internal data without permission
   */
  MJZ_CX_FN base_t &unsafe_handle() noexcept {
    return static_cast<base_t &>(*this);
  } /*
     *gets the internal data without permission
     */
  MJZ_CX_FN const base_t &unsafe_handle() const noexcept {
    return static_cast<const base_t &>(*this);
  }
  MJZ_CX_FN operator base_t() const noexcept {
    auto &&ref = static_cast<base_t &>(*this);
    return base_t(ref);
  }
  MJZ_CX_FN operator const base_t &() const noexcept {
    return static_cast<const base_t &>(*this);
  }
  MJZ_CX_FN const base_t *operator->() const noexcept {
    return static_cast<const base_t *>(this);
  }

 public:
  MJZ_CX_FN basic_string_view_t() noexcept = default;
  MJZ_CX_FN basic_string_view_t &operator=(
      const basic_string_view_t &) noexcept = default;
  MJZ_CX_FN basic_string_view_t &operator=(basic_string_view_t &&) noexcept =
      default;
  MJZ_CX_FN basic_string_view_t(const basic_string_view_t &) noexcept = default;
  MJZ_CX_FN basic_string_view_t(basic_string_view_t &&) noexcept = default;
  MJZ_CX_FN ~basic_string_view_t() noexcept = default;
  MJZ_CX_FN basic_string_view_t(const base_t &b) noexcept : base_t(b) {}

  MJZ_CX_FN basic_string_view_t(nullopt_t) noexcept
      : base_t{base_t::make(nullptr, 0)} {

        };
  template <size_t N>
  MJZ_CE_FN basic_string_view_t(const char (&arr)[N]) noexcept
      : base_t{base_t::make(arr, N - (arr[N - 1] == 0), encodings_e::ascii,
                            (arr[N - 1] == 0), true)} {

        };
  MJZ_CX_FN basic_string_view_t(const char *ptr_, uintlen_t len_,
                                encodings_e encoding_ = encodings_e::ascii,
                                bool has_null_ = false,
                                bool is_static_ = false) noexcept
      : base_t{base_t::make(ptr_, len_, encoding_, has_null_, is_static_)} {

        };

 private:
  /*
   *maxes out the out of bounds indexes
   */
  MJZ_CX_FN bool make_right_then_give_has_null(
      uintlen_t &byte_offset, uintlen_t &byte_count) const noexcept {
    byte_count = std::min(byte_count, length());
    byte_offset = std::min(byte_offset, length());
    byte_count = std::min(byte_offset + byte_count, length()) - byte_offset;
    return byte_offset + byte_count == length();
  }

 public:
  MJZ_MCONSTANT(uintlen_t)
  npos{traits_type::npos};
  MJZ_MCONSTANT(uintlen_t)
  nops{traits_type::npos};
  MJZ_CX_FN static uintlen_t max_size() noexcept { return nops - 1; }
  MJZ_CX_ND_FN std::optional<char> at(const uintlen_t i) const noexcept {
    return i < length() ? std::optional<char>(data()[i]) : std::nullopt;
  }
  /*
   *returns the element at i
   */
  MJZ_CX_ND_FN std::optional<char> operator[](
      const uintlen_t i) const noexcept {
    return at(i);
  }

  MJZ_CX_FN success_t as_subview(uintlen_t offset,
                                 uintlen_t count = nops) noexcept {
    this->has_null_v &= make_right_then_give_has_null(offset, count);
    this->ptr += offset;
    this->len = count;
    return true;
  }
  MJZ_CX_ND_FN self_t make_subview(uintlen_t offset,
                                   uintlen_t count = nops) const noexcept {
    self_t ret{*this};
    ret.as_subview(offset, count);
    return ret;
  }
  MJZ_CX_ND_FN self_t operator()(uintlen_t begin_i = 0,
                                 uintlen_t end_i = nops) const noexcept {
    return make_subview(begin_i, end_i - std::min(end_i, begin_i));
  }
  MJZ_CX_FN success_t remove_prefix(uintlen_t n) noexcept {
    return as_subview(n, npos);
  }

  MJZ_CX_FN success_t remove_suffix(uintlen_t n) noexcept {
    return as_subview(0, size() - std::min(n, size()));
  }
  MJZ_CX_FN std::optional<char> pop_back() noexcept {
    if (auto v = back()) {
      remove_suffix(1);
      return v;
    }
    return std::nullopt;
  }

  MJZ_CX_FN std::optional<char> pop_front() noexcept {
    if (auto v = front()) {
      remove_prefix(1);
      return v;
    }
    return std::nullopt;
  }
  MJZ_CX_ND_FN const char *data() const noexcept { return this->ptr; }
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return this->len; }
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept {
    return encodings_e(this->encodings);
  }
  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, length());
  }
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept { return begin(); }
  MJZ_CX_ND_FN const_iterator cend() const noexcept { return end(); }

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }
  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept { return rend(); }

  MJZ_CX_ND_FN size_type size() const noexcept { return length(); }
  MJZ_CX_ND_FN bool empty() const noexcept { return length() == 0; }

  MJZ_CX_ND_FN std::optional<char> front() const noexcept { return at(0); }
  MJZ_CX_ND_FN std::optional<char> back() const noexcept {
    return at(length() - 1);
  }

  MJZ_CX_FN void swap(self_t &o) noexcept {
    self_t temp{o};
    o = *this;
    *this = std::move(temp);
  }

  MJZ_CX_ND_FN std::optional<intlen_t> compare(
      const self_t &rhs) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return std::nullopt;
    return traits_type{}.compare(data(), length(), rhs.data(), rhs.length());
  }

  MJZ_CX_ND_FN bool starts_with(const self_t &rhs) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return false;
    return traits_type{}.starts_with(data(), length(), rhs.data(),
                                     rhs.length());
  }

  MJZ_CX_ND_FN bool starts_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return false;
    return traits_type{}.starts_with(data(), length(), rhs);
  }

  MJZ_CX_ND_FN bool ends_with(const self_t &rhs) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return false;
    return traits_type{}.ends_with(data(), length(), rhs.data(), rhs.length());
  }
  MJZ_CX_ND_FN bool ends_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return false;
    return traits_type{}.ends_with(data(), length(), rhs);
  }

  MJZ_CX_ND_FN bool contains(const self_t &rhs) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return false;
    return traits_type{}.contains(data(), length(), rhs.data(), rhs.length());
  }
  MJZ_CX_ND_FN bool contains(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return false;
    return traits_type{}.contains(data(), length(), rhs);
  }
  MJZ_CX_ND_FN uintlen_t find(const self_t &rhs,
                              const uintlen_t offset = 0) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.find(data(), length(), offset, rhs.data(),
                              rhs.length());
  }
  MJZ_CX_ND_FN uintlen_t rfind(const self_t &rhs,
                               const uintlen_t offset = nops) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.rfind(data(), length(), offset, rhs.data(),
                               rhs.length());
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(const self_t &rhs, const uintlen_t offset = 0) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.find_first_of(data(), length(), offset, rhs.data(),
                                       rhs.length());
  }

  MJZ_CX_ND_FN uintlen_t find_last_of(
      const self_t &rhs, const uintlen_t offset = nops) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.find_last_of(data(), length(), offset, rhs.data(),
                                      rhs.length());
  }
  MJZ_CX_ND_FN uintlen_t find_first_not_of(
      const self_t &rhs, const uintlen_t offset = 0) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.find_first_not_of(data(), length(), offset, rhs.data(),
                                           rhs.length());
  }
  MJZ_CX_ND_FN uintlen_t find_last_not_of(
      const self_t &rhs, const uintlen_t offset = nops) const noexcept {
    if (rhs.get_encoding() != get_encoding()) return npos;
    return traits_type{}.find_last_not_of(data(), length(), offset, rhs.data(),
                                          rhs.length());
  }

  MJZ_CX_ND_FN uintlen_t
  find(char rhs, const uintlen_t offset = 0,
       encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.find(data(), length(), offset, &rhs, 1);
  }
  MJZ_CX_ND_FN uintlen_t
  rfind(char rhs, const uintlen_t offset = nops,
        encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.rfind(data(), length(), offset, &rhs, 1);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(char rhs, const uintlen_t offset = 0,
                encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.find_first_of(data(), length(), offset, &rhs, 1);
  }

  MJZ_CX_ND_FN uintlen_t
  find_last_of(char rhs, const uintlen_t offset = nops,
               encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.find_last_of(data(), length(), offset, &rhs, 1);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_not_of(char rhs, const uintlen_t offset = 0,
                    encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.find_first_not_of(data(), length(), offset, &rhs, 1);
  }
  MJZ_CX_ND_FN uintlen_t
  find_last_not_of(char rhs, const uintlen_t offset = nops,
                   encodings_e encoding = encodings_e{}) const noexcept {
    if (encoding != get_encoding()) return npos;
    return traits_type{}.find_last_not_of(data(), length(), offset, &rhs, 1);
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral(uint8_t raidex = 0) const noexcept {
    if (get_encoding() != encodings_e::ascii) {
      return std::nullopt;
    }
    return traits_type{}.template to_integral<T>(data(), length(), raidex);
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_real_floating() const noexcept {
    if (get_encoding() != encodings_e::ascii) {
      return std::nullopt;
    }
    return traits_type{}.template to_real_floating<T>(data(), length());
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_floating() const noexcept {
    if (get_encoding() != encodings_e::ascii) {
      return std::nullopt;
    }
    return traits_type{}.template to_floating<T>(data(), length());
  }

  MJZ_CX_ND_FN friend bool operator==(const self_t &rhs,
                                      const self_t &lhs) noexcept {
    if (rhs.length() != lhs.length()) return false;
    auto v = rhs.compare(lhs);
    return v && *v == 0;
  }
  MJZ_CX_ND_FN friend std::partial_ordering operator<=>(
      const self_t &rhs, const self_t &lhs) noexcept {
    if (auto r = rhs.compare(lhs)) {
      if (*r == 0) return std::partial_ordering::equivalent;
      if (*r < 0) return std::partial_ordering::less;
      if (0 < *r) return std::partial_ordering::greater;
    }
    return std::partial_ordering::unordered;
  }
  MJZ_CX_ND_FN bool has_null() const noexcept { return this->has_null_v; }
  MJZ_CX_FN const char *as_c_str() const noexcept {
    return has_null() ? data() : nullptr;
  }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!data(); }
  /*
   *calculates the hash
   */
  MJZ_CX_ND_FN hash_t hash() const noexcept { return hash_t{data(), length()}; }
  MJZ_CX_ND_FN concatabe_hash_t<version_v> concatable_hash() const noexcept {
    return concatabe_hash_t<version_v>{data(), length()};
  }

  /*
   *copies the substring in buf.
   * buf must at least be of  min(byte_count,length)+ uintlen_t(add_null) in
   *size. if add_null then we add a null terminatior.
   */
  MJZ_CX_FN success_t copy_bytes(uintlen_t byte_offset, uintlen_t byte_count,
                                 char *buf,
                                 bool add_null = false) const noexcept {
    auto view = make_subview(byte_offset, byte_count);
    memcpy(buf, view->ptr, view->len);
    if (add_null) {
      buf[view->len] = '\0';
    }
    return true;
  }

#if MJZ_WITH_iostream
  MJZ_NCX_FN friend std::ostream &operator<<(std::ostream &cout_v,
                                             const self_t &obj) {
    if (obj.get_encoding() == encodings_e::ascii) {
      return cout_v << std::string_view{obj.data(), obj.length()};
    }
    if (obj.get_encoding() == encodings_e::err_ascii) {
      return cout_v << "[ERROR!]" << std::string_view{obj.data(), obj.length()};
    }
    for (char c : obj) {
      if (obj.get_encoding() == encodings_e::utf8 && (c & uint8_t(127))) {
        if (c == '\\') {
          cout_v << '\\';
        }
        cout_v << c;
        continue;
      }
      cout_v << "\\x"
             << *traits_type{}.num_to_ascii(
                    uint8_t(uint8_t(c) & ~(uint8_t(-1) >> 4)), false)
             << *traits_type{}.num_to_ascii(
                    uint8_t(uint8_t(c) & (uint8_t(-1) >> 4)), false);
    }
    return cout_v;
  }
#endif  // MJZ_WITH_iostream
};

template <version_t version_v, bool is_static_>
struct generic_string_view_t : public basic_string_view_t<version_v> {
  static_assert(is_totaly_trivial<basic_string_view_t<version_v>>);
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CX_FN base_string_view_t<version_v> to_base_view_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return this->unsafe_handle();
  }
  using basic_string_view_t<version_v>::to_base_lazy_pv_fn_;

 private:
  template <size_t N>
  MJZ_CX_FN generic_string_view_t(const char (&str)[N], encodings_e encodings_,
                                  totally_empty_type_t) noexcept
      : basic_string_view_t<version_v>{} {
    auto &ref{basic_string_view_t<version_v>::unsafe_handle()};
    ref.ptr = str;
    ref.has_null_v = !str[N - 1];
    ref.is_static = is_static_;
    ref.encodings = static_cast<uint8_t>(encodings_);
    ref.len = ref.has_null_v ? N - 1 : N;
  }

 public:
  template <size_t N>
  MJZ_CX_FN generic_string_view_t(
      totally_empty_type_t, const char (&str)[N],
      encodings_e encodings_ = encodings_e::ascii) noexcept
    requires(is_static_)
      : generic_string_view_t{str, encodings_, totally_empty_type} {}
  template <size_t N>
  MJZ_CE_FN generic_string_view_t(
      const char (&str)[N],
      encodings_e encodings_ = encodings_e::ascii) noexcept
    requires(is_static_)
      : generic_string_view_t{str, encodings_, totally_empty_type} {}
  template <size_t N>
  MJZ_CX_FN generic_string_view_t(
      const char (&str)[N],
      encodings_e encodings_ = encodings_e::ascii) noexcept
    requires(not is_static_)
      : generic_string_view_t{str, encodings_, totally_empty_type} {}
  MJZ_CX_FN generic_string_view_t(const char *str, uintlen_t count,
                                  encodings_e encodings_ = encodings_e::ascii,
                                  bool nulled = false) noexcept
    requires(not is_static_)
      : generic_string_view_t{"", encodings_, totally_empty_type} {
    this->has_null_v = nulled;
    asserts(asserts.assume_rn, !this->has_null_v || !str[count]);
    this->ptr = str;
    this->len = count;
  }
  MJZ_CX_FN generic_string_view_t(totally_empty_type_t, const char *str,
                                  uintlen_t count,
                                  encodings_e encodings_ = encodings_e::ascii,
                                  bool nulled = false) noexcept
      : generic_string_view_t{"", encodings_, totally_empty_type} {
    this->has_null_v = nulled;
    asserts(asserts.assume_rn, !this->has_null_v || !str[count]);
    this->ptr = str;
    this->len = count;
  }
  MJZ_CX_FN generic_string_view_t(nullopt_t) noexcept
      : generic_string_view_t(totally_empty_type, nullptr, 0,
                              encodings_e::ascii, false) {}
};

template <version_t version_v>
using static_string_view_t = generic_string_view_t<version_v, true>;
template <version_t version_v>
using dynamic_string_view_t = generic_string_view_t<version_v, false>;

template <version_t version_v>
struct owned_stack_buffer_t {
  char *buffer;
  uintlen_t buffer_size;
};
namespace litteral_ns {
template <std::size_t N>
struct str_litteral_t {
  template <std::size_t... I>
  MJZ_CX_FN str_litteral_t(const char8_t (&r)[N],
                           std::index_sequence<I...>) noexcept
      : s{std::bit_cast<char>(r[I])...}, was_unicode{true} {}
  MJZ_CX_FN str_litteral_t(const char8_t (&r)[N]) noexcept
      : str_litteral_t(r, std::make_index_sequence<N>()) {}
  template <std::size_t... I>
  MJZ_CX_FN str_litteral_t(const char (&r)[N],
                           std::index_sequence<I...>) noexcept
      : s{r[I]...}, was_unicode{false} {}
  MJZ_CX_FN str_litteral_t(const char (&r)[N]) noexcept
      : str_litteral_t(r, std::make_index_sequence<N>()) {}
  MJZ_DEFAULTED_CLASS(str_litteral_t);
  char s[N]{};
  bool was_unicode{};
  MJZ_CX_ND_FN const char *c_str() const noexcept { return s; }
  MJZ_CX_ND_FN const char *data() const noexcept { return s; }
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return N - 1; }
  using value_type = char;
  using pointer = const char *;
  using const_pointer = const char *;
  using reference = const char &;
  using const_reference = const char &;
  using const_iterator = iterator_t<const str_litteral_t>;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = uintlen_t;
  using difference_type = intlen_t;
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept {
    return was_unicode ? encodings_e ::utf8 : encodings_e::ascii;
  }
  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, length());
  }
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept { return begin(); }
  MJZ_CX_ND_FN const_iterator cend() const noexcept { return end(); }

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }
  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept { return rend(); }

  MJZ_CX_ND_FN size_type size() const noexcept { return length(); }
  MJZ_CX_ND_FN bool empty() const noexcept { return length() == 0; }

  MJZ_CX_ND_FN std::optional<char> front() const noexcept { return at(0); }
  MJZ_CX_ND_FN std::optional<char> back() const noexcept {
    return at(length() - 1);
  }
  MJZ_CX_ND_FN std::optional<char> operator[](
      const uintlen_t i) const noexcept {
    return at(i);
  }
  MJZ_CX_ND_FN std::optional<char> at(const uintlen_t i) const noexcept {
    return i < length() ? std::optional<char>(data()[i]) : std::nullopt;
  }
};
template <char... cs>
MJZ_CX_FN auto &operator""_cs() noexcept {
  return static_data_t<decltype([]() noexcept {
    return std::array<char, sizeof...(cs) + 1>{cs...};
  })>()();
}
MJZ_CX_FN char operator""_c(char c) noexcept { return std::bit_cast<char>(c); }
MJZ_CX_FN char operator""_c(char8_t c) noexcept {
  return std::bit_cast<char>(c);
}
template <str_litteral_t L>
MJZ_CX_FN auto &operator""_c() noexcept {
  return L.s;
}

template <str_litteral_t L, version_t vr>
MJZ_CX_FN auto operator_view() noexcept {
  static_string_view_t<vr> ret{nullopt};
  dynamic_string_view_t<vr> view{
      L.s, L.was_unicode ? encodings_e::utf8 : encodings_e::ascii};
  base_string_view_t<vr> &ret_{ret.unsafe_handle()};
  base_string_view_t<vr> &view_{view.unsafe_handle()};
  ret_ = view_;
  ret_.is_static = true;
  return ret;
};
/*
 *specialized operator for newest version
 */
template <str_litteral_t L>
MJZ_CX_FN auto operator""_view() noexcept {
  return operator_view<L, version_t{}>();
}
};  // namespace litteral_ns

template <version_t version_v>
struct status_view_t : public static_string_view_t<version_v> {
  using sview_t = static_string_view_t<version_v>;
  using sview_t::to_base_lazy_pv_fn_;
  using sview_t::to_base_view_pv_fn_;
  using sview_t::unsafe_handle;
  MJZ_MCONSTANT(sview_t) empty { nullopt };
  template <size_t N>
  MJZ_CE_FN status_view_t(const char (&str)[N],
                          encodings_e encodings_ = encodings_e::ascii) noexcept
      : sview_t{totally_empty_type, str, encodings_} {}
  MJZ_CX_FN status_view_t(sview_t view) noexcept : sview_t{view} {}
  MJZ_CX_FN status_view_t(nullopt_t = nullopt) noexcept : sview_t{empty} {}
  MJZ_CX_FN status_view_t(nullptr_t) noexcept : status_view_t{} {}
  MJZ_CX_FN status_view_t(const char (&a)[1]) noexcept : status_view_t{} {
    asserts(asserts.assume_rn, !a[0]);
  }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!*this; }
  MJZ_CX_FN bool operator!() const noexcept {
    return sview_t::operator bool() && this->length();
  }
  MJZ_CX_FN sview_t sview() const noexcept { return *this; }
};

};  // namespace mjz::bstr_ns
template <mjz::version_t version_v>
struct std::hash<mjz::bstr_ns::basic_string_view_t<version_v>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};
template <mjz::version_t version_v, bool is_static_>
struct std::hash<mjz::bstr_ns::generic_string_view_t<version_v, is_static_>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};
template <mjz::version_t version_v>
MJZ_FCONSTANT(bool)
std::ranges::enable_borrowed_range<
    mjz::bstr_ns::basic_string_view_t<version_v>> = true;
template <mjz::version_t version_v>
MJZ_FCONSTANT(bool)
std::ranges::enable_view<mjz::bstr_ns::basic_string_view_t<version_v>> = true;

template <mjz::version_t version_v, bool is_static_>
MJZ_FCONSTANT(bool)
std::ranges::enable_borrowed_range<
    mjz::bstr_ns::generic_string_view_t<version_v, is_static_>> = true;
template <mjz::version_t version_v, bool is_static_>
MJZ_FCONSTANT(bool)
std::ranges::enable_view<
    mjz::bstr_ns::generic_string_view_t<version_v, is_static_>> = true;

#endif  // MJZ_BYTE_STRING_views_LIB_HPP_FILE_