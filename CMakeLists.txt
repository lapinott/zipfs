cmake_minimum_required(VERSION 3.10)

#zipfs
project(zipfs)

#c++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#debug name
set (CMAKE_DEBUG_POSTFIX "d")

#zipfs
add_subdirectory("zipfs")
add_subdirectory("zipfs_tutorial_0")
add_subdirectory("zipfs_tutorial_1")
add_subdirectory("zipfs_tutorial_2")
add_subdirectory("zipfs_tutorial_3")
add_subdirectory("zipfs_tutorial_4")
add_subdirectory("zipfs_tutorial_5")
add_subdirectory("zipfs_tutorial_6")

add_dependencies(zipfs_tutorial_0 zipfs)
add_dependencies(zipfs_tutorial_1 zipfs)
add_dependencies(zipfs_tutorial_2 zipfs)
add_dependencies(zipfs_tutorial_3 zipfs)
add_dependencies(zipfs_tutorial_4 zipfs)
add_dependencies(zipfs_tutorial_5 zipfs)
add_dependencies(zipfs_tutorial_6 zipfs)

find_path(ZLIB_INCLUDE_DIR "zlib include dir" )
find_file(ZLIB_LIBRARY_x64_DEBUG "zlib library x64 Debug")
find_file(ZLIB_LIBRARY_x64_RELEASE "zlib library x64 Release")
find_path(LIBZIP_DIR "libzip dir")
find_path(LIBZIP_INCLUDE_DIR "libzip include dir")
find_file(LIBZIP_LIBRARY_x64_DEBUG "libzip library x64 Debug")
find_file(LIBZIP_LIBRARY_x64_RELEASE "libzip library x64 Release")
find_path(BOOST_DIR "boost dir")
find_path(BOOST_LIBRARY_DIR "boost library dir")
find_path(UTIL_INCLUDE_DIR "util include dir")
find_file(UTIL_LIBRARY_x64_DEBUG "util library x64 Debug")
find_file(UTIL_LIBRARY_x64_RELEASE "util library x64 Release")