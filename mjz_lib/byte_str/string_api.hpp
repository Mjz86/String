
#include "string_abi.hpp"

#ifndef MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_
#define MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_

namespace mjz ::bstr_ns {

template <version_t version_v, bool has_alloc_v_>
struct basic_str_t : void_struct_t {
 private:
  template <class>
  friend class mjz_private_accessed_t;

  template <version_t, wrapped_props_t>
  friend struct wrapped_string_t;

 public:
  using self_t = basic_str_t<version_v, has_alloc_v_>;

  using mut_ref_t = basic_str_abi_ns_::mut_ref_t<version_v>;
  using mut_iterator = basic_str_abi_ns_::mut_iterator_t<version_v>;
  using traits_type = byte_traits_t<version_v>;
  using value_type = char;
  using pointer = const char *;
  using const_pointer = const char *;
  using reference = const char &;
  using const_reference = const char &;
  using const_iterator = iterator_t<const self_t>;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
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

 private:
  MJZ_CONSTANT(dont_mess_up_t)
  dont_mess_up = unsafe_ns::unsafe_v;

 private:
  MJZ_CX_FN void reset_to_error_on_fail(success_t op,
                                        static_string_view view) noexcept;
  MJZ_CX_ND_FN success_t copy_assign_data(const basic_str_t &str,
                                          bool no_allocate,
                                          uintlen_t offset = 0,
                                          uintlen_t count = nops) noexcept;
  MJZ_CX_ND_FN success_t reset(cheap_str_info &info) noexcept;
  MJZ_CX_ND_FN success_t total_reset(bool keep_flags) noexcept;
  MJZ_CX_ND_FN success_t move_init(basic_str_t &&str) noexcept;
  MJZ_CX_ND_FN success_t init_view(const generic_string_view &view) noexcept;

 public:
  MJZ_CX_ND_FN success_t share_init(const basic_str_t &str,
                                    bool no_allocate = false,
                                    uintlen_t offset = 0,
                                    uintlen_t count = nops) noexcept;

  MJZ_CX_ND_FN success_t copy_assign(const basic_str_t &obj,
                                     bool no_allocate = false,
                                     uintlen_t offset = 0,
                                     uintlen_t count = nops) noexcept;

 private:
  m_t m;

 public:
  /*
   *gets the internal data without permission
   */
  MJZ_CX_FN m_t &unsafe_handle(
      const dont_mess_up_t &,
      const dont_mess_up_t &) noexcept; /*
                                         *gets the internal data
                                         *without permission
                                         */
  MJZ_CX_FN const m_t &unsafe_handle(const dont_mess_up_t &,
                                     const dont_mess_up_t &) const noexcept;
  MJZ_CX_FN void uniqe_inout_v() && noexcept = delete;
  MJZ_CX_FN void uniqe_inout_v() const & noexcept = delete;
  MJZ_CX_FN void uniqe_inout_v() const && noexcept = delete;
  /*
   * can be used for a function that needs uniqe ownership of data, but will
   * give the ownership back
   */
  MJZ_CX_ND_FN str_t &&uniqe_inout_v() & noexcept;
  /*
   * can be used for a function that needs uniqe ownership of const data, but
   * will give the ownership back
   */
  MJZ_CX_ND_FN const str_t &&uniqe_in_v()
      const noexcept; /*
                       *like to std::move(obj)
                       */
  MJZ_CX_ND_FN str_t &&move_v()
      & noexcept; /*
                   *like to used to make the share constructor work
                   */
  MJZ_CX_ND_FN const str_t &&share_v() const noexcept;

 private:
  MJZ_CX_ND_FN basic_str_t(cheap_str_info &info) noexcept;

 public:
  MJZ_CX_ND_FN explicit basic_str_t(nullptr_t) noexcept;
  MJZ_CX_ND_FN basic_str_t() noexcept;

