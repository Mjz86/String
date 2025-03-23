#include"../allocs/alloc_ref.hpp"
#include "../traits.hpp"
#include "base.hpp"
#include "traits.hpp"
//
#include <ranges>
#if MJZ_WITH_iostream
#include <stdio.h>

#include <iostream>
#endif
#ifndef MJZ_BYTE_STRING_iterator_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_iterator_LIB_HPP_FILE_
namespace mjz::bstr_ns {
template <class Str_t>
class iterator_t {
 public:
  using iterator_concept = std::contiguous_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = char;
  using difference_type = intlen_t;
  using pointer = decltype([]() { return alias_t<Str_t*>(); }()->data());
  using reference = std::remove_pointer_t<pointer>&;
  MJZ_CX_FN iterator_t() noexcept = default;
  MJZ_CX_FN iterator_t(nullptr_t) noexcept : iterator_t(){};
  MJZ_CX_FN iterator_t(Str_t& str_, uintlen_t index_ = 0) noexcept
      : str(&str_), index(index_) {}

 public:
  MJZ_CX_FN reference operator*() const noexcept {
    MJZ_UNUSED auto checker = check(true);
    return str->data()[index];
  }
  MJZ_CX_FN pointer operator->() const noexcept { return &**this; }

  MJZ_CX_FN encodings_e get_encoding() const noexcept {
    asserts(asserts.assume_rn, !!str);
    return str->get_encoding();
  }

  MJZ_CX_FN Str_t* operator()() const noexcept { return str; }

  MJZ_CX_FN bool operator!() const noexcept { return !str; }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!str; }
  MJZ_CX_FN iterator_t& operator++() noexcept {
    MJZ_UNUSED auto checker = check();
    ++index;
    return *this;
  }

  MJZ_CX_FN iterator_t operator++(int) noexcept {
    MJZ_UNUSED auto checker = check();
    iterator_t Tmp{*this};
    ++*this;
    return Tmp;
  }

  MJZ_CX_FN iterator_t& operator--() noexcept {
    MJZ_UNUSED auto checker = check();
    --index;
    return *this;
  }

  MJZ_CX_ND_FN iterator_t operator--(int) noexcept {
    MJZ_UNUSED auto checker = check();
    iterator_t Tmp{*this};
    --*this;
    return Tmp;
  }

  MJZ_CX_FN iterator_t& operator+=(const difference_type diff) noexcept {
    MJZ_UNUSED auto checker = check();
    index += diff;

    return *this;
  }
  MJZ_CX_ND_FN iterator_t operator+(const difference_type Off) const noexcept {
    MJZ_UNUSED auto checker = check();
    iterator_t Tmp{*this};
    Tmp += Off;
    return Tmp;
  }
  MJZ_CX_FN friend iterator_t operator+(const difference_type Off,
                                        iterator_t Right) noexcept {
    MJZ_UNUSED auto checker = Right.check();
    Right += Off;
    return Right;
  }

  MJZ_CX_FN iterator_t& operator-=(const difference_type Off) noexcept {
    MJZ_UNUSED auto checker = check();
    index -= Off;

    return *this;
  }
  MJZ_CX_ND_FN iterator_t operator-(const difference_type Off) const noexcept {
    MJZ_UNUSED auto checker = check();
    iterator_t Tmp{*this};
    Tmp -= Off;
    return Tmp;
  }
  MJZ_CX_ND_FN difference_type
  operator-(const iterator_t& Right) const noexcept {
    MJZ_UNUSED auto checker = check();
    return difference_type(index) - difference_type(Right.index);
  }
  MJZ_CX_ND_FN reference operator[](const difference_type Off) const noexcept {
    MJZ_UNUSED auto checker = check();
    return *(*this + Off);
  }
  MJZ_CX_ND_FN bool operator==(const iterator_t& Right) const noexcept {
    MJZ_UNUSED auto checker = check();
    return str == Right.str && index == Right.index;
  }
  MJZ_CX_ND_FN std::strong_ordering operator<=>(
      const iterator_t& Right) const noexcept {
    MJZ_UNUSED auto checker = check();
    return index <=> Right.index;
  }

