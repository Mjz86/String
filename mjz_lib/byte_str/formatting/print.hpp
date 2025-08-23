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

#include "basic_formatters.hpp"
#ifdef MJZ_WITH_iostream
#include "../../threads/bit_mutex.hpp"
#endif
#ifndef MJZ_BYTE_FORMATTING_print_HPP_FILE_
#define MJZ_BYTE_FORMATTING_print_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
template <version_t version_v>
struct standard_output_it_t : file_output_it_t<version_v> {
  MJZ_MCONSTANT(uint64_t) timeout { uint16_t(-1) };
  bool bad{};
  MJZ_NO_MV_NO_CPY(standard_output_it_t);
#if MJZ_WITH_iostream

  static threads_ns::bit_mutex_t<> &output_muext() noexcept {
    alignas(
        hardware_destructive_interference_size) static threads_ns::bit_mutex_t<>
        mutex{};
    return mutex;
  }
  MJZ_CX_FN standard_output_it_t(bool &&p = bool{}) noexcept
      : standard_output_it_t(p) {}
  MJZ_CX_FN standard_output_it_t(bool &status) noexcept {
    MJZ_IF_CONSTEVAL {
      bad = true;
      status = false;
      return;
    }
    status = true;
    status &= output_muext().try_lock(false, timeout);
    bad = !status;
    if (!bad) {
      this->Stream = stdout;
    }
  }
  MJZ_CX_FN standard_output_it_t(FILE *Stream_) : standard_output_it_t{true} {
    if (this->Stream) this->Stream = Stream_;
  }
  MJZ_CX_FN ~standard_output_it_t() noexcept {
    if (!bad) {
      output_muext().unlock();
    }
  }

#else
  MJZ_CX_FN standard_output_it_t(void *Stream_) noexcept {}
  MJZ_CX_FN standard_output_it_t(const bool &p = bool{},
                                 void *Stream_ = nullptr) noexcept {}
#endif
};

template <version_t version_v>
struct print_t {
  using format_obj_t = formatting_object_t<version_v>;
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  static const constexpr uintlen_t buffer_raw_size_v_ =
      format_stack_size_v<version_v>;
  struct meta_data_t {
    alloc_ref alloc{};
    uintlen_t cache_size{buffer_raw_size_v_ - sizeof(format_obj_t)};
    bool has_new_line{};
    bool log_print_failure{MJZ_LOG_PRINT_FAILURE_};
    MJZ_DEFAULTED_CLASS(meta_data_t);
    MJZ_CX_FN meta_data_t(bool has_new_line_, alloc_ref alloc_) noexcept
        : alloc{std::move(alloc_)}, has_new_line{has_new_line_} {}
  };

 private:
  static constexpr auto align_v_ = std::max<uintlen_t>(
      allocs_ns::stack_alloc_ns::stack_allocator_meta_t<version_v>::align,
      alignof(format_obj_t));
  struct generic_format_to_t_helper_ {
    MJZ_NO_MV_NO_CPY(generic_format_to_t_helper_);
    meta_data_t &meta_data_{};
    base_out_it_t<version_v> &out{};
    // return
    status_view_t<version_v> &err_view{};
    const alloc_ref &alloc_{};
    std::span<char> aliged_stack_{};
    format_obj_t *ptr_{};
    allocs_ns::block_info_t<version_v> storage_resurve_{};
    enum class out_of_memory_e_ { good, no_alloc, one_alloc } out_of_memory_{};
    success_t succuss{};

