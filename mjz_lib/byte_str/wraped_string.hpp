

#include "../byte_str/string.hpp"
#ifndef MJZ_BYTE_STRING_wraped_string_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_wraped_string_LIB_HPP_FILE_
namespace mjz::bstr_ns {

template <version_t version_v, wrapped_props_t props_v>
struct wrapped_string_data_t
    : public basic_str_t<version_v, props_v.has_alloc> {
 private:
  template <class>
  friend class mjz_private_accessed_t;
};
template <version_t version_v, wrapped_props_t props_v>
  requires requires() {
    requires(basic_str_abi_ns_::nsso_u<version_v>::sso_cap <
             props_v.sso_min_cap);
  }
struct wrapped_string_data_t<version_v, props_v>
    : public basic_str_t<version_v, props_v.has_alloc> {
 protected:
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(uintlen_t)
  sso_cap_v_{props_v.cap()};
  std::array<char, sso_cap_v_> m_stack_buffer_{};
};

template <version_t version_v, wrapped_props_t props_v>
struct wrapped_string_t : private wrapped_string_data_t<version_v, props_v> {
  template <class T>
  MJZ_CX_FN T mptr_static_cast_pv_fn_() noexcept {
    return static_cast<T>(this);
  }
  template <class T>
  MJZ_CX_FN T mptr_static_cast_pv_fn_() const noexcept {
    return static_cast<T>(this);
  }
  MJZ_CX_FN static auto mptr_static_cast_pv_fn_(void_struct_t *p) noexcept {
    return static_cast<wrapped_string_t *>(p);
  }
  MJZ_CX_FN static auto mptr_static_cast_pv_fn_(
      const void_struct_t *p) noexcept {
    return static_cast<const wrapped_string_t *>(p);
  }

 private:
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(bool)
  has_alloc_v_{props_v.has_alloc};

 public:
  using traits_type = byte_traits_t<version_v>;
  using value_type = char;
  using pointer =
      std::conditional_t<props_v.is_ownerized, char *, const char *>;
  using const_pointer = const char *;
  using reference =
      std::conditional_t<props_v.is_ownerized, char &, const char &>;
  using const_reference = const char &;
  using const_iterator = iterator_t<const wrapped_string_t>;
  using iterator = iterator_t<wrapped_string_t>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using size_type = uintlen_t;
  using difference_type = intlen_t;

  using back_insert_iterator_t = base_out_it_t<version_v>;

 private:
  using replace_flags = replace_flags_t<version_v>;
  using static_string_view = static_string_view_t<version_v>;

  using dynamic_string_view = dynamic_string_view_t<version_v>;
  using generic_string_view = basic_string_view_t<version_v>;

  using str_heap_manager = str_heap_manager_t<version_v>;
  using str_t = basic_str_t<version_v, has_alloc_v_>;
  // using    base_str_info = base_str_info_t<version_v>;
  using cheap_str_info = cheap_base_str_info_t<version_v>;
  using EM_t = encodings_e;
  using alloc_ref = allocs_ns::alloc_base_ref_t<version_v>;
  using owned_stack_buffer = owned_stack_buffer_t<version_v>;
  using hash_t = hash_bytes_t<version_v>;

  using m_t = basic_str_abi_ns_::m_t<version_v, has_alloc_v_>;
  using my_details = basic_str_abi_ns_::details_t<version_v>;

  using dont_mess_up_t = unsafe_ns::i_know_what_im_doing_t;

