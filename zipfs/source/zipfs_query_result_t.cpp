#include <zipfs/zipfs_query_result_t.h>
#include <zipfs/zipfs_assert.h>
#include <iostream>
#include <sstream>

namespace zipfs {

	zipfs_query_result_t::zipfs_query_result_t() : m_result_count{ 0 } {}

	void zipfs_query_result_t::push_back(QUERY_RESULT query_result, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path) {
		m_result_query_result.push_back(query_result);
		m_result_zipfs_path.push_back(zipfs_path);
		m_result_fs_path.push_back(fs_path);
		++m_result_count;
	}

	void zipfs_query_result_t::clear() {
		m_result_query_result.clear();
		m_result_zipfs_path.clear();
		m_result_fs_path.clear();
		m_result_count = 0;
	}

	size_t zipfs_query_result_t::size() const {
		zipfs_internal_assert(
			m_result_count == m_result_query_result.size() &&
			m_result_count == m_result_zipfs_path.size() &&
			m_result_count == m_result_fs_path.size());

		return m_result_count;
	}

	zipfs_query_result_t zipfs_query_result_t::get(const QUERY_RESULTS_GET what) const {
		zipfs_query_result_t qr;
		auto data_ = data(what);
		for (size_t r = 0; r < size(); r++)
			qr.push_back(std::get<0>(data_[r]), std::get<1>(data_[r]), std::get<2>(data_[r]));

		return qr;
	}

	std::vector<std::tuple<QUERY_RESULT, zipfs_path_t, filesystem_path_t>> zipfs_query_result_t::data(const QUERY_RESULTS_GET what) const {
		std::vector<std::tuple<QUERY_RESULT, zipfs_path_t, filesystem_path_t>> results;
		for (size_t r = 0; r < size(); r++) {
			if (m_result_query_result[r] & what)
				results.emplace_back(m_result_query_result[r], m_result_zipfs_path[r], m_result_fs_path[r]);
		}
		return results;
	}

	std::string zipfs_query_result_t::to_string() const {
		std::ostringstream oss;
		for (size_t r = 0; r < size(); r++) {
			std::string QUERY_RESULT_str;
			switch (m_result_query_result[r]) {
			case QUERY_RESULT::FILE_WRITE: { QUERY_RESULT_str = "FILE_WRITE"; break; }
			case QUERY_RESULT::FILE_OVERWRITE: { QUERY_RESULT_str = "FILE_OVERWRITE"; break; }
			case QUERY_RESULT::FILE_DONT_OVERWRITE: { QUERY_RESULT_str = "FILE_DONT_OVERWRITE"; break; }
			case QUERY_RESULT::FILE_ORPHAN_KEEP: { QUERY_RESULT_str = "FILE_ORPHAN_KEEP"; break; }
			case QUERY_RESULT::FILE_ORPHAN_DELETE: { QUERY_RESULT_str = "FILE_ORPHAN_DELETE"; break; }
			case QUERY_RESULT::DIR_ADD: { QUERY_RESULT_str = "DIR_ADD"; break; }
			case QUERY_RESULT::DIR_ALREADY_EXISTS: { QUERY_RESULT_str = "DIR_ALREADY_EXISTS"; break; }
			case QUERY_RESULT::DIR_ORPHAN_KEEP: { QUERY_RESULT_str = "DIR_ORPHAN_KEEP"; break; }
			case QUERY_RESULT::DIR_ORPHAN_DELETE: { QUERY_RESULT_str = "DIR_ORPHAN_DELETE"; break; }
			default:
				break;
			}
			oss << "path  ";
			if (!m_result_fs_path[r].u8path().empty())
				oss << m_result_fs_path[r].u8path() << std::endl;
			if (!m_result_zipfs_path[r].is_root())
				oss << m_result_zipfs_path[r] << std::endl;
			oss << "  ->  " << QUERY_RESULT_str << std::endl;
		}
		return oss.str();
	}

	std::ostream& operator<<(std::ostream& os, const zipfs_query_result_t& qr) {
		os << qr.to_string();
		return os;
	}
}