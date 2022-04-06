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

		std::vector<QUERY_RESULT>
			m_result_query_result;

		std::vector<zipfs_path_t>
			m_result_zipfs_path;

		std::vector<filesystem_path_t>
			m_result_fs_path;

		size_t
			m_result_count;

	public:

		zipfs_query_result_t();

		void push_back(QUERY_RESULT query_result, const zipfs_path_t& zipfs_path, const filesystem_path_t& fs_path);

		size_t size() const;

		void clear();

		std::vector<std::tuple<QUERY_RESULT, zipfs_path_t, filesystem_path_t>> get_results() const;

		operator std::string() const;
	};

	std::ostream& operator<<(std::ostream& os, const zipfs_query_result_t& qr);
}