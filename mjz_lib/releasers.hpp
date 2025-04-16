#include "macors.hpp"
#ifndef MJZ_RELEASER_LIB_HPP_FILE_
#define MJZ_RELEASER_LIB_HPP_FILE_
#include <array>
#include <bit>
#include <concepts>
namespace mjz {
template <class unique_accessor_id_t>
class mjz_private_accessed_t {};
struct totally_empty_type_t {};
using nullptr_t = std::nullptr_t;
MJZ_CONSTANT(totally_empty_type_t) totally_empty_type{};
static_assert(std::is_empty_v<totally_empty_type_t>);
using void_struct_t = totally_empty_type_t;
template <>
class mjz_private_accessed_t<void_struct_t(void_struct_t *)> {
 public:
  template <class T>
  MJZ_CX_AL_FN static T mptr_static_cast(auto p) noexcept {
    if constexpr (requires() { static_cast<T>(p); }) {
      return static_cast<T>(p);
    } else if constexpr (std::same_as<std::remove_cvref_t<decltype(p)>,
                                      nullptr_t>) {
      return nullptr;
    } else if constexpr (std::same_as<std::remove_cvref_t<decltype(p)>,
                                      void_struct_t *>) {
      return std::remove_pointer_t<
          std::remove_cvref_t<T>>::mptr_static_cast_pv_fn_(p);
    } else {
      if (!p) return nullptr;
      return p->template mptr_static_cast_pv_fn_<T>();
    }
  }

