#include <zipfs/zipfs_query_results_t.h>
#include <zipfs/zipfs_assert.h>
#include <iostream>
#include <sstream>

namespace zipfs {

	zipfs_query_results_t::zipfs_query_results_t() {}

	zipfs_query_results_t zipfs_query_results_t::get(uint32_t what) const {
		zipfs_query_results_t qrs;
		for (size_t r = 0; r < m_query_results.size(); r++) {
			if (m_query_results[r].query_result & what)
				qrs.m_query_results.push_back(m_query_results[r]);
		}
		return qrs;
	}

	zipfs_query_results_t zipfs_query_results_t::get(QUERY_RESULT what) const {
		return get((uint32_t)what);
	}

	const std::vector<zipfs_query_result_t>& zipfs_query_results_t::query_results() const {
		return m_query_results;
	}

	std::string zipfs_query_results_t::to_string() const {
		std::ostringstream oss;
		for (size_t r = 0; r < m_query_results.size(); r++) {
			std::string QUERY_RESULT_str;
			switch (m_query_results[r].query_result) {
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
			if (!m_query_results[r].zipfs_path.is_root())
				oss << m_query_results[r].zipfs_path << std::endl;
			if (!m_query_results[r].fs_path.u8path().empty())
				oss << m_query_results[r].fs_path.u8path() << std::endl;
			oss << "  ->  " << QUERY_RESULT_str << std::endl;
			/*
				à refaire:
				(if!empty) zipfs path :
				(if!empty) fs path    :
				  -> QUERY_RESULT_str
			*/
		}
		return oss.str();
	}

	std::ostream& operator<<(std::ostream& os, const zipfs_query_results_t& qr) {
		os << qr.to_string();
		return os;
	}
}