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
    MJZ_CX_FN hasher_t(const key_t &k, const hash_fn_t &hf_,
                       uintlen_t depth_ = 0) noexcept
        : key{std::addressof(k)}, hf{std::addressof(hf_)}, depth{depth_} {
      uintlen_t n = depth & omask;
      depth &= ~omask;
      cache = get_hash();
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
    auto [node, parent] = place_find(hr);

    if (node) {
      if (key == m_keys[size_t(node - 1)])
        return uintlen_t(node - 1);
      hasher_t sibl_hr{m_keys[size_t(node - 1)], hash_fn, hr.depth};
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
      m_flat_tree[uintlen_t(back_i_r) >> shift_index_node]
                 [mask & uintlen_t(back_i_r)] = node;
    }
    m_flat_tree[uintlen_t(node_i_r) >> shift_index_node]
               [mask & uintlen_t(node_i_r)] = 0;
    m_reverse_value_indexies.pop_back();
    m_values.pop_back();
    m_keys.pop_back();
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

  using const_iterator =
      bstr_ns::random_access_iterator_of_t<const unordered_vector_t>;
  using iterator = bstr_ns::random_access_iterator_of_t<unordered_vector_t>;
  using value_type = pair_t<key_t, value_t>;
  using reference = pair_t<const key_t &, value_t &>;
  using const_reference = pair_t<const key_t &, const value_t &>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using size_type = uintlen_t;
  using difference_type = intlen_t;
  MJZ_CX_ND_FN const_iterator cbegin() const noexcept { return begin(); }
  MJZ_CX_ND_FN const_iterator cend() const noexcept { return end(); }
  MJZ_CX_ND_FN iterator begin() noexcept { return iterator(*this, 0); }
  MJZ_CX_ND_FN iterator end() noexcept { return iterator(*this, size()); }
  MJZ_CX_ND_FN const_iterator begin() const noexcept {
    return const_iterator(*this, 0);
  }
  MJZ_CX_ND_FN const_iterator end() const noexcept {
    return const_iterator(*this, size());
  }

  MJZ_CX_ND_FN reverse_iterator rbegin() noexcept {
    return reverse_iterator{end()};
  }

  MJZ_CX_ND_FN reverse_iterator rend() noexcept {
    return reverse_iterator{begin()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  MJZ_CX_ND_FN const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }
  MJZ_CX_ND_FN const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  }

  MJZ_CX_ND_FN const_reverse_iterator crend() const noexcept { return rend(); }

  MJZ_CX_ND_FN size_type size() const noexcept { return values().size(); }

  MJZ_CX_FN static size_type size()  noexcept {
    return std::vector<childern_node_t>::max_size() >> shift_index_node;
  }
};

template <version_t version_v, class key_t, class value_t>
struct unordered_vector_str_t
    : unordered_vector_t<
          version_v, key_t, value_t,
          decltype([](const key_t &key, uintlen_t seed) noexcept -> uintlen_t {
            return hash_bytes_t<version_v>::template hash_bytes<uintlen_t>(
                key.data(), key.size(), 0xc70f6907UL ^ seed);
          })> {};

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
#endif // MJZ_SRC_UORD_VEC_base_FILE_