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
#ifndef MJZ_SRC_UORD_VEC_base_FILE_
#define MJZ_SRC_UORD_VEC_base_FILE_
#include "byte_str/iterator.hpp"
namespace mjz {
// can be allocator aware and the capacity and size feilds of the vectors can be
// collapsed into one for tree and one for kw, but right now its not good enough
template <
    version_t version_v, class key_t, class value_t,
    callable_c<uintlen_t(const key_t &, uintlen_t seed) noexcept> hash_fn_t,
    uint8_t branching_power_v = 2>
struct unordered_vector_t {
  static_assert(branching_power_v < 6);

private:
  // any power of 2 works
  constexpr static inline uintlen_t shift_index_node{uintlen_t(1)
                                                     << branching_power_v};
  using childern_node_t = std::array<intlen_t, (1ull << shift_index_node)>;

  /*

  */

  std::vector<childern_node_t> m_flat_tree{};
  std::vector<value_t> m_values{};
  std::vector<key_t> m_keys{};
  std::vector<uintlen_t> hash_caches{};
  std::vector<intlen_t> m_reverse_tree_indexies{};
  std::vector<intlen_t> m_reverse_value_indexies{};
  hash_fn_t hash_fn{};

private:
  constexpr static uintlen_t mask = (uintlen_t(1ull) << shift_index_node) - 1;
  struct hasher_t {
    static_assert(std::has_single_bit(shift_index_node) &&
                  shift_index_node < sizeof(uintlen_t) * 8);
    constexpr static uintlen_t omask =
        ((sizeof(uintlen_t) * 8 / shift_index_node)) - 1;
    MJZ_CX_FN hasher_t(const key_t &k, const hash_fn_t &hf_) noexcept
        : key{std::addressof(k)}, hf{std::addressof(hf_)}, depth{0} {
      cache = get_hash();
    }
    MJZ_CX_FN hasher_t(std::optional<uintlen_t> hash_cache, const key_t &k,
                       const hash_fn_t &hf_, uintlen_t depth_) noexcept
        : key{std::addressof(k)}, hf{std::addressof(hf_)}, depth{depth_} {
      uintlen_t n = depth & omask;
      depth &= ~omask;
      if (hash_cache && n != depth) {
        hash_cache = std::nullopt;
      }

      if (!hash_cache) {
        hash_cache = get_hash();
      }

      cache = *hash_cache;
      cache >>= shift_index_node * n;
    }
    MJZ_CX_FN uintlen_t next() noexcept {
      depth++;
      cache >>= shift_index_node;
      if (!(depth & omask)) {
        cache = get_hash();
      }
      return current();
    }
    MJZ_CX_FN uintlen_t current() const noexcept { return cache & mask; }

    const key_t *key{};
    const hash_fn_t *hf{};
    uintlen_t cache{};
    uintlen_t depth{};

    MJZ_CX_FN uintlen_t get_hash() const noexcept { return (*hf)(*key, depth); }
  };

  MJZ_CX_FN auto place_find(hasher_t &h) const noexcept {
    struct ret_t {
      intlen_t node{-1};
      intlen_t parent{};
    } ret{};
    if (ret.node >= 0)
      return ret;
    do {
      ret.parent =
          std::exchange(ret.node, m_flat_tree[size_t(~ret.node)][h.current()]);
      if (ret.node >= 0)
        break;
      h.next();
    } while (true);
    return ret;
  }

public:
  MJZ_CX_FN unordered_vector_t(hash_fn_t &&hash_fn_ = hash_fn_t{}) noexcept
      : hash_fn{std::move(hash_fn_)} {
    m_flat_tree.reserve(256);
    m_reverse_tree_indexies.reserve(256);
    m_reverse_tree_indexies.emplace_back();
    m_flat_tree.emplace_back();
  }

  MJZ_CX_FN std::optional<uintlen_t> find(const key_t &key) const noexcept {
    hasher_t hr{key, hash_fn};
    auto [node, p] = place_find(hr);
    if (!node)
      return {};
    if (key != m_keys[size_t(node - 1)])
      return {};
    return uintlen_t(node - 1);
  };

  MJZ_CX_FN uintlen_t insert(key_t &&key, value_t &&value) noexcept {
    hasher_t hr{key, hash_fn};
    uintlen_t hash_cache = hr.cache;
    auto [node, parent] = place_find(hr);

    if (node) {
      if (key == m_keys[size_t(node - 1)])
        return uintlen_t(node - 1);
      hasher_t sibl_hr{hash_caches[size_t(node - 1)], m_keys[size_t(node - 1)],
                       hash_fn, hr.depth};
      while (true) {
        m_flat_tree[size_t(~parent)][hr.current()] =
            ~intlen_t(m_flat_tree.size());
        m_reverse_tree_indexies.push_back(
            intlen_t(hr.current()) + intlen_t((~parent) << shift_index_node));
        m_flat_tree.emplace_back();
        parent = -intlen_t(m_flat_tree.size());
        if (sibl_hr.next() != hr.next())
          MJZ_MOSTLY_LIKELY { break; }
      };
      m_reverse_value_indexies[size_t(node - 1)] =
          intlen_t(sibl_hr.current()) + intlen_t((~parent) << shift_index_node);
      m_flat_tree.back()[sibl_hr.current()] = node;
    };
    m_values.push_back(std::move(value));
    m_keys.push_back(std::move(key));
    hash_caches.push_back(std::move(hash_cache));
    m_reverse_value_indexies.push_back(intlen_t(hr.current()) +
                                       intlen_t((~parent) << shift_index_node));
    node = intlen_t(m_values.size());
    m_flat_tree[size_t(~parent)][hr.current()] = node;
    return uintlen_t(node - 1);
  }
  MJZ_CX_FN void erase(const key_t &key) noexcept {
    hasher_t hr{key, hash_fn};
    auto [node, parent] = place_find(hr);
    if (!node || key != m_keys[size_t(node - 1)])
      return;
    intlen_t back_i_r = m_reverse_value_indexies.back();
    intlen_t node_i_r = m_reverse_value_indexies[size_t(node - 1)];
    if (back_i_r != node_i_r) {
      m_reverse_value_indexies[size_t(node - 1)] = back_i_r;
      m_values[size_t(node - 1)] = std::move(m_values.back());
      m_keys[size_t(node - 1)] = std::move(m_keys.back());
      hash_caches[size_t(node - 1)] = std::move(hash_caches.back());
      m_flat_tree[uintlen_t(back_i_r) >> shift_index_node]
                 [mask & uintlen_t(back_i_r)] = node;
    }
    m_flat_tree[uintlen_t(node_i_r) >> shift_index_node]
               [mask & uintlen_t(node_i_r)] = 0;
    m_reverse_value_indexies.pop_back();
    m_values.pop_back();
    m_keys.pop_back();
    hash_caches.pop_back();
    ///////////

    size_t parent_node = size_t(node_i_r >> shift_index_node);
    while (parent_node) {
      if (childern_node_t{} != m_flat_tree[parent_node]) {
        break;
      }
      if ((1 + parent_node) != m_flat_tree.size()) {
        node_i_r = std::exchange(m_reverse_tree_indexies[parent_node],
                                 m_reverse_tree_indexies.back());
        m_flat_tree[parent_node] = m_flat_tree.back();
        intlen_t i{};
        for (intlen_t child : m_flat_tree[parent_node]) {
          auto val = i + (intlen_t(parent_node) << shift_index_node);

          if (0 < child) {
            m_reverse_value_indexies[size_t(child - 1)] = val;
          }
          if (child < 0) {
            m_reverse_tree_indexies[size_t(~child)] = val;
          }
          i++;
        }
        back_i_r = m_reverse_tree_indexies.back();
        m_flat_tree[size_t(back_i_r) >> shift_index_node]
                   [mask & uintlen_t(back_i_r)] = ~intlen_t(parent_node);
      } else {
        node_i_r = m_reverse_tree_indexies.back();
      }
      parent_node = size_t(node_i_r) >> shift_index_node;
      m_flat_tree[parent_node][mask & uintlen_t(node_i_r)] = 0;
      m_reverse_tree_indexies.pop_back();
      m_flat_tree.pop_back();
    };
  }