 private:
  MJZ_CX_ND_FN auto check(bool extra = false) const noexcept {
    auto fn = [this, extra]() mutable noexcept -> void {
      asserts(asserts.assume_rn, [&]() noexcept {
        return str && str->data() && index <= str->length() &&
               (!extra || index != str->length());
      });
    };
    fn();
    return releaser_t{std::move(fn)};
  }
  Str_t* str{};
  uintlen_t index{};
};

namespace basic_str_abi_ns_ {
template <version_t version_v>
struct mut_ref_t {
  uintlen_t len;
  char* ptr;
  encodings_e encodings;
  MJZ_CX_ND_FN char* data() const noexcept { return ptr; }
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return len; }
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept { return encodings; }
  using mut_iterator = iterator_t<const mut_ref_t>;
  using traits_type = byte_traits_t<version_v>;
  using value_type = char;
  using pointer = char*;
  using const_pointer = char*;
  using reference = char&;
  using const_reference = char&;
  using const_iterator = mut_iterator;
  using iterator = mut_iterator;
  using const_reverse_iterator = std::reverse_iterator<mut_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = uintlen_t;
  using difference_type = intlen_t;
  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, len);
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

  MJZ_CX_ND_FN size_type size() const noexcept { return len; }
};
template <version_t version_v>
using mut_iterator_t = typename mut_ref_t<version_v>::iterator;

};  // namespace basic_str_abi_ns_

template <typename T, version_t version_v>
concept base_out_it_viawble_c =
    requires(T obj, void_struct_t* vp,
             base_string_view_t<version_v> opt_view_or_reserve) {
      {
        obj.format_back_insert_append_pv_fn_(unsafe_ns::unsafe_v,
                                             opt_view_or_reserve)
      } noexcept -> std::same_as<success_t>;
      static_cast<T*>(vp);
    };

template <typename T, version_t version_v>
concept base_out_it_lazy_viawble_c =

    requires(T obj, void_struct_t* vp,
             base_lazy_view_t<version_v> opt_view_or_reserve) {
      {
        obj.format_back_insert_append_pv_fn_(unsafe_ns::unsafe_v,
                                             opt_view_or_reserve)
      } noexcept -> std::same_as<success_t>;
      static_cast<T*>(vp);
    };
template <version_t version_v>
class base_out_it_t : public void_struct_t {
 public:
  using bview = base_string_view_t<version_v>;
  using blazy = base_lazy_view_t<version_v>;
  using fn_t = funcptr_of_t<success_t(void_struct_t& obj,
                                      blazy opt_view_or_reserve) noexcept>;
  using it_t = base_out_it_t;
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using pointer = void;
  using reference = void;
  using container_type = void;
  using difference_type = intptr_t;

  template <base_out_it_lazy_viawble_c<version_v> T>
  MJZ_CX_FN base_out_it_t(T* actual_object, void_struct_t) noexcept
      : obj(static_cast<void_struct_t*>(actual_object)),
        function_ptr(+[](void_struct_t& obj_ref,
                         blazy opt_view_or_reserve) noexcept -> success_t {
          return static_cast<T&>(obj_ref).format_back_insert_append_pv_fn_(
              unsafe_ns::unsafe_v, opt_view_or_reserve);
        }) {}

  template <base_out_it_viawble_c<version_v> T>
    requires(!base_out_it_lazy_viawble_c<T, version_v>)
  MJZ_CX_FN base_out_it_t(T* actual_object, void_struct_t) noexcept
      : obj(static_cast<void_struct_t*>(actual_object)),
        function_ptr(+[](void_struct_t& obj_ref,
                         blazy opt_view_or_reserve) noexcept -> success_t {
          auto& actual_obj = static_cast<T&>(obj_ref);
          if (opt_view_or_reserve.is_invalid()) {
            return actual_obj.format_back_insert_append_pv_fn_(
                unsafe_ns::unsafe_v,
                bview::make(nullptr, 0,
                            encodings_e(opt_view_or_reserve.encodings), true));
          }
          return actual_obj.format_back_insert_append_pv_fn_(
                     unsafe_ns::unsafe_v,
                     bview::make(nullptr, opt_view_or_reserve.len,
                                 encodings_e(opt_view_or_reserve.encodings))) &&
                 opt_view_or_reserve.get_value(
                     [&](base_string_view_t<version_v> read_slice) noexcept
                     -> success_t {
                       return actual_obj.format_back_insert_append_pv_fn_(
                           unsafe_ns::unsafe_v, read_slice);
                     });
        }) {}