  template <class T>
  MJZ_CX_AL_FN static decltype(auto) up_cast(T *ptr) noexcept {
    return mptr_static_cast<void_struct_t *>(ptr);
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) up_cast(const T *ptr) noexcept {
    return mptr_static_cast<const void_struct_t *>(ptr);
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) down_cast(void_struct_t *ptr) noexcept {
    return mptr_static_cast<T *>(ptr);
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) down_cast(
      const void_struct_t *ptr) noexcept {
    return mptr_static_cast<const T *>(ptr);
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) up_cast(T &ptr) noexcept {
    return *up_cast(std::addressof(ptr));
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) up_cast(const T &ptr) noexcept {
    return *up_cast(std::addressof(ptr));
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) down_cast(void_struct_t &ptr) noexcept {
    return *down_cast<T>(std::addressof(ptr));
  }
  template <class T>
  MJZ_CX_AL_FN static decltype(auto) down_cast(
      const void_struct_t &ptr) noexcept {
    return *down_cast<T>(std::addressof(ptr));
  }
};
using void_struct_cast_t =
    mjz_private_accessed_t<void_struct_t(void_struct_t *)>;
enum class may_bool_t : char { no = false, yes = true, idk = 2, err = 3 };
MJZ_CONSTANT(bool)
SYSTEM_is_little_endian_{std::endian::little == std::endian::native};
#if MJZ_uintlen_t_as_64_bit
using uintlen_t = uint64_t;
using ushortlen_t = uint32_t;
#else
using uintlen_t = std::conditional_t<(sizeof(uint32_t) <= sizeof(uintptr_t)),
                                     uintptr_t, uint32_t>;
using ushortlen_t = std::conditional_t<(sizeof(uint32_t) <= sizeof(uintlen_t)),
                                       uint32_t, uint16_t>;
#endif
using intlen_t = std::make_signed_t<uintlen_t>;
using shortlen_t = std::make_signed_t<ushortlen_t>;
template <typename T>
using single_object_pointer_t = T *;
template <typename T>
using alias_t = T;

template <size_t I, typename T, typename... Ts>
struct type_at_index_helper_t {
  using type_at = typename type_at_index_helper_t<I - 1, Ts...>::type_at;
};
template <typename T, typename... Ts>
struct type_at_index_helper_t<0, T, Ts...> {
  using type_at = T;
};
template <size_t I, typename... Ts>
using type_at_index_t = typename type_at_index_helper_t<I, Ts...>::type_at;
template <class T, class U>
struct compare_tamplate_codes_helper_t : std::false_type {};
template <template <class...> class TC, class... Ts, class... Us>
struct compare_tamplate_codes_helper_t<TC<Ts...>, TC<Us...>> : std::true_type {
};
template <class T, template <class...> class TC>
struct compare_tamplates_helper_t : std::false_type {};
template <template <class...> class TC, class... Ts>
struct compare_tamplates_helper_t<TC<Ts...>, TC> : std::true_type {};
template <class T, class U>
concept compare_tamplate_codes_c = compare_tamplate_codes_helper_t<T, U>()();
template <class T, template <class...> class TC>
concept compare_tamplates_c = compare_tamplates_helper_t<T, TC>()();
template <size_t size_of_myt>
using uint_sizeof_t =
    type_at_index_t<size_of_myt, uint8_t, uint8_t, uint16_t, uint32_t, uint32_t,
                    uint64_t, uint64_t, uint64_t, uint64_t>;
template <size_t S>
using uint_size_of_t = uint_sizeof_t<S>;
template <class>
class my_totatlly_empty_template1_class_t {};
MJZ_CX_AL_ND_FN static uint8_t log2_of_val_create(
    std::integral auto val) noexcept {
  uint8_t ret = uint8_t(std::bit_width(val));
  return alias_t<uint8_t[2]>{uint8_t(ret - 1), 0}[ret == 0];
}
MJZ_CX_AL_ND_FN static uint64_t log2_of_val_to_val(uint8_t log2_val) noexcept {
  return static_cast<uint64_t>(1ull << log2_val);
}
MJZ_CX_AL_ND_FN static uint8_t log2_ceil_of_val_create(
    std::integral auto val) noexcept
  requires(std::is_unsigned_v<std::remove_cvref_t<decltype(val)>>)
{
  auto log2v = log2_of_val_create(val);
  return uint8_t(uint64_t(log2v) +
                 uint64_t(log2_of_val_to_val(log2v) != uint64_t(val)));
}

template <uint64_t max_val>
using uint_with_max_of_t =
    uint_size_of_t<size_t(log2_ceil_of_val_create(max_val + 1) / 8) +
                   size_t(!!(log2_ceil_of_val_create(max_val + 1) % 8))>;
template <uint64_t min_max_val>
using int_with_max_of_t = std::make_signed_t<uint_with_max_of_t<min_max_val>>;

template <typename T>
  requires(std::is_integral_v<std::remove_cvref_t<T>>)
constexpr inline auto signedof(T val) noexcept {
  return std::bit_cast<std::make_signed_t<T>>(val);
}
template <typename T>
  requires(std::is_integral_v<std::remove_cvref_t<T>>)
constexpr inline auto unsignedof(T val) noexcept {
  return std::bit_cast<std::make_unsigned_t<T>>(val);
}

template <typename T>
constexpr inline std::add_const_t<T &&> constof(T &&val) noexcept {
  return std::forward<std::add_const_t<T>>(val);
}

#define MJZ_FORWRADER_FOR_NON_INTEGRAL_OBJECTS(FN_NAME_)  \
  template <typename T>                                   \
    requires(!std::is_integral_v<std::remove_cvref_t<T>>) \
  MJZ_CX_FN decltype(auto) FN_NAME_##of(                  \
      T &&obj_for_##FN_NAME_##cast) noexcept {            \
    return std::forward<T>(obj_for_##FN_NAME_##cast);     \
  }
MJZ_FORWRADER_FOR_NON_INTEGRAL_OBJECTS(unsigned);
MJZ_FORWRADER_FOR_NON_INTEGRAL_OBJECTS(signed);

template <class releaser_LAMBDA_t, typename... reasorces_t>
  requires requires(releaser_LAMBDA_t l, reasorces_t &&...args) {
    releaser_LAMBDA_t(std::move(l));
    l(std::forward<reasorces_t>(args)...);
  }
class releaser_t {
 public:
  using tuple_t = std::tuple<reasorces_t...>;
  releaser_t(releaser_t &&) = delete;
  releaser_t(const releaser_t &) = delete;
  releaser_t &operator=(releaser_t &&) = delete;
  releaser_t(releaser_t &) = delete;
  releaser_t &operator=(releaser_t &) = delete;
  releaser_t &operator=(const releaser_t &) = delete;
  releaser_t() = delete;
  MJZ_CX_AL_FN releaser_t(releaser_LAMBDA_t &&releaser_lambda,
                          reasorces_t &&...args) noexcept(requires() {
    { releaser_LAMBDA_t(std::move(releaser_lambda)) } noexcept;
    { tuple_t(std::forward<reasorces_t>(args)...) } noexcept;
  })
      : m_releaser_lambda_(std::move(releaser_lambda)),
        data(std::forward<reasorces_t>(args)...) {}
  MJZ_CX_AL_FN ~releaser_t() noexcept(requires(reasorces_t &&...args) {
    { m_releaser_lambda_(std::forward<reasorces_t>(args)...) } noexcept;
  }) {
    call();
  }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  releaser_LAMBDA_t m_releaser_lambda_;
  template <size_t INDEX>
  using type_at = std::tuple_element_t<INDEX, tuple_t>;