  MJZ_CX_FN std::span<value_t> values() noexcept { return m_values; }
  // dont modify the key!.. its not going to end well
  // MJZ_CX_FN std::span<key_t> keys() noexcept { return m_keys; };
  MJZ_CX_FN std::span<const value_t> values() const noexcept {
    return m_values;
  }
  MJZ_CX_FN std::span<const key_t> keys() const noexcept { return m_keys; };
  MJZ_CX_FN pair_t<const key_t &, const value_t &>
  operator[](uintlen_t i) const noexcept {
    return {keys()[i], values()[i]};
  }
  MJZ_CX_FN pair_t<const key_t &, value_t &> operator[](uintlen_t i) noexcept {
    return {keys()[i], values()[i]};
  }

  MJZ_CX_ND_FN uintlen_t size() const noexcept { return values().size(); }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    constexpr auto sz = std::vector<childern_node_t>().max_size();
    return sz >> shift_index_node;
  }

  using value_type = pair_t<key_t, value_t>;
  MJZ_random_access_interface_of(unordered_vector_t);
};

template <version_t version_v, class key_t, class value_t>
struct unordered_vector_str_t
    : unordered_vector_t<
          version_v, key_t, value_t,
          decltype([](const key_t &key, uintlen_t seed) noexcept -> uintlen_t {
            return hash_bytes_t<version_v>::template hash_bytes<uintlen_t>(
                key.data(), key.size(), 0xc70f6907UL ^ seed);
          })> {};

template <version_t version_v, class intern_uinthash_t,
          uintlen_t level_bit_count_v = 4,
          std::signed_integral int_index_t = intlen_t>
struct interning_vector_t {
  static_assert(level_bit_count_v < 64 && level_bit_count_v);
  constexpr static inline uintlen_t level_count_v = uintlen_t(1)
                                                    << level_bit_count_v;
  using level_t = std::array<int_index_t, level_count_v>;
  std::vector<level_t> m_flat_tree{};
  std::vector<intern_uinthash_t> m_hashes{};
  struct hasher_t {
    MJZ_CX_FN hasher_t(intern_uinthash_t hash, uintlen_t depth_ = 0) noexcept
        : cache{hash}, depth{depth_} {}
    MJZ_CX_FN uintlen_t next() noexcept {
      depth += level_bit_count_v;
      return current();
    }
    MJZ_CX_FN uintlen_t current() const noexcept {
      constexpr static uintlen_t omask = level_count_v - 1;
      return uintlen_t(cache >> depth) & omask;
    }
    intern_uinthash_t cache{};
    uintlen_t depth{};
  };

  MJZ_CX_FN static uintlen_t
  reserve_tree_cap_heuristic(uintlen_t count) noexcept {
    constexpr double factor = 1.65 / double(level_bit_count_v);
    constexpr double offset = double(uintlen_t(1) << level_bit_count_v);
    return uintlen_t(double(count) * factor + offset);
  }
  MJZ_CX_FN void reserve(uintlen_t count) noexcept {
    reserve(count, reserve_tree_cap_heuristic(count));
  }
  MJZ_CX_FN void reserve(uintlen_t count, uintlen_t count_flat_tree) noexcept {
    m_hashes.reserve(count);
    m_flat_tree.reserve(count_flat_tree);
  }
  MJZ_CX_FN interning_vector_t(uintlen_t res = 1) noexcept {
    reserve(res);
    m_flat_tree.emplace_back();
  }
  struct place_find_ret_t {
    int_index_t node{-1};
    int_index_t parent{};
  };
  MJZ_CX_FN place_find_ret_t place_find(hasher_t &h) const noexcept {
    place_find_ret_t ret{};
    do {
      ret.parent =
          std::exchange(ret.node, m_flat_tree[size_t(~ret.node)][h.current()]);
      if (ret.node >= 0)
        break;
      h.next();
    } while (true);
    return ret;
  }

  MJZ_CX_FN std::optional<uintlen_t>
  find(intern_uinthash_t key) const noexcept {
    hasher_t hr{key};
    auto [node, p] = place_find(hr);
    if (!node)
      return {};
    if (key != m_hashes[size_t(node - 1)])
      return {};
    return uintlen_t(node - 1);
  };

  MJZ_CX_FN uintlen_t insert(intern_uinthash_t key) noexcept {
    hasher_t hr{key};

    auto [node, parent] = place_find(hr);

    if (node) {
      if (key == m_hashes[size_t(node - 1)])
        return uintlen_t(node - 1);
      hasher_t sibl_hr{m_hashes[size_t(node - 1)], hr.depth};
      do {
        m_flat_tree[size_t(~parent)][hr.current()] =
            ~int_index_t(m_flat_tree.size());
        m_flat_tree.emplace_back();

        asserts(intlen_t(m_flat_tree.size()) ==
                int_index_t(m_flat_tree.size()));

        parent = -int_index_t(m_flat_tree.size());
      } while (sibl_hr.next() == hr.next());
      m_flat_tree.back()[sibl_hr.current()] = node;
    };
    m_hashes.push_back(std::move(key));

    asserts(intlen_t(m_hashes.size()) == int_index_t(m_hashes.size()));

    m_flat_tree[size_t(~parent)][hr.current()] = node =
        int_index_t(m_hashes.size());
    return uintlen_t(node - 1);
  }

