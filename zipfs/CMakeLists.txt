#source
add_library(zipfs STATIC
	"include/zipfs/zipfs.h"
	"include/zipfs/zipfs_assert.h"
	"include/zipfs/zipfs_enums.h"
	"include/zipfs/zipfs_error_strings.h"
	"include/zipfs/zipfs_error_t.h"
	"include/zipfs/zipfs_filesystem_path_t.h"
	"include/zipfs/zipfs_index_t.h"
	"include/zipfs/zipfs_path_t.h"
	"include/zipfs/zipfs_query_result_t.h"
	"include/zipfs/zipfs_t.h"
	"include/zipfs/zipfs_zip_stat_t.h"
	"source/zipfs.cpp"
	"source/zipfs_error_t.cpp"
	"source/zipfs_index_t.cpp"
	"source/zipfs_path_t.cpp"
	"source/zipfs_query_result_t.cpp"
	"source/zipfs_t.cpp"
	"source/zipfs_t_filesystem.cpp"
	"source/zipfs_zip_stat_t.cpp")
	
target_include_directories(zipfs PUBLIC "include")
target_include_directories(zipfs PUBLIC "${ZLIB_INCLUDE_DIR}")
target_include_directories(zipfs PUBLIC "${LIBZIP_DIR}")
target_include_directories(zipfs PUBLIC "${LIBZIP_INCLUDE_DIR}")
target_include_directories(zipfs PUBLIC "${BOOST_DIR}")
target_include_directories(zipfs PUBLIC "${UTIL_INCLUDE_DIR}")