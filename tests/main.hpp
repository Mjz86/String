#define MJZ_WITH_iostream true
// i forgot that !MJZ_TEST_MODE_ disables MJZ_LOG_NEW_ALLOCATIONS_
#define MJZ_TEST_MODE_ true
#define MJZ_LOG_NEW_ALLOCATIONS_ true
#define MJZ_aggregate_tuple_workaround false
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
};
};  // namespace used_mjz_ns