  MJZ_CX_FN std::span<const intern_uinthash_t> values() const noexcept {
    return m_hashes;
  }
  MJZ_CX_FN const intern_uinthash_t &operator[](uintlen_t i) const noexcept {
    return m_hashes[i];
  }

  MJZ_CX_ND_FN uintlen_t size() const noexcept { return values().size(); }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    constexpr auto sz = std::vector<level_t>().max_size();
    return sz >> level_bit_count_v;
  }

  using value_type = intern_uinthash_t;
  MJZ_random_access_interface_of(interning_vector_t);
};

template <version_t version_v, class intern_uinthash_t,
          uintlen_t level_bit_count_extra_v = 0,
          std::unsigned_integral uint_index_t = uintlen_t>
struct leveled_interning_vector_t {
  constexpr static inline uintlen_t level_bit_count_min_v =
      uintlen_t(std::countr_zero(sizeof(uint_index_t) * 8));
  constexpr static inline uintlen_t level_bit_count_v =
      level_bit_count_min_v + level_bit_count_extra_v;
  struct level_t {
    uint_index_t index{};
    using pdata_t =
        std::array<uint_index_t,
                   size_t(uint_index_t(1) << level_bit_count_extra_v)>;
    pdata_t population_data{};
    using uintp_t = uint_min_N_t<version_v, sizeof(pdata_t) * 8>;
    MJZ_CX_FN auto population() const noexcept {
      return std::bit_cast<uintp_t>(population_data);
    }
    MJZ_CX_FN
    bool is_leaf() const noexcept { return !population(); }
    MJZ_CX_FN uint_index_t size() const noexcept {
      return uint_index_t(popcount(population()));
    }
    MJZ_CX_FN uint_index_t capacity() const noexcept {
      uint_index_t sz = size();
      return uint_index_t(sz ? std::bit_ceil(sz) : 0);
    }
    MJZ_CX_FN uint_index_t next_index_in_free_list() const noexcept {
      return uint_index_t(std::bit_width(size()));
    }

    MJZ_CX_FN void born(uint_index_t virtual_i) noexcept {
      population_data = std::bit_cast<pdata_t>(
          make_set_nth_bit(population(), virtual_i, true));
    }

    MJZ_CX_FN uint_index_t
    child_place_impl(uint_index_t virtual_i) const noexcept {
      return uint_index_t(index +
                          uint_index_t(popcount(population(), 0, virtual_i)));
    }
    MJZ_CX_FN std::optional<uint_index_t>
    child_index(uint_index_t virtual_i) const noexcept {
      if (nth_bit(population(), virtual_i))
        return child_place_impl(virtual_i);
      return {};
    }
  };
  std::array<std::vector<uint_index_t>, level_bit_count_v> free_list{};
  std::vector<level_t> m_semi_flat_tree{};
  std::vector<intern_uinthash_t> m_hashes{};
  struct hasher_t {
    MJZ_CX_FN hasher_t(intern_uinthash_t hash, uintlen_t depth_ = 0) noexcept
        : cache{hash}, depth{depth_} {}
    MJZ_CX_FN uintlen_t next() noexcept {
      depth += level_bit_count_v;
      asserts(depth < sizeof(intern_uinthash_t) * 8);
      return current();
    }
    MJZ_CX_FN uint_index_t current() const noexcept {
      constexpr static uint_index_t omask =
          (uint_index_t(1) << level_bit_count_v) - 1;
      return uint_index_t(cache >> depth) & omask;
    }
    intern_uinthash_t cache{};
    uintlen_t depth{};
  };

  MJZ_CX_FN static uintlen_t
  reserve_tree_cap_heuristic(uintlen_t count) noexcept {
    return uintlen_t((count * 3 >> 1) + (uintlen_t(1) << level_bit_count_v));
  }
  MJZ_CX_FN void reserve(uintlen_t count) noexcept {
    reserve(count, reserve_tree_cap_heuristic(count),
            uintlen_t(1) << level_bit_count_v);
  }
  MJZ_CX_FN void reserve(uintlen_t count, uintlen_t count_flat_tree,
                         uintlen_t free_list_reserves) noexcept {
    m_hashes.reserve(count);
    m_semi_flat_tree.reserve(count_flat_tree);
    for (auto &vec : free_list) {
      vec.reserve(free_list_reserves);
    }
  }
  MJZ_CX_FN leveled_interning_vector_t(uintlen_t res = 1) noexcept {
    reserve(res);
    m_semi_flat_tree.emplace_back();
  }

  MJZ_CX_FN pair_t<uint_index_t, std::optional<level_t>>
  place_find(hasher_t &h) const noexcept {
    uint_index_t ret{};

    level_t prev = m_semi_flat_tree[ret];
    for (;;) {

      std::optional<uint_index_t> opt = prev.child_index(h.current());

      if (!opt)
        return {ret, {}};

      prev = m_semi_flat_tree[ret = *opt];
      if (prev.is_leaf())
        break;

      h.next();
    };

    return {ret, prev};
  }

  MJZ_CX_FN std::optional<uintlen_t>
  find(intern_uinthash_t key) const noexcept {
    hasher_t hr{key};

    auto [node, lvl] = place_find(hr);

    if (!lvl)
      return {};

    if (key != m_hashes[lvl->index])
      return {};

    return lvl->index;
  };

