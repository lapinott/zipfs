#include <zipfs/zipfs_t.h>
#include <zipfs/zipfs_assert.h>
#include <zipfs/zipfs_error_strings.h>
#include <zipint.h>//.>zip_source_t

namespace zipfs {

	zipfs_t::zipfs_t(zipfs_error_t& ze) :
		m_compression{ ZIP_CM_DEFLATE }, m_compression_flags{ 0 }, m_zip_source_t{ nullptr }, m_zip_t{ nullptr }, m_zip_source_t_initial_buffer{ nullptr }, m_ze{ zipfs_error_t::no_error() },
		m_file_encrypt_func{ nullptr }, m_file_decrypt_func{ nullptr }, m_file_encrypt{ false }, m_file_decrypt{ false } {

		if (!_zipfs_source_new(nullptr, 0)) {
			ze = m_ze;
			return;
		}

		//.>do we have a valid archive? test open/close
		if (!_zipfs_open(ZIPFS_ZIP_FLAGS_NONE)) {
			ze = m_ze;
			return;
		}
		_zipfs_no_error_and_close();
		ze = m_ze;
	}

	zipfs_t::zipfs_t(char* buffer, size_t byte_sz, zipfs_error_t& ze) :
		m_compression{ ZIP_CM_DEFLATE }, m_compression_flags{ 0 }, m_zip_source_t{ nullptr }, m_zip_t{ nullptr }, m_zip_source_t_initial_buffer{ nullptr }, m_ze{ zipfs_error_t::no_error() },
		m_file_encrypt_func{ nullptr }, m_file_decrypt_func{ nullptr }, m_file_encrypt{ false }, m_file_decrypt{ false } {

		if (!_zipfs_source_new(buffer, byte_sz)) {
			ze = m_ze;
			return;
		}

		if (!zip_source_t_image_update()) {
			ze = m_ze;
			return;
		}

		//.>do we have a valid archive? test open/close
		if (!_zipfs_open(ZIPFS_ZIP_FLAGS_NONE)) {
			ze = m_ze;
			return;
		}
		_zipfs_no_error_and_close();
		ze = m_ze;
	}

	zipfs_t::~zipfs_t() {
		_zipfs_source_free();
	}

	bool zipfs_t::_zipfs_source_new(char* buffer, size_t byte_sz) {
		zipfs_internal_assert(m_zip_t == nullptr);
		zipfs_internal_assert(m_zip_source_t == nullptr);
		zipfs_internal_assert(m_zip_source_t_initial_buffer == nullptr);

		// zip_source_buffer_create https://libzip.org/documentation/zip_source_buffer_create.html
		// If freep is non-zero, the buffer will be freed when it is no longer needed,
		// data must remain valid for the lifetime of the created source.

		if (buffer != nullptr && byte_sz != 0) {
			m_zip_source_t_initial_buffer = new char[byte_sz];//acquire buffer (copy)
			char* it = std::copy(buffer, buffer + byte_sz, m_zip_source_t_initial_buffer);
			zipfs_internal_assert(it == m_zip_source_t_initial_buffer + byte_sz);
			zip_source_t* zs = zip_source_buffer_create(m_zip_source_t_initial_buffer, byte_sz, s_zip_source_t_initial_buffer_auto_free ? 1 : 0, &m_ze.m_zip_error);
			if (zs == nullptr)
				return false;

			m_zip_source_t = zs;
			return true;
		}
		else {
			zip_source_t* zs = zip_source_buffer_create(nullptr, 0, s_zip_source_t_initial_buffer_auto_free ? 1 : 0, &m_ze.m_zip_error);
			if (zs == nullptr)
				return false;

			m_zip_source_t = zs;
			return true;
		}
	}

	void zipfs_t::_zipfs_source_free()  {
		zipfs_internal_assert(m_zip_t == nullptr);
		zipfs_internal_assert(m_zip_source_t != nullptr);
		zipfs_internal_assert(m_zip_source_t->src == nullptr);
		zipfs_internal_assert(m_zip_source_t->refcount == 1);
		//si on utilise freep = 0, il faut supprimer le buffer avec delete[] => (mais buggé si archive créée avec WinRar)
		//si on utilise freep = 1, il faut supprimer le buffer avec zip_source_free() [avec refcount=0]
#if ZIPFS_ZIP_SOURCE_T_INITIAL_BUFFER_AUTO_FREE == 1
		(void)zip_source_free(m_zip_source_t);//ref--
#else
		(void)zip_source_free(m_zip_source_t);//ref-- //<-DO NOT CREATE THE ARCHIVE WITH WINRAR!! //&!zip_rename??
		if (m_zip_source_t_initial_buffer != nullptr)
			delete[] m_zip_source_t_initial_buffer;
#endif
		m_zip_source_t = nullptr;
		m_zip_source_t_initial_buffer = nullptr;
	}

