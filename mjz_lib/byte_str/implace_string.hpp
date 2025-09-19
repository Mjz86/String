
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

#include "../byte_str/string.hpp"
#ifndef MJZ_BYTE_STRING_implace_string_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_implace_string_LIB_HPP_FILE_
MJZ_EXPORT namespace mjz::bstr_ns {
  template <version_t version_v, basic_str_props_t<version_v> props_v,
            uintlen_t stack_cap>
  struct implace_string_data_t : public basic_str_t<version_v, props_v> {
   private:
    template <class>
    friend class mjz_private_accessed_t;
  };
  template <version_t version_v, basic_str_props_t<version_v> props_v,
            uintlen_t stack_cap>
    requires requires() {
      requires(basic_str_t<version_v, props_v>::sso_cap < stack_cap);
    }
  struct implace_string_data_t<version_v, props_v, stack_cap>
      : public basic_str_t<version_v, props_v> {
   protected:
    template <class>
    friend class mjz_private_accessed_t;
    MJZ_MCONSTANT(uintlen_t)
    sso_cap_v_{((stack_cap / sizeof(uintlen_t)) +
                uintlen_t(!!(stack_cap % sizeof(uintlen_t)))) *
               sizeof(uintlen_t)};

    union m_stack_buffer_t_ {
      char dummy_{};
      char data[sso_cap_v_];
    } m_stack_buffer_{};
  };
  // NOT MJZ_trivially_relocatable
  template <
      version_t version_v, uintlen_t stack_cap = 256 - 4 * sizeof(uintlen_t),
      basic_str_props_t<version_v> props_v = basic_str_props_t<version_v>{}>
  struct implace_str_t
      : public implace_string_data_t<version_v, props_v, stack_cap> {
    using implace_str_t_unique_tag_ = void;

   private:
    template <version_t version_v_0_, uintlen_t,
              basic_str_props_t<version_v_0_>>
    friend struct implace_str_t;
    template <class>
    friend class mjz_private_accessed_t;

   public:
    using traits_type = byte_traits_t<version_v>;
    using value_type = char;
    using pointer =
        std::conditional_t<props_v.is_ownerized, char *, const char *>;
    using const_pointer = const char *;
    using reference =
        std::conditional_t<props_v.is_ownerized, char &, const char &>;
    using const_reference = const char &;
    using const_iterator = iterator_t<const implace_str_t>;
    using iterator = iterator_t<implace_str_t>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using size_type = uintlen_t;
    using difference_type = intlen_t;

    using back_insert_iterator_t = base_out_it_t<version_v>;

   private:
    using static_string_view = static_string_view_t<version_v>;
    using dynamic_string_view = dynamic_string_view_t<version_v>;
    using generic_string_view = basic_string_view_t<version_v>;

    using str_t = basic_str_t<version_v, props_v>;
    // using    base_str_info = base_str_info_t<version_v>;
    using cheap_str_info = cheap_base_str_info_t<version_v>;
    using EM_t = encodings_e;
    using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
    using owned_stack_buffer = owned_stack_buffer_t<version_v>;
    using hash_t = hash_bytes_t<version_v>;

    using dont_mess_up_t = unsafe_ns::i_know_what_im_doing_t;
    using str_t::nops;
    using str_t::npos;

    MJZ_CX_FN std::optional<owned_stack_buffer_t<version_v>>
    my_stack_buffer() noexcept {
      return [&]<int = 0>() noexcept
                 -> std::optional<owned_stack_buffer_t<version_v>> {
        if constexpr (requires() { this->m_stack_buffer_; }) {
          this->m_stack_buffer_.data[0] = 0;
          auto temp = std::span(this->m_stack_buffer_.data);
          return owned_stack_buffer_t<version_v>{temp.data(), temp.size()};
        }
        return {};
      }();
    }
    MJZ_CX_FN str_t &m_str() noexcept { return *this; }
    MJZ_CX_FN const str_t &m_str() const noexcept { return *this; }
    MJZ_CX_FN success_t init_stack(str_t &where) noexcept {
      std::optional<owned_stack_buffer_t<version_v>> msb = my_stack_buffer();
      if (!msb) {
        return true;
      }
      std::destroy_at(&where);
      std::construct_at(&where, unsafe_ns::unsafe_v, std::move(*msb));
      return !where.is_error();
    }

   public:
    MJZ_CX_FN implace_str_t() noexcept {
      asserts(asserts.assume_rn, init_stack(m_str()));
      asserts(asserts.assume_rn,
              m_str().unsafe_handle_pv_(unsafe_ns::unsafe_v).no_destroy());
    }

    MJZ_CX_FN implace_str_t(str_t &&source_, void_struct_t) noexcept
        : implace_str_t() {
      m_str() = std::move(source_);
    }
    MJZ_CX_FN implace_str_t(const str_t &source_, void_struct_t) noexcept
        : implace_str_t() {
      m_str() = source_;
    }
    template <class U>
    MJZ_CX_FN implace_str_t(U &&arg) noexcept
        : implace_str_t(std::forward<U>(arg), totally_empty_type) {}
    MJZ_CX_FN implace_str_t(implace_str_t &&arg) noexcept
        : implace_str_t(std::forward<implace_str_t>(arg), totally_empty_type) {}
    MJZ_CX_FN implace_str_t(const implace_str_t &arg) noexcept
        : implace_str_t(std::forward<const implace_str_t &>(arg),
                        totally_empty_type) {}

    template <class U>
    MJZ_CX_FN implace_str_t(U &&source_, void_struct_t) noexcept
      requires requires() {
        requires std::same_as<U, std::remove_cvref_t<U>>;
        typename U::implace_str_t_unique_tag_;
      }
        : implace_str_t() {
      m_str().reset_to_error_on_fail(
          std::move(source_).move_to_dest(m_str()),
          "[Error]implace_str_t::implace_str_t( "
          "wrapped_string_data_t<version_v_src, version_v_src>&&):  failed "
          "string init");
    }
    template <class U>
    MJZ_CX_FN implace_str_t(U &&source_, void_struct_t) noexcept
      requires requires() {
        requires std::same_as<U, std::remove_reference_t<U> &>;
        typename std::remove_cvref_t<U>::implace_str_t_unique_tag_;
      }
        : implace_str_t() {
      m_str().reset_to_error_on_fail(
          m_str().assign(source_.m_str()),
          "[Error]implace_str_t::implace_str_t( "
          "const wrapped_string_data_t<version_v_src, version_v_src>&):  "
          "failed "
          "string construct");
    }

    template <class U>
      requires requires() {
        requires std::same_as<U, std::remove_cvref_t<U>>;
        typename U::implace_str_t_unique_tag_;
      }
    MJZ_CX_FN implace_str_t &operator_assign(U &&source_) noexcept {
      m_str().reset_to_error_on_fail(
          source_.move_to_dest(m_str()),
          "[Error]implace_str_t::implace_str_t &operator_assign( "
          "wrapped_string_data_t<version_v_src, version_v_src>&&):  failed "
          "string assign");
      return *this;
    }
    template <class U>
      requires requires() {
        requires std::same_as<U, std::remove_reference_t<U> &> ||
                     std::is_const_v<U>;
        typename std::remove_cvref_t<U>::implace_str_t_unique_tag_;
      }
    MJZ_CX_FN implace_str_t &operator_assign(U &&source_) noexcept {
      m_str().reset_to_error_on_fail(
          m_str().assign(source_.m_str()),
          "[Error]implace_str_t::implace_str_t &operator_assign( "
          "const wrapped_string_data_t<version_v_src, version_v_src>&):  "
          "failed "
          "string assign");
      return *this;
    }

    MJZ_CX_FN implace_str_t &operator_assign(const str_t &source_) noexcept {
      m_str().reset_to_error_on_fail(m_str().assign(source_),
                                     "[Error]implace_str_t::implace_str_t "
                                     "&operator_assign(const str_t &):  failed "
                                     "string assign");
      return *this;
    }
    MJZ_CX_FN implace_str_t &operator_assign(str_t &&source_) noexcept {
      m_str().reset_to_error_on_fail(
          reinit_stack(source_) && m_str().move_init(std::move(source_)),
          "[Error]implace_str_t::implace_str_t "
          "&operator_assign(str_t&&):  failed "
          "string assign");
      return *this;
    }
    template <class U>
    MJZ_CX_FN implace_str_t &operator=(U &&source_) noexcept {
      return operator_assign(std::forward<U>(source_));
    }
    MJZ_CX_FN implace_str_t &operator=(implace_str_t &&source_) noexcept {
      return operator_assign(std::forward<implace_str_t>(source_));
    }
    MJZ_CX_FN implace_str_t &operator=(const implace_str_t &source_) noexcept {
      return operator_assign(std::forward<const implace_str_t &>(source_));
    }

   public:
    MJZ_CX_FN success_t move_to_dest(str_t &dest) && noexcept {
      if (m_str().is_stacked()) {
        return dest.assign(m_str());
      }
      return dest.assign_move(std::move(m_str()));
    }
    template <class type_t>
      requires(!partial_same_as<implace_str_t, type_t &&> &&
               std::is_rvalue_reference_v<type_t &&>)
    MJZ_CX_FN operator type_t &&() const noexcept = delete;

    MJZ_CX_FN const str_t &get() const noexcept { return m_str(); }

    MJZ_CX_FN success_t as_substring(uintlen_t byte_offset,
                                     uintlen_t byte_count,
                                     bool try_to_add_null = true) noexcept {
      return get().as_substring(byte_offset, byte_count, try_to_add_null);
    }
    MJZ_CX_ND_FN implace_str_t make_implace_substring(
        uintlen_t byte_offset, uintlen_t byte_count) const noexcept {
      implace_str_t ret{};
      str_t &str = ret.m_str();
      m_str().reset_to_error_on_fail(
          str.copy_assign(m_str(), false, byte_offset, byte_count) &&
              ret.reinit_stack(str),
          "[Error]implace_str_t::make_substring: cannot init string");
      return ret;
    }
    MJZ_CX_ND_FN implace_str_t
    operator()(uintlen_t begin_i = 0, uintlen_t end_i = nops) const noexcept {
      if (end_i < begin_i) {
        return {};
      }
      return make_implace_substring(begin_i, end_i - begin_i);
    }

    /* similar to as_substring*/
    MJZ_CX_FN implace_str_t &to_implace_substring(
        uintlen_t byte_offset, uintlen_t byte_count) noexcept {
      m_str().reset_to_error_on_fail(
          as_substring(byte_offset, byte_count),
          "[Error]implace_str_t::to_substring: cannot init string");
      return *this;
    }
    MJZ_CX_ND_FN implace_str_t make_implace_substrview(
        const dont_mess_up_t &idk, uintlen_t byte_offset, uintlen_t byte_count,
        bool propgate_alloc = true,
        bool unsafe_assume_static_ = false) const & noexcept {
      return get().make_substrview(idk, byte_offset, byte_count, propgate_alloc,
                                   unsafe_assume_static_);
    }
  };
  namespace litteral_ns {

  /*
   *makes a gengeric implace_str_t  that views the string
   */
  template <str_litteral_t L, mjz::version_t version_v,
            mjz::uintlen_t stack_cap,
            mjz::bstr_ns::basic_str_props_t<version_v> props_v>
  MJZ_CE_FN implace_str_t<version_v, stack_cap, props_v>
  operator_icxstr() noexcept
    requires(!std::is_empty_v<implace_str_t<version_v, stack_cap, props_v>>)
  {
    return implace_str_t<version_v, stack_cap, props_v>(
        operator_str<L, version_v, props_v>());
  };
  };  // namespace litteral_ns
};  // namespace mjz::bstr_ns
MJZ_EXPORT template <mjz::version_t version_v,
                     mjz::bstr_ns::basic_str_props_t<version_v> props_v,
                     mjz::uintlen_t stack_cap>
struct std::hash<mjz::bstr_ns::implace_str_t<version_v, stack_cap, props_v>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};

#endif  // MJZ_BYTE_STRING_implace_string_LIB_HPP_FILE_
