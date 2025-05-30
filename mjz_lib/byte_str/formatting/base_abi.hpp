/*
 * thanks to msvc for some of the inspraition.
 */
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

#include "../../allocs/alloc_ref.hpp"
#include "../hash_bytes.hpp"
#include "../views.hpp"
#ifndef MJZ_BYTE_FORMATTING_base_abi_LIB_HPP_FILE_
#define MJZ_BYTE_FORMATTING_base_abi_LIB_HPP_FILE_

namespace mjz ::bstr_ns::format_ns {

template <version_t version_v>
struct typeless_arg_ref_t;
template <version_t version_v>
struct basic_arg_ref_t;
template <version_t version_v>
struct typeid_arg_ref_t;
template <version_t version_v>
struct parse_context_t;
template <version_t version_v>
struct format_context_t;

template <version_t version_v>
struct cx_formatter_storage_base_t {};
template <version_t version_v>
struct cx_formatter_storage_base_ref_t {
  uintlen_t formatting_str_index_begin{};
  uintlen_t formatting_str_index_end{};
  uintlen_t index_of_element{};
  const cx_formatter_storage_base_t<version_v> *formatter_ptr{};
};

template <version_t version_v>
using cx_formatter_storage_ref_t =
    optional_ref_t<const cx_formatter_storage_base_t<version_v>>;

template <version_t version_v>
struct base_string_view_arg_t : base_lazy_view_t<version_v> {
  MJZ_CX_FN base_string_view_arg_t(auto &&view) noexcept
      : base_lazy_view_t<version_v>{
            view.to_base_lazy_pv_fn_(unsafe_ns::unsafe_v)} {}
  MJZ_CX_FN base_string_view_arg_t(std::span<const char> buffer) noexcept
      : base_lazy_view_t<version_v>{
            dynamic_string_view_t<version_v>{buffer.data(), buffer.size()}
                .to_base_lazy_pv_fn_(unsafe_ns::unsafe_v)} {}
  MJZ_CX_FN base_lazy_view_t<version_v> to_base_lazy_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t) const noexcept {
    return *this;
  }
};

template <version_t version_v>
using basic_formatted_types_t = void(base_string_view_arg_t<version_v>,
                                     const void *, long long, long, int, short,
                                     signed char, unsigned long long,
                                     unsigned long, unsigned int,
                                     unsigned short, unsigned char, nullptr_t,
                                     float, double);

template <typename F_t, version_t version_v, typename T>
concept valid_format_c = requires(F_t obj, const F_t cobj, T &&arg,
                                  parse_context_t<version_v> &pctx,
                                  format_context_t<version_v> &fctx) {
  { obj.~F_t() } noexcept;
  { F_t() } noexcept;
  {
    obj.parse(pctx)
  } noexcept
      -> std::same_as<typename basic_string_view_t<version_v>::const_iterator>;
  {
    cobj.format(std::forward<T>(arg), fctx)
  } noexcept -> std::same_as<base_out_it_t<version_v>>;
};

template <version_t version_v>
struct hash_context_t;
template <typename F_t, typename T, version_t version_v>
concept named_arg_c = requires(const std::remove_reference_t<T> &s,
                               hash_context_t<version_v> &ctx) {
  { F_t ::arg_name(s, ctx) } noexcept -> std::same_as<success_t>;
};
template <typename F_t, typename T, version_t version_v>
concept sname_arg_c = requires(hash_context_t<version_v> &ctx) {
  { F_t ::arg_name(ctx) } noexcept -> std::same_as<success_t>;
};

template <version_t version_v, typename T>
struct formatter_t {
  MJZ_NO_MV_NO_CPY(formatter_t);
  MJZ_CX_FN formatter_t() noexcept = delete;
  MJZ_CX_FN ~formatter_t() noexcept = delete;
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept = delete;
  MJZ_CX_FN base_out_it_t<version_v> format(
      T &&s, format_context_t<version_v> &ctx) const noexcept = delete;

  MJZ_CX_FN static success_t arg_name(hash_context_t<version_v> &ctx) noexcept =
      delete;
  MJZ_CX_FN static success_t arg_name(const std::remove_reference_t<T> &s,
                                      hash_context_t<version_v> &ctx) noexcept =
      delete;
};

template <version_t version_v, typename T, uint32_t priority>
struct default_formatter_t {
  MJZ_CONSTANT(bool) no_perfect_forwarding_v = false;
  MJZ_CONSTANT(bool) can_bitcast_optimize_v = false;
  MJZ_CONSTANT(bool) can_have_cx_formatter_v = false;
  using decay_optimize_to_t = void;
  MJZ_NO_MV_NO_CPY(default_formatter_t);
  MJZ_CX_FN default_formatter_t() noexcept = delete;
  MJZ_CX_FN ~default_formatter_t() noexcept = delete;
  MJZ_CX_FN typename basic_string_view_t<version_v>::const_iterator parse(
      parse_context_t<version_v> &ctx) noexcept = delete;
  MJZ_CX_FN base_out_it_t<version_v> format(
      T &&s, format_context_t<version_v> &ctx) const noexcept = delete;
  MJZ_CX_FN static success_t arg_name(const std::remove_reference_t<T> &s,
                                      hash_context_t<version_v> &ctx) noexcept =
      delete;
  MJZ_CX_FN static success_t arg_name(hash_context_t<version_v> &ctx) noexcept =
      delete;
};

template <version_t version_v, typename T, uint32_t priority = 0>
struct default_formatter_slector_helper_t {
  using type = typename default_formatter_slector_helper_t<version_v, T,
                                                           priority + 1>::type;
};
template <version_t version_v, typename T, uint32_t priority>
  requires(256 < priority)
struct default_formatter_slector_helper_t<version_v, T, priority> {
  using type = formatter_t<version_v, T>;
};
template <version_t version_v, typename T>
  requires valid_format_c<formatter_t<version_v, T>, version_v, T>
struct default_formatter_slector_helper_t<version_v, T, 0> {
  using type = formatter_t<version_v, T>;
};
template <version_t version_v, typename T, uint32_t priority>
  requires valid_format_c<default_formatter_t<version_v, T, priority>,
                          version_v, T>
struct default_formatter_slector_helper_t<version_v, T, priority> {
  using type = default_formatter_t<version_v, T, priority>;
};
template <version_t version_v>
struct parse_and_format_data_t;

template <version_t version_v>
union raw_storage_ref_u {
   char dummy_{};
  const void *runtime_only_raw_ptr;
  const void_struct_t *compile_time_only_raw_ptr;
  // bit_cast-able and small
  char raw_val[sizeof(void *)];
};

template <version_t version_v>
using parse_and_format_fn_t =
    success_t (*)(typeless_arg_ref_t<version_v> storage,
                  parse_and_format_data_t<version_v> &fn_data,
                  cx_formatter_storage_ref_t<version_v> cx_store) noexcept;

template <version_t version_v>
struct argument_name_t {
  hash_bytes_t<version_v> hash{};
  basic_string_view_t<version_v> name_str{};
};

// configurable
template <version_t version_v>
static const constexpr uintlen_t format_basic_buffer_size_v = 64;
// configurable
template <version_t version_v>
static const constexpr uintlen_t format_stack_size_v = 128 * sizeof(uintlen_t);

