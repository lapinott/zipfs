#pragma once

#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>
#include <zip.h>
#include <string>
#include <iostream>
#include <exception>

namespace zipfs {

	class zipfs_error_t { //handles zip errors and zipfs errors
	private:

		friend struct zipfs_t;

		zip_error_t
			m_zip_error;

		std::string
			m_zipfs_error;

		zipfs_path_t
			m_zipfs_path = "/";

		filesystem_path_t
			m_filesystem_path;

	public:

		zipfs_error_t();

		zipfs_error_t(zip_error_t* zip_error);//from zip_error_t

		zipfs_error_t(const std::string& zipfs_error);//from std::string

		zipfs_error_t(const char* zipfs_error);//from cstring

		zipfs_error_t(const zipfs_error_t&);

		zipfs_error_t(zipfs_error_t&&) noexcept;

		zipfs_error_t& operator=(zipfs_error_t&&) noexcept;

		zipfs_error_t& operator=(const zipfs_error_t&);

		~zipfs_error_t();

	public:

		bool is_error() const;

		operator bool();

		operator std::string() const;

		static zipfs_error_t no_error();

	private: //.>paths set

		void set_zipfs_path(const zipfs_path_t& p);

		void set_fs_path(const filesystem_path_t& p);
	};

	std::ostream& operator<<(std::ostream& os, const zipfs_error_t& ze);
}