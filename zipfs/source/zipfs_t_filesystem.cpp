#include <zipfs/zipfs_t.h>
#include <zipfs/zipfs_assert.h>
#include <zipfs/zipfs_error_strings.h>
#include <zipfs/zipfs_filesystem_path_t.h>
#include <fstream>
#include <filesystem>

namespace zipfs {

	bool zipfs_t::_zipfs_dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t* query_results, OVERWRITE overwrite, ORPHAN orphan, bool is_query) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		if (!fs_path.is_directory()) {
			_zipfs_zipfs_set_error(ZIPFS_ERRSTR_DIRECTORY_BAD_TARGET, "/", fs_path.platform_path());
			return false;
		}

		//query results
		zipfs_query_results_t query_results_;
		goto get_query_results;

		//query first
	get_query_results:
		{
			//parse fs
			{
				std::vector<filesystem_path_t> fs_paths;
				for (auto fs_entry = std::filesystem::recursive_directory_iterator(fs_path.platform_path()); fs_entry != std::filesystem::recursive_directory_iterator(); fs_entry++)
					fs_paths.push_back(fs_entry->path().lexically_normal().c_str());

				for (size_t e = 0; e < fs_paths.size(); e++) {
					filesystem_path_t fs_path_relative = std::filesystem::relative(fs_paths[e].platform_path(), fs_path.platform_path()).c_str();
					zipfs_path_t zipfs_path_ = zipfs_path + fs_path_relative.u8path();
					time_t fs_mtime = fs_paths[e].last_write_time();

					//do query
					QUERY_RESULT qr = _zipfs_get_query_result(overwrite, orphan, zipfs_path_, fs_paths[e]);
					query_results_.m_query_results.emplace_back(qr, zipfs_path_, "", zipfs_path_, fs_paths[e]);
				}
			}

			//parse zipfs (orphan detection)
			{
				std::vector<zipfs_path_t> ls_;
				if (!ls(zipfs_path, ls_, false))
					return false;

				for (size_t e = 0; e < ls_.size(); e++) {
					filesystem_path_t orphan_path = fs_path.u8path() + std::string("/") + ls_[e].string().substr(zipfs_path.string().size());

					//do query
					if (!orphan_path.exists()) {
						QUERY_RESULT qr = _zipfs_get_query_result(overwrite, orphan, ls_[e], orphan_path);
						query_results_.m_query_results.emplace_back(qr, ls_[e], "", ls_[e], orphan_path);
					}
				}
			}

			//query is done.
			if (is_query) {
				*query_results = query_results_;
				goto end;
			}
			else {
				goto pull_from_query_results;
			}
		}

		//pull first, then orphans
	pull_from_query_results:
		{
			for (const auto& qr : query_results_.m_query_results) {
				switch (qr.query_result) {
				case QUERY_RESULT::FILE_WRITE:
				case QUERY_RESULT::FILE_OVERWRITE: {
					if (!_zipfs_file_pull(qr.zipfs_path, qr.fs_path_cmp, qr.query_result))
						goto abort;
					break;
				}
				case QUERY_RESULT::DIR_ADD: {
					zipfs_internal_assert(!qr.zipfs_path.is_dir());//<-paths are taken from the filesystem (unix fail here?)
					zipfs_path_t dir = qr.zipfs_path.to_dir();
					if (!dir_add(dir))
						goto abort;
					if (!_zipfs_set_dir_mtime(dir, qr.fs_path_cmp.last_write_time()))
						goto abort;
					break;
				}
				}
			}

			goto orphans_from_query_results;
		}

		//orphans
	orphans_from_query_results:
		{
			//files first
			for (const auto& qr : query_results_.m_query_results) {
				if (qr.query_result == QUERY_RESULT::FILE_ORPHAN_DELETE) {
					if (!file_delete(qr.zipfs_path))
						goto abort;
				}
			}

			//then dirs
			for (const auto& qr : query_results_.m_query_results) {
				if (qr.query_result == QUERY_RESULT::DIR_ORPHAN_DELETE) {
					size_t count;
					if (!dir_delete(qr.zipfs_path, count))
						goto abort;
				}
			}

			goto end;
		}

	abort:
		return false;

	end:
		return true;
	}

	bool zipfs_t::_zipfs_dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t* query_results, OVERWRITE overwrite, bool is_query) {
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

		//query results
		zipfs_query_results_t query_results_;
		goto get_query_results;

		//query first
	get_query_results:
		{
			//parse zipfs
			{
				std::vector<zipfs_path_t> ls_;
				if (!ls(zipfs_path, ls_, false))
					return false;

				std::vector<std::pair<filesystem_path_t, time_t>>
					dsmtime;

				for (size_t e = 0; e < ls_.size(); e++) {
					filesystem_path_t extract_path = fs_path.u8path() + std::string("/") + ls_[e].string().substr(zipfs_path.string().size());

					//do query
					QUERY_RESULT qr = _zipfs_get_query_result(overwrite, extract_path, ls_[e]);
					query_results_.m_query_results.emplace_back(qr, "/", extract_path, ls_[e], extract_path);
				}
			}

			//query is done.
			if (is_query) {
				*query_results = query_results_;
				goto end;
			}
			else {
				goto extract_from_query_results;
			}
		}

		//extract
	extract_from_query_results:
		{
			//extract
			for (const auto& qr : query_results_.m_query_results) {
				switch (qr.query_result) {
				case QUERY_RESULT::DIR_ADD: {
					if (!std::filesystem::create_directory(qr.fs_path.platform_path()))//assumes ls() doesn't list a nested directory before its parent
						goto abort;
					break;
				}
				case QUERY_RESULT::FILE_WRITE:
				case QUERY_RESULT::FILE_OVERWRITE: {
					if (!_zipfs_file_extract(qr.zipfs_path_cmp, qr.fs_path, qr.query_result))
						goto abort;
					break;
				}
				}
			}

			//directories mtime here (Windows counter-bamboozle)
			for (const auto& qr : query_results_.m_query_results) {
				switch (qr.query_result) {
				case QUERY_RESULT::DIR_ADD: {
					zipfs_zip_stat_t stat_;
					if (!stat(qr.zipfs_path_cmp, stat_))
						goto abort;
					if (qr.fs_path.last_write_time(stat_.mtime))
						goto abort;
					break;
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

	bool zipfs_t::_zipfs_file_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, QUERY_RESULT qr) {
		switch (qr) {
		case QUERY_RESULT::FILE_WRITE:
		case QUERY_RESULT::FILE_OVERWRITE: {

			if (qr == QUERY_RESULT::FILE_WRITE && !dir_add(zipfs_path.parent_path()) || !_zipfs_open(ZIPFS_ZIP_FLAGS_NONE)) {
				return false;
			}
			zip_int64_t index_ = _zipfs_name_locate(zipfs_path);

			zip_source_t* src;
			bool from_buffer = m_file_encrypt && m_file_encrypt_func != nullptr;

			if (from_buffer) {
				_zipfs_source_buffer_encrypt(zipfs_path, fs_path.cat(), &src);
			}
			else {
				src = zip_source_file(m_zip_t, fs_path.u8path().c_str(), 0, -1);//takes care of mtime
			}

			if (src == nullptr) {
				_zipfs_unchange_all();
				_zipfs_zip_get_error_and_close("/", fs_path);
				return false;
			}

			bool from_source;
			switch (qr) {
			case QUERY_RESULT::FILE_WRITE: {
				from_source = _zipfs_file_add_or_pull_from_source(zipfs_path, src, index_);
				break;
			}
			case QUERY_RESULT::FILE_OVERWRITE: {
				from_source = _zipfs_file_add_replace_or_pull_replace_from_source(index_, src);
				break;
			}
			}
			if (!from_source) {
				_zipfs_unchange_all();
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return false;
			}

			/*
				set mtime if zip_source_buffer was used
			*/
			if (from_buffer) {
				time_t fs_mtime = fs_path.last_write_time();
				if (zip_file_set_mtime(m_zip_t, index_, fs_mtime, ZIPFS_ZIP_FLAGS_NONE) == -1) {
					_zipfs_zip_get_error_and_close(zipfs_path, "");
					return false;
				}
			}

			_zipfs_no_error_and_close();
			break;
		}
		case QUERY_RESULT::FILE_ORPHAN_KEEP:
		case QUERY_RESULT::FILE_DONT_OVERWRITE:
		case QUERY_RESULT::FILE_IS_BUT_NOT_FILE:
		case QUERY_RESULT::DISCARD:
		case QUERY_RESULT::NONE: {//nothing to do
			break;
		}
		default: {//not supposed to reach here.
			zipfs_internal_assert(false);
		}
		}

		return true;
	}

	bool zipfs_t::_zipfs_file_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, QUERY_RESULT qr) {
		switch (qr) {
		case QUERY_RESULT::FILE_WRITE:
		case QUERY_RESULT::FILE_OVERWRITE: {

			if (qr == QUERY_RESULT::FILE_WRITE && !fs_path.parent_path().exists() && !std::filesystem::create_directories(fs_path.parent_path().platform_path())) {
				_zipfs_zipfs_set_error("could not create parent directory.", "/", fs_path.parent_path().u8path());
				return false;
			}

			std::ios::openmode open_mode = std::ios::binary |
				(qr == QUERY_RESULT::FILE_OVERWRITE ? std::ios::trunc : NULL);

			zipfs_zip_stat_t stat_;
			std::vector<char> buf;
			if (!cat(zipfs_path, buf)) {
				return false;
			}
			else if (!stat(zipfs_path, stat_)) {
				return false;
			}
			else if (!fs_path.cat(buf, open_mode)) {//write
				m_ze = ZIPFS_ERRSTR_ERROR_WRITING_TO_OUTPUT_FILE;
				m_ze.set_fs_path(fs_path);
				return false;
			}
			else if (!fs_path.last_write_time(stat_.mtime)) {//mtime
				zipfs_debug_assert(false);
			}

			break;
		}
		case QUERY_RESULT::FILE_ORPHAN_KEEP:
		case QUERY_RESULT::FILE_DONT_OVERWRITE:
		case QUERY_RESULT::FILE_IS_BUT_NOT_FILE:
		case QUERY_RESULT::DISCARD:
		case QUERY_RESULT::NONE: {//nothing to do
			break;
		}
		}

		return true;
	}

	zipfs_error_t zipfs_t::file_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		QUERY_RESULT qr = _zipfs_get_query_result(overwrite, ORPHAN::KEEP, zipfs_path, fs_path);
		_zipfs_file_pull(zipfs_path, fs_path, qr);
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite, ORPHAN orphan) {
		if (!
			_zipfs_image_internal_update())//make source backup
			return m_ze;

		if (!
			_zipfs_dir_pull(zipfs_path, fs_path, nullptr, overwrite, orphan, false)) {

			_zipfs_revert_to_image_internal();//restore image
			return m_ze;
		}

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_pull_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t& query_results, OVERWRITE overwrite, ORPHAN orphan) {
		query_results.m_query_results.clear();
		if (!
			_zipfs_dir_pull(zipfs_path, fs_path, &query_results, overwrite, orphan, true))
			return m_ze;

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		QUERY_RESULT qr = _zipfs_get_query_result(overwrite, fs_path, zipfs_path);
		_zipfs_file_extract(zipfs_path, fs_path, qr);
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite) {
		if (!//should be try/catch
			_zipfs_dir_extract(zipfs_path, fs_path, nullptr, overwrite, false))
			return m_ze;

		zipfs_internal_assert(!m_ze.is_error());
		return m_ze;
	}

	 zipfs_error_t zipfs_t::dir_extract_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t& query_results, OVERWRITE overwrite) {
		 query_results.m_query_results.clear();
		 if (!
			 _zipfs_dir_extract(zipfs_path, fs_path, &query_results, overwrite, true))
			 return m_ze;

		 zipfs_internal_assert(!m_ze.is_error());
		 return m_ze;
	}
}