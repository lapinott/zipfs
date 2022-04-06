#pragma once

#include <string>
#include <vector>

namespace zipfs {

	struct zipfs_path_t {//absolute path
	private:

		std::string
			m_path;

	public:

		zipfs_path_t(const char* path);

		zipfs_path_t(const std::string& path);

		zipfs_path_t(const zipfs_path_t& zipfs_path);//cpy

	public:

		bool is_root() const;

		bool is_dir() const;

		bool is_file() const;

		zipfs_path_t to_dir() const;

		zipfs_path_t to_file() const;

	public:

		operator const std::string&() const;

		operator const char*() const;

		const std::string& string() const;

		const char* c_str() const;

	public:

		void operator += (const std::string& path);

		zipfs_path_t operator + (const std::string& path) const;

		bool operator == (const zipfs_path_t& other) const;

	public:

		const char* libzip_path() const;

		std::string libzip_path_dir_add() const;

		std::vector<std::string> tree() const;

		zipfs_path_t parent_path() const;

	public:

		bool operator<(const zipfs_path_t& other) const;
	};
}