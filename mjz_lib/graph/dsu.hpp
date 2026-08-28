
#ifndef MJZ_SRC_GRAPH_DSU_FILE_
#define MJZ_SRC_GRAPH_DSU_FILE_
#include "base.hpp"

MJZ_EXPORT
//
namespace mjz::graph_ns {

template <version_t version_v, mutable_index_range_c<version_v> range_t>
struct disjoint_set_union_t {

  // 2x the size of this bad boy and it will give you ranking
  range_t parent_then_rank{};
  intlen_t m_range_size{};

  MJZ_CX_FN uintlen_t get_range_size() const noexcept {
    return uintlen_t(std::max(m_range_size, -m_range_size));
  }

  MJZ_CX_FN bool has_ranks() const noexcept { return 0 < m_range_size; }
  MJZ_CX_FN uintlen_t get_impl(uintlen_t i) const noexcept {
    return *(std::ranges::begin(parent_then_rank) + intlen_t(i));
  }
  MJZ_CX_FN void set_impl(uintlen_t i, uintlen_t v) noexcept {
    *(std::ranges::begin(parent_then_rank) + intlen_t(i)) = v;
  }
  MJZ_CX_FN uintlen_t get_parent(uintlen_t i) const noexcept {
    return get_impl(i);
  }
  MJZ_CX_FN uintlen_t get_rank(uintlen_t i) const noexcept {
    if (!has_ranks())
      return 0;
    return get_impl(get_range_size() + i);
  }
  MJZ_CX_FN void set_parent(uintlen_t i, uintlen_t parent) noexcept {
    set_impl(i, parent);
  }

  MJZ_CX_FN void set_rank(uintlen_t i, uintlen_t rank) noexcept {
    if (!has_ranks())
      return;
    set_impl(get_range_size() + i, rank);
  }
  MJZ_CX_FN void init(uintlen_t range_size_) noexcept {
    uintlen_t sz = std::ranges::size(parent_then_rank);
    asserts(range_size_ <= sz && range_size_ < (uintlen_t(-1) >> 1));
    m_range_size = range_size_ <= (sz >> 1) ? intlen_t(range_size_)
                                            : -intlen_t(range_size_);
    for (uintlen_t i : std::views::iota(uintlen_t(0), range_size_)) {
      set_parent(i, i);
      set_rank(i, 0);
    }
  }

  MJZ_CX_FN uintlen_t find_no_cache_root(uintlen_t child) const noexcept {
    uintlen_t root_node = child;
    while (true) {
      uintlen_t parent = get_parent(root_node);
      if (parent == std::exchange(root_node, parent))
        break;
    }
    return root_node;
  }
  MJZ_CX_FN bool is_cache_root_of_found_valid(uintlen_t root_node,
                                              uintlen_t child) const noexcept {
    return root_node == get_parent(child) && get_parent(root_node) == root_node;
  }
  MJZ_CX_FN uintlen_t cache_root_of_found(uintlen_t root_node,
                                          uintlen_t child) noexcept {
    while (true) {
      uintlen_t parent = get_parent(child);
      set_parent(child, root_node);
      if (parent == std::exchange(child, parent))
        break;
    };
    return root_node;
  }
  MJZ_CX_FN uintlen_t find_and_cache_root(uintlen_t child) noexcept {
    return cache_root_of_found(find_no_cache_root(child), child);
  }

  MJZ_CX_FN bool find_and_unite_root(uintlen_t i, uintlen_t j) noexcept {
    i = find_and_cache_root(i);
    j = find_and_cache_root(j);
    if (i == j) {
      return false;
    }

    uintlen_t rank_i = get_rank(i);
    uintlen_t rank_j = get_rank(j);

    bool i_child = rank_i < rank_j;
    bool tie = (rank_i == rank_j);

    uintlen_t child = i_child ? i : j;
    uintlen_t parent = i_child ? j : i;

    set_parent(child, parent);

    if (tie) {
      set_rank(parent, rank_i + 1);
    }

    return true;
  }

  MJZ_CX_FN bool rankless_find_and_unite_root(uintlen_t parent,
                                              uintlen_t child) noexcept {
    asserts(!has_ranks());
    parent = find_and_cache_root(parent);
    child = find_and_cache_root(child);
    if (parent == child) {
      return false;
    }
    set_parent(child, parent);
    return true;
  }

  MJZ_CX_FN bool is_united(uintlen_t parent, uintlen_t child) noexcept {
    return find_and_cache_root(parent) == find_and_cache_root(child);
  }
};

}; // namespace mjz::graph_ns
#endif // MJZ_SRC_GRAPH_DSU_FILE_