  MJZ_CX_FN uint_index_t add_leaf_to_node_init_impl(
      uint_index_t parent_i, uint_index_t virtual_i) noexcept {
    level_t lvl = m_semi_flat_tree[parent_i];
    asserts(lvl.is_leaf());
    if (!free_list[0].size()) {
      lvl.index = uint_index_t(m_semi_flat_tree.size());
      m_semi_flat_tree.emplace_back();
      asserts(intlen_t(m_semi_flat_tree.size()) ==
              uint_index_t(m_semi_flat_tree.size()));

    } else {
      lvl.index = free_list[0].back();
      free_list[0].pop_back();
      m_semi_flat_tree[lvl.index] = level_t{};
    }
    lvl.born(virtual_i);
    m_semi_flat_tree[parent_i] = lvl;
    return lvl.index;
  }
  MJZ_CX_FN uint_index_t add_non_existing_child_to_node_impl(
      uint_index_t parent_i, uint_index_t virtual_i,
      uint_index_t child_i) noexcept {
    level_t lvl = m_semi_flat_tree[parent_i];
    uint_index_t sz = lvl.size();
    uint_index_t cap = lvl.capacity();
    if (sz < cap) {
      auto end_it = m_semi_flat_tree.begin() + ptrdiff_t(lvl.index + sz);
      uint_index_t ch_i = lvl.child_place_impl(virtual_i);
      auto it = m_semi_flat_tree.begin() + ptrdiff_t(ch_i);
      std::move_backward(it, end_it, end_it + 1);
      *it = level_t{child_i, 0};
      lvl.born(virtual_i);
      m_semi_flat_tree[parent_i] = lvl;
      return lvl.index;
    }
    uint_index_t next_free_list_loc = lvl.next_index_in_free_list();
    uint_index_t index_new{};

    if (next_free_list_loc == free_list.size() ||
        !free_list[next_free_list_loc].size()) {
      index_new = uint_index_t(m_semi_flat_tree.size());
      m_semi_flat_tree.resize(m_semi_flat_tree.size() +
                              (size_t(1) << next_free_list_loc));

      asserts(intlen_t(m_semi_flat_tree.size()) ==
              uint_index_t(m_semi_flat_tree.size()));

    } else {
      index_new = free_list[size_t(next_free_list_loc)].back();
      free_list[size_t(next_free_list_loc)].pop_back();
    }
    if (next_free_list_loc) {
      free_list[size_t(next_free_list_loc) - 1].emplace_back(lvl.index);
      auto end_it = m_semi_flat_tree.begin() + ptrdiff_t(lvl.index + sz);
      auto begin_it = m_semi_flat_tree.begin() + ptrdiff_t(lvl.index);
      uint_index_t ch_i = lvl.child_place_impl(virtual_i);
      auto it = m_semi_flat_tree.begin() + ptrdiff_t(ch_i);
      auto new_begin = m_semi_flat_tree.begin() + ptrdiff_t(index_new);
      std::ranges::move(begin_it, it, new_begin);
      new_begin += it - begin_it;
      *new_begin = level_t{child_i, 0};
      std::ranges::move(it, end_it, ++new_begin);
    } else {
      m_semi_flat_tree[index_new] = level_t{child_i, 0};
    }
    lvl.born(virtual_i);
    lvl.index = index_new;
    m_semi_flat_tree[parent_i] = lvl;
    return index_new;
  }
  MJZ_CX_FN uintlen_t insert(intern_uinthash_t key) noexcept {
    hasher_t hr{key};

    auto [node, lvl_] = place_find(hr);

    if (lvl_) {
      level_t lvl = *lvl_;
      uint_index_t leaf_i = lvl.index;

      if (key == m_hashes[leaf_i])
        return leaf_i;

      hasher_t sibl_hr{m_hashes[leaf_i], hr.depth};
      while (sibl_hr.next() == hr.next())
        node = add_leaf_to_node_init_impl(node, hr.current());
      add_non_existing_child_to_node_impl(node, sibl_hr.current(), leaf_i);
    };
    asserts(intlen_t(m_hashes.size()) == uint_index_t(m_hashes.size()));

    add_non_existing_child_to_node_impl(node, hr.current(),
                                        uint_index_t(m_hashes.size()));

    m_hashes.push_back(std::move(key));

    return uintlen_t(m_hashes.size() - 1);
  }

  MJZ_CX_FN std::span<const intern_uinthash_t> values() const noexcept {
    return m_hashes;
  }
  MJZ_CX_FN const intern_uinthash_t &operator[](uintlen_t i) const noexcept {
    return m_hashes[i];
  }

  MJZ_CX_ND_FN uintlen_t size() const noexcept { return values().size(); }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    constexpr auto sz = std::vector<level_t>().max_size();
    return sz >> level_bit_count_v;
  }

  using value_type = intern_uinthash_t;
  MJZ_random_access_interface_of(leveled_interning_vector_t);
};

template <version_t version_v, class intern_uinthash_t,
          uintlen_t level_bit_count_v = 6,
          std::unsigned_integral uint_index_t = uintlen_t>
struct two_leveled_interning_vector_t {
  static_assert(!!level_bit_count_v);
  using int_index_t = std::make_signed_t<uint_index_t>;
  using uintp_t = uint_min_N_t<version_v, (uintlen_t(1) << level_bit_count_v)>;

  std::array<std::vector<uint_index_t>, level_bit_count_v> free_lvl1_list{};
  std::vector<uintp_t> m_semi_flat_lvl2_tree{};
  // + is m_hashes , - is index into m_semi_flat_lvl1_tree who points at a block
  // with first being index to m_semi_flat_lvl2_tree, rest being population
  // 0 is empty
  std::vector<int_index_t> m_semi_flat_lvl1_tree{};
  std::vector<intern_uinthash_t> m_hashes{};

  struct level_t {
    std::span<const uintp_t> m_semi_flat_lvl2_tree{};
    std::span<const int_index_t> m_semi_flat_lvl1_tree{};
    uint_index_t lvl1_index{};
    MJZ_CX_FN int_index_t lvl2_index() const noexcept {
      return m_semi_flat_lvl1_tree[lvl1_index];
    }
    MJZ_CX_FN uint_index_t population_lvl1_to_lvl2_index() const noexcept {
      return uint_index_t(~lvl2_index());
    }
    MJZ_CX_FN uint_index_t population_lvl1_index() const noexcept {
      return uint_index_t(-lvl2_index());
    }

    MJZ_CX_FN uint_index_t leaf_index() const noexcept {
      return uint_index_t(lvl2_index() - 1);
    }
    MJZ_CX_FN uintp_t population() const noexcept {
      return has_population()
                 ? m_semi_flat_lvl2_tree[uint_index_t(
                       m_semi_flat_lvl1_tree[population_lvl1_to_lvl2_index()])]
                 : uintp_t();
    }
    MJZ_CX_FN
    bool has_population() const noexcept { return lvl2_index() < 0; }
    MJZ_CX_FN
    bool is_root() const noexcept { return lvl1_index == 0; }
    MJZ_CX_FN
    bool is_tree() const noexcept { return has_population(); }
    MJZ_CX_FN
    bool is_leaf() const noexcept { return 0 < lvl2_index(); }
    MJZ_CX_FN
    bool is_empty() const noexcept { return 0 == lvl2_index(); }
    MJZ_CX_FN uint_index_t size() const noexcept {
      return has_population() ? uint_index_t(popcount(population()))
                              : uint_index_t();
    }
    MJZ_CX_FN uint_index_t capacity() const noexcept {
      if (is_root())
        return uint_index_t(1) << level_bit_count_v;
      return has_population() ? uint_index_t(std::bit_ceil(size()))
                              : uint_index_t();
    }
    MJZ_CX_FN uint_index_t next_index_in_free_list() const noexcept {
      if (is_root())
        return level_bit_count_v;
      return uint_index_t(std::bit_width(size()));
    }
    MJZ_CX_FN uint_index_t
    child_place_impl(uint_index_t virtual_i) const noexcept {
      return uint_index_t(population_lvl1_index() +
                          uint_index_t(popcount(population(), 0, virtual_i)));
    }
    MJZ_CX_FN std::optional<uint_index_t>
    child_index(uint_index_t virtual_i) const noexcept {
      if (has_population() && nth_bit(population(), virtual_i))
        return child_place_impl(virtual_i);
      return {};
    }
  };

