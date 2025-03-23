#include "asserts.hpp"
#ifndef MJZ_VERSIONS_LIB_HPP_FILE_
#define MJZ_VERSIONS_LIB_HPP_FILE_

namespace mjz {
struct version_t {
 public:
  struct value_t {
    uint64_t m_is_BE_ : 1 {!SYSTEM_is_little_endian_};
    // the order is the other way around
    uint64_t m_ver_ : 30 {};
    uint64_t m_ver_mod_id_ : 30 {};
    //
    uint64_t m_any_size_log_offset_ : 2 {1};
    uint64_t m_named_any_ : 1 {1};
    MJZ_CX_FN static value_t make(int64_t value) noexcept {
      value_t ret{};
      ret.make_(uint64_t(value));
      return ret;
    }
    MJZ_CX_FN int64_t operator()() const noexcept {
      return int64_t(uint64_t(m_named_any_) |
                     (uint64_t(m_any_size_log_offset_) << 1) |
                     (uint64_t(m_ver_) << 3) | (uint64_t(m_ver_mod_id_) << 33) |
                     (uint64_t(m_is_BE_) << 63));
    }
    MJZ_CX_FN operator int64_t() const noexcept { return operator()(); }
    MJZ_CX_FN void make_(uint64_t value) noexcept {
      m_named_any_ = (value) & (uint64_t(1 << 1) - 1);
      m_any_size_log_offset_ = (value >> 1) & (uint64_t(1 << 2) - 1);
      m_ver_ = (value >> 3) & ((uint64_t(1) << 30) - 1);
      m_ver_mod_id_ = (value >> 33) & ((uint64_t(1) << 31) - 1);
      m_is_BE_ = (value >> 63) & (uint64_t(1 << 1) - 1);
    }
  };

  int64_t value{};
  MJZ_CX_FN value_t make() const noexcept { return value_t::make(value); }
  MJZ_CONSTANT(uint32_t) newest_m_version = MJZ_LIB_NEWEST_VERSION_;

  MJZ_CE_FN explicit version_t(uint32_t newest_version_ = newest_m_version,
                               uint32_t m_ver_mod_id = 0) noexcept
      : value(value_t{.m_ver_=(newest_version_), .m_ver_mod_id_=(m_ver_mod_id)}) {
    asserts(newest_version_ <= newest_m_version,
            "update the latest api version  var (=newest_m_version). "
            "to include this version.");
  }
  MJZ_CX_FN explicit version_t(uint32_t newest_version_, uint32_t m_ver_mod_id,
                               totally_empty_type_t) noexcept
      : value(value_t{.m_ver_=(newest_version_), .m_ver_mod_id_=(m_ver_mod_id)}) {
  }

  MJZ_CX_FN std::partial_ordering operator<=>(version_t b) const noexcept {
    auto v = uint32_t(make().m_ver_) <=> uint32_t(b.make().m_ver_);
    if (v != std::strong_ordering::equal) return v;
    return (*this == b) ? std::strong_ordering::equal
                        : std::partial_ordering::unordered;
  }
  MJZ_CX_FN bool operator==(version_t b) const noexcept {
    return make().m_ver_ == b.make().m_ver_ &&
           make().m_ver_mod_id_ == b.make().m_ver_mod_id_;
  }

  MJZ_CX_FN bool in_range(version_t inclusive_begin,
                          version_t exclusive_end) const noexcept {
    return inclusive_begin <= *this && *this < exclusive_end;
  }
  MJZ_CX_FN bool less_than(version_t ver) const noexcept { return *this < ver; }
  MJZ_CX_FN bool more_than(version_t ver) const noexcept { return *this > ver; }
  MJZ_CX_FN bool is(version_t ver) const noexcept { return *this == ver; }
  MJZ_CX_FN bool is_LE() const noexcept { return !make().m_is_BE_; }
  MJZ_CX_FN bool is_BE() const noexcept { return make().m_is_BE_; }

  MJZ_DEPRECATED_R(
      "do not use raw numbers as versions, create a named variable first!")
  MJZ_CX_ND_FN operator uint32_t() const noexcept { return make().m_ver_; }
};

template <version_t version_v>
struct concatabe_hash_t {
  uintlen_t hash{};
  uintlen_t length{};
  MJZ_CX_ND_FN concatabe_hash_t(uintlen_t hash_, uintlen_t len) noexcept
      : hash(hash_), length(len) {}
  MJZ_CX_ND_FN concatabe_hash_t(nullptr_t = nullptr) noexcept {}
  MJZ_CX_ND_FN concatabe_hash_t(char c) noexcept {
    hash = uintlen_t(uint8_t(c));
    length = 1;
  }
  MJZ_CX_ND_FN concatabe_hash_t(const char* ptr, uintlen_t len) noexcept {
    if ((!ptr && len)) return;
    for (uintlen_t i{}; i < len; i++) {
      hash ^= uintlen_t(uint8_t(ptr[i])) << bit_rotate_t(i);
    }
    length = len;
  }
  MJZ_CX_ND_FN concatabe_hash_t friend operator+(
      const concatabe_hash_t& rhs, const concatabe_hash_t& lhs) noexcept {
    concatabe_hash_t ret{};
    ret.length = rhs.length + lhs.length;
    bit_rotate_t lhs_beg_shift{rhs.length};
    ret.hash = rhs.hash ^ (lhs.hash << lhs_beg_shift);
    return ret;
  }
  /*
   *note , a hash of rhs must have lhs at its end to negate it.
   */
  MJZ_CX_ND_FN concatabe_hash_t friend operator-(
      const concatabe_hash_t& rhs, const concatabe_hash_t& lhs) noexcept {
    concatabe_hash_t ret{};
    if (rhs.length < lhs.length) return ret;
    ret.length = rhs.length - lhs.length;
    bit_rotate_t lhs_beg_shift{ret.length};
    ret.hash = rhs.hash ^ (lhs.hash << lhs_beg_shift);
    return ret;
  }
  MJZ_CX_ND_FN concatabe_hash_t& operator+=(
      const concatabe_hash_t& lhs) noexcept {
    return *this = *this + lhs;
  }
  MJZ_CX_FN bool friend operator==(const concatabe_hash_t& rhs,
                                   const concatabe_hash_t& lhs) noexcept {
    return rhs.hash == lhs.hash && rhs.length == lhs.length;
  }
};

}  // namespace mjz
#endif  // MJZ_VERSIONS_LIB_HPP_FILE_