  template <typename T>
    requires(!partial_same_as<T, base_out_it_t>)
  MJZ_CX_FN base_out_it_t(T& actual_object) noexcept
      : base_out_it_t(std::addressof(actual_object), void_struct_t{}) {}
  MJZ_CX_FN base_out_it_t() noexcept = default;
  MJZ_CX_FN base_out_it_t(nullptr_t) noexcept : base_out_it_t(){};

  MJZ_CX_FN it_t& push_back(char val, encodings_e e) noexcept {
    return fn(bview::make(&val, 1, e));
  }
  MJZ_CX_FN it_t& multi_push_back(char val, uintlen_t count,
                                  encodings_e e) noexcept {
    blazy lazy{};
    lazy.encodings = uint8_t(e);
    lazy.len = count;
    lazy.data.ch = val;
    lazy.state_type = lazy.char_se;
    return fn(lazy);
  }

  MJZ_CX_FN it_t& reserve(uintlen_t length, encodings_e encoding) noexcept {
    return fn(bview::make(nullptr, length, encoding));
  }

  MJZ_CX_ND_FN it_t& operator*() noexcept { return *this; }

  MJZ_CX_FN it_t& operator++() noexcept { return *this; }

  MJZ_CX_FN it_t operator++(int) noexcept { return *this; }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!obj; }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, bview opt_view_or_reserve) noexcept {
    return !!fn(opt_view_or_reserve);
  }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, blazy opt_view_or_reserve) noexcept {
    return !!fn(opt_view_or_reserve);
  }
  MJZ_CX_FN success_t append(bview opt_view_or_reserve) noexcept {
    if (opt_view_or_reserve.ptr) {
      return !!fn(opt_view_or_reserve);
    }
    return !opt_view_or_reserve.len;
  }

 protected:
  MJZ_CX_FN it_t& fn(blazy opt_view_or_reserve) noexcept {
    if (!obj || !function_ptr) return *this;
    if (!opt_view_or_reserve.is_invalid() && !opt_view_or_reserve.len)
      return *this;
    if (obj && function_ptr(*obj, opt_view_or_reserve)) return *this;
    obj = nullptr;
    function_ptr = nullptr;
    return *this;
  }
  MJZ_CX_FN it_t& fn(bview opt_view_or_reserve) noexcept {
    return fn(opt_view_or_reserve.to_base_lazy_pv_fn_(unsafe_ns::unsafe_v));
  }
  template <class>
  friend class mjz_private_accessed_t;

  void_struct_t* obj{};
  fn_t function_ptr{};
};

template <version_t version_v>
struct count_out_buf_t : void_struct_t {
  uintlen_t count{};
  encodings_e encoding{};
  bool invalid{};
  using blazy = base_lazy_view_t<version_v>;
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, blazy opt_view_or_reserve) noexcept {
    if (invalid || encoding != opt_view_or_reserve.get_encoding() ||
        opt_view_or_reserve.is_invalid()) {
      invalid = true;
      return false;
    }
    if (!opt_view_or_reserve.is_resurve()) {
      count += opt_view_or_reserve.len;
    }
    return true;
  }
};

template <version_t version_v>
struct out_buf_t : void_struct_t {
 private:
  using bview = base_string_view_t<version_v>;
  char* buf{};
  uintlen_t size_left{};
  uintlen_t len{};
  encodings_e encoding{};

 public:
  using blazy = base_lazy_view_t<version_v>;
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, blazy view) noexcept {
    if (!buf) return false;

    if (size_left < view.len || view.get_encoding() != encoding ||
        view.is_invalid()) {
      buf = nullptr;
      len = 0;
      size_left = 0;
      return false;
    }
    return view.get_value_fn(
        view, +no_type_ns::make<lazy_reader_fnt<version_v>>(
                  [this](base_string_view_t<version_v> read_slice) noexcept
                  -> success_t {
                    size_left -= read_slice.len;
                    memcpy(buf + len, read_slice.ptr, read_slice.len);
                    len += read_slice.len;
                    return true;
                  }));
  }

  MJZ_CX_ND_FN const char* data() const noexcept { return this->buf; }
  MJZ_CX_ND_FN uintlen_t size() const noexcept { return this->len; }
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return this->len; }
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept {
    return this->encoding;
  }

 public:
  MJZ_CX_FN out_buf_t(char* buf_, uintlen_t size_,
                      encodings_e encoding_) noexcept
      : buf{buf_}, size_left{size_}, encoding{encoding_} {}
  template <size_t N>
  MJZ_CX_FN out_buf_t(char (&a)[N], encodings_e encoding_) noexcept
      : buf{a}, size_left{N}, encoding{encoding_} {}
};