  template <typename... Ts>
    requires(sizeof...(Ts) < sizeof...(reasorces_t))
  MJZ_CX_AL_FN void call(Ts &&...args) {
    call(std::forward<Ts>(args)...,
         std::forward<type_at<sizeof...(Ts)>>(std::get<sizeof...(Ts)>(data)));
  }
  MJZ_CX_AL_FN void call(reasorces_t &&...args) {
    m_releaser_lambda_(std::forward<reasorces_t>(args)...);
  }

 public:
  tuple_t data;
  MJZ_NO_DYNAMIC_ALLOCATOR(releaser_t);
};
template <class releaser_LAMBDA_t>
  requires requires(releaser_LAMBDA_t l) {
    releaser_LAMBDA_t(std::move(l));
    l();
  }
class releaser_t<releaser_LAMBDA_t> {
 public:
  MJZ_NO_MV_NO_CPY(releaser_t);
  releaser_t() = delete;
  MJZ_CX_AL_FN
  releaser_t(releaser_LAMBDA_t &&releaser_lambda) noexcept(requires() {
    { releaser_LAMBDA_t(std::move(releaser_lambda)) } noexcept;
  }) : m_releaser_lambda_(std::move(releaser_lambda)) {}
  MJZ_CX_AL_FN ~releaser_t() { m_releaser_lambda_(); }

  template <class>
  friend class mjz_private_accessed_t;