  MJZ_CX_FN void born_to_tree_impl(uint_index_t lvl1_index,
                                   uint_index_t virtual_i) noexcept {
    uintp_t &u = m_semi_flat_lvl2_tree[uint_index_t(
        m_semi_flat_lvl1_tree[get_lvl_ref_impl(lvl1_index)
                                  .population_lvl1_to_lvl2_index()])];

    u = make_set_nth_bit(u, virtual_i, true);
  }
  MJZ_CX_FN level_t get_lvl_ref_impl(uint_index_t lvl1_index) const noexcept {
    return {m_semi_flat_lvl2_tree, m_semi_flat_lvl1_tree, lvl1_index};
  }

  struct hasher_t {
    MJZ_CX_FN hasher_t(intern_uinthash_t hash, uintlen_t depth_ = 0) noexcept
        : cache{hash}, depth{depth_} {}
    MJZ_CX_FN uintlen_t next() noexcept {
      depth += level_bit_count_v;
      asserts(depth < sizeof(intern_uinthash_t) * 8);
      return current();
    }
    MJZ_CX_FN uint_index_t current() const noexcept {
      constexpr static uint_index_t omask =
          (uint_index_t(1) << level_bit_count_v) - 1;
      return uint_index_t(cache >> depth) & omask;
    }
    intern_uinthash_t cache{};
    uintlen_t depth{};
  };
  MJZ_CX_FN static uintlen_t
  reserve_lvl1_cap_heuristic(uintlen_t count) noexcept {
    const uintlen_t root_footprint = 2 + (uintlen_t(1) << level_bit_count_v);
    return root_footprint + (count * 4);
  }

  MJZ_CX_FN static uintlen_t
  reserve_lvl2_cap_heuristic(uintlen_t count) noexcept {
    return (count * 100 >> level_bit_count_v) + 32;
  }

  MJZ_CX_FN void reserve(uintlen_t count) noexcept {
    reserve(count, reserve_lvl1_cap_heuristic(count),
            reserve_lvl2_cap_heuristic(count),
            uintlen_t(1) << level_bit_count_v);
  }

  MJZ_CX_FN void reserve(uintlen_t count, uintlen_t count_lvl1_tree,
                         uintlen_t count_lvl2_tree,
                         uintlen_t free_list_reserves) noexcept {
    m_hashes.reserve(count);
    m_semi_flat_lvl1_tree.reserve(count_lvl1_tree);
    m_semi_flat_lvl2_tree.reserve(count_lvl2_tree);

    for (auto &vec : free_lvl1_list) {
      vec.reserve(free_list_reserves);
    }
  }
  MJZ_CX_FN two_leveled_interning_vector_t(uintlen_t res = 1) noexcept {
    reserve(res);
    m_semi_flat_lvl2_tree.emplace_back();
    m_semi_flat_lvl1_tree.resize(2 + (uintlen_t(1) << level_bit_count_v), 0);
    m_semi_flat_lvl1_tree[0] = int_index_t(uint_index_t(~1));
  }

  MJZ_CX_FN uint_index_t place_find(hasher_t &h) const noexcept {
    uint_index_t ret{};
    for (;;) {
      std::optional<uint_index_t> opt =
          get_lvl_ref_impl(ret).child_index(h.current());
      if (!opt)
        break;
      ret = *opt;
      h.next();
    };
    return ret;
  }

  MJZ_CX_FN std::optional<uintlen_t>
  find(intern_uinthash_t key) const noexcept {
    hasher_t hr{key};
    level_t lvl = get_lvl_ref_impl(place_find(hr));
    if (!lvl.is_leaf() || key != m_hashes[lvl.leaf_index()])
      return {};
    return lvl.leaf_index();
  };

