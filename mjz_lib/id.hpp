
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
#include "byte_str/views.hpp"
#ifndef MJZ_ANY_id_FILE_HPP_
#define MJZ_ANY_id_FILE_HPP_
MJZ_EXPORT namespace mjz::any_ns {
  template <typename T>
  struct type_id_t {
#if !(defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER))
    MJZ_DEPRECATED_R(
        "this compiler will not provide any names for basic_any_t::name and "
        "the "
        "casting will be O(#bases) instead of O(log2(#bases)) on arvarge ")
#endif
    MJZ_CX_FN static std::optional<std::string_view>
    fullname_intern() noexcept {
#if defined(__clang__) || defined(__GNUC__)
      return std::string_view(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
      return std::string_view(__FUNCSIG__);
#else
      return {};
#endif
    }
    MJZ_CX_FN static std::optional<std::string_view> name() noexcept {
      if (!fullname_intern()) return {};
      size_t prefix_len = type_id_t<void>::fullname_intern()->find("void");
      size_t multiple = type_id_t<void>::fullname_intern()->size() -
                        type_id_t<int>::fullname_intern()->size();
      if (!multiple) multiple = 1;
      size_t dummy_len =
          type_id_t<void>::fullname_intern()->size() - 4 * multiple;
      size_t target_len = (fullname_intern()->size() - dummy_len) / multiple;
      std::string_view rv = fullname_intern()->substr(prefix_len, target_len);
      return rv;
    }
    template <int>
    struct name_storage_t {
      MJZ_MCONSTANT(auto)
      name_storage{[]() {
        constexpr auto name_ = name();
        std::array<char, (name_ ? name_->size() : 0) + 1> ret{};
        if (name_) {
          memcpy(ret.data(), name_->data(), name_->size());
        }
        return ret;
      }()};
      MJZ_MCONSTANT(std::optional<std::string_view>)
      value{[]() {
        std::optional<std::string_view> ret{};
        if (1 < name_storage.size()) {
          ret.emplace(std::string_view{name_storage.data()});
        }
        return ret;
      }()};
    };

    using type = T;
  };

  template <version_t version_v, class T>
  MJZ_CX_FN static bstr_ns::static_string_view_t<version_v>
  actual_name() noexcept {
    std::optional<std::string_view> opt =
        type_id_t<T>::template name_storage_t<0>::value;
    if (!opt) return nullopt;
    return bstr_ns::static_string_view_t<version_v>{
        totally_empty_type, opt->data(), opt->size(),
        bstr_ns::encodings_e::ascii, false};
  }
  template <version_t version_v, class T>
  MJZ_FCONSTANT(uintlen_t)
  hash_of_t_nzero_v{std::max<uintlen_t>(
      1, uintlen_t(actual_name<version_v, total_decay_t<T>>().hash()))};
};  // namespace mjz::any_ns
#endif  // MJZ_ANY_id_FILE_HPP_