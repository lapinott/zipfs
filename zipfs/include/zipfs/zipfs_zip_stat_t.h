#pragma once

#include <zip.h>
#include <string>

namespace zipfs {

	struct zipfs_zip_stat_t {//.>helper class; copy from zip_stat_t with std::string copy

        zipfs_zip_stat_t();
        zipfs_zip_stat_t(const zip_stat_t& zs);

        zip_uint64_t valid;             /* which fields have valid values */
        std::string name;               /* name of the file */
        zip_uint64_t index;             /* index within archive */
        zip_uint64_t size;              /* size of file (uncompressed) */
        zip_uint64_t comp_size;         /* size of file (compressed) */
        time_t mtime;                   /* modification time */
        zip_uint32_t crc;               /* crc of file data */
        zip_uint16_t comp_method;       /* compression method used */
        zip_uint16_t encryption_method; /* encryption method used */
        zip_uint32_t flags;             /* reserved for future use */
	};
}