  MJZ_CX_FN uint_index_t enhabit_impl(uint_index_t parent_i,
                                      uint_index_t virtual_i) noexcept {
    level_t lvl = get_lvl_ref_impl(parent_i);
    asserts(!lvl.has_population());
    if (!free_lvl1_list[0].size()) {
      m_semi_flat_lvl1_tree[parent_i] =
          int_index_t(uint_index_t(~m_semi_flat_lvl1_tree.size()));
      m_semi_flat_lvl1_tree.resize(m_semi_flat_lvl1_tree.size() + 2);
      asserts(intlen_t(m_semi_flat_lvl1_tree.size()) ==
              int_index_t(m_semi_flat_lvl1_tree.size()));
      lvl = get_lvl_ref_impl(parent_i);
    } else {
      m_semi_flat_lvl1_tree[parent_i] =
          int_index_t(uint_index_t(~free_lvl1_list[0].back()));
      free_lvl1_list[0].pop_back();
    }
    m_semi_flat_lvl1_tree[lvl.population_lvl1_to_lvl2_index()] =
        int_index_t(uint_index_t(m_semi_flat_lvl2_tree.size()));
    m_semi_flat_lvl2_tree.emplace_back();
    asserts(intlen_t(m_semi_flat_lvl2_tree.size()) ==
            int_index_t(m_semi_flat_lvl2_tree.size()));
    born_to_tree_impl(parent_i, virtual_i);
    uint_index_t child_lvl1_i = lvl.population_lvl1_index();
    m_semi_flat_lvl1_tree[child_lvl1_i] = 0;
    return child_lvl1_i;
  }
  MJZ_CX_FN void
  add_non_existing_child_to_node_impl(uint_index_t parent_i,
                                      uint_index_t virtual_i,
                                      uint_index_t child_i) noexcept {
    level_t lvl = get_lvl_ref_impl(parent_i);
    if (!lvl.has_population()) {
      uint_index_t child_lvl1_i = enhabit_impl(parent_i, virtual_i);
      m_semi_flat_lvl1_tree[child_lvl1_i] = child_i + 1;
      return;
    }
    uint_index_t sz = lvl.size();
    uint_index_t cap = lvl.capacity();
    uint_index_t ch_i_old = lvl.child_place_impl(virtual_i);
    uint_index_t index_old = lvl.population_lvl1_to_lvl2_index();
    if (sz < cap) {
      auto end_it =
          m_semi_flat_lvl1_tree.begin() + ptrdiff_t(index_old + 1 + sz);
      auto it = m_semi_flat_lvl1_tree.begin() + ptrdiff_t(ch_i_old);
      std::move_backward(it, end_it, end_it + 1);
      *it = child_i + 1;
      born_to_tree_impl(parent_i, virtual_i);
      return;
    }

    uint_index_t next_free_list_loc = lvl.next_index_in_free_list();
    uint_index_t index_new{};

    if (next_free_list_loc == free_lvl1_list.size() ||
        !free_lvl1_list[next_free_list_loc].size()) {
      index_new = uint_index_t(m_semi_flat_lvl1_tree.size());
      m_semi_flat_lvl1_tree.resize(m_semi_flat_lvl1_tree.size() + 1 +
                                   (size_t(1) << next_free_list_loc));
      asserts(intlen_t(m_semi_flat_lvl1_tree.size()) ==
              int_index_t(m_semi_flat_lvl1_tree.size()));
      lvl = get_lvl_ref_impl(parent_i);
    } else {
      index_new = free_lvl1_list[size_t(next_free_list_loc)].back();
      free_lvl1_list[size_t(next_free_list_loc)].pop_back();
    }
    free_lvl1_list[size_t(next_free_list_loc) - 1].emplace_back(
        int_index_t(uint_index_t(index_old)));

    m_semi_flat_lvl1_tree[parent_i] = int_index_t(uint_index_t(~index_new));

    m_semi_flat_lvl1_tree[index_new++] = m_semi_flat_lvl1_tree[index_old++];

    auto end_it = m_semi_flat_lvl1_tree.begin() + ptrdiff_t(index_old + sz);
    auto begin_it = m_semi_flat_lvl1_tree.begin() + ptrdiff_t(index_old);
    auto it = m_semi_flat_lvl1_tree.begin() + ptrdiff_t(ch_i_old);
    auto new_begin = m_semi_flat_lvl1_tree.begin() + ptrdiff_t(index_new);
    std::ranges::move(begin_it, it, new_begin);
    new_begin += it - begin_it;
    *new_begin = child_i + 1;
    std::ranges::move(it, end_it, ++new_begin);
    born_to_tree_impl(parent_i, virtual_i);

    return;
  }
  MJZ_CX_FN uintlen_t insert(intern_uinthash_t key) noexcept {
    hasher_t hr{key};

    uint_index_t node = place_find(hr);
    level_t lvl = get_lvl_ref_impl(node);
    if (lvl.is_leaf()) {
      uint_index_t leaf_i = lvl.leaf_index();
      if (key == m_hashes[leaf_i])
        return leaf_i;

      hasher_t sibl_hr{m_hashes[leaf_i], hr.depth};
      while (sibl_hr.current() == hr.current()) {
        node = enhabit_impl(node, hr.current());
        sibl_hr.next();
        hr.next();
      };
      add_non_existing_child_to_node_impl(node, sibl_hr.current(), leaf_i);
    };
    asserts(intlen_t(m_hashes.size()) == int_index_t(m_hashes.size()));

    add_non_existing_child_to_node_impl(node, hr.current(),
                                        uint_index_t(m_hashes.size()));

    m_hashes.push_back(std::move(key));

    return uintlen_t(m_hashes.size() - 1);
  }

  MJZ_CX_FN std::span<const intern_uinthash_t> values() const noexcept {
    return m_hashes;
  }
  MJZ_CX_FN const intern_uinthash_t &operator[](uintlen_t i) const noexcept {
    return m_hashes[i];
  }

  MJZ_CX_ND_FN uintlen_t size() const noexcept { return values().size(); }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    constexpr auto sz = std::vector<level_t>().max_size();
    return sz >> level_bit_count_v;
  }

  using value_type = intern_uinthash_t;
  MJZ_random_access_interface_of(two_leveled_interning_vector_t);
};

template <version_t version_v, class intern_uinthash_t,
          uintlen_t level_bit_count_extra_v = 0,
          std::unsigned_integral uint_index_t = uintlen_t>
struct inline_two_leveled_interning_vector_t {

  constexpr static inline uintlen_t level_bit_count_min_v =
      uintlen_t(std::countr_zero(sizeof(uint_index_t) * 8));
  constexpr static inline uintlen_t level_bit_count_v =
      level_bit_count_min_v + level_bit_count_extra_v;
  using int_index_t = std::make_signed_t<uint_index_t>;
  using uintp_t = uint_min_N_t<version_v, (uintlen_t(1) << level_bit_count_v)>;
  constexpr static inline uint_index_t population_word_sz_v =
      uint_index_t(1) << level_bit_count_extra_v;

  std::array<std::vector<uint_index_t>, level_bit_count_v> free_lvl1_list{};
  std::vector<int_index_t> m_semi_flat_lvl_tree{};
  std::vector<intern_uinthash_t> m_hashes{};

  struct level_t {
    std::span<const int_index_t> m_semi_flat_lvl_tree{};
    uint_index_t lvl1_index{};
    MJZ_CX_FN int_index_t lvl2_index() const noexcept {
      return m_semi_flat_lvl_tree[lvl1_index];
    }
    MJZ_CX_FN uint_index_t population_lvl2_index() const noexcept {
      return uint_index_t(~lvl2_index());
    }
    MJZ_CX_FN uint_index_t population_lvl1_index() const noexcept {
      return uint_index_t(population_lvl2_index() + population_word_sz_v);
    }

    MJZ_CX_FN uint_index_t leaf_index() const noexcept {
      return uint_index_t(lvl2_index() - 1);
    }
    MJZ_CX_FN uintp_t population() const noexcept {
      if (!has_population())
        return uintp_t();
      uint_index_t i = population_lvl2_index();
      std::array<uint_index_t, population_word_sz_v> ret{};
      for (uint_index_t &e : ret)
        e = m_semi_flat_lvl_tree[i++];
      return std::bit_cast<uintp_t>(ret);
    }
    MJZ_CX_FN
    bool has_population() const noexcept { return lvl2_index() < 0; }
    MJZ_CX_FN
    bool is_root() const noexcept { return lvl1_index == 0; }
    MJZ_CX_FN
    bool is_tree() const noexcept { return has_population(); }
    MJZ_CX_FN
    bool is_leaf() const noexcept { return 0 < lvl2_index(); }
    MJZ_CX_FN
    bool is_empty() const noexcept { return 0 == lvl2_index(); }
    MJZ_CX_FN uint_index_t size() const noexcept {
      return has_population() ? uint_index_t(popcount(population()))
                              : uint_index_t();
    }
    MJZ_CX_FN uint_index_t capacity() const noexcept {
      if (is_root())
        return uint_index_t(1) << level_bit_count_v;
      return has_population() ? uint_index_t(std::bit_ceil(size()))
                              : uint_index_t();
    }
    MJZ_CX_FN uint_index_t next_index_in_free_list() const noexcept {
      if (is_root())
        return level_bit_count_v;
      return uint_index_t(std::bit_width(size()));
    }
    MJZ_CX_FN uint_index_t
    child_place_impl(uint_index_t virtual_i) const noexcept {
      return uint_index_t(population_lvl1_index() +
                          uint_index_t(popcount(population(), 0, virtual_i)));
    }
    MJZ_CX_FN std::optional<uint_index_t>
    child_index(uint_index_t virtual_i) const noexcept {
      if (has_population() && nth_bit(population(), virtual_i))
        return child_place_impl(virtual_i);
      return {};
    }
  };

