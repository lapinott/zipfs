#pragma once

#include <zipfs/zipfs_error_t.h>
#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>
#include <zipfs/zipfs_zip_stat_t.h>
#include <zipfs/zipfs_enums.h>
#include <zipfs/zipfs_query_results_t.h>
#include <zipfs/zipfs_index_t.h>
#include <zipfs/zipfs_zip_flags.h>
#include <zip.h>
#include <vector>
#include <map>

#define ZIPFS_USE_ZIPFS_INDEX 1

namespace zipfs {

	struct zipfs_t { //zip 'heap' filesystem interface
	private:

		zip_source_t*
			m_zip_source_t;

		std::vector<char>
			m_zip_source_t_image_user,
			m_zip_source_t_image_internal;

		zip_t*
			m_zip_t;

		char*
			m_zip_source_t_buffer;

#define ZIPFS_ZIP_SOURCE_T_BUFFER_AUTO_FREE 1 //<.buggy if 0

		const static bool
			s_zip_source_t_buffer_auto_free = ZIPFS_ZIP_SOURCE_T_BUFFER_AUTO_FREE;

		zip_int32_t
			m_compression;

		zip_uint32_t
			m_compression_flags;

		zipfs_error_t
			m_ze;

	public:

		typedef void(*file_encrypt_func)(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len);
		typedef void(*file_decrypt_func)(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len);

	private:

		file_encrypt_func
			m_file_encrypt_func;

		file_decrypt_func
			m_file_decrypt_func;

		bool
			m_file_encrypt,
			m_file_decrypt;

	private:

		zipfs_index_t					//this index because zip_name_locate() is giving me trouble (should be patched in next libzip version [now=26.03.2022])
			m_zipfs_index_t,			//maps a zipfs_path_t to its corresponding in-archive zip_int64_t index
			m_zipfs_index_t_image_user,
			m_zipfs_index_t_image_internal;

	public:

		zipfs_t(zipfs_error_t& ze); //.>creates an empty archive in memory

		zipfs_t(char* buffer, size_t byte_sz, zipfs_error_t& ze); //.>creates an archive in memory from buffer

		zipfs_t(const zipfs_t&) = delete;

		~zipfs_t();


	//.>internal

	private:

		/* note
			_* names should be allowed here becos they are member functions
		*/

		bool
			_zipfs_source_new(char* buffer, size_t byte_sz);

		void
			_zipfs_source_free();

		void
			_zipfs_error_init();

		bool
			_zipfs_open(int open_flags);

		void
			_zipfs_close(),
			_zipfs_unchange_all(),
			_zipfs_no_error_and_close();

		/*
			we could return m_ze& here
		*/
		void
			_zipfs_zip_get_error(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path),//<-on peut supprimer ça.
			_zipfs_zip_get_error_and_close(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path),
			_zipfs_zipfs_set_error(const std::string& zipfs_error, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path),
			_zipfs_zipfs_set_error_and_close(const std::string& zipfs_error, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path),
			_zipfs_zip_source_error(),
			_zipfs_zip_source_error_and_source_close();

		zip_int64_t
			_zipfs_name_locate(const zipfs_path_t& zipfs_path);

		bool
			_zipfs_file_add_or_pull_from_source(const zipfs_path_t& zipfs_path, zip_source_t* src, zip_int64_t& index),
			_zipfs_file_add_replace_or_pull_replace_from_source(zip_int64_t index, zip_source_t* src);

		bool
			_zipfs_dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t* query_results, OVERWRITE overwrite, ORPHAN orphan, bool is_query),
			_zipfs_dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t* query_results, OVERWRITE overwrite, bool is_query);

		bool
			_zipfs_source_buffer_encrypt(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer, zip_source_t** src);

		bool
			_zipfs_set_dir_mtime(const zipfs_path_t& zipfs_path, time_t mtime);

		bool
			_zipfs_revert_to_image_internal(),
			_zipfs_image_internal_update();

		QUERY_RESULT
			_zipfs_get_query_result(OVERWRITE overwrite, ORPHAN orphan, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path),//pull
			_zipfs_get_query_result(OVERWRITE overwrite, const filesystem_path_t& fs_path, const zipfs_path_t& zipfs_path),//extract
			_zipfs_get_query_result(OVERWRITE overwrite, const zipfs_path_t& zipfs_path);//add

		bool
			_zipfs_file_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, QUERY_RESULT qr),
			_zipfs_file_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, QUERY_RESULT qr),
			_zipfs_file_add(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer, QUERY_RESULT qr);

	//\.end internal


	//.>public interface

	public: //.>write operations [<-filesystem]

		zipfs_error_t
			file_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite = OVERWRITE::NEVER);

		zipfs_error_t
			dir_pull(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite = OVERWRITE::NEVER, ORPHAN orphan = ORPHAN::KEEP);

		zipfs_error_t
			dir_pull_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t& query_results, OVERWRITE overwrite = OVERWRITE::NEVER, ORPHAN orphan = ORPHAN::KEEP);


	public: //.>read-only operations [->filesystem]

		zipfs_error_t
			file_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite = OVERWRITE::NEVER);

		/*
			implementing an ORPHAN flag here is probably not a good idea...
		*/
		zipfs_error_t
			dir_extract(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, OVERWRITE overwrite = OVERWRITE::NEVER);

		zipfs_error_t
			dir_extract_query(const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path, zipfs_query_results_t& query_results, OVERWRITE overwrite = OVERWRITE::NEVER);


	public: //.>write operations [<-memory]

		zipfs_error_t
			file_add(const zipfs_path_t& zipfs_path, const std::vector<char>& buffer, OVERWRITE overwrite = OVERWRITE::NEVER),
			file_add(const zipfs_path_t& zipfs_path, const std::string& buffer, OVERWRITE overwrite = OVERWRITE::NEVER);

		zipfs_error_t
			file_delete(const zipfs_path_t& zipfs_path);

		zipfs_error_t
			file_rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path);

		zipfs_error_t
			dir_add(const zipfs_path_t& zipfs_path);

		zipfs_error_t
			dir_delete(const zipfs_path_t& zipfs_path, size_t& delete_count);

		zipfs_error_t
			dir_rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path);


	public: //.>read-only operations [->memory]

		zipfs_error_t
			cat(const zipfs_path_t& zipfs_path, std::vector<char>& result, bool read_compressed = false);

		zipfs_error_t
			ls(const zipfs_path_t& zipfs_path, std::vector<zipfs_path_t>& result, bool strict = true);

		zipfs_error_t
			stat(const zipfs_path_t& zipfs_path, zipfs_zip_stat_t& result);

		zipfs_error_t
			index(const zipfs_path_t& zipfs_path, zip_int64_t& result);

		zipfs_error_t
			num_entries(zip_int64_t& result);


	public: //.>source data

		zipfs_error_t
			get_source(std::vector<char>& result);
#if 0
		//afaik libzip doesn't enable this
		zipfs_error_t
			get_source_ptr(char** buffer, size_t& byte_sz) const;
#endif

	public: //.>image data

		zipfs_error_t
			zipfs_image_has_modifications(bool& result);

		zipfs_error_t
			zipfs_revert_to_image();

		zipfs_error_t
			zipfs_image_update();


	public: //.>compression

		void
			set_compression(zip_int32_t compression, zip_uint32_t compression_flags = 0);


	public: //.>encryption/decryption

		void
			set_file_encrypt(bool encrypt),
			set_file_decrypt(bool decrypt);

		void
			set_file_encrypt_func(file_encrypt_func f),
			set_file_decrypt_func(file_decrypt_func f);
	};

	//\. end public interface
}