/*
	zipfs_tutorial_6 - filesystem mirroring script

	A simple script used to mirror a filesystem directory.
	We could run it repeatedly from a batch-file or what not.
	The data will be stored and encrypted.
*/

#include <zipfs/zipfs.h>
#include <iostream>
#include <filesystem>
#include <fstream>

void super_secret_encrypt_func(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len) {
	uint8_t* encrypted = new uint8_t[len];
	for (size_t b = 0; b < len; b++) {
		encrypted[b] = buf[b] ^ 0xff;
	}

	*ret_buf = encrypted;
	*ret_len = len;
}

using namespace zipfs;

int main(int argc, char** argv) {

	const char* archive = "archive.zip";
	const char* mirroring = "../zipfs_tutorial_3/dir-extract";

	std::vector<char> source;
	if (std::filesystem::exists(archive)) {
		size_t sz = std::filesystem::file_size(archive);
		source.resize(sz);
		std::ifstream ifs{ archive, std::ios::binary };
		ifs.read(source.data(), sz);
		ifs.close();
	}

	zipfs_error_t ze;
	zipfs_t zfs(source.data(), source.size(), ze);
	if (!ze)
		return -1;

	zfs.set_file_encrypt(false);
	zfs.set_file_encrypt_func(super_secret_encrypt_func);
	zfs.set_compression(ZIP_CM_STORE, NULL);//https://libzip.org/documentation/zip_set_file_compression.html

	zipfs_query_result_t qr;
	ze = zfs.dir_pull_query("/", mirroring, qr, zipfs::OVERWRITE::IF_DATE_OLDER, zipfs::ORPHAN::DELETE_);
	if (!ze)
		return -1;
	std::cout << qr << std::endl;

	ze = zfs.dir_pull("/", mirroring, zipfs::OVERWRITE::IF_DATE_OLDER, zipfs::ORPHAN::DELETE_);
	if (!ze)
		return -1;

	ze = zfs.get_source(source);
	if (!ze)
		return -1;

	std::ofstream ofs{ archive, std::ios::binary | std::ios::trunc };
	ofs.write(source.data(), source.size());
	ofs.close();

	//try to add/remove/modify files in the mirrored directory, rerun the script, and see what happens.
	return 0;
}