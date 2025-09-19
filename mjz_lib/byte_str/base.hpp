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

#include "../restricted_arguments.hpp"
#include "../versions.hpp"
#ifndef MJZ_BYTE_STRING_base_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_base_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz::bstr_ns {
  MJZ_FCONSTANT(uintlen_t)
  std_view_max_size{uintlen_t(
      std::min(static_cast<size_t>(PTRDIFF_MAX), static_cast<size_t>(-1)))};

  enum class encodings_e : uint8_t {
    ascii,
    utf8,      // UTF8
    utf16_le,  // little endian UTF16
    utf16_be,  // big endian UTF16
    utf32_le,  // little  endian UTF32
    utf32_be,  // big endian UTF32
    //////////// user-specified////////////////
    usr_0,
    usr_1,
    usr_2,
    usr_3,
    usr_4,
    usr_5,
    usr_6,
    usr_7,
    usr_8,
    usr_9,
    usr_10,
    usr_11,
    usr_12,
    usr_13,
    usr_14,
    usr_15,
    usr_16,
    usr_17,
    usr_18,
    usr_19,
    usr_20,
    usr_21,
    usr_22,
    usr_23,
    usr_24,
    //--the string error type--// ,
    err_ascii,  // we have only 5 bits for the encoding
  };

  template <version_t version_v>
  struct replace_flags_t {
    uintlen_t prefer_new_cap = 0;
    bool can_choose_back : 1 = true;
    bool can_choose_front : 1 = true;
    bool no_encoding_check : 1 = false;
    bool no_allocation : 1 = false;
    bool can_change_to_ts : 1 = true;
    bool dont_add_null : 1 = true;
    bool allocate_exact : 1 = false;
    bool force_another_buffer : 1 = false;
    bool to_is_threaded_v : 1 = true;
    bool force_ownership : 1 = true;
    bool chose_more_cap_side /*==!chose_less_move_side*/ : 1 = true;
    enum class change_e : char {
      force_change_if_null,
      always_force_change,
      change_before_allocation,
      dont_change_at_all,
      change_by_allocation,
      prefer_change_if_null
    };
    enum class ownerization_e : char {
      none,
      always_ownerize_on,
      always_ownerize_off,
    };
    enum class buffer_placement_e : char {
      // O(1+) push_front , O(1+) push_back ONLY IF they(=the mutation functions
      // that call replace_data_with_char at some point) are not alternating
      // between each other
      relaxed,
      // no amortization, but better overall
      center,
      // O(1+) push_front
      back,
      // O(1+) push_back
      front,
    };
    buffer_placement_e buffer_placement_v{buffer_placement_e::relaxed};
    ownerization_e ownerization_v{ownerization_e::none};
    change_e change_alloc_v{change_e::prefer_change_if_null};
    change_e change_threaded_v{change_e::dont_change_at_all};
    MJZ_CX_FN bool always_ownerize(bool state_) const noexcept {
      using e = ownerization_e;
      switch (ownerization_v) {
        case e::none:
          return false;
        case e::always_ownerize_on:
          return state_ == true;
          break;
        case e::always_ownerize_off:
          return state_ == false;
          break;
      }
      return false;
    }
    MJZ_CX_FN bool new_always_ownerize(bool old_flag) const noexcept {
      using e = ownerization_e;
      switch (ownerization_v) {
        case e::none:
          return old_flag;
        case e::always_ownerize_on:
          return true;
          break;
        case e::always_ownerize_off:
          return false;
          break;
      }
      return old_flag;
    }
    MJZ_CX_FN success_t better_front() noexcept {
      using e = buffer_placement_e;
      if (buffer_placement_v == e::relaxed) {
        can_choose_back = false;
        can_choose_front = true;
        buffer_placement_v = e::back;
        return true;
      }
      return false;
    }
    MJZ_CX_FN success_t better_back() noexcept {
      using e = buffer_placement_e;
      if (buffer_placement_v == e::relaxed) {
        buffer_placement_v = e::front;
        can_choose_back = true;
        can_choose_front = false;
        return true;
      }
      return false;
    }
    MJZ_CX_FN uintlen_t buffer_offset(uintlen_t cap,
                                      uintlen_t len) const noexcept {
      uintlen_t delta = cap - len;
      using e = buffer_placement_e;
      switch (buffer_placement_v) {
        case e::relaxed:
          MJZ_FALLTHROUGH;
        case e::center:
          return delta >> 1;
          break;
        case e::back: {
          auto null_pos = uintlen_t(!dont_add_null);
          return std::max(null_pos, delta) - null_pos;
        } break;
        case e::front:
          return 0;
          break;
      }
      return 0;
    }

    MJZ_CX_FN bool would_change(
        change_e flag, bool already_has_alloc,
        bool wants_to_allocate_or_go_to_sso) const noexcept {
      using e = change_e;
      switch (flag) {
        case e::always_force_change:
          return true;
          break;
        case e::dont_change_at_all:
          return false;
          break;
        case e::force_change_if_null:
          return !already_has_alloc;
          break;
        case e::change_before_allocation:
          return !wants_to_allocate_or_go_to_sso;
          break;
        case e::change_by_allocation:
          return wants_to_allocate_or_go_to_sso;
          break;
        case e::prefer_change_if_null:
          return !already_has_alloc && wants_to_allocate_or_go_to_sso;
          break;
      }
      return false;
    }
    MJZ_CX_FN bool should_change(change_e flag,
                                 bool already_has_alloc) const noexcept {
      return would_change(flag, already_has_alloc, true) &&
             would_change(flag, already_has_alloc, false);
    }
    MJZ_CX_FN bool could_change(change_e flag,
                                bool already_has_alloc) const noexcept {
      return would_change(flag, already_has_alloc, true) ||
             would_change(flag, already_has_alloc, false);
    }
    MJZ_CX_FN bool would_change_alloc(
        bool already_has_alloc,
        bool wants_to_allocate_or_go_to_sso) const noexcept {
      return would_change(change_alloc_v, already_has_alloc,
                          wants_to_allocate_or_go_to_sso);
    }
    MJZ_CX_FN bool should_change_alloc(bool already_has_alloc) const noexcept {
      return should_change(change_alloc_v, already_has_alloc);
    }
    MJZ_CX_FN bool could_change_alloc(bool already_has_alloc) const noexcept {
      return could_change(change_alloc_v, already_has_alloc);
    }

    MJZ_CX_FN bool would_change_threaded(
        bool already_has_alloc,
        bool wants_to_allocate_or_go_to_sso) const noexcept {
      return would_change(change_threaded_v, already_has_alloc,
                          wants_to_allocate_or_go_to_sso);
    }
    MJZ_CX_FN bool should_change_threaded(
        bool already_has_alloc) const noexcept {
      return should_change(change_threaded_v, already_has_alloc);
    }
    MJZ_CX_FN bool could_change_threaded(
        bool already_has_alloc) const noexcept {
      return could_change(change_threaded_v, already_has_alloc);
    }
    MJZ_CX_FN bool would_change_one(
        bool already_has_alloc,
        bool wants_to_allocate_or_go_to_sso) const noexcept {
      return would_change_alloc(already_has_alloc,
                                wants_to_allocate_or_go_to_sso) ||
             would_change_threaded(already_has_alloc,
                                   wants_to_allocate_or_go_to_sso);
    }
    MJZ_CX_FN bool should_change_one(bool already_has_alloc) const noexcept {
      return should_change_alloc(already_has_alloc) ||
             should_change_threaded(already_has_alloc);
    }
    MJZ_CX_FN bool could_change_one(bool already_has_alloc) const noexcept {
      return could_change_alloc(already_has_alloc) ||
             could_change_threaded(already_has_alloc);
    }
    MJZ_CX_FN uintlen_t new_cap_calc(uintlen_t mincap) const noexcept {
      return uintlen_t(!dont_add_null) + mincap;
    }
  };

  template <version_t version_v>
  struct base_lazy_view_t;
  template <version_t version_v>
  struct base_string_view_t : void_struct_t {
    const char *ptr;
    uintlen_t len : (sizeof(uintlen_t) * 8 - 8);
    uintlen_t unused_ : 1;
    uintlen_t has_null_v : 1;
    uintlen_t is_static : 1;
    uintlen_t encodings : 5;
    MJZ_CX_AL_FN static base_string_view_t make(
        const char *ptr_, uintlen_t len_,
        encodings_e encodings_ = encodings_e::ascii, bool has_null_ = false,
        bool is_static_ = false) noexcept {
      base_string_view_t ret{};
      ret.ptr = ptr_;
      ret.has_null_v = has_null_;
      ret.is_static = is_static_;
      ret.encodings = uint8_t(encodings_);
      ret.len = len_;
      return ret;
    }
    MJZ_CX_FN encodings_e get_encoding() const noexcept {
      return encodings_e(encodings);
    }

    MJZ_CX_FN base_string_view_t
    to_base_view_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept {
      return *this;
    }
    using lazy_t = base_lazy_view_t<version_v>;
    MJZ_CX_FN lazy_t
        to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept;
  };

  // this function is sequentually called
  template <version_t version_v>
  using lazy_reader_fnt =
      alias_t<success_t(base_string_view_t<version_v> read_slice) noexcept>;
  template <version_t version_v>
  using lazy_reader_fn_base_t_ =
      no_type_ns::typeless_function_t<lazy_reader_fnt<version_v>>;
  template <version_t version_v>
  struct lazy_reader_fn_t : lazy_reader_fn_base_t_<version_v> {
    using base_t = lazy_reader_fn_base_t_<version_v>;
    optional_ref_t<char *> shortcut_dest{};
    MJZ_CX_FN lazy_reader_fn_t(
        base_t base, optional_ref_t<char *> shortcut_dest_ = nullptr) noexcept
        : base_t(base), shortcut_dest{shortcut_dest_} {}

    MJZ_CX_FN success_t run(base_string_view_t<version_v> read_slice) noexcept {
      if (shortcut_dest) {
        memcpy(*shortcut_dest, read_slice.ptr, read_slice.len);
        *shortcut_dest += read_slice.len;
        return true;
      }
      return base_t::run(read_slice);
    }
  };

  template <version_t version_v>
  struct base_lazy_view_t;
  template <version_t version_v>
  struct base_lazy_view_data_t {
    success_t (*get_value_fnp)(base_lazy_view_t<version_v> self,
                               lazy_reader_fn_t<version_v> reader) noexcept;
    const void_struct_t *obj;
  };
  template <version_t version_v>
  struct base_lazy_view_t : void_struct_t {
    enum state_types_e : uint8_t {
      invalid_se,
      char_se,
      resurve_se,
      sso_se,
      view_se,
      lazy_se,
    };
    union data_t {
      char ch;
      char sso[sizeof(uintlen_t) * 2];
      base_lazy_view_data_t<version_v> lazy;
      base_string_view_t<version_v> view;
    };
    data_t data{};
    uintlen_t state_type : 3;
    uintlen_t encodings : 5;
    uintlen_t len : (sizeof(uintlen_t) * 8 - 8);
    uintlen_t offset;
    MJZ_CX_FN encodings_e get_encoding() const noexcept {
      return encodings_e(encodings);
    }
    MJZ_CX_FN bool is_invalid() const noexcept {
      return state_type == invalid_se;
    }
    MJZ_CX_FN bool is_resurve() const noexcept {
      return state_type == resurve_se;
    }

    MJZ_CX_FN base_lazy_view_t
    to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept {
      return *this;
    }
    MJZ_CX_FN success_t get_value_fn_pv_(auto &&reader) const noexcept {
      base_string_view_t<version_v> me{};
      switch (state_type) {
        case sso_se:
          me.ptr = data.sso;
          me.len = sizeof(data.sso);
          me.encodings = encodings;
          MJZ_FALLTHROUGH;
        case view_se: {
          base_lazy_view_t self{*this};
          if (view_se == state_type) {
            me = self.data.view;
          }
          self.offset = std::min(self.offset, me.len);
          self.len = std::min(self.offset + self.len, me.len) - self.offset;
          me.has_null_v &= self.offset + self.len == me.len;

          me.ptr += self.offset;
          me.len = uintlen_t(self.len);
          return reader(me);
        } break;
        case lazy_se:
          return data.lazy.get_value_fnp(
              *this, +no_type_ns::make<lazy_reader_fnt<version_v>>(reader));
          break;
        case char_se: {
          char buf_ch_[hardware_constructive_interference_size]{};
          memset(buf_ch_, sizeof(buf_ch_), data.ch);
          for (uintlen_t i{}; i < len;) {
            base_string_view_t<version_v> view{};
            view.len = std::min(sizeof(buf_ch_), len - i);
            view.ptr = &buf_ch_[0];
            view.encodings = encodings;
            if (!reader(view)) return false;
            i += view.len;
          }
          return true;
        } break;
        case resurve_se:
          return true;
          break;
        case invalid_se:
          return false;
          break;
        default:
          return false;
          break;
      }
      return false;
    }

    MJZ_CX_FN char * /*destination+len , or null*/ get_value(
        char *destination) const noexcept {
      auto reader = [&destination](base_string_view_t<version_v> val) noexcept {
        memcpy(destination, val.ptr, val.len);
        destination += val.len;
        return true;
      };
      char *const old = destination;
      base_string_view_t<version_v> me{};
      switch (state_type) {
        case sso_se:
          me.ptr = data.sso;
          me.len = sizeof(data.sso);
          me.encodings = encodings;
          MJZ_FALLTHROUGH;
        case view_se: {
          base_lazy_view_t self{*this};
          if (view_se == state_type) {
            me = self.data.view;
          }
          self.offset = std::min(self.offset, me.len);
          self.len = std::min(self.offset + self.len, me.len) - self.offset;
          me.has_null_v &= self.offset + self.len == me.len;

          me.ptr += self.offset;
          me.len = uintlen_t(self.len);
          if (!reader(me)) return nullptr;
        } break;
        case char_se: {
          memset(destination, len, data.ch);
          destination += len;
        } break;
        case lazy_se:
          if (!data.lazy.get_value_fnp(
                  *this, lazy_reader_fn_t<version_v>{
                             +no_type_ns::make<lazy_reader_fnt<version_v>>(
                                 std::move(reader)),
                             destination})) {
            return nullptr;
          }
          break;
        case resurve_se:
          return destination;
          break;
        case invalid_se:
          return nullptr;
          break;
        default:
          return nullptr;
          break;
      }
      asserts(asserts.assume_rn, destination == old + len);
      return old + len;
    }

    MJZ_CX_FN success_t get_value(
        callable_c<lazy_reader_fnt<version_v>> auto &&reader) const noexcept {
      return get_value_fn_pv_(reader);
    }
  };

  template <version_t version_v>
  MJZ_CX_FN base_lazy_view_t<version_v>
  base_string_view_t<version_v>::to_base_lazy_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    lazy_t lazy{};
    lazy.encodings = encodings;
    lazy.len = uintlen_t(len);
    lazy.data.view = *this;
    lazy.state_type = !ptr && len ? lazy.resurve_se : lazy.view_se;
    lazy.state_type =
        !ptr && has_null_v ? uint8_t(lazy.invalid_se) : lazy.state_type;
    return lazy;
  }

};  // namespace mjz::bstr_ns

#endif  // MJZ_BYTE_STRING_base_LIB_HPP_FILE_