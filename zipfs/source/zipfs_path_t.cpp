#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_assert.h>
#include <util/split.h>
#include <filesystem>
#include <algorithm>
#include <cstring>

namespace zipfs {

	zipfs_path_t::zipfs_path_t(const char* path) {
		zipfs_usage_assert(path[0] == '/' || path[0] == '\\', "zipfs_path_t: path must be absolute and start with '/'");

		std::string _path = std::filesystem::path(path).lexically_normal().string();
		std::replace(_path.begin(), _path.end(), '\\', '/');//unix-style //or, lexically_normal().generic_string()
		m_path = _path;
	}

	zipfs_path_t::zipfs_path_t(const std::string& path) : zipfs_path_t{ path.c_str() } {
		//ident.
	}

	zipfs_path_t::zipfs_path_t(const zipfs_path_t& zipfs_path) :
		m_path{ zipfs_path.m_path } {}

	bool zipfs_path_t::is_root() const {
		return m_path == "/";
	}

	bool zipfs_path_t::is_dir() const {
		return m_path.back() == '/';
	}

	bool zipfs_path_t::is_file() const {
		return !is_dir();
	}

	zipfs_path_t zipfs_path_t::to_dir() const {
		if (is_dir())
			return *this;
		else
			return m_path + "/";
	}

	zipfs_path_t zipfs_path_t::to_file() const {
		if (is_file())
			return *this;
		else
			return m_path.substr(0, m_path.size() - 1);
	}

	zipfs_path_t::operator const std::string& () const {
		return m_path;
	}

	zipfs_path_t::operator const char*() const {
		return m_path.c_str();
	}

	const std::string& zipfs_path_t::string() const {
		return m_path;
	}

	const char* zipfs_path_t::c_str() const {
		return m_path.c_str();
	}

	void zipfs_path_t::operator += (const std::string& path) {
		if (!path.empty()) {
			zipfs_usage_assert(!is_file(), "zipfs_path_t: can't append a path to a file path.");
			zipfs_usage_assert(path.front() != '/' && path.front() != '\\', "zipfs_path_t: appended path argument is not relative.");
		}

		std::string _path = m_path + path;
		_path = std::filesystem::path(_path).lexically_normal().string();
		std::replace(_path.begin(), _path.end(), '\\', '/');//unix-style
		m_path = _path;
	}

	zipfs_path_t zipfs_path_t::operator + (const std::string& path) const {
		zipfs_path_t p = m_path;
		p += path;
		return p;
	}

	bool zipfs_path_t::operator == (const zipfs_path_t& other) const {
		return m_path == other.m_path;
	}

	const char* zipfs_path_t::libzip_path() const {
		auto find = m_path.find('\\');
		zipfs_internal_assert(find == std::string::npos);
		zipfs_internal_assert(m_path.front() == '/');
		return m_path.c_str() + 1;
	}

	std::string zipfs_path_t::libzip_path_dir_add() const {//strip leading and trailing '/'
		zipfs_internal_assert(is_dir());
		return m_path.substr(1, m_path.size() - 2);
	}

	std::vector<std::string> zipfs_path_t::tree() const {
		return util::split(m_path.substr(1), '/');
	}

	zipfs_path_t zipfs_path_t::parent_path() const {
		if (is_root())
			return *this;

		std::string parent_path_ = "/";
		std::vector<std::string> tree_ = tree();
		for (size_t d = 0; d < tree_.size() - 1; d++) {
			parent_path_ += (tree_[d] + "/");
		}
		return parent_path_;
	}

	bool zipfs_path_t::operator<(const zipfs_path_t& other) const {
		if (strcmp(c_str(), other.c_str()) < 0)
			return true;
		else
			return false;
	}
}