#define MJZ_WITH_iostream true
// i forgot that !MJZ_TEST_MODE_ disables MJZ_LOG_NEW_ALLOCATIONS_
#define MJZ_TEST_MODE_ 0
#define MJZ_LOG_NEW_ALLOCATIONS_ 0
#define MJZ_aggregate_tuple_workaround false
#include <chrono>
#include <thread>

#include "../mjz_lib/allocs/bump_alloc.hpp"
#include "../mjz_lib/byte_str/formatting/basic_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/common_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/easy_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/error_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/format.hpp"
#include "../mjz_lib/byte_str/formatting/formated_stream.hpp"
#include "../mjz_lib/byte_str/formatting/named_args.hpp"
#include "../mjz_lib/byte_str/formatting/opt_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/print.hpp"
#include "../mjz_lib/byte_str/formatting/range_formatters.hpp"
#include "../mjz_lib/byte_str/formatting/std_view_formatters.hpp"
#include "../mjz_lib/byte_str/string.hpp"
#include "../mjz_lib/byte_str/wraped_string.hpp"

namespace used_mjz_ns {
using namespace mjz;
using namespace bstr_ns;
using namespace litteral_ns;
using namespace allocs_ns;
/*
 *this formatting library is not the most optimal
 */
using namespace format_ns;
using namespace fmt_litteral_ns;
using namespace print_ns;

MJZ_CONSTANT(version_t) version_v{};
struct main_t {
  MJZ_NO_MV_NO_CPY(main_t);
  using str_t = basic_str_t<version_v, false>;

  MJZ_CX_FN void run() const;
  MJZ_CX_FN main_t() noexcept {
    if (MJZ_NOEXCEPT { run(); }) {
      return;
    }
  }
};
struct scoped_timer_t {
  MJZ_NO_MV_NO_CPY(scoped_timer_t);
  int64_t nanos{};
  uintlen_t count{};
  basic_str_t<version_v, false> view{nullopt};
  MJZ_CX_FN scoped_timer_t(basic_str_t<version_v, false> name,
                           uintlen_t count_ = 100000ull) noexcept {
    MJZ_IF_CONSTEVAL { return; }
    view = std::move(name);
    MJZ_IF_CONSTEVAL { count_ = 3; }
    count = count_;
    nanos =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
  }
  MJZ_CX_FN ~scoped_timer_t() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    nanos -=
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    nanos = -nanos;
    println("[{}:{}ns]"_fmt, view, double(nanos) / double(count));
  }

  struct iter {
    scoped_timer_t* ptr{};
    uintlen_t i{};
    MJZ_CX_FN auto& operator++() noexcept {
      i++;
      return *this;
    }
    MJZ_CX_FN auto& operator*() noexcept { return *ptr; }
    MJZ_CX_FN bool operator==(uintlen_t count_) const noexcept {
      return count_ == i;
    }
  };

  MJZ_CX_FN auto begin() noexcept { return iter{this, 0}; }
  MJZ_CX_FN uintlen_t end() noexcept { return count; }
  MJZ_CX_FN void no_optimize(auto&&... vals) noexcept {
    MJZ_IF_CONSTEVAL { return; };
    [&]() noexcept { just_do(vals...); }();
  }
};
MJZ_CX_FN void main_t::run() const {
  constexpr wrapped_props_t wrapped_props{.sso_min_cap = 128,
                                          .is_ownerized = true};
  bool disable_cow{false};
  wrapped_string_t<version_v, wrapped_props> result{};
  str_t s =
      "i am a big massage 0123456789abcdefghijklmnop53666666666666666666666666"_str;
  if (disable_cow) {
    std::ignore = s.as_always_ownerized(true);
  }
  uintlen_t i = s.find("0"_str, 0);
  str_t front_part = s.make_substring(0, i);
  s.remove_prefix(i);
  str_t num = s.make_substring(0, s.find("a"_str));
  std::ignore = num.pop_front();
  std::optional<uintlen_t> val = num.to_integral<uintlen_t>();
  format_to(result, "{}{:s}"_fmt, front_part, std::views::iota(0, 10));
  println("{} num : {} "_fmt, result, val);

  auto fn_do_work = [&](auto&& fn1, auto&& fn2, auto&& fn3, str_t name = ""_str,
                        uintlen_t count = 100000ull) {
    for (auto&& timer : scoped_timer_t{name, count}) {
      timer.no_optimize(0);
    }
    for (auto&& timer : scoped_timer_t{name + ":mjz"_str, count}) {
      timer.no_optimize(fn1());
    }
    for (auto&& timer : scoped_timer_t{name + ":mmjz"_str, count}) {
      timer.no_optimize(
          wrapped_string_t<version_v, wrapped_props_t{.is_ownerized = true}>(
              fn1()));
    }
    for (auto&& timer : scoped_timer_t{name + ":std"_str, count}) {
      timer.no_optimize(fn2());
    }
    for (auto&& timer : scoped_timer_t{name + ":stdv"_str, count}) {
      timer.no_optimize(fn3());
    }
  };

  fn_do_work([]() -> str_t { return ""_str; },
             []() -> std::string { return ""; },
             []() -> std::string_view { return ""; });

  fn_do_work([]() -> str_t { return "hello world"_str; },
             []() -> std::string { return "hello world"; },
             []() -> std::string_view { return "hello world"; },
             "hello world"_str);

  fn_do_work([]() -> str_t { return "long hello world"_str; },
             []() -> std::string { return "long hello world"; },
             []() -> std::string_view { return "long hello world"; },
             "long hello world"_str);
}
};  // namespace used_mjz_ns