 private:
  releaser_LAMBDA_t m_releaser_lambda_;
  MJZ_NO_DYNAMIC_ALLOCATOR(releaser_t);
};
template <std::same_as<void> = void>
struct releaser_helper_t {
  MJZ_CE_FN releaser_helper_t() noexcept {}
  template <typename T>
  MJZ_CX_AL_NDR_FN("this should be stored for it to be called at end of scope")
  auto operator->*(T &&fn) const
      noexcept(noexcept(releaser_t{std::forward<T>(fn)}))
          -> decltype(releaser_t{std::forward<T>(fn)}) {
    return releaser_t{std::forward<T>(fn)};
  };
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN explicit operator bool() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN bool operator!() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN void operator&() const noexcept = delete;
};
static_assert(std::is_empty_v<releaser_helper_t<>>);
typedef bool success_t;
MJZ_CONSTANT(success_t) success_v = true;
MJZ_CONSTANT(success_t) failiure_v = false;

template <class Lmabda_t>
MJZ_CX_FN success_t run_and_block_exeptions(
    Lmabda_t &&code, const bool no_exeptions = false) noexcept {
  if constexpr (requires(Lmabda_t &&code_) {
                  { std::forward<Lmabda_t>(code_)() } noexcept;
                }) {
    std::forward<Lmabda_t>(code)();
    return true;
  } else if (no_exeptions) {
    std::forward<Lmabda_t>(code)();
    return true;
  } else {
    MJZ_COMMENT("compiler code  is  noexcept.   ");
    MJZ_IF_CONSTEVAL {
      std::forward<Lmabda_t>(code)();
      return true;
    }
    else {
      MJZ_DISABLE_ALL_WANINGS_START_;
#if MJZ_CATCHES_EXCEPTIONS_
      try {
        std::forward<Lmabda_t>(code)();
      } catch (...) {
        return false;
      }
#else
      std::forward<Lmabda_t>(code)();
#endif
      MJZ_DISABLE_ALL_WANINGS_END_;
    }
    return true;
  }
}
template <std::same_as<void> = void>
struct noexcept_er_helper_t {
 public:
  MJZ_CE_FN noexcept_er_helper_t() noexcept {}
  template <class Lmabda_t>
  MJZ_CX_FN success_t operator->*(Lmabda_t &&fn) const noexcept {
    return run_and_block_exeptions(std::forward<Lmabda_t>(fn));
  };
  template <class Lmabda_t>
  MJZ_CX_FN success_t operator*(Lmabda_t &&fn) const noexcept {
    success_t ret{};
    run_and_block_exeptions(
        [&]() noexcept(noexcept(success_t(std::forward<Lmabda_t>(fn)()))) {
          ret = success_t(std::forward<Lmabda_t>(fn)());
        });
    return ret;
  };
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN explicit operator bool() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN bool operator!() const noexcept = delete;
  MJZ_DEPRECATED_R("confusion")
  MJZ_CX_FN void operator&() const noexcept = delete;
};
static_assert(std::is_empty_v<noexcept_er_helper_t<>>);

#define MJZ_RELEASE_NAME_helper_0_(WHAT) releaserr_##WHAT
#define MJZ_RELEASE_NAME_helper_1_(WHAT) MJZ_RELEASE_NAME_helper_0_(WHAT)
#define MJZ_RELEASE_NAME(WHAT) MJZ_RELEASE_NAME_helper_1_(WHAT)
/*
this is a temporary RAII object for being used for something like this:
void f(){
std::mutex m;
m.lock();
MJZ_W_RELEASE(m){m.unlock();};
...use...m...
}
void g(){
 auto p= new int;
  MJZ_W_RELEASE(p) { delete p;};
...use...p...
}
*/
#define MJZ_C_RELEASE(WHAT, CODE_BLOCK)                       \
  MJZ_UNUSED const ::mjz::releaser_t MJZ_RELEASE_NAME(WHAT) { \
    [&]() mutable noexcept -> void CODE_BLOCK                 \
  }
#define MJZ_W_RELEASE(WHAT)                        \
  MJZ_UNUSED const auto &&MJZ_RELEASE_NAME(WHAT) = \
      ::mjz::releaser_helper_t<>{}->*[&]() mutable noexcept -> void

#define MJZ_W_TRELEASE(WHAT)                       \
  MJZ_UNUSED const auto &&MJZ_RELEASE_NAME(WHAT) = \
      ::mjz::releaser_helper_t<>{}->*[&, this ]() mutable noexcept -> void

#define MJZ_NOEXCEPT ::mjz::noexcept_er_helper_t<>{}->*[&]() mutable -> void

#define MJZ_TNOEXCEPT \
  ::mjz::noexcept_er_helper_t<>{}->*[&, this ]() mutable -> void

/*
this is a temporary RAII object for being used for something like this:
m.lock();
MJZ_RELEASE {m.unlock();};
p= new int;
MJZ_RELEASE  { delete p; p=nullptr;};
...use...m...
...use...p...
*/
#define MJZ_RELEASE MJZ_W_RELEASE(MJZ_UNIQUE_VAR_NAME(line))
/*
this is a temporary RAII object for being used for something like this:
void ACLASS::f(){
this->m.lock();
MJZ_RELEASE {m.unlock();};
...use...m...
}
void ACLASS::g(){
  this->p= new int;
  MJZ_RELEASE  { delete p; p=nullptr;};
...use...p...
}
*/
#define MJZ_TRELEASE MJZ_W_TRELEASE(MJZ_UNIQUE_VAR_NAME(line))

/*
this is a temporary RAII object for being used for something like this:
void ACLASS::f(){
this->m.lock();
MJZ_W_RELEASE(m){m.unlock();};
...use...m...
}
void ACLASS::g(){
  this->p= new int;
  MJZ_W_RELEASE(p) { delete p; p=nullptr;};
...use...p...
}
*/
#define MJZ_C_TRELEASE(WHAT, CODE_BLOCK)                      \
  MJZ_UNUSED const ::mjz::releaser_t MJZ_RELEASE_NAME(WHAT) { \
    [&, this ]() mutable noexcept -> void CODE_BLOCK          \
  }

template <typename T>
using nullable_t = T;
template <typename T>
using no_null_t = T;

template <typename lambda_try_function, typename lambda_catch_function>
MJZ_CX_FN void raii_try_catch_rethrow(lambda_try_function &&try_func,
                                      lambda_catch_function &&catch_func)
  requires requires() {
    { std::forward<lambda_catch_function>(catch_func)() } noexcept;
  }
{
  bool has_thrown = true;
  MJZ_RELEASE {
    if (has_thrown) {
      std::forward<lambda_catch_function>(catch_func)();
    }
  };
  std::forward<lambda_try_function>(try_func)();
  has_thrown = false;
}
#define MJZ_try_catch_rethrow(TRY_BLOCK_, CATCH_BLOCK)            \
  ::mjz::raii_try_catch_rethrow([&]() mutable -> void TRY_BLOCK_, \
                                [&]() mutable noexcept -> void CATCH_BLOCK)

/* do not use in production */
MJZ_DISABLE_WANINGS_START_;
MJZ_NCX_FN void just_do_ptr(volatile void *arg) noexcept {
  MJZ_DISABLE_WANINGS_END_;
  MJZ_UNUSED static thread_local volatile auto p =
      const_cast<volatile void *>(arg);
  p = arg;
};
template <typename... Ts>
MJZ_NCX_FN void just_do(Ts &&...args) noexcept {  //-V3541 //-V2565
  MJZ_UNUSED totally_empty_type_t a[]{
      (just_do_ptr(std::addressof(args)),
       totally_empty_type)...};  //-V3519 //-V2528
}
template <typename T>
using non_null_rvalue_ptr_t = T *const;

// no perfect forward
template <typename T>
using Rvalueref_t = T &&;
template <class T>
  requires(!std::is_const_v<T> && !std::is_abstract_v<T> &&
           !std::is_reference_v<T> && std::is_class_v<T>)
class conditional_releaser_t : private T {
  template <class>
  friend class mjz_private_accessed_t;
  conditional_releaser_t(const conditional_releaser_t &) = delete;
  conditional_releaser_t(conditional_releaser_t &) = delete;
  conditional_releaser_t &operator=(conditional_releaser_t &) = delete;
  conditional_releaser_t &operator=(const conditional_releaser_t &) = delete;
  bool has_lambda;

