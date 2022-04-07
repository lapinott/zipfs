#pragma once

#include <zipfs/zipfs_enums.h>
#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>
#include <vector>
#include <string>
#include <tuple>

namespace zipfs {

	class zipfs_query_result_t { //a container for query results
	private:

		friend struct zipfs_t;

		std::vector<QUERY_RESULT>
			m_result_query_result;

		std::vector<zipfs_path_t>
			m_result_zipfs_path;

		std::vector<filesystem_path_t>
			m_result_fs_path;

		size_t
			m_result_count;

		void push_back(QUERY_RESULT query_result, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path);

		void clear();

	public:

		zipfs_query_result_t();

		size_t size() const;

		zipfs_query_result_t get(const QUERY_RESULTS_GET what) const;

		std::vector<std::tuple<QUERY_RESULT, zipfs_path_t, filesystem_path_t>> data(const QUERY_RESULTS_GET what = QUERY_RESULTS_GET::ALL) const;

		std::string to_string() const;
	};

	std::ostream& operator<<(std::ostream& os, const zipfs_query_result_t& qr);
}