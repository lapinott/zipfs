#include <zipfs/zipfs_t.h>
#include <zipfs/zipfs_assert.h>
#include <zipfs/zipfs_error_strings.h>
#include <zipfs/zipfs_filesystem_path_t.h>

namespace zipfs {

	//pull
	QUERY_RESULT zipfs_t::_zipfs_get_query_result(OVERWRITE overwrite, ORPHAN orphan, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		zipfs_internal_assert(m_zip_t == nullptr);

		//fs_path is directory
		if (fs_path.is_directory()) {
			zipfs_internal_assert(!zipfs_path.is_dir());
			zipfs_path_t p = zipfs_path.to_dir();

			zip_int64_t index_;
			if (!index(p, index_))//<.should return the index and throw on error
				return QUERY_RESULT::NONE;//=error

			//doesn't exist
			if (index_ == -1) {
				return QUERY_RESULT::DIR_ADD;
			}

			//exists
			else {
				return QUERY_RESULT::DIR_ALREADY_EXISTS;
			}
		}

		//fs_path is regular file
		else if (fs_path.is_regular_file()) {
			zip_int64_t index_;
			if (!index(zipfs_path, index_))//<.should return the index and throw on error
				return QUERY_RESULT::NONE;//=error

			//doesn't exist
			if (index_ == -1) {
				return QUERY_RESULT::FILE_WRITE;
			}

			//exists
			else {
				size_t fs_sz = fs_path.file_size();
				time_t fs_mtime = fs_path.last_write_time();
				zipfs_zip_stat_t stat_;
				if (!stat(zipfs_path, stat_))//<.should return the stat and throw on error
					return QUERY_RESULT::NONE;//=error

				bool do_overwrite = false;

				switch (overwrite) {
				case OVERWRITE::ALWAYS: {
					do_overwrite = true;
					break;
				}
				case OVERWRITE::NEVER: {
					do_overwrite = false;
					break;
				}
				case OVERWRITE::IF_DATE_OLDER: {
					if (stat_.mtime < fs_mtime)
						do_overwrite = true;
					break;
				}
				case OVERWRITE::IF_SIZE_MISMATCH: {
					if (stat_.size != fs_sz)
						do_overwrite = true;
					break;
				}
				case OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH: {
					if (stat_.mtime < fs_mtime && stat_.size != fs_sz)
						do_overwrite = true;
					break;
				}
				default:
					do_overwrite = false;
					break;
				}

				if (do_overwrite) {
					return QUERY_RESULT::FILE_OVERWRITE;
				}
				else {
					return QUERY_RESULT::FILE_DONT_OVERWRITE;
				}
			}
		}

		//fs_path doesn't exist & zipfs_path is file
		else if (!fs_path.exists() && zipfs_path.is_file()) {
			switch (orphan) {
			case ORPHAN::KEEP: {
				return QUERY_RESULT::FILE_ORPHAN_KEEP;
			}
			case ORPHAN::DELETE_: {
				return QUERY_RESULT::FILE_ORPHAN_DELETE;
			}
			}
		}

		//fs_path doesn't exist & zipfs_path is dir
		else if (!fs_path.exists() && zipfs_path.is_dir()) {
			switch (orphan) {
			case ORPHAN::KEEP: {
				return QUERY_RESULT::DIR_ORPHAN_KEEP;
			}
			case ORPHAN::DELETE_: {
				return QUERY_RESULT::DIR_ORPHAN_DELETE;
			}
			}
		}

		//fs_path is other file type (symlink/socket/other/fifo/character file device/block file) -> std::filesystem::is_*
		else if (fs_path.exists()) {

			// ¯\_(¬.¬)_/¯
			return QUERY_RESULT::DISCARD;
		}

		//failsafe
		return QUERY_RESULT::NONE;
	}

	//extract
	QUERY_RESULT zipfs_t::_zipfs_get_query_result(OVERWRITE overwrite, const filesystem_path_t& fs_path, const zipfs_path_t& zipfs_path) {
		zipfs_internal_assert(m_zip_t == nullptr);
		
		//zipfs_path is directory
		if (zipfs_path.is_dir()) {

			//doesn't exist
			if (!fs_path.exists()) {
				return QUERY_RESULT::DIR_ADD;
			}

			//is dir on filesystem
			else if (fs_path.is_directory()) {
				return QUERY_RESULT::DIR_ALREADY_EXISTS;
			}

			//is but not dir on filesystem; =conflict
			else {
				return QUERY_RESULT::DIR_IS_BUT_NOT_DIR;
			}
		}

		//zipfs_path is file
		else if (zipfs_path.is_file()) {

			//doesn't exist
			if (!fs_path.exists()) {
				return QUERY_RESULT::FILE_WRITE;
			}

			//is regular file on filesystem
			else if (fs_path.is_regular_file()) {
				size_t fs_sz = fs_path.file_size();
				time_t fs_mtime = fs_path.last_write_time();
				zipfs_zip_stat_t stat_;
				if (!stat(zipfs_path, stat_))//<.should return the stat and throw on error
					return QUERY_RESULT::NONE;//=error

				bool do_overwrite = false;

				switch (overwrite) {
				case OVERWRITE::ALWAYS: {
					do_overwrite = true;
					break;
				}
				case OVERWRITE::NEVER: {
					do_overwrite = false;
					break;
				}
				case OVERWRITE::IF_DATE_OLDER: {
					if (stat_.mtime > fs_mtime) {
						do_overwrite = true;
					}
					break;
				}
				case OVERWRITE::IF_SIZE_MISMATCH: {
					if (stat_.size != fs_sz) {
						do_overwrite = true;
					}
					break;
				}
				case OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH: {
					if (stat_.mtime > fs_mtime && stat_.size != fs_sz) {
						do_overwrite = true;
					}
					break;
				}
				default:
					do_overwrite = false;
					break;
				}

				if (do_overwrite) {
					return QUERY_RESULT::FILE_OVERWRITE;
				}
				else {
					return QUERY_RESULT::FILE_DONT_OVERWRITE;
				}
			}

			//is but not regular file on filesystem; =conflict
			else {
				return QUERY_RESULT::FILE_IS_BUT_NOT_FILE;
			}
		}

		//zipfs_path can only be a directory or a file (should not be reachable)
		else {
			return QUERY_RESULT::NONE;
		}
	}
}