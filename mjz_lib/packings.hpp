
#include "releasers.hpp"
#ifndef MJZ_PACKING_LIB_HPP_FILE_
#define MJZ_PACKING_LIB_HPP_FILE_
namespace mjz {
MJZ_PACKING_START_NAW_;
/*NOTE:
1.don't cause UB
2.be careful
3.this only removes alignments , dose not change the object size, even if has
padding 4.by storing this in a class , you dont add to your class alignment
*/
template <typename T>
struct unaligned_holder_t {
 public:
  T unaligned_data;

 public:
  MJZ_CX_FN T *assume_aligned_ptr() & noexcept {
    return std::assume_aligned<alignof(T)>(std::addressof(unaligned_data));
  }
  MJZ_CX_FN const T *assume_aligned_ptr() const & noexcept {
    return std::assume_aligned<alignof(T)>(std::addressof(unaligned_data));
  }
  MJZ_CX_FN const T &&assume_aligned() const && noexcept {
    return std::move(*assume_aligned_ptr());
  }
  MJZ_CX_FN T &&assume_aligned() && noexcept {
    return std::move(*assume_aligned_ptr());
  }
  MJZ_CX_FN const T &assume_aligned() const & noexcept {
    return *assume_aligned_ptr();
  }
  MJZ_CX_FN T &assume_aligned() & noexcept { return *assume_aligned_ptr(); }
  MJZ_CX_FN T create_aligned() noexcept
    requires requires(T obj) {
      { std::bit_cast<T>(obj) } noexcept -> std::same_as<T>;
    }
  {
    return std::bit_cast<T>(unaligned_data);
  }

  // abbreviation, careful
  MJZ_CX_FN const T &AA() const & noexcept { return assume_aligned(); }
  MJZ_CX_FN const T &&AA() const && noexcept { return assume_aligned(); }
  MJZ_CX_FN T &&AA() && noexcept { return assume_aligned(); }
  MJZ_CX_FN T &AA() & noexcept { return assume_aligned(); }
  MJZ_CX_FN T *AAP() & noexcept { return assume_aligned_ptr(); }
  MJZ_CX_FN const T *AAP() const & noexcept { return assume_aligned_ptr(); }
};

template <typename T, size_t minsize_>
union unaligned_optional_holder_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  char minsize_helper_dummy_[minsize_];

 public:
  T unaligned_data;

 public:
  MJZ_CX_FN T *assume_aligned_dead_ptr() & noexcept {
    return std::assume_aligned<alignof(T)>(std::addressof(unaligned_data));
  }
  MJZ_CX_FN const T *assume_aligned_dead_ptr() const & noexcept {
    return std::assume_aligned<alignof(T)>(std::addressof(unaligned_data));
  }
  MJZ_CX_FN T *assume_aligned_dead_ptr() && noexcept = delete;
  MJZ_CX_FN const T *assume_aligned_dead_ptr() const && noexcept = delete;

  MJZ_CX_FN T *assume_aligned_ptr() & noexcept {
    return std::assume_aligned<alignof(T)>(
        std::launder(std::addressof(unaligned_data)));
  }
  MJZ_CX_FN const T *assume_aligned_ptr() const & noexcept {
    return std::assume_aligned<alignof(T)>(
        std::launder(std::addressof(unaligned_data)));
  }
  MJZ_CX_FN const T &&assume_aligned() const && noexcept {
    return std::move(*assume_aligned_ptr());
  }
  MJZ_CX_FN T &&assume_aligned() && noexcept {
    return std::move(*assume_aligned_ptr());
  }
  MJZ_CX_FN const T &assume_aligned() const & noexcept {
    return *assume_aligned_ptr();
  }
  MJZ_CX_FN T &assume_aligned() & noexcept { return *assume_aligned_ptr(); }
  MJZ_CX_FN T create_aligned() noexcept
    requires requires(T obj) {
      { std::bit_cast<T>(obj) } noexcept -> std::same_as<T>;
    }
  {
    return std::bit_cast<T>(unaligned_data);
  }

  // abbreviation, careful
  MJZ_CX_FN const T &AA() const & noexcept { return assume_aligned(); }
  MJZ_CX_FN const T &&AA() const && noexcept {
    return std::move(assume_aligned());
  }
  MJZ_CX_FN T &&AA() && noexcept { return std::move(assume_aligned()); }
  MJZ_CX_FN T &AA() & noexcept { return assume_aligned(); }
  MJZ_CX_FN T *AAP() & noexcept { return assume_aligned_ptr(); }
  MJZ_CX_FN const T *AAP() const & noexcept { return assume_aligned_ptr(); }
};
MJZ_PACKING_END_NAW_;