  MJZ_CX_FN void born_to_tree_impl(uint_index_t lvl1_index,
                                   uint_index_t virtual_i) noexcept {
    set_population_impl(
        lvl1_index, make_set_nth_bit(get_lvl_ref_impl(lvl1_index).population(),
                                     virtual_i, true));
  }
  MJZ_CX_FN level_t get_lvl_ref_impl(uint_index_t lvl1_index) const noexcept {
    return {m_semi_flat_lvl_tree, lvl1_index};
  }

  struct hasher_t {
    MJZ_CX_FN hasher_t(intern_uinthash_t hash, uintlen_t depth_ = 0) noexcept
        : cache{hash}, depth{depth_} {}
    MJZ_CX_FN uintlen_t next() noexcept {
      depth += level_bit_count_v;
      asserts(depth < sizeof(intern_uinthash_t) * 8);
      return current();
    }
    MJZ_CX_FN uint_index_t current() const noexcept {
      constexpr static uint_index_t omask =
          (uint_index_t(1) << level_bit_count_v) - 1;
      return uint_index_t(cache >> depth) & omask;
    }
    intern_uinthash_t cache{};
    uintlen_t depth{};
  };
  MJZ_CX_FN static uintlen_t
  reserve_lvl1_cap_heuristic(uintlen_t count) noexcept {
    const uintlen_t root_footprint = 2 + (uintlen_t(1) << level_bit_count_v);
    return root_footprint + (count * 4) + (count * 100 >> level_bit_count_v) +
           32;
  }

  MJZ_CX_FN void reserve(uintlen_t count) noexcept {
    reserve(count, reserve_lvl1_cap_heuristic(count),
            uintlen_t(1) << level_bit_count_v);
  }

  MJZ_CX_FN void reserve(uintlen_t count, uintlen_t count_lvl1_tree,
                         uintlen_t free_list_reserves) noexcept {
    m_hashes.reserve(count);
    m_semi_flat_lvl_tree.reserve(count_lvl1_tree);

    for (auto &vec : free_lvl1_list) {
      vec.reserve(free_list_reserves);
    }
  }
  MJZ_CX_FN inline_two_leveled_interning_vector_t(uintlen_t res = 1) noexcept {
    reserve(res);
    m_semi_flat_lvl_tree.resize(
        population_word_sz_v + 1 + (uintlen_t(1) << level_bit_count_v), 0);
    m_semi_flat_lvl_tree[0] = int_index_t(uint_index_t(~1));
  }

  MJZ_CX_FN uint_index_t place_find(hasher_t &h) const noexcept {
    uint_index_t ret{};
    for (;;) {
      std::optional<uint_index_t> opt =
          get_lvl_ref_impl(ret).child_index(h.current());
      if (!opt)
        break;
      ret = *opt;
      h.next();
    };
    return ret;
  }

  MJZ_CX_FN std::optional<uintlen_t>
  find(intern_uinthash_t key) const noexcept {
    hasher_t hr{key};
    level_t lvl = get_lvl_ref_impl(place_find(hr));
    if (!lvl.is_leaf() || key != m_hashes[lvl.leaf_index()])
      return {};
    return lvl.leaf_index();
  };
  MJZ_CX_FN void set_population_impl(uint_index_t parent_i,
                                     uintp_t p) noexcept {
    level_t lvl = get_lvl_ref_impl(parent_i);
    asserts(lvl.has_population());
    uint_index_t i = lvl.population_lvl2_index();
    for (uint_index_t e :
         std::bit_cast<std::array<uint_index_t, population_word_sz_v>>(p))
      m_semi_flat_lvl_tree[i++] = e;
  }
  MJZ_CX_FN uint_index_t enhabit_impl(uint_index_t parent_i,
                                      uint_index_t virtual_i) noexcept {
    level_t lvl = get_lvl_ref_impl(parent_i);
    asserts(!lvl.has_population());
    if (!free_lvl1_list[0].size()) {
      m_semi_flat_lvl_tree[parent_i] =
          int_index_t(uint_index_t(~m_semi_flat_lvl_tree.size()));
      m_semi_flat_lvl_tree.resize(m_semi_flat_lvl_tree.size() + 1 +
                                  population_word_sz_v);
      asserts(intlen_t(m_semi_flat_lvl_tree.size()) ==
              int_index_t(m_semi_flat_lvl_tree.size()));
      lvl = get_lvl_ref_impl(parent_i);
    } else {
      m_semi_flat_lvl_tree[parent_i] =
          int_index_t(uint_index_t(~free_lvl1_list[0].back()));
      free_lvl1_list[0].pop_back();
    }
    set_population_impl(parent_i, uintp_t());
    born_to_tree_impl(parent_i, virtual_i);
    uint_index_t child_lvl1_i = lvl.population_lvl1_index();
    m_semi_flat_lvl_tree[child_lvl1_i] = 0;
    return child_lvl1_i;
  }
  MJZ_CX_FN void
  add_non_existing_child_to_node_impl(uint_index_t parent_i,
                                      uint_index_t virtual_i,
                                      uint_index_t child_i) noexcept {
    level_t lvl = get_lvl_ref_impl(parent_i);
    if (!lvl.has_population()) {
      uint_index_t child_lvl1_i = enhabit_impl(parent_i, virtual_i);
      m_semi_flat_lvl_tree[child_lvl1_i] = child_i + 1;
      return;
    }
    uint_index_t sz = lvl.size();
    uint_index_t cap = lvl.capacity();
    uint_index_t ch_i_old = lvl.child_place_impl(virtual_i);
    uint_index_t index_old = lvl.population_lvl2_index();
    if (sz < cap) {
      auto end_it = m_semi_flat_lvl_tree.begin() +
                    ptrdiff_t(index_old + population_word_sz_v + sz);
      auto it = m_semi_flat_lvl_tree.begin() + ptrdiff_t(ch_i_old);
      std::move_backward(it, end_it, end_it + 1);
      *it = child_i + 1;
      born_to_tree_impl(parent_i, virtual_i);
      return;
    }

    uint_index_t next_free_list_loc = lvl.next_index_in_free_list();
    uint_index_t index_new{};

    if (next_free_list_loc == free_lvl1_list.size() ||
        !free_lvl1_list[next_free_list_loc].size()) {
      index_new = uint_index_t(m_semi_flat_lvl_tree.size());
      m_semi_flat_lvl_tree.resize(m_semi_flat_lvl_tree.size() +
                                  population_word_sz_v +
                                  (size_t(1) << next_free_list_loc));
      asserts(intlen_t(m_semi_flat_lvl_tree.size()) ==
              int_index_t(m_semi_flat_lvl_tree.size()));
      lvl = get_lvl_ref_impl(parent_i);
    } else {
      index_new = free_lvl1_list[size_t(next_free_list_loc)].back();
      free_lvl1_list[size_t(next_free_list_loc)].pop_back();
    }
    free_lvl1_list[size_t(next_free_list_loc) - 1].emplace_back(
        int_index_t(uint_index_t(index_old)));

    m_semi_flat_lvl_tree[parent_i] = int_index_t(uint_index_t(~index_new));
    for (auto _ : std::views::iota(uint_index_t(), population_word_sz_v))
      m_semi_flat_lvl_tree[index_new++] = m_semi_flat_lvl_tree[index_old++];

    auto end_it = m_semi_flat_lvl_tree.begin() + ptrdiff_t(index_old + sz);
    auto begin_it = m_semi_flat_lvl_tree.begin() + ptrdiff_t(index_old);
    auto it = m_semi_flat_lvl_tree.begin() + ptrdiff_t(ch_i_old);
    auto new_begin = m_semi_flat_lvl_tree.begin() + ptrdiff_t(index_new);
    std::ranges::move(begin_it, it, new_begin);
    new_begin += it - begin_it;
    *new_begin = child_i + 1;
    std::ranges::move(it, end_it, ++new_begin);
    born_to_tree_impl(parent_i, virtual_i);

    return;
  }
  MJZ_CX_FN uintlen_t insert(intern_uinthash_t key) noexcept {
    hasher_t hr{key};

    uint_index_t node = place_find(hr);
    level_t lvl = get_lvl_ref_impl(node);
    if (lvl.is_leaf()) {
      uint_index_t leaf_i = lvl.leaf_index();
      if (key == m_hashes[leaf_i])
        return leaf_i;

      hasher_t sibl_hr{m_hashes[leaf_i], hr.depth};
      while (sibl_hr.current() == hr.current()) {
        node = enhabit_impl(node, hr.current());
        sibl_hr.next();
        hr.next();
      };
      add_non_existing_child_to_node_impl(node, sibl_hr.current(), leaf_i);
    };
    asserts(intlen_t(m_hashes.size()) == int_index_t(m_hashes.size()));

    add_non_existing_child_to_node_impl(node, hr.current(),
                                        uint_index_t(m_hashes.size()));

    m_hashes.push_back(std::move(key));

    return uintlen_t(m_hashes.size() - 1);
  }