template <version_t version_v>
struct out_buf_it_t : void_struct_t {
 private:
  using bview = base_string_view_t<version_v>;
  base_out_it_t<version_v> it{};
  char* buf{};
  uintlen_t cap{};
  uintlen_t len{};
  encodings_e encoding{};

 public:
  using blazy = base_lazy_view_t<version_v>;
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t idk, blazy view) noexcept {
    if (!buf || !it) return false;
    if (view.is_invalid()) return fail();
    if (!opt_flush(bview::make(nullptr, view.len, encoding))) return false;
    if (view.is_resurve()) return true;
    if (cap < view.len + len) {
      if (!flush()) return false;
      return it.format_back_insert_append_pv_fn_(idk, view) || fail();
    }
    return view.get_value([&](base_string_view_t<version_v> read_slice) noexcept
                          -> success_t { return opt_flush(read_slice); }) ||
           fail();
  }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, bview view) noexcept {
    if (!buf || !it) return false;
    return opt_flush(view);
  }

  MJZ_CX_FN success_t flush() noexcept {
    return write(bview::make("", 0, encoding));
  }

 private:
  MJZ_CX_FN success_t opt_flush(bview view) noexcept {
    if (!buf || encoding != view.get_encoding() ||
        (!view.ptr && view.has_null_v))
      return fail();
    if (cap < view.len + len) return write(view);
    if (!view.ptr) return true;
    memcpy(buf + len, view.ptr, view.len);
    len += view.len;
    return true;
  }

  MJZ_CX_FN success_t fail() noexcept {
    it.format_back_insert_append_pv_fn_(
        unsafe_ns::unsafe_v, bview::make(nullptr, 0, encoding, true));

    it = base_out_it_t<version_v>{};
    cap = 0;
    buf = nullptr;
    len = 0;
    return false;
  }
  MJZ_CX_FN success_t write(bview view) noexcept {
    if (!buf || (!view.ptr && view.has_null_v)) return fail();
    if (len) {
      auto len_buf = std::exchange(len, 0);
      if (!write(bview::make(nullptr, len_buf + view.len, encoding)))
        return false;

      if (!view.ptr) {
        len = len_buf;
        return true;
      }
      if (!write(bview::make(buf, len_buf, encoding))) return false;
    }
    if (view.get_encoding() != encoding ||
        !it.format_back_insert_append_pv_fn_(unsafe_ns::unsafe_v, view)) {
      return fail();
    }
    return true;
  }

 public:
  MJZ_NO_CPY(out_buf_it_t);
  MJZ_CX_FN out_buf_it_t(out_buf_it_t&& obj) noexcept {
    *this = std::move(obj);
  }
  MJZ_CX_FN out_buf_it_t& operator=(out_buf_it_t&& obj) noexcept {
    if (this == &obj) return *this;
    flush();
    it = std::exchange(obj.it, {});
    buf = std::exchange(obj.buf, {});
    cap = std::exchange(obj.cap, {});
    encoding = std::exchange(obj.encoding, {});

    len = std::exchange(obj.len, {});
    return *this;
  }
  MJZ_CX_FN ~out_buf_it_t() noexcept { flush(); }
  MJZ_CX_FN out_buf_it_t(base_out_it_t<version_v> out, char* buf_,
                         uintlen_t size_, encodings_e encoding_) noexcept
      : it{out}, buf{buf_}, cap{size_}, encoding{encoding_} {}
  template <size_t N>
  MJZ_CX_FN out_buf_it_t(base_out_it_t<version_v> out, char (&a)[N],
                         encodings_e encoding_) noexcept
      : it{out}, buf{a}, cap{N}, encoding{encoding_} {}
};