  MJZ_CX_FN T &operator*() noexcept { return *this; }

 public:
  MJZ_CONSTANT(bool)
  noexcept_v = requires(T lamda) {
    { std::move(lamda)() } noexcept;
  };
  MJZ_CX_FN conditional_releaser_t(T &&lambda_,
                                   bool has_lambda_ = true) noexcept
      : T(std::move(lambda_)), has_lambda(has_lambda_) {}
  MJZ_CX_AL_FN void run_now() noexcept(noexcept_v) {
    if (has_lambda) {
      has_lambda = false;
      std::move (**this)();
    }
  }
  MJZ_CX_AL_FN void dont_run() noexcept { has_lambda = false; }
  MJZ_CX_AL_ND_FN explicit operator bool() const noexcept { return has_lambda; }
  MJZ_CX_AL_ND_FN bool operator!() const noexcept { return !has_lambda; }
  MJZ_CX_AL_FN ~conditional_releaser_t() noexcept(noexcept_v) { run_now(); }

  MJZ_CX_AL_FN conditional_releaser_t(conditional_releaser_t &&other) noexcept(
      requires(T other_l) {
        { T(std::move(other_l)) } noexcept;
      })
      : T(std::move(*other)),
        has_lambda(std::exchange(other.has_lambda, false)) {}
  MJZ_CX_AL_FN conditional_releaser_t &operator=(
      conditional_releaser_t &&other) noexcept(requires(T other_l, T lamda) {
    { lamda = std::move(other_l) } noexcept;
  }) {
    has_lambda = std::exchange(other.has_lambda, false);
    **this = std::move(*other);
    return *this;
  }

  MJZ_NO_DYNAMIC_ALLOCATOR(conditional_releaser_t);
};

};  // namespace mjz
#endif  // MJZ_RELEASER_LIB_HPP_FILE_