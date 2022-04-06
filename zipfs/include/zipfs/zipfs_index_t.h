#pragma once

#include <zipfs/zipfs_path_t.h>
#include <zip.h>
#include <map>
#include <list>

namespace zipfs {

	class zipfs_index_t {
	private:

		friend struct zipfs_t;

		std::map<zipfs_path_t, zip_int64_t> //maps a zipfs_path_t to its corresponding in-archive zip_int64_t index
			m_map;

		bool init(zip_t* z);

		bool verify(zip_t* z) const;

		bool rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path);

		void clear();

		bool empty() const;

		zip_int64_t index(const zipfs_path_t& zipfs_path) const;
	};
}