	void zipfs_t::_zipfs_error_init() {
		m_ze = zipfs_error_t::no_error();
	}

	bool zipfs_t::_zipfs_open(int open_flags) {
		zipfs_internal_assert(m_zip_t == nullptr);

		_zipfs_error_init();//init error
		m_zip_t = zip_open_from_source(m_zip_source_t, ZIP_CHECKCONS | open_flags, &m_ze.m_zip_error);

		if (m_zip_t == nullptr && m_ze.m_zip_error.zip_err == ZIP_ER_DELETED) {//archive was emptied and is not valid anymore, recreate source
			_zipfs_error_init();//init error
			_zipfs_source_free();
			_zipfs_source_new(nullptr, 0);
			m_zip_t = zip_open_from_source(m_zip_source_t, ZIP_CHECKCONS | open_flags, &m_ze.m_zip_error);
		}

		if (m_zip_t != nullptr) {
			if (m_zipfs_index_t.empty()) {//build index if it's empty
				m_zipfs_index_t.init(m_zip_t);
#ifdef _DEBUG
				zipfs_internal_assert(m_zipfs_index_t.verify(m_zip_t));
#endif
			}
		}

		return !m_ze.is_error();
	}

	void zipfs_t::_zipfs_close() {
		zipfs_internal_assert(m_zip_t != nullptr);

		(void)zip_source_keep(m_zip_source_t);//ref++
		if (zip_close(m_zip_t) == -1) {
			zipfs_internal_assert(false);//couldn't close, debug it
		}

		m_zip_t = nullptr;

#if 0
		if ((open_flags & ZIP_RDONLY) == 0) {//invalidate index only if !ZIP_RDONLY operation?
			m_zipfs_index_t.clear();
		}
#else
		m_zipfs_index_t.clear();
#endif
	}

	void zipfs_t::_zipfs_unchange_all() {
		zipfs_internal_assert(m_zip_t != nullptr);

		if (zip_unchange_all(m_zip_t) == -1) {
			zipfs_internal_assert(false);//couldn't unchange, debug it
		}
	}

	void zipfs_t::_zipfs_no_error_and_close() {
		zipfs_internal_assert(!m_ze.is_error());
		_zipfs_close();
	}

	void zipfs_t::_zipfs_zip_get_error(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		m_ze = zip_get_error(m_zip_t);

		if (!zipfs_path.is_root())
			m_ze.set_zipfs_path(zipfs_path);
		if (!fs_path.platform_path().empty())
			m_ze.set_fs_path(fs_path);

		zipfs_internal_assert(m_ze.is_error());
	}

	void zipfs_t::_zipfs_zipfs_set_error(const std::string& zipfs_error, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		m_ze = zipfs_error.c_str();

		if (!zipfs_path.is_root())
			m_ze.set_zipfs_path(zipfs_path);
		if (!fs_path.platform_path().empty())
			m_ze.set_fs_path(fs_path);

		zipfs_internal_assert(m_ze.is_error());
	}

	void zipfs_t::_zipfs_zip_get_error_and_close(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		_zipfs_zip_get_error(zipfs_path, fs_path);
		_zipfs_close();
	}

	void zipfs_t::_zipfs_zipfs_set_error_and_close(const std::string& zipfs_error, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		_zipfs_zipfs_set_error(zipfs_error, zipfs_path, fs_path);
		_zipfs_close();
	}

	zip_int64_t zipfs_t::_zipfs_name_locate(const zipfs_path_t& zipfs_path) {
		zipfs_internal_assert(m_zip_t != nullptr);
#if ZIPFS_USE_ZIPFS_INDEX
		return m_zipfs_index_t.index(zipfs_path);
#else
		return zip_name_locate(m_zip_t, zipfs_path.libzip_path(), ZIPFS_FL_ENC);//will probably be patched in next libzip release; today = 29.03.2022
#endif
	}

