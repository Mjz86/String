
#include "alloc_ref.hpp"
#include "block_states.hpp"
#include "blocks_alloc.hpp"
#include "bump_alloc.hpp"
#include "generic_alloc.hpp"
#include "page_alloc.hpp"
#include "page_alloc_base.hpp"
#include "pmr_adaptor.hpp"
#include "pool_alloc.hpp"

// optional file
#if __has_include("unique_ptr.hpp")
#include "unique_ptr.hpp"
#endif

#if __has_include("shared_ptr.hpp")
#include "shared_ptr.hpp"
#endif