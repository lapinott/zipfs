project(zipfs_tutorial_5)
add_executable(zipfs_tutorial_5 "main.cpp")

target_include_directories(zipfs_tutorial_5 PUBLIC "../zipfs/include")
target_include_directories(zipfs_tutorial_5 PUBLIC "${ZLIB_INCLUDE_DIR}")
target_include_directories(zipfs_tutorial_5 PUBLIC "${LIBZIP_DIR}")
target_include_directories(zipfs_tutorial_5 PUBLIC "${LIBZIP_INCLUDE_DIR}")
target_include_directories(zipfs_tutorial_5 PUBLIC "${LIBZIP_CONFIG_H_DIR}")
target_include_directories(zipfs_tutorial_5 PUBLIC "${BOOST_DIR}")
target_include_directories(zipfs_tutorial_5 PUBLIC "${UTIL_INCLUDE_DIR}")

target_link_directories(zipfs_tutorial_5 PUBLIC "${BOOST_LIBRARY_DIR}")

target_link_libraries(zipfs_tutorial_5
	debug zipfs
	optimized zipfs
	debug "${ZLIB_LIBRARY_x64_DEBUG}" debug "${LIBZIP_LIBRARY_x64_DEBUG}" debug "${UTIL_LIBRARY_x64_DEBUG}" debug "${BOOST_FILESYSTEM_LIBRARY_x64_DEBUG}"
	optimized "${ZLIB_LIBRARY_x64_RELEASE}" optimized "${LIBZIP_LIBRARY_x64_RELEASE}" optimized "${UTIL_LIBRARY_x64_RELEASE}" optimized "${BOOST_FILESYSTEM_LIBRARY_x64_RELEASE}")