  MJZ_CX_FN ~basic_str_t() noexcept;
  MJZ_CX_ND_FN basic_str_t(cheap_str_info &&info) noexcept;
  /*
   *
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &, owned_stack_buffer &&stack_buffer,
              cheap_str_info &&info, uintlen_t byte_offset = 0,
              uintlen_t byte_count = 0) noexcept;
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &ok, owned_stack_buffer &&stack_buffer,
              uintlen_t byte_offset = 0, uintlen_t byte_count = 0) noexcept;
  /*
   * initilizes the string as a view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &,
              generic_string_view
                  view) noexcept; /*
                                   * reserves space if appropreate then copies,
                                   * if not initilizes the string as a view
                                   */
  MJZ_CX_ND_FN basic_str_t(const dont_mess_up_t &, generic_string_view view,
                           cheap_str_info &&info) noexcept;
  /*
   * reserves space if appropreate then copies, if not
   * initilizes the string as a   static  view (string litteral view)
   *
   */
  MJZ_CX_ND_FN basic_str_t(static_string_view view,
                           cheap_str_info &&info) noexcept;
  /*
   * initilizes the string as a   static  view (string litteral view)
   */
  MJZ_CX_ND_FN basic_str_t(static_string_view view) noexcept;
  /*
   * reserves space if appropreate then copies, if not
   *initilizes the string as a non sharable dynamic view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &ok, dynamic_string_view view,
              cheap_str_info &&info) noexcept;

  /*
   * initilizes the string as a non sharable dynamic view
   */
  MJZ_CX_ND_FN
  basic_str_t(const dont_mess_up_t &ok, dynamic_string_view view) noexcept;
  /*
   * moves the content of obj in
   * then makes the obj empty
   */
  MJZ_CX_ND_FN basic_str_t(basic_str_t &&obj) noexcept;
  /*
   *safely copies the data without allocation of the obj if possible, if not
   *,shares it if possible, if not copies it with an allocation.
   */
  MJZ_CX_ND_FN basic_str_t(const basic_str_t &obj) noexcept;
  /*
   * its visibly similar to   str(const&)
   *safely shares the data of the obj if possible, if not ,copies it.
   */
  MJZ_CX_ND_FN basic_str_t(const basic_str_t &&obj) noexcept;
  /*
   * reserves info.reserve_capacity if appropriate
   *safely copies the data without allocation of the obj if possible, if not
   *,shares it if possible, if not copies it with an allocation.
   */
  MJZ_CX_ND_FN basic_str_t(const basic_str_t &obj,
                           cheap_str_info &&info) noexcept;
  MJZ_DEPRECATED_R("the cheap_str_info is used but discarded afterwards")
  MJZ_CX_FN basic_str_t(basic_str_t &&obj,
                        cheap_str_info &&info) noexcept = delete;

  MJZ_DEPRECATED_R("the cheap_str_info is used but discarded afterwards")
  MJZ_CX_FN
  basic_str_t(const basic_str_t &&obj, cheap_str_info &&info) noexcept = delete;
  /*
   * reserves space if appropreate then copies, if not
   * initilizes the string as "c" in sso
   */
  MJZ_CX_ND_FN basic_str_t(char c, cheap_str_info &&info) noexcept;
  /*
   * initilizes the string as "c" in sso
   */
  MJZ_CX_ND_FN basic_str_t(char c) noexcept; /*
                                              * deallocates if needed,
                                              * moves the content of obj in
                                              * then makes the obj empty
                                              */
  MJZ_CX_FN basic_str_t &operator=(char c) noexcept;
  /*
   * deallocates if needed,
   * moves the content of obj in
   * then makes the obj empty
   */
  MJZ_CX_FN basic_str_t &operator=(basic_str_t &&obj) noexcept;
  /*
   *safely copies the data without allocation of the obj if possible, if not
   *,shares it if possible, if not copies it with an allocation.
   */
  MJZ_CX_FN basic_str_t &operator=(const basic_str_t &obj) noexcept;
  /*
   * its visibly similar to  operator=(const&)
   *safely shares the data of the obj if possible, if not ,copies it.
   */
  MJZ_CX_FN basic_str_t &operator=(const basic_str_t &&obj) noexcept;

 private:
  /*
   *maxes out the out of bounds indexes
   */
  MJZ_CX_FN bool make_right_then_give_has_null(
      uintlen_t &byte_offset, uintlen_t &byte_count) const noexcept;

 private:
  MJZ_CX_ND_FN success_t make_substrview_helper_(
      const basic_str_t &obj, uintlen_t byte_offset, uintlen_t byte_count,
      bool propgate_alloc = true, bool unsafe_assume_static_ = false) noexcept;