template <version_t version_v>
struct sub_out_iter_t : void_struct_t {
  using bview = base_string_view_t<version_v>;

 private:
  base_out_it_t<version_v> base{};
  uintlen_t left_offset{};
  uintlen_t left_len{};
  encodings_e encoding{};

  using blazy = base_lazy_view_t<version_v>;

 private:
  MJZ_CX_FN success_t format_back_insert_append_pv_fn(
      unsafe_ns::i_know_what_im_doing_t idk, blazy view) noexcept {
    if (!base || view.is_invalid()) return false;

    if (view.is_resurve()) {
      if (view.len < left_offset) return true;
      view.len -= left_offset;
      return base.format_back_insert_append_pv_fn_(
          idk, bview::make(nullptr, std::min(view.len, left_len), encoding));
    }
    auto delta = std::min(view.len, left_offset);
    view.len -= delta;
    left_offset -= delta;
    view.offset += delta;
    view.len = std::min(view.len, left_len);
    left_len -= view.len;
    if (!view.len) return true;
    return base.format_back_insert_append_pv_fn_(idk, view);
  }

 public:
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t idk, blazy view) noexcept {
    if (format_back_insert_append_pv_fn(idk, view)) return true;
    base.format_back_insert_append_pv_fn_(
        unsafe_ns::unsafe_v, bview::make(nullptr, 0, encoding, true));
    base = base_out_it_t<version_v>{};
    left_offset = 0;
    left_len = 0;
    return false;
  }
  MJZ_CX_FN sub_out_iter_t(base_out_it_t<version_v> base_,
                           encodings_e encoding_, uintlen_t left_offset_ = 0,
                           uintlen_t left_len_ = uintlen_t(-1)) noexcept
      : base{base_},
        left_offset{left_offset_},
        left_len{left_len_},
        encoding{encoding_} {}
  MJZ_CX_FN sub_out_iter_t(nullptr_t = nullptr) noexcept {};

  MJZ_CX_FN explicit operator bool() const noexcept { return !!base; }
};

template <version_t version_v>
struct out_errored_it_t : void_struct_t {
  using blazy = base_lazy_view_t<version_v>;
  base_out_it_t<version_v> it{};
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t idk, blazy view) noexcept {
    view.encodings = uint8_t(encodings_e::err_ascii);
    return it.format_back_insert_append_pv_fn_(idk, view);
  }
};
/*
 *a minimal output buffer
 */
template <version_t version_v>
struct buffer_out_buf_t : void_struct_t {
  MJZ_NO_MV_NO_CPY(buffer_out_buf_t);
  allocs_ns::alloc_base_ref_t<version_v> alloc{};
  char* buffer{};
  uintlen_t capacity{};
  uintlen_t length{};
  encodings_e encoding{};
  bool invalid{};
  bool was_allocated{};
  replace_flags_t<version_v> rp{};
  allocs_ns::alloc_info_t<version_v> info{};
  using blazy = base_lazy_view_t<version_v>;
  MJZ_CX_FN buffer_out_buf_t() noexcept = default;