  MJZ_CX_FN std::span<const intern_uinthash_t> values() const noexcept {
    return m_hashes;
  }
  MJZ_CX_FN const intern_uinthash_t &operator[](uintlen_t i) const noexcept {
    return m_hashes[i];
  }

  MJZ_CX_ND_FN uintlen_t size() const noexcept { return values().size(); }

  MJZ_CX_FN static uintlen_t max_size() noexcept {
    constexpr auto sz = std::vector<level_t>().max_size();
    return sz >> level_bit_count_v;
  }

  using value_type = intern_uinthash_t;
  MJZ_random_access_interface_of(inline_two_leveled_interning_vector_t);
};

}; // namespace mjz

template <
    mjz::version_t version_v, class key_t, class value_t,
    mjz::callable_c<mjz::uintlen_t(const key_t &, mjz::uintlen_t seed) noexcept>
        hash_fn_t,
    uint8_t branching_power_v>
constexpr bool std::ranges::enable_borrowed_range<mjz::unordered_vector_t<
    version_v, key_t, value_t, hash_fn_t, branching_power_v>> = false;

template <
    mjz::version_t version_v, class key_t, class value_t,
    mjz::callable_c<mjz::uintlen_t(const key_t &, mjz::uintlen_t seed) noexcept>
        hash_fn_t,
    uint8_t branching_power_v>
constexpr bool std::ranges::enable_view<mjz::unordered_vector_t<
    version_v, key_t, value_t, hash_fn_t, branching_power_v>> = false;

template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_v, std::signed_integral int_index_t>
constexpr bool std::ranges::enable_borrowed_range<mjz::interning_vector_t<
    version_v, intern_uinthash_t, level_bit_count_v, int_index_t>> = false;
template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_v, std::signed_integral int_index_t>
constexpr bool std::ranges::enable_view<mjz::interning_vector_t<
    version_v, intern_uinthash_t, level_bit_count_v, int_index_t>> = false;

template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_extra_v,
          std::signed_integral int_index_t>
constexpr bool
    std::ranges::enable_borrowed_range<mjz::leveled_interning_vector_t<
        version_v, intern_uinthash_t, level_bit_count_extra_v, int_index_t>> =
        false;
template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_extra_v,
          std::signed_integral int_index_t>
constexpr bool std::ranges::enable_view<mjz::leveled_interning_vector_t<
    version_v, intern_uinthash_t, level_bit_count_extra_v, int_index_t>> =
    false;

template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_v, std::signed_integral int_index_t>
constexpr bool
    std::ranges::enable_borrowed_range<mjz::two_leveled_interning_vector_t<
        version_v, intern_uinthash_t, level_bit_count_v, int_index_t>> = false;
template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_v, std::signed_integral int_index_t>
constexpr bool std::ranges::enable_view<mjz::two_leveled_interning_vector_t<
    version_v, intern_uinthash_t, level_bit_count_v, int_index_t>> = false;


    
template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_extra_v,
          std::signed_integral int_index_t>
constexpr bool
    std::ranges::enable_borrowed_range<mjz::inline_two_leveled_interning_vector_t<
        version_v, intern_uinthash_t, level_bit_count_extra_v, int_index_t>> =
        false;
template <mjz::version_t version_v, class intern_uinthash_t,
          mjz::uintlen_t level_bit_count_extra_v,
          std::signed_integral int_index_t>
constexpr bool std::ranges::enable_view<mjz::inline_two_leveled_interning_vector_t<
    version_v, intern_uinthash_t, level_bit_count_extra_v, int_index_t>> =
    false;

#endif // MJZ_SRC_UORD_VEC_base_FILE_