namespace raw_string_object_laoyouts_maybe_ {
MJZ_BAD_COMPILER(sizeof(char) == 1);
MJZ_BAD_COMPILER(sizeof(unsigned char) == 1);
MJZ_BAD_COMPILER(sizeof(uint8_t) == 1);
MJZ_BAD_COMPILER(sizeof(int8_t) == 1);
MJZ_BAD_COMPILER(sizeof(uint16_t) == 2);
MJZ_BAD_COMPILER(sizeof(int16_t) == 2);
MJZ_BAD_COMPILER(sizeof(uint32_t) == 4);  //-V112
MJZ_BAD_COMPILER(sizeof(int32_t) == 4);   //-V112
MJZ_BAD_COMPILER(sizeof(uint64_t) == 8);
MJZ_BAD_COMPILER(sizeof(int64_t) == 8);
MJZ_BAD_COMPILER(sizeof(uintptr_t) % 2 == 0);
MJZ_BAD_COMPILER(sizeof(void *) == sizeof(char *));
MJZ_BAD_COMPILER(sizeof(void *) == sizeof(uintptr_t));
MJZ_BAD_COMPILER(sizeof(ptrdiff_t) == sizeof(uintptr_t));
MJZ_BAD_COMPILER(sizeof(intptr_t) == sizeof(uintptr_t));
MJZ_BAD_COMPILER(std::endian::little == std::endian::native ||
                 std::endian::native == std::endian::big);
MJZ_SASSERT(4 <= sizeof(uintlen_t));
MJZ_SASSERT(alignof(void *) <= alignof(uintlen_t));
MJZ_BAD_MCOMPILER(alignof(char) == 1 && sizeof(char) == 1,
                  " we use char as one byte and as storage ");
struct padding_test_packed_t_ {
  char c;
  unaligned_holder_t<uint32_t> i;
};
MJZ_BAD_COMPILER(sizeof(padding_test_packed_t_) ==  //-V119
                 sizeof(char) + sizeof(uint32_t));
MJZ_BAD_COMPILER(alignof(padding_test_packed_t_) == 1);
struct padding_test_unpacked_t_ {
  char c;
  uint32_t i;
};
MJZ_BAD_COMPILER(alignof(char) <= alignof(uint32_t));
MJZ_BAD_COMPILER(sizeof(padding_test_unpacked_t_) ==
                 alignof(uint32_t) + sizeof(uint32_t));
MJZ_BAD_COMPILER(alignof(padding_test_unpacked_t_) == alignof(uint32_t));

struct bit_field_test_t_ {
  char b0 : 1;
  char b1 : 1;
  char b2 : 1;
  char b3 : 1;
  char b4 : 1;
  char b5 : 1;
  char b6 : 1;
  char b7 : 1;
};

MJZ_BAD_COMPILER(sizeof(bit_field_test_t_) == 1);

};  // namespace raw_string_object_laoyouts_maybe_

template <std::integral T>
  requires(sizeof(T) <= 16) 

MJZ_CX_AL_FN static uint8_t get_end_bit_index(T mask) noexcept {
  int16_t index{};
  while (mask) {
    mask >>= 1;
    index++;
  }
  return static_cast<uint8_t>(index);
}
template <std::integral T>
  requires(sizeof(T) <= 16)
MJZ_CX_AL_FN static uint8_t get_begin_bit_index(T mask) noexcept {
  int16_t index{(sizeof(T) * 8) - 1};
  while (static_cast<bool>(static_cast<T &>(mask <<= 1))) {  //-V2561
    index--;
  }
  return static_cast<uint8_t>(index);
}

struct bit_rotate_t {
  template <std::integral T>
  MJZ_CX_FN static T rotate_left(T x, const uintlen_t how_much_) noexcept {
    MJZ_FCONSTANT(intlen_t) nbits = intlen_t(sizeof(T) * 8);
    MJZ_FCONSTANT(T) all_one{~T{}};
    const intlen_t how_much = intlen_t(how_much_ % nbits);
    const intlen_t neg_how_much = nbits - how_much;
    const T mask{to_left(all_one, -how_much)};
    const T neg_mask{to_left(all_one, -neg_how_much)};
    const T val{to_left((mask & x), how_much)};
    const T neg_val{neg_mask & to_left(x, -neg_how_much)};
    T ret{val | neg_val};
    return ret;
  }
  MJZ_CX_FN static auto rotate_right(std::integral auto x,
                                     const uintlen_t how_much_) noexcept {
    return rotate_left(x, (~how_much_) + 1);
  }
  MJZ_CX_FN friend auto operator<<(std::integral auto x,
                                   bit_rotate_t r) noexcept {
    return rotate_left(x, r.amount);
  }
  MJZ_CX_FN friend auto operator>>(std::integral auto x,
                                   bit_rotate_t r) noexcept {
    return rotate_right(x, r.amount);
  }
  MJZ_CX_FN friend auto &operator<<=(std::integral auto &x,
                                     bit_rotate_t r) noexcept {
    return x = rotate_left(x, r.amount);
  }
  MJZ_CX_FN friend auto &operator>>=(std::integral auto &x,
                                     bit_rotate_t r) noexcept {
    return x = rotate_right(x, r.amount);
  }
  uintlen_t amount{};

  MJZ_CX_FN bit_rotate_t(uintlen_t r) noexcept : amount{r} {}
  MJZ_CX_FN bit_rotate_t() noexcept {}

 private:
  template <std::integral T>
  MJZ_CX_FN static T to_left(T x, intlen_t how_much_) noexcept {
    bool neg{};
    if (how_much_ < 0) {
      neg = true;
      how_much_ = -how_much_;
    }
    if ((neg &&
         sizeof(T) * 8 <= uintlen_t(how_much_) + get_begin_bit_index(x)) ||
        (!neg &&
         sizeof(T) * 8 <= uintlen_t(how_much_) + get_end_bit_index(x))) {
      return 0;
    }
    return neg ? x >> how_much_ : x << how_much_;
  }
};

}  // namespace mjz
#endif  // MJZ_PACKING_LIB_HPP_FILE_