    MJZ_CX_FN generic_format_to_t_helper_(
        meta_data_t &meta_data_0_, base_out_it_t<version_v> &out_0_,
        status_view_t<version_v> &return_0_,
        std::span<char> aliged_stack_resurve) noexcept
        : meta_data_(meta_data_0_),
          out(out_0_),
          err_view(return_0_),
          alloc_(meta_data_.alloc),
          aliged_stack_{aliged_stack_resurve} {
      MJZ_RELEASE { succuss = out_of_memory_ == out_of_memory_e_::good; };
      allocate_formatter_0_();
      if (!storage_resurve_.ptr) {
        //   return "[Error]generic_format_to : out of memory";
        out_of_memory_ = out_of_memory_e_::no_alloc;
        return;
      }
      storage_resurve_.length -= sizeof(format_obj_t);
      storage_resurve_.ptr += sizeof(format_obj_t);
      MJZ_RELEASE {
        if (out_of_memory_ != out_of_memory_e_::good) MJZ_IS_UNLIKELY {
            releser_1_();
          }
      };
      MJZ_IF_CONSTEVAL {
        ptr_ = alloc_.template allocate_node_uninit<format_obj_t>(false);
        if (!ptr_) {
          //  return ;

          out_of_memory_ = out_of_memory_e_::one_alloc;
          return;
        }
      }
      else {
        ptr_ = reinterpret_cast<format_obj_t *>(std::assume_aligned<align_v_>(
            storage_resurve_.ptr - sizeof(format_obj_t)));
      }
      ptr_ = std::construct_at(
          ptr_, alloc_,
          std::span{storage_resurve_.ptr, storage_resurve_.length}, align_v_);
      MJZ_RELEASE {
        if (out_of_memory_ != out_of_memory_e_::good) MJZ_IS_UNLIKELY {
            releser_2_();
          }
      };
    }
    MJZ_CX_FN ~generic_format_to_t_helper_() noexcept {
      if (out_of_memory_ != out_of_memory_e_::good) MJZ_IS_UNLIKELY {
          err_view = status_view_t<version_v>{
              "[Error]generic_format_to : out of memory"};
          return;
        }
      MJZ_RELEASE { releser_1_(); };
      MJZ_RELEASE { releser_2_(); };
      format_obj_t &obj{*ptr_};
      base_out_it_t<version_v> out_it = obj.main_ctx().format_ctx().out();
      if (meta_data_.has_new_line) {
        base_out_it_t<version_v>(out_it).push_back('\n', encodings_e::ascii);
      }
      if (succuss && out_it.flush_buffer()) MJZ_IS_LIKELY {
          err_view = status_view_t<version_v>{};
          return;
        }
      [&]() noexcept {
        err_view.unsafe_handle() =
            obj.main_ctx().base_ctx().err_content.unsafe_handle();
        if (err_view) {
          err_view =
              "[Error]status_view_t<version_v>print_t::format_to: failed to "
              "output";
          return;
        }

        if (!meta_data_.log_print_failure) {
          return;
        }
        basic_string_view_t<version_v> format_text =
            obj.base_ctx().format_string;
        uintlen_t index = obj.base_ctx().err_index;
        obj.reset();
        out_errored_it_t<version_v> err_it{};
        err_it.it = out_it;
        std::ignore = obj.format_to(
            err_it,
            fmt_litteral_ns::operator_fmt<
                version_v,
                "\n\n at index {3} \"{0}\":\n\n{1}\n\n<<VV-----["
                "ERROR]----------here\n\n{2}\n">(),
            err_view, format_text(0, index), format_text(index), index);
        std::ignore = out_it.flush_buffer();
        return;
      }();
    }

   private:
    MJZ_CX_FN void allocate_formatter_0_() noexcept {
      meta_data_.cache_size = std::max(meta_data_.cache_size, align_v_ * 2);
      meta_data_.cache_size += sizeof(format_obj_t);
      if (meta_data_.cache_size <= aliged_stack_.size_bytes()) {
        asserts(asserts.assume_rn, !!aliged_stack_.data());
        storage_resurve_.ptr =
            std::assume_aligned<align_v_>(aliged_stack_.data());
        storage_resurve_.length = aliged_stack_.size_bytes();
        return;
      }

      storage_resurve_ = alloc_.alloca_bytes(meta_data_.cache_size, align_v_);
    }
    MJZ_CX_FN void releser_1_() noexcept {
      if (meta_data_.cache_size <= aliged_stack_.size_bytes()) {
        return;
      }
      storage_resurve_.length += sizeof(format_obj_t);
      storage_resurve_.ptr -= sizeof(format_obj_t);
      alloc_.dealloca_bytes(std::move(storage_resurve_), align_v_);
    }
    MJZ_CX_FN void releser_2_() noexcept {
      std::destroy_at(ptr_);
      MJZ_IF_CONSTEVAL { alloc_.deallocate_node_uninit(ptr_, false); }
    }
  };

 public:
  MJZ_CX_FN static status_view_t<version_v> generic_format_to_impl_(
      meta_data_t &meta_data_, base_out_it_t<version_v> out,
      callable_c<success_t(format_obj_t &,
                           base_out_it_t<version_v> out_it) noexcept> auto
          &&format_fn,
      std::span<char> stack_opt_ = std::span<char>{}) noexcept {
    status_view_t<version_v> ret{};
    {
      generic_format_to_t_helper_ function_stack{meta_data_, out, ret,
                                                 stack_opt_};
      if (function_stack.succuss) {
        function_stack.succuss =
            format_fn(*function_stack.ptr_, function_stack.out);
      }
    };
    return ret;
  }

