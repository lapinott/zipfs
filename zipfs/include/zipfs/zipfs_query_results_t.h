#pragma once

#include <zipfs/zipfs_enums.h>
#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>
#include <zipfs/zipfs_query_result_t.h>
#include <vector>
#include <string>
#include <tuple>

namespace zipfs {

	class zipfs_query_results_t {//a container for query results
	private:

		friend struct zipfs_t;

		std::vector<zipfs_query_result_t>
			m_query_results;

	public:

		zipfs_query_results_t();

		zipfs_query_results_t
			get(uint32_t what) const;

		zipfs_query_results_t
			get(QUERY_RESULT what) const;

		const std::vector<zipfs_query_result_t>&
			query_results() const;

		virtual std::string
			to_string() const;
	};

	std::ostream& operator<<(std::ostream& os, const zipfs_query_results_t& qr);
}