  MJZ_CX_FN std::optional<owned_stack_buffer_t<version_v>>
  my_stack_buffer() noexcept {
    return [&]<int = 0>() noexcept
               -> std::optional<owned_stack_buffer_t<version_v>> {
      if constexpr (requires() { this->m_stack_buffer_; }) {
        return owned_stack_buffer_t<version_v>{this->m_stack_buffer_.data(),
                                               this->m_stack_buffer_.size()};
      }
      return {};
    }();
  }
  MJZ_CX_FN str_t &m_str() noexcept {
    return *void_struct_cast_t::down_cast<str_t>(
        void_struct_cast_t::up_cast(this));
  }
  MJZ_CX_FN const str_t &m_str() const noexcept {
    return *void_struct_cast_t::down_cast<str_t>(
        void_struct_cast_t::up_cast(this));
  }
  MJZ_CX_FN static success_t ensure_props(str_t &where) noexcept {
    return where.ensure_props(props_v);
  }
  MJZ_CX_FN auto prop_guard() noexcept {
    return releaser_t{[this]() noexcept {
      if (init_stack(m_str()) && ensure_props(m_str())) {
        return;
      }
      // this err has to fit in sso  :( , so  data() can modify it.
      m_str().reset_to_error_on_fail(false, "[Err]");
      asserts(asserts.assume_rn, ensure_props(m_str()));
    }};
  }
  MJZ_CX_FN success_t init_stack(str_t &where) noexcept {
    std::optional<owned_stack_buffer_t<version_v>> msb = my_stack_buffer();
    if (!msb) return true;
    return where.may_reconsider_stack(unsafe_ns::unsafe_v, *msb,
                                      props_v.has_null);
  }

