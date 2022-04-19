set(ZIPFS_PUBLIC_HEADERS
	"include/zipfs/zipfs.h"
	"include/zipfs/zipfs_assert.h"
	"include/zipfs/zipfs_enums.h"
	"include/zipfs/zipfs_error_strings.h"
	"include/zipfs/zipfs_error_t.h"
	"include/zipfs/zipfs_filesystem_path_t.h"
	"include/zipfs/zipfs_index_t.h"
	"include/zipfs/zipfs_path_t.h"
	"include/zipfs/zipfs_query_result_t.h"
	"include/zipfs/zipfs_query_results_t.h"
	"include/zipfs/zipfs_t.h"
	"include/zipfs/zipfs_zip_stat_t.h")
	
set(ZIPFS_SOURCE_FILES
	"source/zipfs.cpp"
	"source/zipfs_error_t.cpp"
	"source/zipfs_index_t.cpp"
	"source/zipfs_path_t.cpp"
	"source/zipfs_query_result_t.cpp"
	"source/zipfs_query_results_t.cpp"
	"source/zipfs_t.cpp"
	"source/zipfs_t_query.cpp"
	"source/zipfs_t_filesystem.cpp"
	"source/zipfs_t_filesystem_query.cpp"
	"source/zipfs_zip_stat_t.cpp")

#source
add_library(zipfs STATIC ${ZIPFS_PUBLIC_HEADERS} ${ZIPFS_SOURCE_FILES})
	
target_include_directories(zipfs PUBLIC "include")
target_include_directories(zipfs PUBLIC "${ZLIB_INCLUDE_DIR}")
target_include_directories(zipfs PUBLIC "${LIBZIP_DIR}")
target_include_directories(zipfs PUBLIC "${LIBZIP_INCLUDE_DIR}")
target_include_directories(zipfs PUBLIC "${LIBZIP_CONFIG_H_DIR}")
target_include_directories(zipfs PUBLIC "${BOOST_DIR}")
target_include_directories(zipfs PUBLIC "${UTIL_INCLUDE_DIR}")

#configure file
set(ZIPFS_ZIP_SOURCE_T_EXTRA_CHECKS 0)
configure_file("include/zipfs/zipfs_config.h.in" "include/zipfs/zipfs_config.h")
target_include_directories(zipfs PUBLIC "${PROJECT_BINARY_DIR}/zipfs/include")

install(TARGETS zipfs LIBRARY DESTINATION lib)
install(FILES ${ZIPFS_PUBLIC_HEADERS} "${PROJECT_BINARY_DIR}/zipfs/include/zipfs/zipfs_config.h" DESTINATION "include/zipfs")