 public:
  /*
   *gives the allocator reference.
   */
  MJZ_CX_ND_FN const alloc_ref &get_alloc() const noexcept;
  /*
   *makes a substring that is from range of :
   * [byte_offset,byte_count+byte_offset)
   * if try_to_add_null then we attempt to add a null if is an owner, accessing
   *the buffer,pothentially bad.
   */
  MJZ_CX_FN success_t as_substring(uintlen_t byte_offset, uintlen_t byte_count,
                                   bool try_to_add_null = true) noexcept;
  /* similar to as_substring*/
  MJZ_CX_ND_FN basic_str_t
  make_substring(uintlen_t byte_offset, uintlen_t byte_count,
                 bool try_to_add_null = true) const noexcept;
  /*similar to make_substring but gives range from [begin_i,end_i) */
  MJZ_CX_ND_FN basic_str_t operator()(uintlen_t begin_i = 0,
                                      uintlen_t end_i = nops) noexcept;

  /* similar to as_substring*/
  MJZ_CX_FN basic_str_t &to_substring(uintlen_t byte_offset,
                                      uintlen_t byte_count,
                                      bool try_to_add_null = true) noexcept;

  void data() && = delete;
  void data() const && = delete;
  /* gives a pointer to the data */
  MJZ_CX_ND_FN const char *data() const & noexcept;
  /*
   *is the data a "valid" C string?
   * (does it have a null terminator(='\0') at the end?
   */
  MJZ_CX_ND_FN bool has_null() const noexcept;
  /* gives  length of the data */
  MJZ_CX_ND_FN uintlen_t length() const noexcept;
  /* gives  encoding of the data */
  MJZ_CX_ND_FN encodings_e
  get_encoding() const noexcept; /* sets  encoding of the data */
  MJZ_CX_FN void set_encoding(const dont_mess_up_t &,
                              encodings_e encoding) noexcept;