	bool zipfs_t::_zipfs_file_add_or_pull_from_source(const zipfs_path_t& zipfs_path, zip_source_t* src, zip_int64_t& index) {
		zipfs_internal_assert(m_zip_t != nullptr);

		if ((index = zip_file_add(m_zip_t, zipfs_path.libzip_path(), src, ZIPFS_FL_ENC)) == -1) {
			(void)zip_source_free(src);
			return false;
		}

		if (zip_set_file_compression(m_zip_t, index, m_compression, m_compression_flags) == -1) {
			_zipfs_unchange_all();//.>zip_file_add revert
			//zip_source_free();//.>not here: https://libzip.org/documentation/zip_source_free.html
			return false;
		}

		return true;
	}

	bool zipfs_t::_zipfs_file_add_replace_or_pull_replace_from_source(zip_int64_t index, zip_source_t* src) {
		zipfs_internal_assert(m_zip_t != nullptr);

		if (zip_file_replace(m_zip_t, index, src, ZIPFS_FL_ENC) == -1) {
			(void)zip_source_free(src);
			return false;
		}

		if (zip_set_file_compression(m_zip_t, index, m_compression, m_compression_flags) == -1) {
			_zipfs_unchange_all();//.>zip_file_replace revert
			//zip_source_free();//.>not here: https://libzip.org/documentation/zip_source_free.html
			return false;
		}

		return true;
	}

