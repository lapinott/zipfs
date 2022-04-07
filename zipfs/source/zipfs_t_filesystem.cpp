#include <zipfs/zipfs_t.h>
#include <zipfs/zipfs_assert.h>
#include <zipfs/zipfs_error_strings.h>
#include <util/filesystem_path_t.h>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <filesystem>

namespace zipfs {

	bool zipfs_t::_zipfs_dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_result_t* query_result, OVERWRITE overwrite, ORPHAN orphan, bool is_query) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		if (!fs_path.is_directory()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_DIRECTORY_BAD_TARGET, "/", fs_path.platform_path());
			return false;
		}

		//create destination dir
		zip_int64_t index_;
		if (!index(zipfs_path, index_))
			return false;

		if (index_ == -1) {
			if (!dir_add(zipfs_path))
				return false;
		}

		goto pull;

	pull:
		{
			std::vector<filesystem_path_t> fs_paths;
			for (auto fs_entry = std::filesystem::recursive_directory_iterator(fs_path.platform_path()); fs_entry != std::filesystem::recursive_directory_iterator(); fs_entry++)
				fs_paths.push_back(fs_entry->path().lexically_normal().c_str());

			for (size_t e = 0; e < fs_paths.size(); e++) {
				filesystem_path_t relative = std::filesystem::relative(fs_paths[e].platform_path(), fs_path.platform_path()).c_str();
				zipfs_path_t p = zipfs_path + relative.u8path();
				time_t fs_mtime = fs_paths[e].last_write_time();

				//directory
				if (std::filesystem::is_directory(fs_paths[e].platform_path())) {
					zipfs_internal_assert(!p.is_dir());
					p = p.to_dir();
					zip_int64_t index_;
					if (!index(p, index_)) goto abort;
					//doesn't exist
					if (index_ == -1) {
						switch (is_query) {
						case false: {
							if (!dir_add(p)) goto abort;
							if (!_zipfs_set_dir_mtime(p, fs_mtime)) goto abort;
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ADD, p, "");
							break;
						}
						}
					}
					//exists
					else {
						switch (is_query) {
						case false: {
							/*dir already exists; nothing to do*/
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ALREADY_EXISTS, p, "");
							break;
						}
						}
					}
				}
				//regular file
				else if (std::filesystem::is_regular_file(fs_paths[e].platform_path())) {
					zip_int64_t index_;
					if (!index(p, index_)) goto abort;

					size_t fs_sz = std::filesystem::file_size(fs_paths[e].platform_path());

					//write
					if (index_ == -1) {
						switch (is_query) {
						case false: {
							if (!file_pull(p, fs_paths[e])) goto abort;
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::FILE_WRITE, p, "");
							break;
						}
						}
					}

					//overwrite
					else {
						zipfs_zip_stat_t stat_;
						if (!stat(p, stat_)) goto abort;

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
							break;
						}

						if (do_overwrite) {
							switch (is_query) {
							case false: {
								if (!file_pull_replace(p, fs_paths[e])) goto abort;
								break;
							}
							case true: {
								query_result->push_back(QUERY_RESULT::FILE_OVERWRITE, p, "");
								break;
							}
							}
						}
						else {
							switch (is_query) {
							case false: { /* nothing to do */ break; }
							case true: {
								query_result->push_back(QUERY_RESULT::FILE_DONT_OVERWRITE, p, "");
							}
							}
						}
					}
				}
				//other
				else {
					// ¯\_(¬.¬)_/¯
				}
			}
			goto orphans;
		}

	orphans:
		{
			std::vector<zipfs_path_t> ls_;
			if (!ls(zipfs_path, ls_, false))
				return false;

			//files first
			for (size_t e = 0; e < ls_.size(); e++) {
				filesystem_path_t orphan_path = fs_path.u8path() + std::string("/") + ls_[e].string().substr(zipfs_path.string().size());
				if (!orphan_path.exists() && ls_[e].is_file()) {
					bool do_delete = false;
					switch (orphan) {
					case ORPHAN::KEEP: { do_delete = false; break; }
					case ORPHAN::DELETE_: { do_delete = true; break; }
					default: break;
					}
					if (do_delete) {
						switch (is_query) {
						case false: {
							if (!file_delete(ls_[e])) goto abort;
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::FILE_ORPHAN_DELETE, ls_[e], "");
							break;
						}
						}
					}
					else {
						switch (is_query) {
						case false: {
							/* nothing to do */
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::FILE_ORPHAN_KEEP, ls_[e], "");
							break;
						}
						}
					}
				}
			}
			//then dirs
			for (size_t e = 0; e < ls_.size(); e++) {
				filesystem_path_t orphan_path = fs_path.u8path() + std::string("/") + ls_[e].string().substr(zipfs_path.string().size());
				if (!orphan_path.exists() && ls_[e].is_dir()) {
					bool do_delete = false;
					switch (orphan) {
					case ORPHAN::KEEP: { do_delete = false; break; }
					case ORPHAN::DELETE_: { do_delete = true; break; }
					default: break;
					}
					if (do_delete) {
						switch (is_query) {
						case false: {
							size_t d;
							if (!dir_delete(ls_[e], d)) goto abort;
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ORPHAN_DELETE, ls_[e], "");
							break;
						}
						}
					}
					else {
						switch (is_query) {
						case false: {
							/* nothing to do */
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ORPHAN_KEEP, ls_[e], "");
							break;
						}
						}
					}
				}
			}
			goto end;
		}

	abort:
		return false;

	end:
		return true;
	}

	bool zipfs_t::_zipfs_dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_result_t* query_result, OVERWRITE overwrite, bool is_query) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		if (!fs_path.is_directory()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_DIRECTORY_BAD_TARGET, "/", fs_path);
			return false;
		}

		zip_int64_t index_;
		if (!index(zipfs_path, index_))
			return false;

		if (index_ == -1 && !zipfs_path.is_root()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_SOURCE_DIR_DOESNT_EXIST, zipfs_path, "");
			return false;
		}

		goto extract;
	
	extract:
		{
			std::vector<zipfs_path_t> ls_;
			if (!ls(zipfs_path, ls_, false))
				return false;

			std::vector<std::pair<filesystem_path_t, time_t>>
				dsmtime;

			for (size_t e = 0; e < ls_.size(); e++) {

				zipfs_zip_stat_t stat_;
				if (!stat(ls_[e], stat_))
					return false;

				time_t zipfs_mtime = stat_.mtime;
				size_t zipfs_sz = stat_.size;

				filesystem_path_t extract_path = fs_path.u8path() + std::string("/") + ls_[e].string().substr(zipfs_path.string().size());

				//directory
				if (ls_[e].is_dir()) {
					//doesn't exist on filesystem
					if (!extract_path.exists()) {
						switch (is_query) {
						case false: {
							if (std::filesystem::create_directory(extract_path.platform_path()))
								dsmtime.push_back({ extract_path.platform_path(), zipfs_mtime });//mtime later
							else {
								_zipfs_zipfs_set_error(ZIPFS_ERRSTR_COULD_NOT_CREATE_DIR, "/", extract_path);
								return false;
							}
							break;
						}
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ADD, "/", extract_path);
							break;
						}
						}
					}
					//is dir on filesystem
					else if (extract_path.is_directory()) {
						switch (is_query) {
						case false: { /*dir already exists; nothing to do*/ break; }
						case true: {
							query_result->push_back(QUERY_RESULT::DIR_ALREADY_EXISTS, "/", extract_path);
							break;
						}
						}
					}
					//exists but is not a dir on filesystem (conflict)
					else {
						//path exists and is not a dir
						continue;//? DISCARD flag?
					}
				}

				//file
				else if (ls_[e].is_file()) {

					bool do_write = false;
					bool do_overwrite = false;

					//write
					if (!extract_path.exists()) {
						do_write = true;
					}

					//overwrite
					else if (extract_path.is_regular_file()) {

						time_t fs_mtime = extract_path.last_write_time();
						size_t fs_sz = extract_path.file_size();

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
							if (zipfs_mtime > fs_mtime) {
								do_overwrite = true;
							}
							break;
						}
						case OVERWRITE::IF_SIZE_MISMATCH: {
							if (zipfs_sz != fs_sz) {
								do_overwrite = true;
							}
							break;
						}
						case OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH: {
							if (zipfs_mtime > fs_mtime && zipfs_sz != fs_sz) {
								do_overwrite = true;
							}
							break;
						}
						default:
							break;
						}
					}

					//non regular file
					else {
						// ¯\_(¬.¬)_/¯
						continue;//? DISCARD flag?
					}

					switch (is_query) {
					case false: {
						if (do_write)
							file_extract(ls_[e], extract_path);
						else if (do_overwrite)
							file_extract_replace(ls_[e], extract_path);
						break;
					}
					case true: {
						if (do_write)
							query_result->push_back(QUERY_RESULT::FILE_WRITE, "/", extract_path);
						else if (do_overwrite)
							query_result->push_back(QUERY_RESULT::FILE_OVERWRITE, "/", extract_path);
						else
							query_result->push_back(QUERY_RESULT::FILE_DONT_OVERWRITE, "/", extract_path);
					}
					}
				}

				else {}//zipfs_path_t can only be dir or file
			}

			//directories mtime here
			if (!is_query)
				for (auto& dmtime : dsmtime) {
					if (!dmtime.first.last_write_time(dmtime.second))
						zipfs_debug_assert(false);
				}
		}

		return true;
	}

	bool zipfs_t::_zipfs_source_buffer_encrypt(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zip_source_t** src) {
		return _zipfs_source_buffer_encrypt(zipfs_path, fs_path.cat(), src);
	}

	zipfs_error_t zipfs_t::file_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!fs_path.is_regular_file()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_FS_PATH_NOT_A_REGULAR_FILE, "/", fs_path);
			return m_ze;
		}

		if (!
			dir_add(zipfs_path.parent_path()))
			return m_ze;

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index != -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_FILE_ALREADY_EXISTS, zipfs_path, "");
			return m_ze;
		}

		zip_source_t* src;
		if (m_file_encrypt && m_file_encrypt_func != nullptr)
			goto source_buffer_encrypt;
		else
			goto source_file;

	source_buffer_encrypt: //using <filesystem> to read file data and then encrypt them
		{
			_zipfs_source_buffer_encrypt(zipfs_path, fs_path, &src);
			goto source_finish;
		}

	source_file: //using built-in zip_source_file()
		{
			src = zip_source_file(m_zip_t, fs_path.u8path().c_str(), 0, -1);//sets mtime
			goto source_finish;
		}

	source_finish:
		{
			if (src == nullptr) {
				_zipfs_unchange_all();//.>mkdir revert
				_zipfs_zip_get_error_and_close("", fs_path);
				return m_ze;
			}

			if (!_zipfs_file_add_or_pull_from_source(zipfs_path, src, index)) {
				_zipfs_unchange_all();//.>mkdir revert
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return m_ze;
			}
			goto mtime;
		}

	mtime:
		{
			if (m_file_encrypt && m_file_encrypt_func != nullptr) {//zip_source_buffer was used
				time_t fs_mtime = fs_path.last_write_time();
				if (zip_file_set_mtime(m_zip_t, index, fs_mtime, ZIPFS_ZIP_FLAGS_NONE) == -1) {
					_zipfs_zip_get_error_and_close(zipfs_path, "");
					return m_ze;
				}
			}
			goto end;
		}

	end:
		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_pull_replace(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!fs_path.is_regular_file()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_FS_PATH_NOT_A_REGULAR_FILE, "/", fs_path);
			return m_ze;
		}

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return m_ze;
		}

		/*
			we could delete the file here,
			and forward to file_pull() ?
			= less duplicate code
		*/

		zip_source_t* src;
		if (m_file_encrypt && m_file_encrypt_func != nullptr)
			goto source_buffer_encrypt;
		else
			goto source_file;

	source_buffer_encrypt: //using <filesystem> to read file data and then encrypt them
		{
			_zipfs_source_buffer_encrypt(zipfs_path, fs_path, &src);
			goto source_finish;
		}

	source_file: //using built-in zip_source_file()
		{
			src = zip_source_file(m_zip_t, fs_path.u8path().c_str(), 0, -1);
			goto source_finish;
		}

	source_finish:
		{
			if (src == nullptr) {
				_zipfs_zip_get_error_and_close("", fs_path);
				return m_ze;
			}

			if (!_zipfs_file_add_replace_or_pull_replace_from_source(index, src)) {
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return m_ze;
			}
			goto end;
		}

	end:
		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite, ORPHAN orphan) {
		if (!
			zip_source_t_image_update())//make source backup
			return m_ze;

		if (!
			_zipfs_dir_pull(zipfs_path, fs_path, nullptr, overwrite, orphan, false)) {

			zip_source_t_revert_to_image();//restore image
			return m_ze;
		}

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_pull_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_result_t& query_result, OVERWRITE overwrite, ORPHAN orphan) {
		query_result.clear();
		if (!
			_zipfs_dir_pull(zipfs_path, fs_path, &query_result, overwrite, orphan, true))
			return m_ze;

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (fs_path.exists()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_TARGET_FILE_ALREADY_EXISTS, "/", fs_path);
			return m_ze;
		}

		if (!fs_path.parent_path().exists()) {
			std::filesystem::create_directories(fs_path.parent_path().platform_path());
		}

		zipfs_zip_stat_t stat_;
		std::vector<char> buf;
		if (!cat(zipfs_path, buf)) {
			return m_ze;
		}
		else if (!stat(zipfs_path, stat_)) {
			return m_ze;
		}
		else if (!fs_path.cat(buf)) {
			m_ze = ZIPFS_ERRSTR_ERROR_WRITING_TO_OUTPUT_FILE;
			m_ze.set_fs_path(fs_path);
			return m_ze;
		}
		else if (!fs_path.last_write_time(stat_.mtime)) {
			zipfs_debug_assert(false);
		}

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_extract_replace(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!fs_path.exists()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_TARGET_FILE_DOESNT_EXIST, "/", fs_path);
			return m_ze;
		}

		/*
			we could delete the file here,
			and forward to file_extract() ?
			= less duplicate code
		*/

		zipfs_zip_stat_t stat_;
		std::vector<char> buf;
		if (!cat(zipfs_path, buf)) {
			return m_ze;
		}
		else if (!stat(zipfs_path, stat_)) {
			return m_ze;
		}
		else if (!fs_path.cat(buf, std::ios::binary | std::ios::trunc)) {
			m_ze = ZIPFS_ERRSTR_ERROR_WRITING_TO_OUTPUT_FILE;
			m_ze.set_fs_path(fs_path);
			return m_ze;
		}
		else if (!fs_path.last_write_time(stat_.mtime)) {
			zipfs_debug_assert(false);
		}

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}


	zipfs_error_t zipfs_t::dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite) {
		if (!
			_zipfs_dir_extract(zipfs_path, fs_path, nullptr, overwrite, false))
			return m_ze;

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	 zipfs_error_t zipfs_t::dir_extract_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_result_t& query_result, OVERWRITE overwrite) {
		 query_result.clear();
		 if (!
			 _zipfs_dir_extract(zipfs_path, fs_path, &query_result, overwrite, true))
			 return m_ze;

		 zipfs_internal_assert(!m_ze.is_error());
		 return m_ze;
	}
}