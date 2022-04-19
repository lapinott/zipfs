#include <zipfs/zipfs_t.h>
#include <zipfs/zipfs_assert.h>

namespace zipfs {

	QUERY_RESULT zipfs_t::_zipfs_get_query_result(OVERWRITE overwrite, const zipfs_path_t& zipfs_path) {
		zipfs_internal_assert(m_zip_t == nullptr);
		zipfs_internal_assert(zipfs_path.is_file());

		zip_int64_t index_;
		if (!index(zipfs_path, index_))//<.should return the index and throw on error
			return QUERY_RESULT::NONE;//=error

		//doesn't exist
		if (index_ == -1) {
			return QUERY_RESULT::FILE_WRITE;
		}

		//exists
		else {
			switch (overwrite) {
			case OVERWRITE::ALWAYS: {
				return QUERY_RESULT::FILE_OVERWRITE;
			}
			case OVERWRITE::NEVER: {
				return QUERY_RESULT::FILE_DONT_OVERWRITE;
			}
			case OVERWRITE::IF_DATE_OLDER:
			case OVERWRITE::IF_SIZE_MISMATCH:
			case OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH:
			default: {//non-valid flags
				return QUERY_RESULT::DISCARD;
			}
			}
		}
	}
}