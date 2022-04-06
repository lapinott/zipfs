#include <zipfs/zipfs_index_t.h>
#include <zipfs/zipfs_assert.h>

namespace zipfs {

	bool zipfs_index_t::init(zip_t* z) {
		zipfs_internal_assert(z != nullptr);
		zipfs_internal_assert(m_map.empty());

		zip_int64_t num_entries = zip_get_num_entries(z, NULL);
		if (num_entries == -1)
			return false;

		for (zip_int64_t e = 0; e < num_entries; e++) {
			const char* name = zip_get_name(z, e, ZIP_FL_ENC_GUESS);
			if (name == nullptr)
				zipfs_internal_assert(false);

			auto insert = m_map.insert({ "/" + std::string(name), e });
			zipfs_internal_assert(insert.second);
		}
		return true;
	}

	bool zipfs_index_t::verify(zip_t* z) const {
		zip_int64_t num_entries = zip_get_num_entries(z, NULL);
		if (num_entries == -1 || num_entries != m_map.size())
			return false;

		for (zip_int64_t e = 0; e < num_entries; e++) {
			const char* name = zip_get_name(z, e, ZIP_FL_ENC_GUESS);
			if (name == nullptr)
				zipfs_internal_assert(false);

			auto find = m_map.find("/" + std::string(name));
			if (find == m_map.end())
				return false;
			else if (find->second != e)
				return false;
		}

		return true;
	}

	bool zipfs_index_t::rename(const zipfs_path_t& zipfs_path, const zipfs_path_t& zipfs_rename_path) {
		auto find = m_map.find(zipfs_path);
		if (find != m_map.end()) {
			auto erase = m_map.erase(find);
			auto insert = m_map.insert({ zipfs_rename_path, erase->second });
			zipfs_internal_assert(insert.second);
			return true;
		}
		else {
			return false;
		}
	}

	void zipfs_index_t::clear() {
		m_map.clear();
	}

	bool zipfs_index_t::empty() const {
		return m_map.empty();
	}

	zip_int64_t zipfs_index_t::index(const zipfs_path_t& zipfs_path) const {
		auto find = m_map.find(zipfs_path);
		if (find != m_map.end())
			return find->second;
		else
			return -1;
	}
}