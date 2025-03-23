
#include "basic_formatters.hpp"
#ifdef MJZ_WITH_iostream
#include "../../threads/bit_mutex.hpp"
#endif
#ifndef MJZ_BYTE_FORMATTING_print_HPP_FILE_
#define MJZ_BYTE_FORMATTING_print_HPP_FILE_
namespace mjz::bstr_ns::format_ns {
template <version_t version_v>
struct standard_output_it_t : file_output_it_t<version_v> {
  MJZ_CONSTANT(uint64_t) timeout{uint16_t(-1)};
  bool bad{};
  MJZ_NO_MV_NO_CPY(standard_output_it_t);
#if MJZ_WITH_iostream

  threads_ns::bit_mutex_t<>& output_muext() noexcept {
    static threads_ns::bit_mutex_t<> mutex{};
    return mutex;
  }
  MJZ_CX_FN standard_output_it_t(bool&& p = bool{}) noexcept
      : standard_output_it_t(p) {}
  MJZ_CX_FN standard_output_it_t(bool& status) noexcept {
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
  MJZ_CX_FN standard_output_it_t(FILE* Stream_)
      : standard_output_it_t{true, Stream_} {}
  MJZ_CX_FN ~standard_output_it_t() noexcept {
    if (!bad) {
      output_muext().unlock();
    }
  }

#else
  MJZ_CX_FN standard_output_it_t(void* Stream_) noexcept {}
  MJZ_CX_FN standard_output_it_t(const bool& p = bool{},
                                 void* Stream_ = nullptr) noexcept {}
#endif
};
template <version_t version_v>
struct print_t {
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  struct meta_data_t {
    bool has_new_line{};
    alloc_ref alloc{};
    bool log_print_failure{MJZ_LOG_PRINT_FAILURE_};
  };
  MJZ_CX_FN static status_view_t<version_v> generic_format_to(
      meta_data_t meta_data_, base_out_it_t<version_v> out,
      no_type_ns::typeless_function_t<
          success_t(formatting_object_t<version_v>&,
                    base_out_it_t<version_v> out_it) noexcept>
          format_fn) noexcept {
    formatting_object_t<version_v> obj{};
    obj.base_context.alloc = meta_data_.alloc;
    char buf2[1024]{};
    out_buf_it_t<version_v> out_it{out, buf2, /*always null*/ sizeof(buf2) - 1,
                                   encodings_e::ascii};
    success_t succuss = format_fn.run(obj, out_it);
    if (meta_data_.has_new_line) {
      base_out_it_t<version_v>(out_it).push_back('\n', encodings_e::ascii);
    }
    if (succuss && out_it.flush()) return status_view_t<version_v>{};
    status_view_t<version_v> err_view{};
    err_view.unsafe_handle() = obj.base_context.err_content.unsafe_handle();
    if (err_view) {
      err_view =
          "[Error]status_view_t<version_v>print_t::format_to: failed to output";
      return err_view;
    }
    if (!meta_data_.log_print_failure) return err_view;
    basic_string_view_t<version_v> format_text = obj.base_context.format_string;
    uintlen_t index = obj.base_context.err_index;
    obj.reset();
    out_errored_it_t<version_v> err_it{};
    err_it.it = out;
    out_it = out_buf_it_t<version_v>{err_it, buf2, sizeof(buf2) - 1,
                                     encodings_e::ascii};
    std::ignore = obj.format_to(err_it,
                                fmt_litteral_ns::operator_fmt<
                                    version_v,
                                    "\n\n{0}:\n\n{1;[:{2}]:}\n\n<<VV-----["
                                    "ERROR]----------here\n\n{1;[{2}:]:}\n">(),
                                err_view, format_text, index);
    std::ignore = out_it.flush();
    return err_view;
  }

  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> format_to(
      meta_data_t meta_data_, base_out_it_t<version_v> out, auto fmt,
      Ts&&... args) noexcept {
    return generic_format_to(
        meta_data_, out,
        +no_type_ns::make<success_t(formatting_object_t<version_v>&,
                                    base_out_it_t<version_v> out_it) noexcept>(
            [&](formatting_object_t<version_v>& obj,
                base_out_it_t<version_v> out_it) noexcept -> success_t {
              return obj.format_to(out_it, fmt, std::forward<Ts>(args)...);
            }));
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vformat_to(
      meta_data_t meta_data_, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts&&... args) noexcept {
    return generic_format_to(
        meta_data_, out,
        +no_type_ns::make<success_t(formatting_object_t<version_v>&,
                                    base_out_it_t<version_v> out_it) noexcept>(
            [&](formatting_object_t<version_v>& obj,
                base_out_it_t<version_v> out_it) noexcept -> success_t {
              return obj.format_to(out_it, fmt, std::forward<Ts>(args)...);
            }));
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> format_to(
      base_out_it_t<version_v> out, auto fmt, Ts&&... args) noexcept {
    return format_to(meta_data_t{}, out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vformat_to(
      base_out_it_t<version_v> out, basic_string_view_t<version_v> fmt,
      Ts&&... args) noexcept {
    return vformat_to(meta_data_t{}, out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vformatln_to(
      base_out_it_t<version_v> out, basic_string_view_t<version_v> fmt,
      Ts&&... args) noexcept {
    return vformat_to(meta_data_t{true, nullptr}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> formatln_to(
      base_out_it_t<version_v> out, auto fmt, Ts&&... args) noexcept {
    return format_to(meta_data_t{true, nullptr}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> format_to(
      alloc_ref alloc, base_out_it_t<version_v> out, auto fmt,
      Ts&&... args) noexcept {
    return format_to(meta_data_t{false, alloc}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vformat_to(
      alloc_ref alloc, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts&&... args) noexcept {
    return vformat_to(meta_data_t{false, alloc}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vformatln_to(
      alloc_ref alloc, base_out_it_t<version_v> out,
      basic_string_view_t<version_v> fmt, Ts&&... args) noexcept {
    return vformat_to(meta_data_t{true, alloc}, out, fmt,
                      std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> formatln_to(
      alloc_ref alloc, base_out_it_t<version_v> out, auto fmt,
      Ts&&... args) noexcept {
    return format_to(meta_data_t{true, alloc}, out, fmt,
                     std::forward<Ts>(args)...);
  }
  using ret_t = status_view_t<version_v>;
  using standard_output = standard_output_it_t<version_v>;
  MJZ_CONSTANT(ret_t)
  output_timeout{static_string_view_t<version_v>{"[Error]:output timeout"}};
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vprint(
      basic_string_view_t<version_v> fmt, Ts&&... args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return vformat_to(out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> print(auto fmt,
                                                  Ts&&... args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return format_to(out, fmt, std::forward<Ts>(args)...);
  }

  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> vprintln(
      basic_string_view_t<version_v> fmt, Ts&&... args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return vformatln_to(out, fmt, std::forward<Ts>(args)...);
  }
  template <typename... Ts>
  MJZ_CX_FN static status_view_t<version_v> println(auto fmt,
                                                    Ts&&... args) noexcept {
    bool good{};
    standard_output out{good};
    if (!good) return output_timeout;
    return formatln_to(out, fmt, std::forward<Ts>(args)...);
  }
};

namespace print_ns {
template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vprint(basic_string_view_t<version_v> fmt,
                             Ts&&... args) noexcept {
  return print_t<version_v>::vprint(fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_FN static auto print(auto fmt, Ts&&... args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::print(
      fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vprintln(basic_string_view_t<version_v> fmt,
                               Ts&&... args) noexcept {
  return print_t<version_v>::vprintln(fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_FN static auto println(auto fmt, Ts&&... args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::println(
      fmt, std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformat_to(auto& out, basic_string_view_t<version_v> fmt,
                                 Ts&&... args) noexcept {
  return print_t<version_v>::vformat_to(out, fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_FN static auto format_to(auto& out, auto fmt, Ts&&... args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::format_to(
      out, fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformatln_to(auto& out,
                                   basic_string_view_t<version_v> fmt,
                                   Ts&&... args) noexcept {
  return print_t<version_v>::vformat_to(out, fmt, std::forward<Ts>(args)...);
}
template <typename... Ts>
MJZ_CX_FN static auto formatln_to(auto& out, auto fmt, Ts&&... args) noexcept {
  return print_t<std::remove_cvref_t<decltype(fmt())>::Version_v>::formatln_to(
      out, fmt, std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformat_to(allocs_ns::alloc_base_ref_t<version_v> alloc,
                                 auto& out, basic_string_view_t<version_v> fmt,
                                 Ts&&... args) noexcept {
  return print_t<version_v>::vformat_to(alloc, out, fmt,
                                        std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto format_to(allocs_ns::alloc_base_ref_t<version_v> alloc,
                                auto& out, auto fmt, Ts&&... args) noexcept {
  return print_t<version_v>::format_to(alloc, out, fmt,
                                       std::forward<Ts>(args)...);
}

template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto vformatln_to(allocs_ns::alloc_base_ref_t<version_v> alloc,
                                   auto& out,
                                   basic_string_view_t<version_v> fmt,
                                   Ts&&... args) noexcept {
  return print_t<version_v>::vformat_to(alloc, out, fmt,
                                        std::forward<Ts>(args)...);
}
template <version_t version_v, typename... Ts>
MJZ_CX_FN static auto formatln_to(allocs_ns::alloc_base_ref_t<version_v> alloc,
                                  auto& out, auto fmt, Ts&&... args) noexcept {
  return print_t<version_v>::formatln_to(alloc, out, fmt,
                                         std::forward<Ts>(args)...);
}
}  // namespace print_ns

}  // namespace mjz::bstr_ns::format_ns

#endif  // MJZ_BYTE_FORMATTING_print_HPP_FILE_