 public:
  MJZ_CX_FN wrapped_string_t() noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(
        init_stack(m_str()) && ensure_props(m_str()),
        "[Error]wrapped_string_t::wrapped_string_t():  failed string init");
  }
  MJZ_CX_FN wrapped_string_t(str_t source_) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(init_stack(source_) &&
                                       ensure_props(source_) &&
                                       m_str().move_init(std::move(source_)),
                                   "[Error]wrapped_string_t::wrapped_string_t("
                                   "str_t):  failed string init");
  }
  template <wrapped_props_t version_v_src>
  MJZ_CX_FN wrapped_string_t(
      wrapped_string_data_t<version_v, version_v_src> &&source_) noexcept
      : wrapped_string_t() {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(
        source_.move_to_dest(m_str()) && ensure_props(m_str()),
        "[Error]wrapped_string_t::wrapped_string_t( "
        "wrapped_string_data_t<version_v_src, version_v_src>&&):  failed "
        "string init");
  }
  template <wrapped_props_t version_v_src>
  MJZ_CX_FN wrapped_string_t(
      const wrapped_string_data_t<version_v, version_v_src> &source_) noexcept
      : wrapped_string_t(source_.m_str()) {}

  template <wrapped_props_t version_v_src>
  MJZ_CX_FN wrapped_string_t &operator=(
      wrapped_string_data_t<version_v, version_v_src> &&source_) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(
        source_.move_to_dest(m_str()),
        "[Error]wrapped_string_t::wrapped_string_t &operator=( "
        "wrapped_string_data_t<version_v_src, version_v_src>&&):  failed "
        "string assign");
    return *this;
  }
  template <wrapped_props_t version_v_src>
  MJZ_CX_FN wrapped_string_t &operator=(
      const wrapped_string_data_t<version_v, version_v_src> &source_) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(
        m_str().assign(source_.m_str()),
        "[Error]wrapped_string_t::wrapped_string_t &operator=( "
        "const wrapped_string_data_t<version_v_src, version_v_src>&):  failed "
        "string assign");
    return *this;
  }

  MJZ_CX_FN wrapped_string_t &operator=(const str_t &source_) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(m_str().assign(source_),
                                   "[Error]wrapped_string_t::wrapped_string_t "
                                   "&operator=(const str_t &):  failed "
                                   "string assign");
    return *this;
  }
  MJZ_CX_FN wrapped_string_t &operator=(str_t &&source_) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str().reset_to_error_on_fail(init_stack(source_) &&
                                       ensure_props(source_) &&
                                       m_str().move_init(std::move(source_)),
                                   "[Error]wrapped_string_t::wrapped_string_t "
                                   "&operator=(str_t&&):  failed "
                                   "string assign");
    return *this;
  }

 public:
  MJZ_CX_FN success_t move_to_dest(str_t &dest) noexcept {
    if (m_str().is_stacked()) {
      return dest.assign(m_str());
    }
    return dest.assign_move(std::move(m_str()));
  }
  MJZ_CX_FN operator const str_t &() const noexcept { return m_str(); }
  MJZ_CX_FN const str_t &get() const noexcept { return m_str(); }
  MJZ_CX_ND_FN const auto &get_alloc() const noexcept {
    return get().get_alloc();
  }
  MJZ_CX_FN success_t as_substring(uintlen_t byte_offset, uintlen_t byte_count,
                                   bool try_to_add_null = true) noexcept {
    return get().as_substring(byte_offset, byte_count, try_to_add_null);
  }
  MJZ_CX_ND_FN wrapped_string_t
  make_substring(uintlen_t byte_offset, uintlen_t byte_count) const noexcept {
    wrapped_string_t ret{};
    str_t &str = ret.m_str();
    m_str().reset_to_error_on_fail(
        str.copy_assign(m_str(), false, byte_offset, byte_count) &&
            ret.init_stack(str) && ret.ensure_props(str),
        "[Error]wrapped_string_t::make_substring: cannot init string");
    return ret;
  }
  MJZ_CX_ND_FN wrapped_string_t operator()(uintlen_t begin_i = 0,
                                           uintlen_t end_i = nops) noexcept {
    if (end_i < begin_i) return {};
    return make_substring(begin_i, end_i - begin_i);
  }

  /* similar to as_substring*/
  MJZ_CX_FN wrapped_string_t &to_substring(uintlen_t byte_offset,
                                           uintlen_t byte_count) noexcept {
    m_str().reset_to_error_on_fail(
        as_substring(byte_offset, byte_count),
        "[Error]wrapped_string_t::to_substring: cannot init string");
    return *this;
  }
  void data() && = delete;
  void data() const && = delete;
  /* gives a pointer to the data */
  MJZ_CX_ND_FN const char *data() const & noexcept { return m_str().data(); }
  template <int = 0>
    requires(!props_v.is_ownerized)
  MJZ_CX_ND_FN const char *data() & noexcept {
    return m_str().data();
  }
  template <int = 0>
    requires(props_v.is_ownerized)
  MJZ_CX_ND_FN char *data() & noexcept {
    str_t &str = m_str();
    return str.m.mut_begin();
  }
  MJZ_CX_ND_FN bool has_null() const noexcept { return get().has_null(); }
  MJZ_CX_ND_FN uintlen_t length() const noexcept { return get().length(); }
  MJZ_CX_ND_FN encodings_e get_encoding() const noexcept {
    return get().get_encoding();
  }
  MJZ_CX_FN void set_encoding(const dont_mess_up_t &idk,
                              encodings_e encoding) noexcept {
    return m_str().set_encoding(idk, encoding);
  }

  MJZ_CONSTANT(uintlen_t)
  npos{traits_type::npos};
  MJZ_CONSTANT(uintlen_t)
  nops{traits_type::npos};
  MJZ_CX_FN static uintlen_t max_size() noexcept { return str_t::max_size(); }
  MJZ_CX_ND_FN optional_ref_t<const char> at(const uintlen_t i) const noexcept {
    if (i < length()) return get().data()[i];
    return {};
  }
  template <int = 0>
    requires(!props_v.is_ownerized)
  MJZ_CX_ND_FN optional_ref_t<const char> at(const uintlen_t i) noexcept {
    if (i < length()) return get().data()[i];
    return {};
  }
  template <int = 0>
    requires(props_v.is_ownerized)
  MJZ_CX_ND_FN optional_ref_t<char> at(const uintlen_t i) noexcept {
    if (i < length()) return data()[i];
    return {};
  }
  MJZ_CX_ND_FN decltype(auto) operator[](const uintlen_t i) const noexcept {
    return at(i);
  }
  MJZ_CX_ND_FN decltype(auto) operator[](const uintlen_t i) noexcept {
    return at(i);
  }

  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) const && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) && = delete;

  /* similar to make_substrview, exept this even views the SSO */
  MJZ_CX_ND_FN generic_string_view
  make_subview(const dont_mess_up_t &idk, uintlen_t byte_offset = 0,
               uintlen_t byte_count = nops,
               bool unsafe_assume_static_ = false) const & noexcept {
    return get().make_subview(idk, byte_offset, byte_count,
                              unsafe_assume_static_);
  }
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) const && = delete;
  MJZ_CX_ND_FN wrapped_string_t
  make_substrview(const dont_mess_up_t &idk, uintlen_t byte_offset,
                  uintlen_t byte_count, bool propgate_alloc = true,
                  bool unsafe_assume_static_ = false) const & noexcept {
    return get().make_substrview(idk, byte_offset, byte_count, propgate_alloc,
                                 unsafe_assume_static_);
  }
  MJZ_CX_FN success_t remove_suffix(uintlen_t byte_count) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().remove_suffix(byte_count);
  }
  MJZ_CX_FN success_t remove_prefix(uintlen_t byte_count) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().remove_prefix(byte_count);
  }
  MJZ_CX_FN std::optional<uintlen_t> copy_bytes(
      uintlen_t byte_offset, uintlen_t byte_count, char *buf,
      bool add_null = false) const noexcept {
    return get().copy_bytes(byte_offset, byte_count, buf, add_null);
  }
  MJZ_CX_ND_FN uintlen_t capacity(bool must_owner = true) const noexcept {
    return get().capacity(must_owner);
  }
  MJZ_CX_FN success_t reserve(uintlen_t mincap, uintlen_t prefer_cap = 0,
                              const alloc_ref &alloc = m_t::empty_alloc,
                              replace_flags flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().reserve(mincap, prefer_cap, alloc, flags);
  }
  MJZ_CX_FN success_t consider_stack(const dont_mess_up_t &idk,
                                     owned_stack_buffer &&where) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().consider_stack(idk, std::move(where));
  }
  MJZ_CX_FN success_t shrink_to_fit(bool force_ownership = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().shrink_to_fit(force_ownership);
  }

  MJZ_CX_FN success_t clear(bool force_ownership = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().clear(force_ownership);
  }

  MJZ_CX_FN success_t add_null() noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().add_null();
  }

  MJZ_CX_FN const char *c_str() const noexcept
    requires(props_v.has_null)
  {
    return m_str().data();
  }
  MJZ_CX_FN const char *as_c_str() & noexcept { return m_str().as_c_str(); }
  MJZ_CX_ND_FN hash_t hash() const noexcept { return get().hash(); }
  MJZ_CX_ND_FN bool is_owner() const noexcept {
    if constexpr (props_v.is_ownerized) {
      return true;
    }
    return get().is_owner();
  }
  MJZ_CX_ND_FN auto get_states() const noexcept { return get().get_states(); }
  MJZ_CX_FN bool get_threaded() const noexcept { return get().get_threaded(); }
  MJZ_CX_FN bool is_stacked() const noexcept { return get().is_stacked(); }
  MJZ_CX_ND_FN success_t replace_data_with_char(
      uintlen_t offset, uintlen_t byte_count, uintlen_t length_of_val,
      std::optional<char> val, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().replace_data_with_char(offset, byte_count, length_of_val,
                                          val, val_alloc, rep_flags);
  }

  MJZ_CX_ND_FN success_t
  as_ownerized(const alloc_ref &val_alloc = m_t::empty_alloc,
               replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().as_ownerized(val_alloc, rep_flags);
  }
  MJZ_CX_ND_FN success_t as_always_ownerized(
      bool flag_state_, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().as_always_ownerized(flag_state_, val_alloc, rep_flags);
  }
  template <class R_t>
    requires std::ranges::sized_range<R_t> && std::ranges::forward_range<R_t>
  MJZ_CX_ND_FN success_t
  replace_data_with_range(uintlen_t offset, uintlen_t byte_count, R_t &&r,
                          const alloc_ref &val_alloc = m_t::empty_alloc,
                          replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().replace_data_with_range(
        offset, byte_count, std::forward<R_t>(r), val_alloc, rep_flags);
  }
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  replace_data_with_range(uintlen_t offset, uintlen_t byte_count, R_t &&r,
                          const alloc_ref &val_alloc = m_t::empty_alloc,
                          replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().replace_data_with_range(
        offset, byte_count, std::forward<R_t>(r), val_alloc, rep_flags);
  }

  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t insert_data_with_range(
      uintlen_t offset, R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().insert_data_with_range(offset, std::forward<R_t>(r),
                                          val_alloc, rep_flags);
  }
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  append_data_with_range(R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
                         replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().append_data_with_range(std::forward<R_t>(r), val_alloc,
                                          rep_flags);
  }
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  assign_data_with_range(R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
                         replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign_data_with_range(std::forward<R_t>(r), val_alloc,
                                          rep_flags);
  }

  MJZ_CX_ND_FN success_t
  replace_data(uintlen_t offset, uintlen_t byte_count, const str_t &&other,
               replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().replace_data(offset, byte_count, other, rep_flags);
  }
  MJZ_CX_ND_FN success_t
  replace_data(uintlen_t offset, uintlen_t byte_count, const str_t &other,
               replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().replace_data(offset, byte_count, other, rep_flags);
  }

  MJZ_CX_ND_FN success_t
  erase_data(uintlen_t offset, uintlen_t byte_count, const alloc_ref &val_alloc,
             replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().erase_data(offset, byte_count, val_alloc, rep_flags);
  }
  MJZ_CX_ND_FN success_t
  insert_data(uintlen_t offset, const str_t &other,
              replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().erase_data(offset, other, rep_flags);
  }
  MJZ_CX_ND_FN success_t insert_data_with_char(
      uintlen_t offset, uintlen_t length_of_val, std::optional<char> val,
      const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().insert_data_with_char(offset, length_of_val, val, val_alloc,
                                         rep_flags);
  }
  MJZ_CX_ND_FN success_t append_data(
      const str_t &other, replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().append_data(other, rep_flags);
  }
  MJZ_CX_ND_FN success_t
  append_data_with_char(uintlen_t length_of_val, std::optional<char> val,
                        const alloc_ref &val_alloc = m_t::empty_alloc,
                        replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().append_data_with_char(length_of_val, val, val_alloc,
                                         rep_flags);
  }

  MJZ_CX_ND_FN success_t
  push_back(std::optional<char> c,
            replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().push_back(c, rep_flags);
  }
  MJZ_CX_ND_FN success_t
  push_front(std::optional<char> c,
             replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().push_front(c, rep_flags);
  }
  MJZ_CX_ND_FN std::optional<char> pop_back() noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().pop_back();
  }
  MJZ_CX_ND_FN std::optional<char> pop_front() noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().pop_front();
  }
  MJZ_CX_ND_FN success_t assign_own(const str_t &other,
                                    bool no_allocate = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign_own(other, no_allocate);
  }
  MJZ_CX_ND_FN success_t assign(const str_t &other,
                                bool no_allocate = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign(other, no_allocate);
  }
  MJZ_CX_ND_FN success_t assign_share(const str_t &other,
                                      bool no_allocate = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign_share(other, no_allocate);
  }
  MJZ_CX_ND_FN success_t assign_move(str_t &&other) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign_move(std::move(other));
  }
  MJZ_CX_ND_FN success_t assign(char c, bool no_allocator = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().assign(c, no_allocator);
  }
  MJZ_CX_ND_FN success_t
  resize(uintlen_t new_len, std::optional<char> val = std::nullopt,
         bool force_ownership = false,
         const alloc_ref &val_alloc = m_t::empty_alloc,
         replace_flags rep_flags = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().resize(new_len, val, force_ownership, val_alloc, rep_flags);
  }

  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, length());
  }
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept { return begin(); }
  MJZ_CX_ND_FN const_iterator cend() const noexcept { return end(); }

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }
  MJZ_CX_ND_FN reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  MJZ_CX_ND_FN reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  MJZ_CX_ND_FN iterator begin() noexcept { return iterator(*this, 0); }
  MJZ_CX_ND_FN iterator end() noexcept { return iterator(*this, length()); }

  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept { return rend(); }

  MJZ_CX_ND_FN size_type size() const noexcept { return length(); }
  MJZ_CX_ND_FN bool empty() const noexcept { return length() == 0; }
  MJZ_CX_ND_FN decltype(auto) front() const noexcept { return at(0); }
  MJZ_CX_ND_FN decltype(auto) back() const noexcept { return at(length() - 1); }
  MJZ_CX_ND_FN decltype(auto) front() noexcept { return at(0); }
  MJZ_CX_ND_FN decltype(auto) back() noexcept { return at(length() - 1); }

  MJZ_CX_FN void swap(str_t &o) noexcept;

  MJZ_CX_ND_FN std::optional<intlen_t> compare(
      const str_t &rhs) const noexcept {
    return get().compare(rhs);
  }

  MJZ_CX_ND_FN bool starts_with(const str_t &rhs) const noexcept {
    return get().starts_with(rhs);
  }

  MJZ_CX_ND_FN bool starts_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return get().starts_with(rhs, encoding);
  }

  MJZ_CX_ND_FN bool ends_with(const str_t &rhs) const noexcept {
    return get().ends_with(rhs);
  }
  MJZ_CX_ND_FN bool ends_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return get().ends_with(rhs, encoding);
  }

  MJZ_CX_ND_FN bool contains(const str_t &rhs) const noexcept {
    return get().contains(rhs);
  }
  MJZ_CX_ND_FN bool contains(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept {
    return get().contains(rhs, encoding);
  }
  MJZ_CX_ND_FN uintlen_t find(const str_t &rhs,
                              const uintlen_t offset = 0) const noexcept {
    return get().find(rhs, offset);
  }
  MJZ_CX_ND_FN uintlen_t rfind(const str_t &rhs,
                               const uintlen_t offset = nops) const noexcept {
    return get().rfind(rhs, offset);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(const str_t &rhs, const uintlen_t offset = 0) const noexcept {
    return get().find_first_of(rhs, offset);
  }

  MJZ_CX_ND_FN uintlen_t
  find_last_of(const str_t &rhs, const uintlen_t offset = nops) const noexcept {
    return get().find_last_of(rhs, offset);
  }
  MJZ_CX_ND_FN uintlen_t find_first_not_of(
      const str_t &rhs, const uintlen_t offset = 0) const noexcept {
    return get().find_first_not_of(rhs, offset);
  }
  MJZ_CX_ND_FN uintlen_t find_last_not_of(
      const str_t &rhs, const uintlen_t offset = nops) const noexcept {
    return get().find_last_not_of(rhs, offset);
  }

  MJZ_CX_ND_FN uintlen_t
  find(char rhs, const uintlen_t offset = 0,
       encodings_e encoding = encodings_e{}) const noexcept {
    return get().find(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  rfind(char rhs, const uintlen_t offset = nops,
        encodings_e encoding = encodings_e{}) const noexcept {
    return get().rfind(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_of(char rhs, const uintlen_t offset = 0,
                encodings_e encoding = encodings_e{}) const noexcept {
    return get().find_first_of(rhs, offset, encoding);
  }

  MJZ_CX_ND_FN uintlen_t
  find_last_of(char rhs, const uintlen_t offset = nops,
               encodings_e encoding = encodings_e{}) const noexcept {
    return get().find_last_of(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_first_not_of(char rhs, const uintlen_t offset = 0,
                    encodings_e encoding = encodings_e{}) const noexcept {
    return get().find_first_not_of(rhs, offset, encoding);
  }
  MJZ_CX_ND_FN uintlen_t
  find_last_not_of(char rhs, const uintlen_t offset = nops,
                   encodings_e encoding = encodings_e{}) const noexcept {
    return get().find_last_not_of(rhs, offset, encoding);
  }
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral(uint8_t raidex = 0) const noexcept {
    return get().template to_integral<T>(raidex);
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_real_floating() const noexcept {
    return get().template to_real_floating<T>();
  }
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_floating() const noexcept {
    return get().template to_floating<T>();
  }

 public:
  MJZ_CX_FN success_t append_data_temp(str_t &&str) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().append_data_temp(str);
  }

  MJZ_CX_FN wrapped_string_t &operator+=(str_t &&obj) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str() += std::move(obj);
    return *this;
  }
  MJZ_CX_FN wrapped_string_t &operator+=(const str_t &obj) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    m_str() += (obj);
    return *this;
  }
  MJZ_CX_FN static wrapped_string_t operator_add(str_t &rhs,
                                                 str_t &lhs) noexcept {
    wrapped_string_t ret{rhs};
    ret.m_str() += lhs;
    return ret;
  }

  MJZ_CX_FN bool is_error() const noexcept { return get().is_error(); }

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_FN success_t as_integral(T val, const uint8_t raidex = 10,
                                  bool upper_case = false) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().as_integral(val, raidex, upper_case);
  }
  template <std::floating_point T>
  MJZ_CX_FN success_t as_floating(
      T val, uintlen_t accuracacy = sizeof(uintlen_t), bool upper_case = false,
      floating_format_e floating_format = floating_format_e::general,
      char point_ch = '.') noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().as_floating(val, accuracacy, upper_case, floating_format,
                               point_ch);
  }
  MJZ_CX_FN success_t append_with_insert_iter(auto &&fn) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().append_with_insert_iter(fn);
  }

  using bview = base_string_view_t<version_v>;

  MJZ_CX_FN bview
  to_base_view_pv_fn_(unsafe_ns::i_know_what_im_doing_t idk) const noexcept {
    return m_str().to_base_view_pv_fn_(idk);
  }
  using blazy_t = base_lazy_view_t<version_v>;
  MJZ_CX_FN blazy_t
  to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t idk) const noexcept {
    return m_str().to_base_lazy_pv_fn_(idk);
  }
  MJZ_CX_FN success_t format_back_insert_append_pv_fn_(
      unsafe_ns::i_know_what_im_doing_t idk, blazy_t v,
      replace_flags rp = replace_flags{}) noexcept {
    MJZ_UNUSED auto gard_ = prop_guard();
    return m_str().format_back_insert_append_pv_fn_(idk, v, rp);
  }

#if MJZ_WITH_iostream

  MJZ_NCX_FN friend std::ostream &operator<<(std::ostream &cout_v,
                                             const wrapped_string_t &obj) {
    return cout_v << obj.m_str();
  }
  MJZ_NCX_FN friend std::istream &operator>>(std::istream &cin_v,
                                             wrapped_string_t &obj) {
    MJZ_UNUSED auto gard_ = obj.prop_guard();
    return cin_v >> obj.m_str();
  }
  MJZ_NCX_FN friend std::istream &getline(std::istream &cin_v,
                                          wrapped_string_t &obj,
                                          char delim = '\n') {
    MJZ_UNUSED auto gard_ = obj.prop_guard();
    return getline(cin_v, obj.m_str(), delim);
  }
#endif  // MJZ_WITH_iostream
};
namespace litteral_ns {
/*
 *makes a gengeric wrapped_string_t  that views the string
 */
template <str_litteral_t L, version_t vr, wrapped_props_t props_v>
MJZ_CE_FN wrapped_string_t<vr, props_v> operator_wstr() noexcept
  requires(!std::is_empty_v<wrapped_string_t<vr, props_v>>)
{
  return wrapped_string_t<vr, props_v>(operator_view<L, vr>());
};

};  // namespace litteral_ns
};  // namespace mjz::bstr_ns
template <mjz::version_t version_v, mjz::bstr_ns::wrapped_props_t props_v>
struct std::hash<mjz::bstr_ns::wrapped_string_t<version_v, props_v>> {
  std::size_t operator()(const auto &s) const noexcept {
    return std::size_t(s.hash());
  }
};

#endif  // MJZ_BYTE_STRING_wraped_string_LIB_HPP_FILE_