  MJZ_CX_FN ~buffer_out_buf_t() noexcept { free(); }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t, blazy opt_view_or_reserve) noexcept {
    if (invalid || encoding != opt_view_or_reserve.get_encoding() ||
        opt_view_or_reserve.is_invalid()) {
      invalid = true;
      return false;
    }
    if ((uintlen_t(-1) >> 1) < opt_view_or_reserve.len) return false;
    if (!reserve(length + opt_view_or_reserve.len)) {
      return false;
    }
    if (opt_view_or_reserve.is_resurve()) return true;
    uintlen_t at_offset{length};
    uintlen_t offset_of_substr{};
    char* append_begin = at_offset + buffer;
    success_t succsess{true};
    MJZ_RELEASE {
      if (!succsess) return;
      asserts(asserts.condition_rn,
              offset_of_substr == opt_view_or_reserve.len);
      length += opt_view_or_reserve.len;
    };
    succsess = opt_view_or_reserve.get_value(
        [&](base_string_view_t<version_v> read_slice) noexcept -> success_t {
          uintlen_t new_offset_of_substr{offset_of_substr + read_slice.len};
          asserts(asserts.condition_rn,
                  new_offset_of_substr <= opt_view_or_reserve.len);
          memcpy(&append_begin[offset_of_substr], read_slice.ptr,
                 read_slice.len);
          offset_of_substr = new_offset_of_substr;
          return true;
        });
    return succsess;
  }
  MJZ_CX_FN success_t reserve(uintlen_t new_cap) noexcept {
    if (new_cap <= capacity) return true;
    if ((uintlen_t(-1) >> 1) < capacity) return false;
    new_cap = rp.new_cap_calc(new_cap);
    allocs_ns::block_info_t<version_v> blk =
        alloc.allocate_bytes(new_cap, info);
    uintlen_t len = length;
    if (!blk.ptr) {
      free();
      invalid = true;
      return false;
    }
    memcpy(blk.ptr, buffer, length);
    free();
    length = len;
    capacity = blk.length;
    buffer = blk.ptr;
    was_allocated = true;
    return true;
  }

  MJZ_CX_FN void free() noexcept {
    MJZ_RELEASE {
      length = 0;
      capacity = 0;
      buffer = nullptr;
      was_allocated = false;
    };
    if (!was_allocated) return;
    allocs_ns::block_info_t<version_v> blk{};
    blk.length = capacity;
    blk.ptr = buffer;
    asserts(asserts.assume_rn, alloc.deallocate_bytes(std::move(blk), info));
  }

  MJZ_CX_FN base_string_view_t<version_v> bview() const noexcept {
    base_string_view_t<version_v> ret{};
    ret.encodings = uint8_t(encoding);
    ret.len = length;
    ret.ptr = buffer;
    return ret;
  }
  using lazy_t = base_lazy_view_t<version_v>;
  MJZ_CX_FN lazy_t
  to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t ok) const noexcept {
    return bview().to_base_lazy_pv_fn_(ok);
  }
};

template <version_t version_v>
struct generic_base_output_it_t : void_struct_t {
  MJZ_CX_FN success_t
  format_back_insert_append_pv_fn_(unsafe_ns::i_know_what_im_doing_t,
                                   base_lazy_view_t<version_v> view) noexcept {
    if (!back_insert_fnp) return false;
    bool failed{false};
    auto output_fn = [&](const char* ptr, uintlen_t len) noexcept {
      failed |=
          !(MJZ_NOEXCEPT { failed |= !back_insert_fnp(*this, ptr, len); });
      return !failed;
    };
    if (view.is_resurve()) return output_fn(nullptr, view.len);
    if (encodings_e::utf8 <= view.get_encoding() || view.is_invalid()) {
      back_insert_fnp = nullptr;
      return false;
    }

    char buffer[1024]{};
    uintlen_t length{};
    auto flush_fn = [&]() noexcept {
      output_fn(buffer, length);
      length = 0;
      return !failed;
    };
    failed |= !view.get_value(
        [&](base_string_view_t<version_v> read_slice) noexcept -> success_t {
          bool first_time{true};

          do {
            if (length + read_slice.len < sizeof(buffer)) {
              memcpy(&buffer[length], read_slice.ptr, read_slice.len);
              length += read_slice.len;
              read_slice.len = 0;
              return true;
            }
            if (first_time) {
              if (!flush_fn()) return false;
            } else {
              return output_fn(read_slice.ptr, read_slice.len);
            }
            first_time = false;
          } while (read_slice.len);
          return !failed;
        });

    return flush_fn();
  }

 private:
  using back_insert_fnp_t = success_t (*)(generic_base_output_it_t& obj,
                                          const char* ptr, uintlen_t len);
  template <class T>
  MJZ_CX_FN static success_t back_insert_fn(generic_base_output_it_t& obj,
                                            const char* ptr, uintlen_t len) {
    auto& self = static_cast<T&>(obj);
    if (ptr) {
      return self.back_insert(ptr, len);
    }
    return self.reserve(len);
  };

  template <class T>
  MJZ_CONSTANT(back_insert_fnp_t)
  back_insert_fn_v =
      &generic_base_output_it_t<version_v>::template back_insert_fn<T>;
  template <class T>
  MJZ_CX_FN static back_insert_fnp_t back_insert_fn_v_fns() noexcept {
    return generic_base_output_it_t<version_v>::template back_insert_fn_v<T>;
  }

 private:
  back_insert_fnp_t back_insert_fnp{};

