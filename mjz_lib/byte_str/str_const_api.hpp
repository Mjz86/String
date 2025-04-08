
#include "string_api.hpp"

#ifndef MJZ_BYTE_STRING_string_const_ABI_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_const_ABI_LIB_HPP_FILE_
namespace mjz ::bstr_ns {
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const typename basic_str_t<version_v, has_alloc_v_>::m_t &
basic_str_t<version_v, has_alloc_v_>::unsafe_handle(
    const dont_mess_up_t &, const dont_mess_up_t &) const noexcept {
  return m;
};

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const allocs_ns::alloc_base_ref_t<version_v> &
basic_str_t<version_v, has_alloc_v_>::get_alloc() const noexcept {
  return m.get_alloc();
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN base_string_view_t<version_v>
basic_str_t<version_v, has_alloc_v_>::to_base_view_pv_fn_(
    unsafe_ns::i_know_what_im_doing_t idk) const noexcept {
  return make_subview(idk, 0, nops).to_base_view_pv_fn_(idk);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN base_lazy_view_t<version_v>
basic_str_t<version_v, has_alloc_v_>::to_base_lazy_pv_fn_(
    unsafe_ns::i_know_what_im_doing_t idk) const noexcept {
  return make_subview(idk, 0, nops).to_base_lazy_pv_fn_(idk);
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>
basic_str_t<version_v, has_alloc_v_>::make_substring(
    uintlen_t byte_offset, uintlen_t byte_count,
    bool try_to_add_null) const noexcept {
  basic_str_t<version_v, has_alloc_v_> ret{};
  std::ignore = ret.copy_assign(*this, false, byte_offset, byte_count) &&
                (!try_to_add_null || ret.has_null() || ret.m.add_null(true));
  return ret;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const basic_str_t<version_v, has_alloc_v_> &&
basic_str_t<version_v, has_alloc_v_>::uniqe_in_v() const noexcept {
  return static_cast<const basic_str_t<version_v, has_alloc_v_> &&>(*this);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const basic_str_t<version_v, has_alloc_v_> &&
basic_str_t<version_v, has_alloc_v_>::share_v() const noexcept {
  return uniqe_in_v();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_AL_FN bool
basic_str_t<version_v, has_alloc_v_>::make_right_then_give_has_null(
    uintlen_t &byte_offset, uintlen_t &byte_count) const noexcept {
  byte_offset = std::min(byte_offset, m.length);
  byte_count = std::min(byte_offset + byte_count, m.length) - byte_offset;
  return byte_offset + byte_count == m.length && has_null();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::has_null() const noexcept {
  return m.template d_get_cntrl<bool>(my_details::has_null);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN const char *basic_str_t<version_v, has_alloc_v_>::data()
    const & noexcept {
  return m.begin;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t
basic_str_t<version_v, has_alloc_v_>::length() const noexcept {
  return m.length;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN encodings_e
basic_str_t<version_v, has_alloc_v_>::get_encoding() const noexcept {
  return m.template d_get_cntrl<encodings_e>(my_details::encodings_bits);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::max_size() noexcept {
  return nops - 1;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char> basic_str_t<version_v, has_alloc_v_>::at(
    const uintlen_t i) const noexcept {
  return i < length() ? std::optional<char>(data()[i]) : std::nullopt;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char> basic_str_t<version_v, has_alloc_v_>::operator[](
    const uintlen_t i) const noexcept {
  return at(i);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_string_view_t<version_v>
basic_str_t<version_v, has_alloc_v_>::make_subview(
    const dont_mess_up_t &, uintlen_t byte_offset, uintlen_t byte_count,
    bool unsafe_assume_static_) const & noexcept {
  basic_string_view_t<version_v> ret_{};
  auto &&ret = ret_.unsafe_handle();
  ret.is_static = unsafe_assume_static_ || m.is_s_view();
  ret.encodings =
      uint8_t(m.template d_get_cntrl<encodings_e>(my_details::encodings_bits));
  ret.has_null_v = make_right_then_give_has_null(byte_offset, byte_count);
  ret.len = byte_count;
  ret.ptr = m.begin + byte_offset;
  return ret_;
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN basic_str_t<version_v, has_alloc_v_>
basic_str_t<version_v, has_alloc_v_>::make_substrview(
    const dont_mess_up_t &, uintlen_t byte_offset, uintlen_t byte_count,
    bool propgate_alloc, bool unsafe_assume_static_) const & noexcept {
  basic_str_t<version_v, has_alloc_v_> ret{};
  ret.reset_to_error_on_fail(
      ret.make_substrview_helper_(*this, byte_offset, byte_count,
                                  propgate_alloc, unsafe_assume_static_),
      "[ERROR]:basic_str_t::make_substrview(...): failed");
  return ret;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<uintlen_t>
basic_str_t<version_v, has_alloc_v_>::copy_bytes(uintlen_t byte_offset,
                                                 uintlen_t byte_count,
                                                 char *buf,
                                                 bool add_null) const noexcept {
  basic_string_view_t<version_v> view =
      make_subview(dont_mess_up, byte_offset, byte_count);
  memcpy(buf, view->ptr, view->len);
  if (add_null) {
    buf[view->len] = '\0';
  }
  return view->len + uintlen_t(add_null);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::hash_t
basic_str_t<version_v, has_alloc_v_>::hash() const noexcept {
  return hash_t{m.begin, m.length};
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::is_owner() const noexcept {
  return m.is_owner();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN auto basic_str_t<version_v, has_alloc_v_>::get_states()
    const noexcept {
  return m.cntrl_states();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::get_threaded()
    const noexcept {
  return !m.cntrl_states().as_not_threaded_bit;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_iterator
basic_str_t<version_v, has_alloc_v_>::begin() const noexcept {
  return
      typename basic_str_t<version_v, has_alloc_v_>::const_iterator{*this, 0};
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_iterator
basic_str_t<version_v, has_alloc_v_>::end() const noexcept {
  return typename basic_str_t<version_v, has_alloc_v_>::const_iterator{
      *this, m.length};
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_iterator
basic_str_t<version_v, has_alloc_v_>::cbegin() const noexcept {
  return begin();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_iterator
basic_str_t<version_v, has_alloc_v_>::cend() const noexcept {
  return end();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator
basic_str_t<version_v, has_alloc_v_>::rbegin() const noexcept {
  return typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator{
      end()};
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator
basic_str_t<version_v, has_alloc_v_>::rend() const noexcept {
  return typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator{
      begin()};
}
template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator
basic_str_t<version_v, has_alloc_v_>::crbegin() const noexcept {
  return rbegin();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN typename basic_str_t<version_v, has_alloc_v_>::const_reverse_iterator
basic_str_t<version_v, has_alloc_v_>::crend() const noexcept {
  return rend();
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t
basic_str_t<version_v, has_alloc_v_>::size() const noexcept {
  return m.length;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::empty() const noexcept {
  return m.length == 0;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char> basic_str_t<version_v, has_alloc_v_>::front()
    const noexcept {
  return (*this)[0];
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<char> basic_str_t<version_v, has_alloc_v_>::back()
    const noexcept {
  return (*this)[m.length - 1];
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::optional<intlen_t> basic_str_t<version_v, has_alloc_v_>::compare(
    const basic_str_t<version_v, has_alloc_v_> &rhs) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return std::nullopt;
  return traits_type{}.compare(data(), length(), rhs.data(), rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::starts_with(
    const basic_str_t<version_v, has_alloc_v_> &rhs) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return false;
  return traits_type{}.starts_with(data(), length(), rhs.data(), rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::starts_with(
    char rhs, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return false;
  return traits_type{}.starts_with(data(), length(), rhs);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::ends_with(
    const basic_str_t<version_v, has_alloc_v_> &rhs) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return false;
  return traits_type{}.ends_with(data(), length(), rhs.data(), rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::ends_with(
    char rhs, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return false;
  return traits_type{}.ends_with(data(), length(), rhs);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::contains(
    const basic_str_t<version_v, has_alloc_v_> &rhs) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return false;
  return traits_type{}.contains(data(), length(), rhs.data(), rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::contains(
    char rhs, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return false;
  return traits_type{}.contains(data(), length(), rhs);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.find(data(), length(), offset, rhs.data(), rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::rfind(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.rfind(data(), length(), offset, rhs.data(),
                             rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_first_of(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.find_first_of(data(), length(), offset, rhs.data(),
                                     rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_last_of(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.find_last_of(data(), length(), offset, rhs.data(),
                                    rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_first_not_of(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.find_first_not_of(data(), length(), offset, rhs.data(),
                                         rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_last_not_of(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const uintlen_t offset) const noexcept {
  if (rhs.get_encoding() != get_encoding()) return npos;
  return traits_type{}.find_last_not_of(data(), length(), offset, rhs.data(),
                                        rhs.length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.find(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::rfind(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.rfind(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_first_of(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.find_first_of(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_last_of(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.find_last_of(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_first_not_of(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.find_first_not_of(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN uintlen_t basic_str_t<version_v, has_alloc_v_>::find_last_not_of(
    char rhs, const uintlen_t offset, encodings_e encoding) const noexcept {
  if (encoding != get_encoding()) return npos;
  return traits_type{}.find_last_not_of(data(), length(), offset, &rhs, 1);
}

template <version_t version_v, bool has_alloc_v_>
template <std::integral T>
  requires(!std::same_as<T, bool>)
MJZ_CX_FN std::optional<T> basic_str_t<version_v, has_alloc_v_>::to_integral(
    uint8_t raidex) const noexcept {
  if (get_encoding() != encodings_e::ascii) {
    return std::nullopt;
  }
  return traits_type{}.template to_integral<T>(data(), length(), raidex);
}
template <version_t version_v, bool has_alloc_v_>
template <std::floating_point T>
  requires(!std::same_as<T, bool>)

MJZ_CX_FN std::optional<T>
basic_str_t<version_v, has_alloc_v_>::to_real_floating() const noexcept {
  if (get_encoding() != encodings_e::ascii) {
    return std::nullopt;
  }
  return traits_type{}.template to_real_floating<T>(data(), length());
}

template <version_t version_v, bool has_alloc_v_>
template <std::floating_point T>
  requires(!std::same_as<T, bool>)

MJZ_CX_FN std::optional<T> basic_str_t<version_v, has_alloc_v_>::to_floating()
    const noexcept {
  if (get_encoding() != encodings_e::ascii) {
    return std::nullopt;
  }
  return traits_type{}.template to_floating<T>(data(), length());
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool operator==(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const basic_str_t<version_v, has_alloc_v_> &lhs) noexcept {
  if (rhs.length() != lhs.length()) return false;
  auto v = rhs.compare(lhs);
  return v && *v == 0;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN std::partial_ordering operator<=>(
    const basic_str_t<version_v, has_alloc_v_> &rhs,
    const basic_str_t<version_v, has_alloc_v_> &lhs) noexcept {
  if (auto r = rhs.compare(lhs)) {
    if (*r == 0) return std::partial_ordering::equivalent;
    if (*r < 0) return std::partial_ordering::less;
    if (0 < *r) return std::partial_ordering::greater;
  }
  return std::partial_ordering::unordered;
}

template <version_t version_v, bool has_alloc_v_>
MJZ_CX_FN bool basic_str_t<version_v, has_alloc_v_>::is_error() const noexcept {
  return get_encoding() == encodings_e::err_ascii;
}

};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_const_ABI_LIB_HPP_FILE_