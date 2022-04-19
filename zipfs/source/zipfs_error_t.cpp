#include <zipfs/zipfs_error_t.h>
#include <zipfs/zipfs_assert.h>
#include <cstring>

namespace zipfs {

	zipfs_error_t::zipfs_error_t() {
		zip_error_init(&m_zip_error);
	}

	zipfs_error_t::zipfs_error_t(zip_error_t* zip_error) : zipfs_error_t() {
		m_zip_error.sys_err = zip_error->sys_err;
		m_zip_error.zip_err = zip_error->zip_err;
		if (zip_error->str != nullptr) {
			m_zip_error.str = (char*)malloc(strlen(zip_error->str));
			std::copy(zip_error->str, zip_error->str + strlen(zip_error->str), m_zip_error.str);
		}
	}

	zipfs_error_t::zipfs_error_t(const std::string& zipfs_error) {
		m_zipfs_error = zipfs_error;
	}

	zipfs_error_t::zipfs_error_t(const char* zipfs_error) : zipfs_error_t() {
		m_zipfs_error = zipfs_error;
	}

	zipfs_error_t::zipfs_error_t(const zipfs_error_t& other) : zipfs_error_t() {
		m_zip_error.sys_err = other.m_zip_error.sys_err;
		m_zip_error.zip_err = other.m_zip_error.zip_err;
		if (other.m_zip_error.str != nullptr) {//deep copy
			m_zip_error.str = (char*)malloc(strlen(other.m_zip_error.str));
			std::copy(other.m_zip_error.str, other.m_zip_error.str + strlen(other.m_zip_error.str), m_zip_error.str);
		}

		m_zipfs_error = other.m_zipfs_error;
		m_zipfs_path = other.m_zipfs_path;
		m_filesystem_path = other.m_filesystem_path;
	}

	zipfs_error_t::zipfs_error_t(zipfs_error_t&& other) noexcept {
		m_zip_error = other.m_zip_error;//shallow copy
		other.m_zip_error.str = nullptr;//.>important so that zip_error_fini() doesn't free() the string

		m_zipfs_error = other.m_zipfs_error;
		m_zipfs_path = other.m_zipfs_path;
		m_filesystem_path = other.m_filesystem_path;
	}

	zipfs_error_t& zipfs_error_t::operator=(zipfs_error_t&& other) noexcept {
		m_zip_error = other.m_zip_error;//shallow copy
		other.m_zip_error.str = nullptr;//.>important so that zip_error_fini() doesn't free() the string

		m_zipfs_error = other.m_zipfs_error;
		m_zipfs_path = other.m_zipfs_path;
		m_filesystem_path = other.m_filesystem_path;

		return *this;
	}

	zipfs_error_t& zipfs_error_t::operator=(const zipfs_error_t& other) {
		zip_error_fini(&m_zip_error);
		zip_error_init(&m_zip_error);

		m_zip_error.sys_err = other.m_zip_error.sys_err;
		m_zip_error.zip_err = other.m_zip_error.zip_err;
		if (other.m_zip_error.str != nullptr) {//deep copy
			m_zip_error.str = (char*)malloc(strlen(other.m_zip_error.str));
			std::copy(other.m_zip_error.str, other.m_zip_error.str + strlen(other.m_zip_error.str), m_zip_error.str);
		}

		m_zipfs_error = other.m_zipfs_error;
		m_zipfs_path = other.m_zipfs_path;
		m_filesystem_path = other.m_filesystem_path;

		return *this;
	}

	zipfs_error_t::~zipfs_error_t() {
		zip_error_fini(&m_zip_error);
	}

	bool zipfs_error_t::is_error() const {
		return
			m_zip_error.sys_err != 0 ||
			m_zip_error.zip_err != 0 ||
			!m_zipfs_error.empty();
	}

	zipfs_error_t::operator bool() {
		return !is_error();
	}

	zipfs_error_t::operator std::string() const {
		if (!is_error())
			return std::string("zipfs: no error.");

		std::string err;
		if (m_zip_error.str != nullptr)
			err = std::string("libzip error: ") + m_zip_error.str;
		else if (m_zip_error.sys_err != 0 || m_zip_error.zip_err != 0)
			err = std::string("libzip error: ") + zip_error_strerror(const_cast<zip_error_t*>(&m_zip_error));
		else if (!m_zipfs_error.empty())
			err = std::string("zipfs error: ") + m_zipfs_error;
		else
			err = std::string("zipfs error: unknown error.");

		if (!m_zipfs_path.is_root())
			err += " (zipfs path: " + m_zipfs_path.string() + ")";
		if (!m_filesystem_path.platform_path().empty())
			err += " (fs path: " + m_filesystem_path.u8path() + ")";

		return err;
	}

	zipfs_error_t zipfs_error_t::no_error() {
		return zipfs_error_t{};
	}

	void zipfs_error_t::set_zipfs_path(const zipfs_path_t& p) {
		zipfs_internal_assert(m_filesystem_path.platform_path().empty());
		m_zipfs_path = p;
	}

	void zipfs_error_t::set_fs_path(const filesystem_path_t& p) {
		zipfs_internal_assert(m_zipfs_path.is_root());//==empty
		m_filesystem_path = p;
	}

	std::ostream& operator<<(std::ostream& os, const zipfs_error_t& ze) {
		os << (std::string)ze;
		return os;
	}
}