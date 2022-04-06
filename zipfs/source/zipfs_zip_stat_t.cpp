#include <zipfs/zipfs_zip_stat_t.h>
#include <zip.h>

namespace zipfs {

    zipfs_zip_stat_t::zipfs_zip_stat_t() {
        valid = 0;
        index = 0;
        size = 0;
        comp_size = 0;
        mtime = 0;
        crc = 0;
        comp_method = 0;
        encryption_method = 0;
        flags = 0;
    }

    zipfs_zip_stat_t::zipfs_zip_stat_t(const zip_stat_t& zs) : zipfs_zip_stat_t() {
        valid = zs.valid;
        name = zs.name;
        index = zs.index;
        size = zs.size;
        comp_size = zs.comp_size;
        mtime = zs.mtime;
        crc = zs.crc;
        comp_method = zs.comp_method;
        encryption_method = zs.encryption_method;
        flags = zs.flags;
    }
}