  MJZ_CONSTANT(uintlen_t)
  npos{traits_type::npos};
  MJZ_CONSTANT(uintlen_t)
  nops{traits_type::npos};
  MJZ_CX_FN static uintlen_t max_size() noexcept;
  MJZ_CX_ND_FN std::optional<char> at(const uintlen_t i) const noexcept;
  /*
   *returns the element at i
   */
  MJZ_CX_ND_FN std::optional<char> operator[](const uintlen_t i) const noexcept;

  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) const && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_subview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                    bool = false) && = delete;

  /* similar to make_substrview, exept this even views the SSO */
  MJZ_CX_ND_FN generic_string_view
  make_subview(const dont_mess_up_t &, uintlen_t byte_offset = 0,
               uintlen_t byte_count = nops,
               bool unsafe_assume_static_ = false) const & noexcept;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) && = delete;
  MJZ_DEPRECATED_R("clear lifetime issue")
  void make_substrview(const dont_mess_up_t &, uintlen_t, uintlen_t,
                       bool = true) const && = delete;
  /*
   * makes a view.
   * the view lifetime must be a subset of the string lifetime.
   * if unsafe_assume_static_ is true, the created view will not get a memcpy if
   * it was shared/copied by value. and the return type is treated as if it was
   * from a staticstring litteral. if propgate_alloc is false , the return will
   * not have any custom allocators(this is useless if the string has only
   * global allocators).
   */
  MJZ_CX_ND_FN basic_str_t
  make_substrview(const dont_mess_up_t &, uintlen_t byte_offset,
                  uintlen_t byte_count, bool propgate_alloc = true,
                  bool unsafe_assume_static_ = false) const & noexcept;
  /*
   * removes min(byte_count,length) from end
   */
  MJZ_CX_FN success_t remove_suffix(
      uintlen_t
          byte_count) noexcept; /*
                                 * removes min(byte_count,length) from begin
                                 */
  MJZ_CX_FN success_t remove_prefix(uintlen_t byte_count) noexcept;
  /*
   *copies the substring in buf.
   * buf must at least be of  min(byte_count,length)+ uintlen_t(add_null) in
   *size. if add_null then we add a null terminatior.
   */
  MJZ_CX_FN std::optional<uintlen_t> copy_bytes(
      uintlen_t byte_offset, uintlen_t byte_count, char *buf,
      bool add_null = false) const noexcept;
  /*
   * returns the number of bytes the string can modify in its buffer.
   */
  MJZ_CX_ND_FN uintlen_t capacity(bool must_owner = true) const noexcept;
  /*
   * if cap <= capacity(force_ownership) then returns.
   * else if allocate_exact
   *allocates minimum cap bytes of capacity.
   * else
   *allocates a "convinient round" number bigger than cap bytes of capacity.
   */
  MJZ_CX_FN success_t reserve(uintlen_t mincap, uintlen_t prefer_cap = 0,
                              const alloc_ref &alloc = m_t::empty_alloc,
                              replace_flags flags = replace_flags{}) noexcept;
  MJZ_CX_FN success_t consider_stack(const dont_mess_up_t &,
                                     owned_stack_buffer &&where) noexcept;

  MJZ_CX_FN success_t may_reconsider_stack(const dont_mess_up_t &idk,
                                           owned_stack_buffer &where,
                                           bool consider_null_) noexcept {
    if ((where.buffer_size < length() + uintlen_t(consider_null_)) ||
        (is_stacked() && m.buffer_location_ptr() == where.buffer) ||
        (where.buffer_size < capacity(true)))
      return true;
    return consider_stack(idk, std::move(where));
  }
  MJZ_CX_FN success_t shrink_to_fit(bool force_ownership = false) noexcept;

  MJZ_CX_FN success_t clear(bool force_ownership = false) noexcept;

  MJZ_CX_FN success_t add_null() noexcept;

  MJZ_CX_FN const char *as_c_str() & noexcept;
  /*
   *calculates the hash
   */
  MJZ_CX_ND_FN hash_t hash() const noexcept;
  /*
   *checks to see if modification of the data is allowed
   */
  MJZ_CX_ND_FN bool is_owner() const noexcept;
  /*
   *gets the falg bits
   */
  MJZ_CX_ND_FN auto get_states() const noexcept;
  MJZ_CX_FN bool get_threaded() const noexcept;
  MJZ_CX_FN bool is_stacked() const noexcept {
    return !m.is_sso() &&
           !m.template d_get_cntrl<bool>(my_details::is_sharable) &&
           m.non_sso_buffer_location_ptr();
  }

 private:
 public:
  /*
   * replacas the data in range [offset,offset+byte_count) with other.
   *failes if the string object doesnt satisfy the criteria of rep_flags
   * or
   * if allocation fails
   */

  MJZ_CX_ND_FN success_t replace_data_with_char(
      uintlen_t offset, uintlen_t byte_count, uintlen_t length_of_val,
      std::optional<char> val, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept;

  MJZ_CX_ND_FN success_t
  as_ownerized(const alloc_ref &val_alloc = m_t::empty_alloc,
               replace_flags rep_flags = replace_flags{}) noexcept {
    rep_flags.force_ownership = true;
    return replace_data_with_char(0, 0, 0, nullopt, val_alloc, rep_flags);
  }
  MJZ_CX_ND_FN success_t as_always_ownerized(
      bool flag_state_, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept {
    if (!flag_state_) {
      m.d_set_cntrl(my_details::is_ownerized, false);
      return true;
    }
    rep_flags.ownerization_v =
        replace_flags::ownerization_e::always_ownerize_on;
    return replace_data_with_char(0, 0, 0, nullopt, val_alloc, rep_flags);
  }
  template <class R_t>
    requires std::ranges::sized_range<R_t> && std::ranges::forward_range<R_t>
  MJZ_CX_ND_FN success_t
  replace_data_with_range(uintlen_t offset, uintlen_t byte_count, R_t &&r,
                          const alloc_ref &val_alloc = m_t::empty_alloc,
                          replace_flags rep_flags = replace_flags{}) noexcept;
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  replace_data_with_range(uintlen_t offset, uintlen_t byte_count, R_t &&r,
                          const alloc_ref &val_alloc = m_t::empty_alloc,
                          replace_flags rep_flags = replace_flags{}) noexcept;

  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t insert_data_with_range(
      uintlen_t offset, R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept;
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  append_data_with_range(R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
                         replace_flags rep_flags = replace_flags{}) noexcept;
  template <std::ranges::forward_range R_t>
  MJZ_CX_ND_FN success_t
  assign_data_with_range(R_t &&r, const alloc_ref &val_alloc = m_t::empty_alloc,
                         replace_flags rep_flags = replace_flags{}) noexcept;

  MJZ_CX_ND_FN success_t replace_data(
      uintlen_t offset, uintlen_t byte_count, const basic_str_t &&other,
      replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t
  replace_data(uintlen_t offset, uintlen_t byte_count, const basic_str_t &other,
               replace_flags rep_flags = replace_flags{}) noexcept;

  MJZ_CX_ND_FN success_t
  erase_data(uintlen_t offset, uintlen_t byte_count, const alloc_ref &val_alloc,
             replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t
  insert_data(uintlen_t offset, const basic_str_t &other,
              replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t insert_data_with_char(
      uintlen_t offset, uintlen_t length_of_val, std::optional<char> val,
      const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t
  append_data(const basic_str_t &other,
              replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t
  append_data_with_char(uintlen_t length_of_val, std::optional<char> val,
                        const alloc_ref &val_alloc = m_t::empty_alloc,
                        replace_flags rep_flags = replace_flags{}) noexcept;

  MJZ_CX_ND_FN success_t
  push_back(std::optional<char> c,
            replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t
  push_front(std::optional<char> c,
             replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN std::optional<char> pop_back() noexcept;
  MJZ_CX_ND_FN std::optional<char> pop_front() noexcept;
  MJZ_CX_ND_FN success_t assign_own(const basic_str_t &other,
                                    bool no_allocate = false) noexcept;
  MJZ_CX_ND_FN success_t assign(const basic_str_t &other,
                                bool no_allocate = false) noexcept;
  MJZ_CX_ND_FN success_t assign_share(const basic_str_t &other,
                                      bool no_allocate = false) noexcept;
  MJZ_CX_ND_FN success_t assign_move(basic_str_t &&other) noexcept;
  MJZ_CX_ND_FN success_t assign(char c, bool no_allocator = false) noexcept;
  /*
   *its as if you do this :
   * dest.replace_data(dest_byte_replace_offset, dest_byte_replace_count,
   *this->make_substring(src_byte_offset, src_byte_count),
   *dest_replace_flags);
   */
  MJZ_CX_FN success_t get_substr(
      uintlen_t src_byte_offset, uintlen_t src_byte_count, basic_str_t &&dest,
      uintlen_t dest_byte_replace_offset, uintlen_t dest_byte_replace_count,
      replace_flags dest_replace_flags = replace_flags{}) const & noexcept;
  /*
   *its as if you do this :
   * dest.replace_data(dest_byte_replace_offset, dest_byte_replace_count,
   *this->make_substring(src_byte_offset, src_byte_count), dest_replace_flags);
   * but the this object is pothentially moved out.
   */
  MJZ_CX_FN success_t get_substr(
      uintlen_t src_byte_offset, uintlen_t src_byte_count, basic_str_t &&dest,
      uintlen_t dest_byte_replace_offset, uintlen_t dest_byte_replace_count,
      replace_flags dest_replace_flags = replace_flags{}) && noexcept;
  /*
   * gives a mutable buffer (begin and length) with the sames values as the
   * currunnt buffer that will be in the buffer after the callback exit.
   */
  MJZ_CX_ND_FN success_t work_with_mut_data(
      callable_c<success_t(const mut_ref_t &&temp_mut_ref) noexcept> auto
          &&read_writer_fn,
      replace_flags flags = replace_flags{}) noexcept;
  /*
   *     (begin and length)
   */
  MJZ_CX_ND_FN success_t
  work_with_data(callable_c<success_t(const basic_str_t<version_v, has_alloc_v_>
                                          &) noexcept> auto &&reader_fn);
  MJZ_CX_ND_FN success_t
  resize(uintlen_t new_len, std::optional<char> val = std::nullopt,
         bool force_ownership = false,
         const alloc_ref &val_alloc = m_t::empty_alloc,
         replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN success_t resize_and_overwrite(
      uintlen_t new_len,
      callable_c<success_t(const mut_ref_t &&range) noexcept> auto
          &&read_writer_fn,
      const alloc_ref &val_alloc = m_t::empty_alloc,
      replace_flags rep_flags = replace_flags{}) noexcept;
  MJZ_CX_ND_FN const_iterator begin() const noexcept;
  MJZ_CX_ND_FN const_iterator end() const noexcept;
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept;
  MJZ_CX_ND_FN const_iterator cend() const noexcept;

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept;
  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept;
  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept;

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept;

  MJZ_CX_ND_FN size_type size() const noexcept;

  MJZ_CX_ND_FN bool empty() const noexcept;

  MJZ_CX_ND_FN std::optional<char> front() const noexcept;
  MJZ_CX_ND_FN std::optional<char> back() const noexcept;

  MJZ_CX_FN void swap(basic_str_t &o) noexcept;

  MJZ_CX_ND_FN std::optional<intlen_t> compare(
      const basic_str_t &rhs) const noexcept;

  MJZ_CX_ND_FN bool starts_with(const basic_str_t &rhs) const noexcept;

  MJZ_CX_ND_FN bool starts_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept;

  MJZ_CX_ND_FN bool ends_with(const basic_str_t &rhs) const noexcept;
  MJZ_CX_ND_FN bool ends_with(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept;

  MJZ_CX_ND_FN bool contains(const basic_str_t &rhs) const noexcept;
  MJZ_CX_ND_FN bool contains(
      char rhs, encodings_e encoding = encodings_e{}) const noexcept;
  MJZ_CX_ND_FN uintlen_t find(const basic_str_t &rhs,
                              const uintlen_t offset = 0) const noexcept;
  MJZ_CX_ND_FN uintlen_t rfind(const basic_str_t &rhs,
                               const uintlen_t offset = nops) const noexcept;
  MJZ_CX_ND_FN uintlen_t find_first_of(
      const basic_str_t &rhs, const uintlen_t offset = 0) const noexcept;

  MJZ_CX_ND_FN uintlen_t find_last_of(
      const basic_str_t &rhs, const uintlen_t offset = nops) const noexcept;
  MJZ_CX_ND_FN uintlen_t find_first_not_of(
      const basic_str_t &rhs, const uintlen_t offset = 0) const noexcept;
  MJZ_CX_ND_FN uintlen_t find_last_not_of(
      const basic_str_t &rhs, const uintlen_t offset = nops) const noexcept;

  MJZ_CX_ND_FN uintlen_t
  find(char rhs, const uintlen_t offset = 0,
       encodings_e encoding = encodings_e{}) const noexcept;
  MJZ_CX_ND_FN uintlen_t
  rfind(char rhs, const uintlen_t offset = nops,
        encodings_e encoding = encodings_e{}) const noexcept;
  MJZ_CX_ND_FN uintlen_t
  find_first_of(char rhs, const uintlen_t offset = 0,
                encodings_e encoding = encodings_e{}) const noexcept;

  MJZ_CX_ND_FN uintlen_t
  find_last_of(char rhs, const uintlen_t offset = nops,
               encodings_e encoding = encodings_e{}) const noexcept;
  MJZ_CX_ND_FN uintlen_t
  find_first_not_of(char rhs, const uintlen_t offset = 0,
                    encodings_e encoding = encodings_e{}) const noexcept;
  MJZ_CX_ND_FN uintlen_t
  find_last_not_of(char rhs, const uintlen_t offset = nops,
                   encodings_e encoding = encodings_e{}) const noexcept;
  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_integral(uint8_t raidex = 0) const noexcept;
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_real_floating() const noexcept;
  template <std::floating_point T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_ND_FN std::optional<T> to_floating() const noexcept;
  MJZ_CX_FN success_t ensure_props(wrapped_props_t props_v) noexcept {
    if (props_v.is_ownerized) {
      if (!as_always_ownerized(true)) return false;
    }
    if (props_v.has_null) {
      if (!add_null()) return false;
    }
    return true;
  }

 public:
  MJZ_CX_FN success_t append_data_temp(basic_str_t &&str) noexcept;

  MJZ_CX_FN basic_str_t &operator+=(basic_str_t &&obj) noexcept;
  MJZ_CX_FN basic_str_t &operator+=(const basic_str_t &obj) noexcept;
  MJZ_CX_FN static basic_str_t operator_add(basic_str_t &rhs,
                                            basic_str_t &lhs) noexcept;

  MJZ_CX_FN bool is_error() const noexcept;

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  MJZ_CX_FN success_t as_integral(T val, const uint8_t raidex = 10,
                                  bool upper_case = false) noexcept;
  template <std::floating_point T>
  MJZ_CX_FN success_t as_floating(
      T val, uintlen_t accuracacy = sizeof(uintlen_t), bool upper_case = false,
      floating_format_e floating_format = floating_format_e::general,
      char point_ch = '.') noexcept;
  MJZ_CX_FN success_t append_with_insert_iter(
      callable_c<success_t(back_insert_iterator_t iter) noexcept> auto
          &&fn) noexcept;

  using bview = base_string_view_t<version_v>;

  MJZ_CX_FN bview
      to_base_view_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept;
  using blazy_t = base_lazy_view_t<version_v>;
  MJZ_CX_FN blazy_t
      to_base_lazy_pv_fn_(unsafe_ns::i_know_what_im_doing_t) const noexcept;
  MJZ_CX_FN success_t
  format_back_insert_append_pv_fn_(unsafe_ns::i_know_what_im_doing_t, blazy_t v,
                                   replace_flags rp = replace_flags{}) noexcept;
};

};  // namespace mjz::bstr_ns
#endif  // MJZ_BYTE_STRING_string_ABI_LIB_HPP_FILE_