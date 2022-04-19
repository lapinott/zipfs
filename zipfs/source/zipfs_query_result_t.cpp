#include <zipfs/zipfs_query_result_t.h>
#include <stdexcept>

namespace zipfs {
	zipfs_query_result_t::zipfs_query_result_t(QUERY_RESULT query_result_, const zipfs_path_t& zipfs_path_, const filesystem_path_t& fs_path_, const zipfs_path_t& zipfs_path_cmp_, const filesystem_path_t& fs_path_cmp_) :
		query_result{ query_result_ },
		zipfs_path{ zipfs_path_ },
		fs_path{ fs_path_ },
		zipfs_path_cmp{ zipfs_path_cmp_ },
		fs_path_cmp{ fs_path_cmp_ }
	{}
}