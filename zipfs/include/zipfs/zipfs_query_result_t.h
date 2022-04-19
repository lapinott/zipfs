#pragma once

#include <zipfs/zipfs_enums.h>
#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>

namespace zipfs {

	struct zipfs_query_result_t {
	public:

		QUERY_RESULT
			query_result;

		zipfs_path_t
			zipfs_path,
			zipfs_path_cmp;

		filesystem_path_t
			fs_path,
			fs_path_cmp;

		zipfs_query_result_t(QUERY_RESULT query_result, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, const zipfs_path_t& zipfs_path_cmp, const filesystem_path_t& fs_path_cmp);
	};
}