 protected:
  template <class T>
  MJZ_CX_FN void init() noexcept {
    back_insert_fnp =
        generic_base_output_it_t<version_v>::template back_insert_fn_v<T>;
  }
  template <class T>
  MJZ_CX_FN static void init(T* This) noexcept {
    This->template init<T>();
  }
  MJZ_DEFAULTED_CLASS(generic_base_output_it_t);
};

template <version_t version_v, class T>
struct generic_output_it_t : generic_base_output_it_t<version_v> {
  optional_ref_t<T> value{};
  MJZ_CX_FN generic_output_it_t() noexcept {
    generic_base_output_it_t<version_v>::init(this);
  }
  MJZ_CX_FN generic_output_it_t(T& obj) noexcept : generic_output_it_t{} {
    value = &obj;
  }
  MJZ_CX_FN success_t back_insert(const char* ptr, uintlen_t len) {
    if (!value) return false;
    if constexpr (requires() { value->insert(value->end(), ptr, ptr + len); }) {
      value->insert(value->end(), ptr, ptr + len);
    } else {
      std::ranges::copy(ptr, ptr + len, std::back_insert_iterator{*value});
    }
    return true;
  };
  MJZ_CX_FN success_t reserve(uintlen_t len) {
    if (!value) return false;
    if constexpr (requires() {
                    value->reserve(len + std::ranges::size(*value));
                  }) {
      value->reserve(len + std::ranges::size(*value));
    }
    return true;
  };
};

template <version_t version_v>
struct file_output_it_t : generic_base_output_it_t<version_v> {
#if MJZ_WITH_iostream
  FILE* Stream{};
#else
  void* Stream{};
#endif

  MJZ_CX_FN file_output_it_t() noexcept {
    generic_base_output_it_t<version_v>::init(this);
  }
  MJZ_CX_FN file_output_it_t(auto* ptr) noexcept : file_output_it_t() {
    Stream = ptr;
  }
  MJZ_NCX_FN success_t back_insert(const char* ptr, uintlen_t len) {
#if MJZ_WITH_iostream
    return len == std::fwrite(ptr, 1, std::size_t(len), Stream);
#else
    return false;
#endif
  };
  MJZ_NCX_FN success_t reserve(uintlen_t) { return true; };
};
#if MJZ_WITH_iostream
template <version_t version_v>
struct stream_output_it_t : generic_base_output_it_t<version_v> {
  optional_ref_t<std::ostream> Stream{};
  MJZ_NCX_FN stream_output_it_t(std::ostream& ref) noexcept : Stream{ref} {
    generic_base_output_it_t<version_v>::init(this);
  }
  MJZ_NCX_FN success_t back_insert(const char* ptr, uintlen_t len) {
    if (!Stream) return false;
    Stream->write(ptr, std::streamsize(len));
    Stream->width(0);
    return !Stream->bad();
  };
  MJZ_NCX_FN success_t reserve(uintlen_t) { return !!Stream; };
};
template <version_t version_v>
struct it_output_stream_t : private std::streambuf, public std::ostream {
  MJZ_NO_MV_NO_CPY(it_output_stream_t);
  typedef std::streambuf base_type;
  typedef typename base_type::int_type int_type;
  typedef typename std::streamsize streamsize;
  typedef typename base_type::traits_type traits_type;
  MJZ_NCX_FN it_output_stream_t(base_out_it_t<version_v> it,
                                encodings_e encoding_ = encodings_e::ascii)
      : std::ostream{This()}, out{it}, encoding{encoding_} {}

 private:
  MJZ_NCX_FN int_type overflow(int_type ch) override {
    if (traits_type::eq_int_type(ch, traits_type::eof()) ||
        !out.push_back(traits_type::to_char_type(ch), encoding))
      return traits_type::eof();
    return ch;
  }
  MJZ_NCX_FN base_type* This() { return this; }
  MJZ_NCX_FN streamsize xsputn(const char* ptr, streamsize count) override {
    if (out.append(base_string_view_t<version_v>::make(ptr, uintlen_t(count),
                                                       encoding)))
      return count;
    return 0;
  }

 private:
  base_out_it_t<version_v> out{};
  encodings_e encoding{};
};
#endif

}  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_iterator_LIB_HPP_FILE_