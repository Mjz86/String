#include "../mjz_lib/alllib_import.hpp"

// sowwy :/ its a test

  using namespace mjz;
  using namespace bstr_ns;
  using namespace litteral_ns;
  using namespace allocs_ns;
  using namespace format_ns;
  using namespace fmt_litteral_ns;
  using namespace print_ns;
  inline static constexpr version_t version_v{};
  using sview_t = static_string_view_t<version_v>;
  using dview_t = dynamic_string_view_t<version_v>;
  using view_t = basic_string_view_t<version_v>;
  using str_t = basic_str_t<version_v, basic_str_props_t<version_v>{}>;

MJZ_EXPORT struct scoped_timer_t {
  MJZ_NO_MV_NO_CPY(scoped_timer_t);
  int64_t nanos{};
  uintlen_t count{};
  basic_str_t<version_v> view{nullopt};
  MJZ_CX_FN scoped_timer_t(basic_str_t<version_v> name,
                           uintlen_t count_ = 10000000ull) noexcept
      : view{std::move(name)} {
    MJZ_IF_CONSTEVAL {
      count_ = 3;
      return;
    }
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
    scoped_timer_t *ptr{};
    uintlen_t i{};
    MJZ_CX_FN auto &operator++() noexcept {
      i++;
      return *this;
    }
    MJZ_CX_FN auto &operator*() noexcept { return *ptr; }
    MJZ_CX_FN bool operator==(uintlen_t count_) const noexcept {
      return count_ == i;
    }
  };

  MJZ_CX_FN auto begin() noexcept { return iter{this, 0}; }
  MJZ_CX_FN uintlen_t end() const noexcept { return count; }
};
#define MJZ_BENCHMARK(X) scoped_timer_t X##_benchmarker{basic_str_t<version_v>(#X), 1}
int main() {
  using sa = std::array<char, 8>;
  auto gen = std::views::iota(0, 1000000);

  auto tos = [](auto i) noexcept {
    return std::bit_cast<sa>(
        details_ns::hybrid_iota_8digits_ascii_noif_noload(uint64_t(i)));
  };
  auto toa = [](auto i) noexcept { return std::bit_cast<sa>(uint64_t(i)); };

  {
    MJZ_BENCHMARK(baseline);
    for (auto i : gen | std::views::reverse) {
      just_do(tos(i), toa(i));
    }
  }
  {

    unordered_vector_str_t<version_v, sa, sa> ht{};
    {
      MJZ_BENCHMARK(insert);
      for (auto i : gen | std::views::reverse) {
        just_do(ht.insert(tos(i), toa(i)));
      }
    }
    {
      MJZ_BENCHMARK(find_good);
      for (auto i : gen) {
        just_do(ht.find(tos(i)));
      }
    }
    {
      MJZ_BENCHMARK(erase);
      for (auto i : gen | std::views::take(gen.size() / 2)) {
        ht.erase(tos(i));
      }
    }
    {
      MJZ_BENCHMARK(find_mix);
      for (auto i : gen) {
        just_do(ht.find(tos(i)));
      }
    };
    {
      MJZ_BENCHMARK(find_bad);
      for (auto i : gen | std::views::take(gen.size() / 2)) {
        just_do(ht.find(tos(i)));
      }
    }

    {
      MJZ_BENCHMARK(iterate_load);
      for (auto [key, value] : ht) {
        just_do(std::bit_cast<uint64_t>(key), std::bit_cast<uint64_t>(value));
      }
    }
  }

  struct stdhash{
    std::hash<std::string_view> hs{}; 
     std::size_t operator()(const sa& s) const noexcept
    {
      return hs(std::string_view{s.data(),s.size()});
    }
  };
  std::unordered_map<sa,sa,stdhash > ht{};
  
  {
    MJZ_BENCHMARK(std_insert);
    for (auto i : gen | std::views::reverse) {
      just_do(ht.insert({tos(i), toa(i)}));
    }
  }
  {
    MJZ_BENCHMARK(std_find_good);
    for (auto i : gen) {
      just_do(ht.find(tos(i)));
    }
  }
  {
    MJZ_BENCHMARK(std_erase);
    for (auto i : gen | std::views::take(gen.size() / 2)) {
      ht.erase(tos(i));
    }
  }
  {
    MJZ_BENCHMARK(std_find_mix);
    for (auto i : gen) {
      just_do(ht.find(tos(i)));
    }
  };
  {
    MJZ_BENCHMARK(std_find_bad);
    for (auto i : gen | std::views::take(gen.size() / 2)) {
      just_do(ht.find(tos(i)));
    }
  }
  {
    MJZ_BENCHMARK(std_iterate_load);
    for (auto [key, value] : ht) {
      just_do(std::bit_cast<uint64_t>(key), std::bit_cast<uint64_t>(value));
    }
  }
  return 0;
}