template <version_t version_v>
struct alignas(
    allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>::align)
    base_context_t : void_struct_t {
  using stack_alloc_t =
      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>;
  static_assert(stack_alloc_t::align * 2 <=
                format_basic_buffer_size_v<version_v>);
  // alignment
  static_assert(
      !((format_stack_size_v<version_v> |
         format_basic_buffer_size_v<version_v>)&(stack_alloc_t::align - 1)));

  template <class>
  friend class mjz_private_accessed_t;
  template <version_t>
  friend struct parse_and_format_data_t;

  using out_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;
  using name_t = argument_name_t<version_v>;

 public:
  // first 8 words
  uintlen_t remaining_format_string_index{};                      // 1
  uintlen_t next_arg_index{/*uintlen_t(-1) means manual mode*/};  // 1
  uintlen_t number_of_args{};                                     // 1
  out_it_t output_it{};                                           // 3w
  view_t format_string{};                                         // 2w
  //---//
  // second 8 words
  const cx_formatter_storage_base_ref_t<version_v>
      *cx_parse_storage_of_args{};                 // 1w
  uintlen_t number_of_cx_parse_storage_of_args{};  // 1w
  stack_base_out_buffer_t<version_v> buf_view{};   // 6w
  //---//
  // third 8 words
  alloc_ref_t alloc{};                                                    // 1w
  const raw_storage_ref_u<version_v> *data_of_args{};                     // 1w
  const parse_and_format_fn_t<version_v> *parse_and_format_fn_of_args{};  // 1w
  const name_t *name_ptr{};                                               // 1w
  stack_alloc_t stack_alloc{};                                            // 2w
  std::span<char> format_cache_ref{};                                     // 2w
  //---//
  // forth 8 words
  uintlen_t err_index{};                                      // 1w
  static_string_view_t<version_v> err_content_bfr_{nullopt};  // 2w
  view_t err_content{};                                       // 2w
  out_it_t err_output{};                                      // 3w
                                                              //---//

 public:
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN std::span<char> fn_alloca(uintlen_t min_size,
                                      uintlen_t align) noexcept {
    allocs_ns::block_info_t<version_v> blk =
        alloc.alloca_bytes(stack_alloc, min_size, align);
    return {blk.ptr, blk.length};
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN void fn_dealloca(std::span<char> &&blk, uintlen_t align) noexcept {
    return alloc.dealloca_bytes(
        stack_alloc, allocs_ns::block_info_t<version_v>{blk.data(), blk.size()},
        align);
  }

  struct parse_and_format_call_one_at_non_virt_impl_t_ {
    typeless_arg_ref_t<version_v> obj{};
    parse_and_format_data_t<version_v> *This{};
    cx_formatter_storage_ref_t<version_v> cx_parse{};
    success_t ret{};
  };
  MJZ_CX_FN static success_t parse_and_format_call_at_virt_impl_(
      parse_and_format_call_one_at_non_virt_impl_t_ &ret) noexcept {
    return (*ret.obj.parse_and_format)(ret.obj, *ret.This, ret.cx_parse);
  }
  MJZ_CX_FN bool parse_only() const noexcept {
    MJZ_IFN_CONSTEVAL { return false; }
    return intlen_t(err_index) < 0;
  } 
  MJZ_CX_FN success_t parse_and_format_call_at(uintlen_t index,bool find_names) noexcept {
    parse_and_format_call_one_at_non_virt_impl_t_ ret0{};
    ret0.This = &main_ctx();
    auto args = data_of_args;
    typeless_arg_ref_t<version_v> &obj = ret0.obj;
    obj.m = args ? args[index] : raw_storage_ref_u<version_v>{};
    obj.parse_and_format = parse_and_format_fn_of_args[index];

    if (this->cx_parse_storage_of_args &&! find_names) {
      const uintlen_t arg_i{uintlen_t(-1) - base_ctx().err_index};

      const cx_formatter_storage_base_ref_t<version_v> &fs_cref =
          base_ctx().cx_parse_storage_of_args[arg_i];
      if (parse_only()) {
        cx_formatter_storage_base_ref_t<version_v> &fs_ref =
            const_cast<cx_formatter_storage_base_ref_t<version_v> &>(fs_cref);
        fs_ref.index_of_element = index;
      }
      ret0.cx_parse = fs_cref.formatter_ptr;
    } 
   
    if (parse_and_format_call_at_non_virt_impl_(
            ret0, alias_t<basic_formatted_types_t<version_v> *>{}))
      return ret0.ret;

    return parse_and_format_call_at_virt_impl_(ret0);
  }

  MJZ_CX_FN success_t cache_format_call_at(uintlen_t index,
      const cx_formatter_storage_base_t<version_v> *formatter_ptr) noexcept {
    parse_and_format_call_one_at_non_virt_impl_t_ ret0{};
    ret0.This = &main_ctx();
    auto args = data_of_args;
    typeless_arg_ref_t<version_v> &obj = ret0.obj;
    obj.m = args ? args[index] : raw_storage_ref_u<version_v>{};
    obj.parse_and_format = parse_and_format_fn_of_args[index];
    ret0.cx_parse = formatter_ptr;
    if (parse_and_format_call_at_non_virt_impl_(
            ret0, alias_t<basic_formatted_types_t<version_v> *>{}))
      return ret0.ret;
    return parse_and_format_call_at_virt_impl_(ret0);
  }
  template <typename T>
  MJZ_CX_FN static bool parse_and_format_call_one_at_non_virt_impl_(
      parse_and_format_call_one_at_non_virt_impl_t_ &ret) noexcept;
  template <typename... Ts>
  MJZ_CX_FN static bool parse_and_format_call_at_non_virt_impl_(
      parse_and_format_call_one_at_non_virt_impl_t_ &ret,
      void (*)(Ts...)) noexcept {
    return ((parse_and_format_call_one_at_non_virt_impl_<const Ts &>(ret)) ||
            ...);
  }
  MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx() const noexcept;
  MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept;

  MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept;
  MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept;

  MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept;
  MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept;

  MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept;
  MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept;

  MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept;
  MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept;

 private:
  MJZ_NO_MV_NO_CPY(base_context_t);
  MJZ_CX_FN base_context_t() noexcept = default;
};

template <version_t version_v>
struct parse_context_t {
  using stack_alloc_t =
      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>;
  template <class>
  friend class mjz_private_accessed_t;

  template <version_t, typename...>
  friend struct cx_parser_t;
  template <version_t>
  friend struct formatting_object_t;
  template <version_t>
  friend struct parse_and_format_data_t;

  template <version_t>
  friend struct hash_context_t;

 private:
  using err_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;

  MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx() const noexcept;
  MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept;

  MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept;
  MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept;

  MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept;
  MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept;

  MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept;
  MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept;
  MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept;
  MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept;

 private:
  MJZ_CX_FN explicit parse_context_t() noexcept {}
  MJZ_NO_MV_NO_CPY(parse_context_t);

  using const_iterator = typename view_t::const_iterator;
  using iterator = const_iterator;
  using char_type = char;

 private:
  MJZ_CX_FN void erred_cx() const noexcept {
#if MJZ_VERBOSE_FORMAT_ERROR
#else
    MJZ_IF_CONSTEVAL {
      if (main_ctx().err_index) {
        // look at above functions
        std::ignore = *reinterpret_cast<const volatile uint8_t *>(
            &"look at the function above");
      }
    }

#endif
  }
  MJZ_CX_NL_FN void as_error_impl_() noexcept {
    static_string_view_t<version_v> error_description =
        main_ctx().err_content_bfr_;
    erred_cx();
    main_ctx().err_index =
        uintlen_t(data_left() - main_ctx().format_string.data());
    if (!error_description) error_description = "[Error]";
    main_ctx().err_content = error_description;
    main_ctx().remaining_format_string_index =
        main_ctx().format_string.length();
  }

 public:
  MJZ_CX_FN explicit operator bool() const noexcept {
    return !main_ctx().err_content;
  }

  MJZ_CX_FN err_it_t
  as_error(static_string_view_t<version_v> error_description) noexcept {
    main_ctx().err_content_bfr_ = error_description;
    as_error_impl_();
    return main_ctx().err_output;
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN std::span<char> fn_alloca(uintlen_t min_size,
                                      uintlen_t align) noexcept {
    return base_ctx().fn_alloca(min_size, align);
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN void fn_dealloca(std::span<char> &&blk, uintlen_t align) noexcept {
    return base_ctx().fn_dealloca(std::move(blk), align);
  }
  MJZ_CX_FN encodings_e encoding() const noexcept {
    return main_ctx().format_string.get_encoding();
  }
  MJZ_CX_FN const allocs_ns::alloc_base_ref_t<version_v> &allocator()
      const noexcept {
    return main_ctx().alloc;
  }
  MJZ_CX_FN view_t view() const noexcept {
    asserts(asserts.assume_rn, main_ctx().remaining_format_string_index <=
                                   main_ctx().format_string.length());
    return main_ctx().format_string(main_ctx().remaining_format_string_index);
  }
  MJZ_CX_FN uintlen_t length_left() const noexcept {
    return main_ctx().format_string.length() -
           main_ctx().remaining_format_string_index;
  }
  MJZ_CX_FN const char *data_left() const noexcept {
    return main_ctx().format_string.data() +
           main_ctx().remaining_format_string_index;
  }
  MJZ_CX_FN const_iterator begin() const noexcept {
    return main_ctx().format_string.begin() +
           intlen_t(main_ctx().remaining_format_string_index);
  }
  MJZ_CX_FN std::optional<char> front() const noexcept {
    if (length_left()) return *data_left();
    return nullopt;
  }
  MJZ_CX_FN std::optional<char> at(uintlen_t index) const noexcept {
    return main_ctx().format_string.at(
        index + main_ctx().remaining_format_string_index);
  }
  MJZ_CX_FN std::optional<char> at(
      std::optional<uintlen_t> index) const noexcept {
    return index ? main_ctx().format_string.at(
                       *index + main_ctx().remaining_format_string_index)
                 : nullopt;
  }
  MJZ_CX_FN const_iterator end() const noexcept {
    return main_ctx().format_string.end();
  }
  MJZ_CX_FN success_t advance_amount(std::optional<uintlen_t> amount) noexcept {
    if (!amount || length_left() < amount) {
      as_error(
          "[Error]parse_context_t::advance_amount(std::optional<uintlen_t>): "
          "couldn't "
          "advance ,(maybe parse funtion failed)");
      return false;
    }
    main_ctx().remaining_format_string_index += *amount;
    return true;
  }
  MJZ_CX_FN success_t unchecked_advance_amount_(uintlen_t amount) noexcept {
    main_ctx().remaining_format_string_index += amount;
    return true;
  }
  MJZ_CX_FN success_t advance_amount(uintlen_t amount) noexcept {
    if (length_left() < amount) {
      as_error(
          "[Error]parse_context_t::advance_amount(uintlen_t): "
          "couldn't "
          "advance ,(maybe parse funtion failed)");
      return false;
    }
    main_ctx().remaining_format_string_index += amount;
    return true;
  }
  MJZ_CX_FN success_t advance_to(const_iterator it) noexcept {
    if (!it || /*happend on usr ignoring empty returns that are errors ->*/ it <
                   begin()) {
      as_error(
          "[Error]parse_context_t::advance_to(const_iterator): couldn't "
          "advance "
          "to it,(maybe parse funtion failed)");
      return false;
    }
    unchecked_advance_amount_(uintlen_t(it - begin()));
    return true;
  }
  MJZ_CX_FN std::optional<uintlen_t> next_arg_id() noexcept {
    if (main_ctx().next_arg_index == uintlen_t(-1)) {
      as_error(
          "[Error]parse_context_t::next_arg_id( ): couldn't enter automatic "
          "index mode from manual mode");
      return nullopt;
    }
    if (main_ctx().number_of_args <= main_ctx().next_arg_index) {
      as_error(
          "[Error]parse_context_t::next_arg_id( ): expected an argument but "
          "had none (id out of range)");
      return nullopt;
    }
    return main_ctx().next_arg_index++;
  }
  MJZ_CX_FN success_t check_arg_id(uintlen_t id) noexcept {
    if (main_ctx().next_arg_index &&
        main_ctx().next_arg_index != uintlen_t(-1)) {
      as_error(
          "[Error]parse_context_t::check_arg_id(uintlen_t): couldn't enter "
          "manual "
          "index mode from  automatic mode");
      return false;
    }
    if (main_ctx().number_of_args <= id) {
      as_error(
          "[Error]parse_context_t::check_arg_id(uintlen_t): expected an "
          "argument but had "
          "none (id out of range)");
      return false;
    }
    main_ctx().next_arg_index = uintlen_t(-1);
    return true;
  }
  MJZ_CX_FN typeid_arg_ref_t<version_v> type_v(uintlen_t id) noexcept;

  MJZ_CX_FN std::optional<uintlen_t> parse_starting_ulen() noexcept;

  MJZ_CX_FN std::optional<uintlen_t> parse_arg_id() noexcept;
  MJZ_CX_FN bool parse_only() const noexcept { return main_ctx().parse_only(); }

  MJZ_CX_FN std::optional<uintlen_t> get_numeric(uintlen_t defult) noexcept;
  MJZ_CX_FN std::optional<std::pair<uintlen_t /*index*/, uintlen_t /*length*/>>
  get_slice() noexcept;

 private:
  MJZ_CX_FN std::optional<hash_bytes_t<version_v>>
  parse_starting_name() noexcept;
  MJZ_CX_FN std::optional<uintlen_t> parse_arg_index() noexcept;
  MJZ_CX_FN std::optional<uintlen_t> find_name_index(
      hash_bytes_t<version_v> name, view_t name_str) noexcept;
  template <version_t>
  friend struct format_context_t;
};
template <version_t version_v, typename T>
using formatter_type_t = std::conditional_t<
    valid_format_c<
        typename default_formatter_slector_helper_t<version_v, const T>::type,
        version_v, const T>,
    typename default_formatter_slector_helper_t<version_v, const T>::type,
    typename default_formatter_slector_helper_t<version_v, T>::type>;
template <typename T, version_t version_v>
concept is_formatted_exact_c =
    valid_format_c<formatter_type_t<version_v, T>, version_v, T> ||
    valid_format_c<formatter_type_t<version_v, const T>, version_v, const T> ||
    valid_format_c<formatter_type_t<version_v, const T &>, version_v,
                   const T &>;

template <version_t version_v>
struct format_context_t {
  using stack_alloc_t =
      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>;
  template <class>
  friend class mjz_private_accessed_t;
  template <version_t>
  friend struct parse_and_format_data_t;

  template <version_t>
  friend struct hash_context_t;
  template <version_t>
  friend struct formatting_object_t;
  template <version_t, typename...>
  friend struct cx_parser_t;

 private:
  using out_it_t = base_out_it_t<version_v>;
  using err_it_t = base_out_it_t<version_v>;

  MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx() const noexcept;
  MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept;
  MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept;
  MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept;

  MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept;
  MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept;

  MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept;
  MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept;

  MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept;
  MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept;

 private:
  /*
   *parse-only
   */
  MJZ_CX_FN explicit format_context_t() noexcept {}
  MJZ_NO_MV_NO_CPY(format_context_t);

 public:
  using iterator = out_it_t;
  using char_type = char;
  template <typename T>
  using formatter_type = formatter_type_t<version_v, T>;

 public:
  MJZ_CX_FN basic_arg_ref_t<version_v> arg(uintlen_t id) noexcept;
  MJZ_CX_FN encodings_e encoding() const noexcept {
    return parse_ctx().encoding();
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */

  MJZ_CX_FN std::span<char> fn_alloca(uintlen_t min_size,
                                      uintlen_t align) noexcept {
    return parse_ctx().fn_alloca(min_size, align);
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */

  MJZ_CX_FN void fn_dealloca(std::span<char> &&blk, uintlen_t align) noexcept {
    return parse_ctx().fn_dealloca(std::move(blk), align);
  }
  MJZ_CX_FN const allocs_ns::alloc_base_ref_t<version_v> &allocator()
      const noexcept {
    return parse_ctx().allocator();
  }
  MJZ_CX_FN iterator out() noexcept { return main_ctx().output_it; }

  MJZ_CX_FN success_t advance_to(iterator it) noexcept {
    if (!it || !(main_ctx().output_it.operator=(std::move(it))) ||
        !main_ctx().output_it.entangle_to_manual_buffer(main_ctx().buf_view))
      MJZ_IS_UNLIKELY {
        as_error(
            "[Error]format_context_t::advance_to(const_iterator): couldn't "
            "advance "
            "to nullptr,(maybe format funtion failed)(maybe output iterator "
            "failed)");
        return false;
      };
    return true;
  }
  MJZ_CX_FN err_it_t
  as_error(static_string_view_t<version_v> error_description) noexcept {
    MJZ_RELEASE { main_ctx().output_it = out_it_t{}; };
    return parse_ctx().as_error(error_description);
  }
  MJZ_CX_FN explicit operator bool() const noexcept { return !!parse_ctx(); }
};

template <version_t version_v>
struct hash_context_t {
  using stack_alloc_t =
      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>;
  template <class>
  friend class mjz_private_accessed_t;

  template <version_t, typename...>
  friend struct cx_parser_t;
  template <version_t>
  friend struct formatting_object_t;
  template <version_t>
  friend struct format_context_t;
  template <version_t>
  friend struct parse_and_format_data_t;

 private:
  using err_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;

  MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx() const noexcept;
  MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept;
  MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept;
  MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept;

  MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept;
  MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept;

  MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept;
  MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept;

  MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept;
  MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept;

 private:
  MJZ_CX_FN explicit hash_context_t() noexcept {}
  MJZ_NO_MV_NO_CPY(hash_context_t);

  using const_iterator = typename view_t::const_iterator;
  using iterator = const_iterator;
  using char_type = char;

 public:
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN std::span<char> fn_alloca(uintlen_t min_size,
                                      uintlen_t align) noexcept {
    return parse_ctx().fn_alloca(min_size, align);
  }
  /*CATION !!!!!!!!!!!!!
   *WILL LEAD TO UB IF THE STACK IS MISUSED,
   * USE THE FOLLOWING TO ENSURE SAFE USE
   * blk=alloca_bytes(...);
   * MJZ_RELEASE{dealloca_bytes(std::move(blk));};
   * ...
   * CODE THAT DOSE NOT TRANSFER OWNERSHIP OF blk
   * ...
   */
  MJZ_CX_FN void fn_dealloca(std::span<char> &&blk, uintlen_t align) noexcept {
    return parse_ctx().fn_dealloca(std::move(blk), align);
  }
  MJZ_CX_FN encodings_e encoding() const noexcept {
    return parse_ctx().encoding();
  }
  MJZ_CX_FN const allocs_ns::alloc_base_ref_t<version_v> &allocator()
      const noexcept {
    return parse_ctx().allocator();
  }
  MJZ_CX_FN err_it_t
  as_error(static_string_view_t<version_v> error_description) noexcept {
    return parse_ctx().as_error(error_description);
  }
  MJZ_CX_FN optional_ref_t<typename base_context_t<version_v>::name_t const>
  name() const noexcept {
    return main_ctx().name_ptr;
  }
  MJZ_CX_FN success_t matched() noexcept {
    if (!parse_ctx()) return false;
    main_ctx().name_ptr = nullptr;
    return true;
  }
};
template <version_t version_v, typename Formatter_t>
struct MJZ_MSVC_ONLY_CODE_(__declspec(empty_bases)) cx_formatter_storage_t
    : cx_formatter_storage_base_t<version_v>,
      Formatter_t {};

template <version_t version_v, typename formatter_type_T_>
concept can_have_cx_formatter_F_c_ = requires() {
  { formatter_type_T_::can_have_cx_formatter_v } -> std::convertible_to<bool>;
  requires(!!formatter_type_T_::can_have_cx_formatter_v);
};

template <version_t version_v, typename Formatter_t>
using optional_cx_formatter_storage_F_t =
    std::conditional_t<!!can_have_cx_formatter_F_c_<version_v, Formatter_t>,
                       cx_formatter_storage_t<version_v, Formatter_t>,
                       cx_formatter_storage_base_t<version_v>>;

template <version_t version_v>
/* this is the only valid parser base type */
struct MJZ_MSVC_ONLY_CODE_(__declspec(empty_bases)) parse_and_format_data_t
    : base_context_t<version_v>,
      parse_context_t<version_v>,
      format_context_t<version_v>,
      hash_context_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;
  template <version_t, typename...>
  friend struct cx_parser_t;
  MJZ_NO_MV_NO_CPY(parse_and_format_data_t);
  using out_it_t = base_out_it_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using sview_t = static_string_view_t<version_v>;
  using alloc_ref_t = allocs_ns::alloc_base_ref_t<version_v>;

 public:
  MJZ_CX_FN const parse_and_format_data_t<version_v> &main_ctx()
      const noexcept {
    return *this;
  }
  MJZ_CX_FN parse_and_format_data_t<version_v> &main_ctx() noexcept {
    return *this;
  }
  MJZ_CX_FN const parse_context_t<version_v> &parse_ctx() const noexcept {
    return *this;
  }
  MJZ_CX_FN parse_context_t<version_v> &parse_ctx() noexcept { return *this; }

  MJZ_CX_FN const hash_context_t<version_v> &hash_ctx() const noexcept {
    return *this;
  }
  MJZ_CX_FN hash_context_t<version_v> &hash_ctx() noexcept { return *this; }

  MJZ_CX_FN const format_context_t<version_v> &format_ctx() const noexcept {
    return *this;
  }
  MJZ_CX_FN format_context_t<version_v> &format_ctx() noexcept { return *this; }

  MJZ_CX_FN const base_context_t<version_v> &base_ctx() const noexcept {
    return *this;
  }
  MJZ_CX_FN base_context_t<version_v> &base_ctx() noexcept { return *this; }

 public:
  MJZ_CX_FN explicit parse_and_format_data_t(
      out_it_t output_it_ = nullptr) noexcept {
    this->output_it = std::move(output_it_);
  }

 private:
  template <version_t, typename>
  friend struct the_typed_arg_ref_t;

  template <typename T_, typename T>
  MJZ_CX_FN success_t
  parse_and_format(optional_ref_t<T_> arg, void(T),
                   cx_formatter_storage_ref_t<version_v> cxref) noexcept {
    using F_t =
        typename format_context_t<version_v>::template formatter_type<T>;
    constexpr bool has_formatter = valid_format_c<F_t, version_v, T>;
    static_assert(valid_format_c<F_t, version_v, T>,
                  " the formatter is not defined for this type!");
    if constexpr (has_formatter) {
      using formatter_obj_t_ = cx_formatter_storage_t<version_v, F_t>;
      if (this->name_ptr) {
        return hash_impl_<T_, T, F_t>(arg);
      }
      if (parse_only()) {
        return parse_only_impl_<F_t, formatter_obj_t_>(cxref);
      }
      if (!arg) {
        return true;
      }
      if constexpr (can_have_cx_formatter_F_c_<version_v, F_t>) {
        if (cxref.get()) {
          return format_impl_<T_, T, F_t>(
              *arg, *static_cast<const formatter_obj_t_ *>(cxref.get()));
        }
      }

      return parse_and_format_impl_<T_, T, F_t>(*arg);
    }
    return false;
  }

  template <typename T_, typename T>
  MJZ_CX_NL_FN success_t
  parse_and_format_nl_(optional_ref_t<T_> arg, void(T),
                       cx_formatter_storage_ref_t<version_v> cxref) noexcept {
    return parse_and_format(arg, alias_t<void (*)(T)>{}, cxref);
  }
  template <class F_t, class formatter_obj_t_>
  MJZ_CX_NL_FN success_t
  parse_only_impl_(cx_formatter_storage_ref_t<version_v> cxref) noexcept {
    formatter_obj_t_ *ptr{};
    std::optional<formatter_obj_t_> may_store{};
    if constexpr (!can_have_cx_formatter_F_c_<version_v, F_t>) {
      may_store.emplace();
      ptr = &*may_store;
    } else if (!cxref) {
      may_store.emplace();
      ptr = &*may_store;
    } else {
      ptr = const_cast<formatter_obj_t_ *>(
          static_cast<const formatter_obj_t_ *>(cxref.get()));
    }

    base_ctx().err_index += uintlen_t(-1); 
    std::optional<uintlen_t> distance_ = parse_impl_<F_t>(*ptr);
    if (!distance_) {
      return false;
    }

    return true;
  }

  template <typename T_, typename T, class F_t>
  MJZ_CX_FN success_t hash_impl_(optional_ref_t<T_> arg) noexcept {
    if constexpr (sname_arg_c<F_t, T, version_v>) {
      return F_t::arg_name(*this);
    } else if constexpr (named_arg_c<F_t, T, version_v>) {
      return F_t::arg_name(std::as_const(*arg), *this);
    } else {
      return true;
    }
  }

  template <typename T_, typename T, class F_t>
  MJZ_CX_FN success_t parse_and_format_impl_(T_ &arg) noexcept {
    F_t formatter{};
    if (!parse_impl_<F_t>(formatter)) {
      return false;
    }
    return format_impl_<T_, T, F_t>(arg, formatter);
  }

  template <class F_t>
  MJZ_CX_FN std::optional<uintlen_t> parse_impl_(F_t &formatter) noexcept {
    const auto past_it = parse_ctx().begin();
    if (!parse_ctx().advance_to(
            formatter.parse((parse_context_t<version_v> &)(parse_ctx()))))
      return {};
    return uintlen_t(parse_ctx().begin() - past_it);
  }

  template <typename T_, typename T, class F_t>
  MJZ_CX_FN success_t format_impl_(T_ &arg, const F_t &formatter) noexcept {
    return format_ctx().advance_to(std::as_const(formatter).format(
        std::forward<T>(arg), (format_context_t<version_v> &)(format_ctx())));
  }

 public:
  MJZ_CX_FN success_t parse_format_replacement_field() noexcept;
  MJZ_CX_FN success_t parse_formating_string() noexcept;
  MJZ_CX_FN success_t append_text(view_t) noexcept;
  MJZ_CX_FN std::optional<std::pair<uintlen_t /*index*/, uintlen_t /*length*/>>
  get_slice_parse_filter(bool check_after_slice = true) noexcept;
  MJZ_CX_FN std::optional<uintlen_t> get_parse_filter_numeric(
      uintlen_t defult) noexcept;
  MJZ_CX_FN success_t call_argument_formatter(uintlen_t id) noexcept;
  MJZ_CX_FN bool parse_only() const noexcept { return base_ctx().parse_only(); }
};

template <version_t version_v>
struct typeless_arg_ref_t : void_struct_t {
  /*
   *note that in parse only calls ,the active member of the m member is null
   *initilized and call of visit.. is UB!
   */
  raw_storage_ref_u<version_v> m{};
  parse_and_format_fn_t<version_v> parse_and_format{};
};
template <version_t version_v>
struct basic_arg_ref_t : protected typeless_arg_ref_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;

 protected:
  using base_t = typeless_arg_ref_t<version_v>;
  using base_t::m;
  using base_t::parse_and_format;

 public:
  MJZ_CX_FN basic_arg_ref_t() noexcept : base_t{} {}
  MJZ_CX_FN explicit basic_arg_ref_t(base_t ref) noexcept : base_t{ref} {}
  MJZ_CX_FN explicit operator bool() const noexcept {
    return !!parse_and_format;
  }

 public:
  template <class... Ts>
  MJZ_CX_FN bool is_one_of() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xc() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xv() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xr() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xcv() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xcr() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xvr() const noexcept;
  template <class... Ts>
  MJZ_CX_FN bool is_one_of_xcvr() const noexcept;

  template <typename... Ts>
  MJZ_CX_FN success_t visit(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xv(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xc(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xr(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xvr(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xcv(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xcr(auto &&noexcept_with_void_return_fn) noexcept;
  template <typename... Ts>
  MJZ_CX_FN success_t visit_xcvr(auto &&noexcept_with_void_return_fn) noexcept;
};
template <version_t version_v>
struct typeid_arg_ref_t : protected basic_arg_ref_t<version_v> {
  template <class>
  friend class mjz_private_accessed_t;
  using base_t = basic_arg_ref_t<version_v>;
  using bbase_t = typeless_arg_ref_t<version_v>;
  MJZ_CX_FN typeid_arg_ref_t() noexcept : base_t{} {}
  MJZ_CX_FN explicit typeid_arg_ref_t(bbase_t ref) noexcept : base_t{ref} {}
  using base_t::operator bool;
  using base_t::is_one_of;
  using base_t::is_one_of_xc;
  using base_t::is_one_of_xcr;
  using base_t::is_one_of_xcv;
  using base_t::is_one_of_xcvr;
  using base_t::is_one_of_xr;
  using base_t::is_one_of_xv;
  using base_t::is_one_of_xvr;
};
/*
 *this NEEDS to be alive at compile time format calls
 * but is discarded for runtime ones
 */
template <version_t version_v, typename type_t>
struct the_typed_arg_ref_t : void_struct_t {
  template <class>
  friend class mjz_private_accessed_t;
  using Type_t = type_t;
  using typeless_ref = typeless_arg_ref_t<version_v>;
  using T = std::remove_reference_t<type_t>;
  MJZ_CX_FN static success_t parse_and_format_outer_fn(
      typeless_arg_ref_t<version_v> storage,
      parse_and_format_data_t<version_v> &fn_data,
      cx_formatter_storage_ref_t<version_v> cxref) noexcept {
    if (fn_data.parse_only()) {
      return fn_data.parse_and_format_nl_(
          optional_ref_t<std::remove_reference_t<type_t>>{},
          alias_t<void (*)(type_t)>{}, cxref);
    }
    auto &&ref_v = get(storage);
    auto &&ref = *ref_v;
    return fn_data.parse_and_format(
        optional_ref_t<std::remove_reference_t<type_t>>{ref},
        alias_t<void (*)(type_t)>{}, cxref);
  }
  MJZ_CX_FN typeless_ref operator+() & noexcept {
    typeless_ref ref{};
    ref.parse_and_format = &parse_and_format_outer_fn;
    MJZ_IF_CONSTEVAL { ref.m.compile_time_only_raw_ptr = this; }
    else {
      ref.m.runtime_only_raw_ptr = ptr;
    }
    return ref;
  }

  MJZ_CX_FN typeless_ref operator-() noexcept {
    MJZ_IF_CONSTEVAL { asserts(false); }
    return +*this;
  }
  MJZ_CX_FN static optional_ref_t<T> get(typeless_ref ref) noexcept {
    if (ref.parse_and_format != &parse_and_format_outer_fn) return nullopt;
    MJZ_IF_CONSTEVAL {
      return static_cast<const the_typed_arg_ref_t *>(
                 ref.m.compile_time_only_raw_ptr)
          ->ptr;
    }
    else {
      return const_cast<T *>(
          static_cast<const T *>(ref.m.runtime_only_raw_ptr));
    }
  }
  MJZ_CX_FN the_typed_arg_ref_t(T &arg) noexcept : ptr(&arg) {}
  MJZ_NO_MV_NO_CPY(the_typed_arg_ref_t);

 private:
  T *ptr{};
};

template <typename T, version_t version_v>
concept bit_cast_able_and_small_c =
    requires() {
      {
        format_context_t<version_v>::template formatter_type<
            T>::can_bitcast_optimize_v
      } -> std::convertible_to<bool>;
      requires(!!format_context_t<version_v>::template formatter_type<
                T>::can_bitcast_optimize_v);
    } && is_totaly_trivial_after_init<T> && !std::is_array_v<T> &&
    sizeof(T) <= sizeof(void *) && alignof(T) <= alignof(void *) &&
    requires(T obj, std::array<char, sizeof(T)> buf) {
      std::bit_cast<std::array<char, sizeof(T)>>(obj);
      std::bit_cast<T>(buf);
    };

template <typename T, version_t version_v>
concept can_have_cx_formatter_c = can_have_cx_formatter_F_c_<
    version_v,
    typename format_context_t<version_v>::template formatter_type<T>>;

template <version_t version_v, typename T>
using optional_cx_formatter_storage_t = optional_cx_formatter_storage_F_t<
    version_v,
    typename format_context_t<version_v>::template formatter_type<T>>;
template <version_t version_v, typename... Ts>
using cx_formatter_storages_t =
    std::tuple<optional_cx_formatter_storage_t<version_v, Ts>...>;
template <typename T, version_t version_v>
concept no_perfect_forwarding_c = requires() {
  {
    format_context_t<version_v>::template formatter_type<
        T>::no_perfect_forwarding_v
  } -> std::convertible_to<bool>;
  requires(!!format_context_t<version_v>::template formatter_type<
            T>::no_perfect_forwarding_v);
};
template <version_t version_v, typename type_t>
  requires(bit_cast_able_and_small_c<std::remove_cvref_t<type_t>, version_v> &&
           !std::is_volatile_v<type_t>)
struct the_typed_arg_ref_t<version_v, type_t> : void_struct_t {
  template <class>
  friend class mjz_private_accessed_t;
  using Type_t = type_t;
  using typeless_ref = typeless_arg_ref_t<version_v>;
  using T = std::remove_cvref_t<type_t>;
  MJZ_CX_FN static success_t parse_and_format_outer_fn(
      typeless_arg_ref_t<version_v> storage,
      parse_and_format_data_t<version_v> &fn_data,
      cx_formatter_storage_ref_t<version_v> cxref) noexcept {
    if (fn_data.parse_only()) {
      return fn_data.parse_and_format_nl_(
          optional_ref_t<std::remove_reference_t<type_t>>{},
          alias_t<void (*)(type_t)>{}, cxref);
    } else {
      auto &&ref_v = get(storage);
      auto &&ref = *ref_v;
      return fn_data.parse_and_format(
          optional_ref_t<std::remove_reference_t<type_t>>{ref},
          alias_t<void (*)(type_t)>{}, cxref);
    }
  }
  MJZ_CX_FN typeless_ref operator+() & noexcept {
    typeless_ref ref{};
    ref.parse_and_format = &parse_and_format_outer_fn;
    ref.m.raw_val[0] = 0;
    memset(ref.m.raw_val, sizeof(ref.m.raw_val), 0);
    if constexpr (!std::is_empty_v<T> ||
                  !std::is_trivially_constructible_v<T>) {
      auto buf = std::bit_cast<std::array<char, sizeof(T)>>(val);
      for (uintlen_t i{}; i < sizeof(T); i++) ref.m.raw_val[i] = buf[(size_t)i];
    }
    return ref;
  }
  MJZ_CX_FN typeless_ref operator-() noexcept { return +*this; }

  MJZ_CX_FN static std::optional<T> get(typeless_ref ref) noexcept {
    if (ref.parse_and_format != &parse_and_format_outer_fn ||
        sizeof(ref.m.raw_val) < sizeof(T) ||
        !bit_cast_able_and_small_c<T, version_v>)
      return nullopt;
    if constexpr (!std::is_empty_v<T> ||
                  !std::is_trivially_constructible_v<T>) {
      std::array<char, sizeof(T)> buf{};
      for (uintlen_t i{}; i < sizeof(T); i++) buf[(size_t)i] = ref.m.raw_val[i];
      return std::bit_cast<T>(buf);
    } else {
      return T{};
    }
  }
  MJZ_CX_FN the_typed_arg_ref_t(T arg) noexcept : val(arg) {}

 private:
  T val;
};
struct basic_format_args_cx_tag {};
struct basic_format_args_ncx_tag {};
struct basic_format_args_parse_tag {};

template <typename T, version_t version_v>
using formatter_type_helper_t =
    format_context_t<version_v>::template formatter_type<T>;
template <typename T, version_t version_v>
concept decay_optimize_to_c = requires(T obj) {
  typename formatter_type_helper_t<T, version_v>::decay_optimize_to_t;
};

template <version_t version_v, typename T>
struct typed_arg_ref_decay_type_helper_t {
  using type = T;
};
template <version_t version_v, typename T>
  requires decay_optimize_to_c<T, version_v>
struct typed_arg_ref_decay_type_helper_t<version_v, T> {
  using type =
      typename formatter_type_helper_t<T, version_v>::decay_optimize_to_t;
};

template <version_t version_v, typename T>
using typed_arg_ref_decay_type_t =
    typename typed_arg_ref_decay_type_helper_t<version_v, T>::type;
template <version_t version_v, typename T>
using typed_arg_ref_final_type_t_helper_t =
    std::conditional_t<no_perfect_forwarding_c<T, version_v>,
                       const std::remove_const_t<std::remove_reference_t<T>> &,
                       T>;
template <version_t version_v, typename T>
using typed_arg_ref_final_type_t = typed_arg_ref_final_type_t_helper_t<
    version_v,
    typed_arg_ref_decay_type_t<
        version_v, typed_arg_ref_final_type_t_helper_t<version_v, T>>>;
template <typename T, version_t version_v>
concept is_formatted_c =
    is_formatted_exact_c<typed_arg_ref_final_type_t<version_v, T>, version_v>;
template <version_t version_v, typename T>
using original_typed_arg_ref_t =
    the_typed_arg_ref_t<version_v,
                        typed_arg_ref_final_type_t_helper_t<version_v, T>>;
template <version_t version_v, typename T>
using final_typed_arg_ref_t =
    original_typed_arg_ref_t<version_v,
                             typed_arg_ref_final_type_t<version_v, T>>;

template <version_t version_v, typename T>
MJZ_CX_FN std::remove_reference_t<typed_arg_ref_final_type_t<version_v, T>>
to_final_type_fn(T &&arg) noexcept {
  return static_cast<
      std::remove_reference_t<typed_arg_ref_final_type_t<version_v, T>>>(
      std::forward<T>(arg));
}
template <version_t version_v, typename T>
  requires(partial_same_as<typed_arg_ref_final_type_t<version_v, T>, T>)
MJZ_CX_FN typed_arg_ref_final_type_t<version_v, T> to_final_type_fn(
    T &&arg) noexcept {
  return std::forward<T>(arg);
}

template <version_t version_v, uintlen_t num_of_args>
struct basic_format_args_t : void_struct_t {
  raw_storage_ref_u<version_v> data_of_args[num_of_args];
  const parse_and_format_fn_t<version_v> *parse_and_format_fn_of_args;
  MJZ_CONSTANT(uintlen_t) count_args { num_of_args };

  template <typename... Ts>
    requires(sizeof...(Ts) == num_of_args)
  struct func_storage_t {
    MJZ_DISABLE_WANINGS_START_;
    MJZ_CONSTANT(auto)
    functions{std::array{parse_and_format_fn_t<version_v>{
        &original_typed_arg_ref_t<version_v,
                                  Ts>::parse_and_format_outer_fn}...}};
    MJZ_DISABLE_WANINGS_END_;
  };

  template <std::size_t... I>
    requires(sizeof...(I) == num_of_args)
  MJZ_CX_FN basic_format_args_t(basic_format_args_cx_tag, auto &tuple,
                                std::index_sequence<I...>) noexcept
      : data_of_args{(+std::get<I>(tuple)).m...},
        parse_and_format_fn_of_args{} {
    parse_and_format_fn_of_args =
        &func_storage_t<typename std::remove_cvref_t<std::tuple_element_t<
            I, std::remove_cvref_t<decltype(tuple)>>>::Type_t...>::functions[0];
  }
  template <typename... Ts>
    requires(sizeof...(Ts) == num_of_args)
  MJZ_CX_FN basic_format_args_t(basic_format_args_ncx_tag,
                                Ts &&...args) noexcept
      : data_of_args{(-original_typed_arg_ref_t<version_v, Ts>(args)).m...},
        parse_and_format_fn_of_args{} {
    parse_and_format_fn_of_args = &func_storage_t<Ts...>::functions[0];
  }
  template <typename... Ts>
    requires(sizeof...(Ts) == num_of_args)
  MJZ_CX_FN basic_format_args_t(basic_format_args_parse_tag,
                                void (*)(Ts...)) noexcept
      : data_of_args{}, parse_and_format_fn_of_args{} {
    parse_and_format_fn_of_args = &func_storage_t<Ts...>::functions[0];
  }
};

template <version_t version_v, typename... Ts>
struct basic_cx_format_args_t : void_struct_t {
  MJZ_NO_MV_NO_CPY(basic_cx_format_args_t);
  std::tuple<original_typed_arg_ref_t<version_v, Ts>...> arg_typed;
  basic_format_args_t<version_v, sizeof...(Ts)> arg_no_typed;
  MJZ_CX_FN basic_cx_format_args_t(Ts &&...args) noexcept
      : arg_typed{args...},
        arg_no_typed{basic_format_args_cx_tag{}, arg_typed,
                     std::make_index_sequence<sizeof...(Ts)>()} {}
};

/*
 *note that BOTH type and version must EXACTLY match , even to cv and ref
 *qualifiers (and number of elements in array types, and everything in function
 *types)
 */
template <version_t version_v, typename T>
MJZ_CX_FN bool is_type_heper_pv(typeless_arg_ref_t<version_v> ref) noexcept {
  return ref.parse_and_format ==
         &final_typed_arg_ref_t<version_v, T>::parse_and_format_outer_fn;
}
template <version_t version_v, typename... Ts>
MJZ_CX_FN bool is_type_heper_pv_vt(
    const typeless_arg_ref_t<version_v> &ref) noexcept {
  MJZ_DISABLE_ALL_WANINGS_START_;
  return (is_type_heper_pv<version_v, Ts>(ref) || ...);
  MJZ_DISABLE_ALL_WANINGS_END_;
}
template <version_t version_v, class fn_t, typename... Ts>
  requires(callable_c<fn_t &, void(Ts) noexcept> && ...)
MJZ_CX_FN success_t visit_heper_pv(fn_t &noexcept_with_void_return_fn,
                                   typeless_arg_ref_t<version_v> ref) noexcept {
  return (([&]() noexcept -> success_t {
            if (!is_type_heper_pv<version_v, Ts>(ref)) return false;
            auto opt_ref = final_typed_arg_ref_t<version_v, Ts>::get(ref);
            if (!opt_ref) return false;
            auto &obj = *opt_ref;
            noexcept_with_void_return_fn(obj);
            return true;
          }()) ||
          ...);
}

template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of() const noexcept {
  if constexpr (sizeof...(Ts)) {
    return is_type_heper_pv_vt<version_v, Ts...>(
        static_cast<const typeless_arg_ref_t<version_v> &>(*this));
  } else {
    return false;
  }
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xc() const noexcept {
  return is_one_of<const std::remove_const_t<Ts>...>() ||
         is_one_of<std::remove_const_t<Ts>...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xv() const noexcept {
  return is_one_of<volatile std::remove_volatile_t<Ts>...>() ||
         is_one_of<std::remove_volatile_t<Ts>...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xr() const noexcept {
  return is_one_of<std::remove_reference_t<Ts>...>() ||
         is_one_of<std::remove_reference_t<Ts> &...>() ||
         is_one_of<std::remove_reference_t<Ts> &&...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xcv() const noexcept {
  return is_one_of_xv<const std::remove_const_t<Ts>...>() ||
         is_one_of_xv<std::remove_const_t<Ts>...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xcr() const noexcept {
  return is_one_of_xr<const std::remove_const_t<Ts>...>() ||
         is_one_of_xr<std::remove_const_t<Ts>...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xvr() const noexcept {
  return is_one_of_xr<volatile std::remove_volatile_t<Ts>...>() ||
         is_one_of_xr<std::remove_volatile_t<Ts>...>();
}
template <version_t version_v>
template <class... Ts>
MJZ_CX_FN bool basic_arg_ref_t<version_v>::is_one_of_xcvr() const noexcept {
  return is_one_of_xvr<const std::remove_const_t<Ts>...>() ||
         is_one_of_xvr<std::remove_const_t<Ts>...>();
}

template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit_heper_pv<version_v, decltype(noexcept_with_void_return_fn),
                        Ts...>(noexcept_with_void_return_fn,
                               static_cast<base_t &>(*this));
}

template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xv(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit<alias_t<std::remove_volatile_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit<alias_t<volatile std::remove_volatile_t<Ts>>...>(
             noexcept_with_void_return_fn);
}
template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xc(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit<alias_t<std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit<alias_t<const std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn);
}
template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xr(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit<alias_t<std::remove_reference_t<Ts> &>...>(
             noexcept_with_void_return_fn) ||
         visit<alias_t<std::remove_reference_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit<alias_t<std::remove_reference_t<Ts> &&>...>(
             noexcept_with_void_return_fn);
}

template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xvr(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit_xr<alias_t<std::remove_volatile_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit_xr<alias_t<volatile std::remove_volatile_t<Ts>>...>(
             noexcept_with_void_return_fn);
}
template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xcv(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit_xv<alias_t<std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit_xv<alias_t<const std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn);
}
template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xcr(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit_xr<alias_t<std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit_xr<alias_t<const std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn);
}
template <version_t version_v>
template <typename... Ts>
MJZ_CX_FN success_t basic_arg_ref_t<version_v>::visit_xcvr(
    auto &&noexcept_with_void_return_fn) noexcept {
  return visit_xvr<alias_t<std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn) ||
         visit_xvr<alias_t<const std::remove_const_t<Ts>>...>(
             noexcept_with_void_return_fn);
}

template <version_t version_v>
MJZ_CX_FN typeid_arg_ref_t<version_v> parse_context_t<version_v>::type_v(
    uintlen_t id) noexcept {
  if (main_ctx().number_of_args <= id) {
    as_error(
        "[Error]parse_context_t::type_v(uintlen_t): expected an "
        "argument but had "
        "none");
    return {};
  }
  auto fn = main_ctx().parse_and_format_fn_of_args[id];
  auto args = main_ctx().data_of_args;
  typeless_arg_ref_t<version_v> obj{};
  obj.m = args ? args[id] : raw_storage_ref_u<version_v>{};
  obj.parse_and_format = fn;
  return typeid_arg_ref_t<version_v>(obj);
}

template <version_t version_v>
MJZ_CX_FN basic_arg_ref_t<version_v> format_context_t<version_v>::arg(
    uintlen_t id) noexcept {
  if (main_ctx().number_of_args <= id) {
    as_error(
        "[Error]format_context_t::arg(uintlen_t): expected an "
        "argument but had "
        "none");
    return {};
  }
  auto fn = main_ctx().parse_and_format_fn_of_args[id];
  auto args = main_ctx().data_of_args;
  typeless_arg_ref_t<version_v> obj{};
  obj.m = args ? args[id] : raw_storage_ref_u<version_v>{};
  obj.parse_and_format = fn;
  return basic_arg_ref_t<version_v>(obj);
}
 
template <version_t version_v>
template <typename T>
MJZ_CX_FN bool
base_context_t<version_v>::parse_and_format_call_one_at_non_virt_impl_(
    parse_and_format_call_one_at_non_virt_impl_t_ &ret) noexcept {
  if constexpr (is_formatted_exact_c<T, version_v>) {
    if constexpr (partial_same_as<typed_arg_ref_decay_type_t<version_v, T>,
                                  T>) {
      if (ret.obj.parse_and_format !=
          &original_typed_arg_ref_t<version_v, T>::parse_and_format_outer_fn) {
        return false;
      }
      ret.ret =
          original_typed_arg_ref_t<version_v, T>::parse_and_format_outer_fn(
              ret.obj, *ret.This, ret.cx_parse);
      return true;
    }
  }
  return false;
}

template <version_t version_v>
MJZ_CX_FN const parse_and_format_data_t<version_v> &
base_context_t<version_v>::main_ctx() const noexcept {
  return static_cast<const parse_and_format_data_t<version_v> &>(*this);
}
template <version_t version_v>
MJZ_CX_FN parse_and_format_data_t<version_v> &
base_context_t<version_v>::main_ctx() noexcept {
  return static_cast<parse_and_format_data_t<version_v> &>(*this);
}

template <version_t version_v>
MJZ_CX_FN const parse_and_format_data_t<version_v> &

parse_context_t<version_v>::main_ctx() const noexcept {
  return static_cast<const parse_and_format_data_t<version_v> &>(*this);
}
template <version_t version_v>
MJZ_CX_FN parse_and_format_data_t<version_v> &

parse_context_t<version_v>::main_ctx() noexcept {
  return static_cast<parse_and_format_data_t<version_v> &>(*this);
}

template <version_t version_v>
MJZ_CX_FN const parse_and_format_data_t<version_v> &
format_context_t<version_v>::main_ctx() const noexcept {
  return static_cast<const parse_and_format_data_t<version_v> &>(*this);
}
template <version_t version_v>
MJZ_CX_FN parse_and_format_data_t<version_v> &
format_context_t<version_v>::main_ctx() noexcept {
  return static_cast<parse_and_format_data_t<version_v> &>(*this);
}

template <version_t version_v>
MJZ_CX_FN const parse_and_format_data_t<version_v> &
hash_context_t<version_v>::main_ctx() const noexcept {
  return static_cast<const parse_and_format_data_t<version_v> &>(*this);
}
template <version_t version_v>
MJZ_CX_FN parse_and_format_data_t<version_v> &
hash_context_t<version_v>::main_ctx() noexcept {
  return static_cast<parse_and_format_data_t<version_v> &>(*this);
}

template <version_t version_v>
MJZ_CX_FN const parse_context_t<version_v> &
base_context_t<version_v>::parse_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN parse_context_t<version_v> &
base_context_t<version_v>::parse_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const hash_context_t<version_v> &base_context_t<version_v>::hash_ctx()
    const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN hash_context_t<version_v> &
base_context_t<version_v>::hash_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const format_context_t<version_v> &
base_context_t<version_v>::format_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN format_context_t<version_v> &
base_context_t<version_v>::format_ctx() noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN const parse_context_t<version_v> &
parse_context_t<version_v>::parse_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN parse_context_t<version_v> &
parse_context_t<version_v>::parse_ctx() noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN const base_context_t<version_v> &
parse_context_t<version_v>::base_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN base_context_t<version_v> &
parse_context_t<version_v>::base_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const hash_context_t<version_v> &
parse_context_t<version_v>::hash_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN hash_context_t<version_v> &
parse_context_t<version_v>::hash_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const format_context_t<version_v> &
parse_context_t<version_v>::format_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN format_context_t<version_v> &
parse_context_t<version_v>::format_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const parse_context_t<version_v> &
format_context_t<version_v>::parse_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN parse_context_t<version_v> &
format_context_t<version_v>::parse_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const base_context_t<version_v> &
format_context_t<version_v>::base_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN base_context_t<version_v> &
format_context_t<version_v>::base_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const hash_context_t<version_v> &
format_context_t<version_v>::hash_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN hash_context_t<version_v> &
format_context_t<version_v>::hash_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const format_context_t<version_v> &
format_context_t<version_v>::format_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN format_context_t<version_v> &
format_context_t<version_v>::format_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const parse_context_t<version_v> &
hash_context_t<version_v>::parse_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN parse_context_t<version_v> &
hash_context_t<version_v>::parse_ctx() noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN const base_context_t<version_v> &hash_context_t<version_v>::base_ctx()
    const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN base_context_t<version_v> &
base_context_t<version_v>::base_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const hash_context_t<version_v> &hash_context_t<version_v>::hash_ctx()
    const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN hash_context_t<version_v> &
hash_context_t<version_v>::hash_ctx() noexcept {
  return main_ctx();
}

template <version_t version_v>
MJZ_CX_FN const format_context_t<version_v> &
hash_context_t<version_v>::format_ctx() const noexcept {
  return main_ctx();
}
template <version_t version_v>
MJZ_CX_FN format_context_t<version_v> &
hash_context_t<version_v>::format_ctx() noexcept {
  return main_ctx();
}

};  // namespace mjz::bstr_ns::format_ns
#endif  // MJZ_BYTE_FORMATTING_base_abi_LIB_HPP_FILE_