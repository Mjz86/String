#ifdef MJZ_LIB_FILE_alllib_import_
#define MJZ_LIB_FILE_alllib_import_
#include "all_macors.hpp"

#if MJZ_IS_CXX_MODULES_
export import mjz_lib;
#else
#include "alllib.hpp"
#endif
#endif