	bool zipfs_t::_zipfs_source_buffer_encrypt(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer, zip_source_t** src) {
		uint8_t* ret_buf = nullptr;
		size_t ret_len;
		{
			m_file_encrypt_func(zipfs_path.c_str(), reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), &ret_buf, &ret_len);
			*src = zip_source_buffer(m_zip_t, ret_buf, ret_len, 1);//auto-free ici (on a besoin de ret_buf* plus tard)
		}
		return *src != nullptr;
	}

	bool zipfs_t::_zipfs_set_dir_mtime(const zipfs_path_t& zipfs_path, time_t mtime) {
		zipfs_internal_assert(m_zip_t == nullptr);
		zipfs_internal_assert(zipfs_path.is_dir());

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return false;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return false;
		}

		if (zip_file_set_mtime(m_zip_t, index, mtime, ZIPFS_ZIP_FLAGS_NONE) == -1) {
			_zipfs_zip_get_error_and_close(zipfs_path, "");
			return false;
		}

		_zipfs_no_error_and_close();
		return true;
	}

	zipfs_error_t zipfs_t::file_add(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!
			dir_add(zipfs_path.parent_path()))
			return m_ze;

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index != -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_TARGET_FILE_ALREADY_EXISTS, zipfs_path, "");
			return m_ze;
		}

		zip_source_t* src;
		if (m_file_encrypt && m_file_encrypt_func != nullptr)
			goto source_buffer_encrypt;
		else
			goto source_buffer;

	source_buffer_encrypt:
		{
			_zipfs_source_buffer_encrypt(zipfs_path, buffer, &src);
			goto source_finish;
		}

	source_buffer:
		{
			src = zip_source_buffer(m_zip_t, buffer.data(), buffer.size(), 0);//pas de auto-free ici (buffer est std::vector<char>)
			goto source_finish;
		}

	source_finish:
		{
			if (src == nullptr) {
				_zipfs_unchange_all();//.>mkdir revert
				_zipfs_zip_get_error_and_close("/", "");//.>buffer error
				return m_ze;
			}

			if (!_zipfs_file_add_or_pull_from_source(zipfs_path, src, index)) {
				_zipfs_unchange_all();//.>mkdir revert
				_zipfs_zip_get_error_and_close("/", "");//.>buffer error
				return m_ze;
			}
			goto end;
		}

	end:
		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_add(const zipfs_path_t& zipfs_path, const std::string& buffer) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		return file_add(zipfs_path, std::vector<char>{ buffer.begin(), buffer.end() });
	}

	zipfs_error_t zipfs_t::file_add_replace(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_TARGET_FILE_DOESNT_EXIST, zipfs_path, "");
			return m_ze;
		}

		/*
			we could delete the file here,
			and forward to file_add() ?
			= less duplicate code
		*/

		zip_source_t* src;
		if (m_file_encrypt && m_file_encrypt_func != nullptr)
			goto source_buffer_encrypt;
		else
			goto source_buffer;

	source_buffer_encrypt:
		{
			_zipfs_source_buffer_encrypt(zipfs_path, buffer, &src);
			goto source_finish;
		}

	source_buffer:
		{
			src = zip_source_buffer(m_zip_t, buffer.data(), buffer.size(), 0);//pas de auto-free ici (buffer est std::vector<char>)
			goto source_finish;
		}

	source_finish:
		{
			if (src == nullptr) {
				_zipfs_zip_get_error_and_close("/", "");//.>buffer error
				return m_ze;
			}

			if (!_zipfs_file_add_replace_or_pull_replace_from_source(index, src)) {
				_zipfs_zip_get_error_and_close("/", "");//.>buffer error
				return m_ze;
			}
			goto end;
		}

	end:
		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_add_replace(const zipfs_path_t& zipfs_path, const std::string& buffer) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		return file_add_replace(zipfs_path, std::vector<char>{ buffer.begin(), buffer.end() });
	}

	zipfs_error_t zipfs_t::file_delete(const zipfs_path_t& zipfs_path) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return m_ze;
		}

		if (zip_delete(m_zip_t, index) == -1) {
			_zipfs_zip_get_error_and_close(zipfs_path, "");
			return m_ze;
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::file_rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path) {
		zipfs_usage_assert(zipfs_path.is_file() && zipfs_rename_path.is_file(), ZIPFS_ERRSTR_FILE_PATHS_EXPECTED);

		if (!
			dir_add(zipfs_path.parent_path()))
			return m_ze;

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_unchange_all();//.>mkdir revert
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return m_ze;
		}

		int rename = zip_file_rename(m_zip_t, index, zipfs_rename_path.libzip_path(), ZIPFS_FL_ENC);
		if (rename == -1) {
			_zipfs_zip_get_error_and_close(zipfs_path, "");
			return m_ze;
		}
		else if (!m_zipfs_index_t.rename(zipfs_path, zipfs_rename_path)) {
			zipfs_internal_assert(false);
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_add(const zipfs_path_t& zipfs_path) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		std::vector<std::string> tree = zipfs_path.tree();
		zipfs_path_t dir = "/";
		for (size_t d = 0; d < tree.size(); d++) {
			dir += (tree[d] + "/");
			zipfs_internal_assert(dir.is_dir());
			zip_int64_t index = _zipfs_name_locate(dir);
			if (index == -1) {
				if ((index = zip_dir_add(m_zip_t, dir.libzip_path_dir_add().c_str(), ZIPFS_FL_ENC)) == -1) {//zip_dir_add doesn't expect trailing '/'
					_zipfs_unchange_all();
					_zipfs_zip_get_error_and_close(dir, "");
					return m_ze;
				}
			}
			else {
				//dir exists; not an error
			}
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_delete(const zipfs_path_t& zipfs_path, size_t& delete_count) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		std::vector<zipfs_path_t> ls_;
		if (!
			ls(zipfs_path, ls_))
			return m_ze;

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		delete_count = 0;
		for (const zipfs_path_t& p : ls_) {
			zip_int64_t index = _zipfs_name_locate(p);
			if (index == -1) {
				_zipfs_unchange_all();
				delete_count = 0;
				_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, p, "");
				return m_ze;
			}
			else if (zip_delete(m_zip_t, index) == -1) {
				_zipfs_unchange_all();
				delete_count = 0;
				_zipfs_zip_get_error_and_close(p, "");
				return m_ze;
			}

			delete_count++;
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::dir_rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path) {
		zipfs_usage_assert(zipfs_path.is_dir() && zipfs_rename_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATHS_EXPECTED);

		std::vector<zipfs_path_t> ls_;
		if (!
			ls(zipfs_path, ls_))
			return m_ze;

		if (!
			_zipfs_open(ZIPFS_ZIP_FLAGS_NONE))
			return m_ze;

		for (zipfs_path_t& p : ls_) {
			zip_int64_t index = _zipfs_name_locate(p);
			if (index == -1) {
				_zipfs_unchange_all();
				_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, p, "");
				return m_ze;
			}
			else {
				zipfs_path_t rename_path = zipfs_rename_path + p.string().substr(zipfs_path.string().length());
				if (zip_file_rename(m_zip_t, index, rename_path.libzip_path(), ZIPFS_FL_ENC) == -1) {
					_zipfs_unchange_all();
					_zipfs_zip_get_error_and_close(rename_path, "");
					return m_ze;
				}
				else if (!m_zipfs_index_t.rename(p, rename_path)) {
					zipfs_internal_assert(false);
				}
			}
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::cat(const zipfs_path_t& zipfs_path, std::vector<char>& result, bool read_compressed) {
		zipfs_usage_assert(zipfs_path.is_file(), ZIPFS_ERRSTR_FILE_PATH_EXPECTED);

		if (!
			_zipfs_open(ZIP_RDONLY))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return m_ze;
		}
		else {
			zip_stat_t stat;
			zip_stat_init(&stat);
			if (zip_stat_index(m_zip_t, index, NULL, &stat) == -1) {
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return m_ze;
			}
			else if (!(stat.valid & ZIP_STAT_SIZE)) {
				_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_FILE_CANNOT_READ_SIZE, zipfs_path, "");
				return m_ze;
			}
			else {
				zip_file_t* file = zip_fopen_index(m_zip_t, index, /*ZIPFS_FL_ENC*/ 0 | (read_compressed ? ZIP_FL_COMPRESSED : ZIPFS_ZIP_FLAGS_NONE));
				if (file == nullptr) {
					_zipfs_zip_get_error_and_close(zipfs_path, "");
					return m_ze;
				}
				else {
					std::vector<char> buf(stat.size);
					zip_int64_t read = zip_fread(file, buf.data(), stat.size);
					if (read == -1) {
						zip_fclose(file);//Upon successful completion 0 is returned. Otherwise, the error code is returned.
						_zipfs_zip_get_error_and_close(zipfs_path, "");
						return m_ze;
					}
					else if (read != stat.size) {
						zip_fclose(file);//Upon successful completion 0 is returned. Otherwise, the error code is returned.
						_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_FILE_CANNOT_READ_ALL, zipfs_path, "");
						return m_ze;
					}
					else if (zip_fclose(file) != 0) {
						_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_FILE_CANNOT_CLOSE, zipfs_path, "");
						return m_ze;
					}
					else if (m_file_decrypt && m_file_decrypt_func != nullptr) {//decryption
						uint8_t* ret_buf = nullptr;
						size_t ret_len;
						{
							m_file_decrypt_func(zipfs_path.c_str(), reinterpret_cast<uint8_t*>(buf.data()), buf.size(), &ret_buf, &ret_len);
							result.assign(ret_buf, ret_buf + ret_len);
						}
						delete[] ret_buf;
					}
					else {//no decryption
						result = std::move(buf);
					}
				}
			}
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::ls(const zipfs_path_t& zipfs_path, std::vector<zipfs_path_t>& result, bool strict) {
		zipfs_usage_assert(zipfs_path.is_dir(), ZIPFS_ERRSTR_DIRECTORY_PATH_EXPECTED);

		zip_int64_t num_entries_;
		if (!
			num_entries(num_entries_))
			return m_ze;
		
		if (!
			_zipfs_open(ZIP_RDONLY))
			return m_ze;

		result.clear();
#if 0
		if (!strict)
			result.push_back(zipfs_path);
#endif
		for (zip_int64_t e = 0; e < num_entries_; e++) {
			const char* name = zip_get_name(m_zip_t, e, ZIPFS_FL_ENC);
			if (name == nullptr) {
				result.clear();
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return m_ze;
			}
#if 0
			std::string name_ = name;
#endif
			if (std::string(name).find(zipfs_path.libzip_path()) == 0)
				result.emplace_back("/" + std::string(name));
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::stat(const zipfs_path_t& zipfs_path, zipfs_zip_stat_t& result) {
		if (!
			_zipfs_open(ZIP_RDONLY))
			return m_ze;

		zip_int64_t index = _zipfs_name_locate(zipfs_path);
		if (index == -1) {
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_CANNOT_LOCATE_NAME, zipfs_path, "");
			return m_ze;
		}
		else {
			zip_stat_t stat;
			zip_stat_init(&stat);
			if (zip_stat_index(m_zip_t, index, ZIPFS_ZIP_FLAGS_NONE, &stat) == -1) {
				_zipfs_zip_get_error_and_close(zipfs_path, "");
				return m_ze;
			}
			result = stat;
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::index(const zipfs_path_t& zipfs_path, zip_int64_t& result) {
		if (!
			_zipfs_open(ZIP_RDONLY))
			return m_ze;

		result = _zipfs_name_locate(zipfs_path);
		if (result == -1) { /*not an error*/ }

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::num_entries(zip_int64_t& result) {
		if (!
			_zipfs_open(ZIP_RDONLY))
			return m_ze;

		result = zip_get_num_entries(m_zip_t, ZIPFS_ZIP_FLAGS_NONE);
		if (result == -1) {//archive is null
			_zipfs_zipfs_set_error_and_close(ZIPFS_ERRSTR_ARCHIVE_IS_NULL, "/", "");
			return m_ze;
		}

		_zipfs_no_error_and_close();
		return m_ze;
	}

	zipfs_error_t zipfs_t::get_source(std::vector<char>& result) {
		zip_stat_t stat;
		if (zip_source_stat(m_zip_source_t, &stat) == -1) {
			result = {};
			m_ze = zip_source_error(m_zip_source_t);
			return m_ze;
		}
		else if (!(stat.valid & ZIP_STAT_SIZE)) {
			result = {};
			m_ze = ZIPFS_ERRSTR_SOURCE_STAT_SIZE_NOT_VALID;
			return m_ze;
		}
		else if (stat.size == 0) {
			result = {};
			m_ze = zipfs_error_t::no_error();
			return m_ze;
		}

		//read source
		if (zip_source_open(m_zip_source_t) == -1) {
			result = {};
			m_ze = zip_source_error(m_zip_source_t);
			return m_ze;
		}
		else {
			std::vector<char> buf(stat.size);
			zip_int64_t read = zip_source_read(m_zip_source_t, buf.data(), stat.size);
			if (read == -1) {
				result = {};
				m_ze = zip_source_error(m_zip_source_t);
				zip_source_close(m_zip_source_t);//-1 on failure
				return m_ze;
			}
			else if (read != stat.size) {
				result = {};
				m_ze = ZIPFS_ERRSTR_SOURCE_CANNOT_READ_ALL;
				zip_source_close(m_zip_source_t);//-1 on failure
				return m_ze;
			}

			result = std::move(buf);
			zip_source_close(m_zip_source_t);//-1 on failure
		}

		return zipfs_error_t::no_error();
	}

	void zipfs_t::set_compression(zip_int32_t compression, zip_uint32_t compression_flags) {
		m_compression = compression;
		m_compression_flags = compression_flags;
	}

	zipfs_error_t zipfs_t::zip_source_t_has_modifications(bool& result) {
		std::vector<char> buf;
		if (!
			get_source(buf)) {
			result = (bool)-1;
			return m_ze;
		}
		
		if (buf.size() != m_zip_source_t_image.size()) {
			result = true;
		}
		else {
			for (size_t b = 0; b < buf.size(); b++) {
				if (buf[b] != m_zip_source_t_image[b]) {
					result = true;
					break;
				}
			}
			result = false;
		}

		return zipfs_error_t::no_error();
	}

	zipfs_error_t zipfs_t::zip_source_t_revert_to_image() {
		_zipfs_source_free();
		if (!
			_zipfs_source_new(m_zip_source_t_image.data(), m_zip_source_t_image.size()))
			return m_ze;

		m_zipfs_index_t = m_zipfs_index_t_image;

		return zipfs_error_t::no_error();
	}

	zipfs_error_t zipfs_t::zip_source_t_image_update() {
		std::vector<char> buf;
		if (!
			get_source(buf))
			return m_ze;

		m_zip_source_t_image.clear();
		m_zip_source_t_image.assign(buf.data(), buf.data() + buf.size());
		m_zipfs_index_t_image = m_zipfs_index_t;

		return zipfs_error_t::no_error();
	}

	void zipfs_t::set_file_encrypt(bool encrypt) {
		m_file_encrypt = encrypt;
	}

	void zipfs_t::set_file_decrypt(bool decrypt) {
		m_file_decrypt = decrypt;
	}

	void zipfs_t::set_file_encrypt_func(file_encrypt_func f) {
		m_file_encrypt_func = f;
	}

	void zipfs_t::set_file_decrypt_func(file_decrypt_func f) {
		m_file_decrypt_func = f;
	}
}