  MJZ_CX_AL_FN static status_view_t<version_v> generic_format_to(
      meta_data_t &meta_data_, base_out_it_t<version_v> &out,
      callable_c<success_t(format_obj_t &,
                           base_out_it_t<version_v> out_it) noexcept> auto
          &&format_fn) noexcept {
    MJZ_IFN_CONSTEVAL {
      alignas(align_v_) char buffer_[buffer_raw_size_v_];
      return generic_format_to_impl_(meta_data_, out, format_fn, buffer_);
    }
    else {
      alignas(align_v_) char buffer_[buffer_raw_size_v_]{};
      return generic_format_to_impl_(meta_data_, out, format_fn, buffer_);
    }
  }  // namespace mjz::bstr_ns::format_ns

  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> format_to(
      meta_data_t meta_data_, base_out_it_t<version_v> out, auto fmt,
      Ts &&...args) noexcept {
    return generic_format_to(
        meta_data_, out,
        [&](formatting_object_t<version_v> &obj,
            base_out_it_t<version_v> out_it) noexcept -> success_t {
          return obj.format_to(out_it, fmt, std::forward<Ts>(args)...);
        });
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vformat_to(
      meta_data_t meta_data_, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
    return generic_format_to(
        meta_data_, out,
        [&](formatting_object_t<version_v> &obj,
            base_out_it_t<version_v> out_it) noexcept -> success_t {
          return obj.format_to(out_it, fmt, std::forward<Ts>(args)...);
        });
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> format_to(
      base_out_it_t<version_v> out, auto fmt, Ts &&...args) noexcept {
    return format_to(meta_data_t{}, out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vformat_to(
      base_out_it_t<version_v> out, basic_string_view_t<version_v> fmt,
      Ts &&...args) noexcept {
    return vformat_to(meta_data_t{}, out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vformatln_to(
      base_out_it_t<version_v> out, basic_string_view_t<version_v> fmt,
      Ts &&...args) noexcept {
    return vformat_to(meta_data_t{true, nullptr}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> formatln_to(
      base_out_it_t<version_v> out, auto fmt, Ts &&...args) noexcept {
    return format_to(meta_data_t{true, nullptr}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> formata_to(
      alloc_ref alloc, base_out_it_t<version_v> out, auto fmt,
      Ts &&...args) noexcept {
    return format_to(meta_data_t{false, alloc}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vformata_to(
      alloc_ref alloc, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
    return vformat_to(meta_data_t{false, alloc}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vformatlna_to(
      alloc_ref alloc, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
    return vformat_to(meta_data_t{true, alloc}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> formatlna_to(
      alloc_ref alloc, base_out_it_t<version_v> out, auto fmt,
      Ts &&...args) noexcept {
    return format_to(meta_data_t{true, alloc}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  using ret_t = status_view_t<version_v>;
  using standard_output = standard_output_it_t<version_v>;
  MJZ_MCONSTANT(ret_t)
  output_timeout{static_string_view_t<version_v>{"[Error]:output timeout"}};
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vprint(
      basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return vformat_to(out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> print(auto fmt,
                                                     Ts &&...args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return format_to(out, fmt, std::forward<Ts>(args)...);
  }

  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> vprintln(
      basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return vformatln_to(out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_AL_FN static status_view_t<version_v> println(auto fmt,
                                                       Ts &&...args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return formatln_to(out, fmt, std::forward<Ts>(args)...);
  }
};

namespace print_ns {
template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN  auto vprint(basic_string_view_t<version_v> fmt,
                                Ts &&...args) noexcept {
  return print_t<version_v>::vprint(fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_AL_FN  auto print(auto fmt, Ts &&...args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::print(
      fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN  auto vprintln(basic_string_view_t<version_v> fmt,
                                  Ts &&...args) noexcept {
  return print_t<version_v>::vprintln(fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_AL_FN  auto println(auto fmt, Ts &&...args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::println(
      fmt, std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN  auto vformat_to(auto &out,
                                    basic_string_view_t<version_v> fmt,
                                    Ts &&...args) noexcept {
  return print_t<version_v>::vformat_to(out, fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_AL_FN  auto format_to(auto &out, auto fmt, Ts &&...args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::format_to(
      out, fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN  auto vformatln_to(auto &out,
                                      basic_string_view_t<version_v> fmt,
                                      Ts &&...args) noexcept {
  return print_t<version_v>::vformat_to(out, fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_AL_FN  auto formatln_to(auto &out, auto fmt,
                                     Ts &&...args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::formatln_to(
      out, fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN  auto vformata_to(
    allocs_ns::alloc_base_ref_t<version_v> alloc, auto &out,
    basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
  return print_t<version_v>::vformata_to(alloc, out, fmt,
                                         std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN   auto formata_to(
    allocs_ns::alloc_base_ref_t<version_v> alloc, auto &out, auto fmt,
    Ts &&...args) noexcept {
  return print_t<version_v>::formata_to(alloc, out, fmt,
                                        std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN   auto vformatlna_to(
    allocs_ns::alloc_base_ref_t<version_v> alloc, auto &out,
    basic_string_view_t<version_v> fmt, Ts &&...args) noexcept {
  return print_t<version_v>::vformata_to(alloc, out, fmt,
                                         std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_AL_FN   auto formatlna_to(
    allocs_ns::alloc_base_ref_t<version_v> alloc, auto &out, auto fmt,
    Ts &&...args) noexcept {
  return print_t<version_v>::formatlna_to(alloc, out, fmt,
                                          std::forward<Ts>(args)...);
}
}  // namespace print_ns

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_print_HPP_FILE_