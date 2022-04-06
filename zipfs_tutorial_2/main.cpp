/*
	zipfs_tutorial_2 - *read-only* memory operations

	We will use the archive created in zipfs_sample_1

	- §1 cat
	- §2 ls
	- §3 stat
	- §4 index
	- §5 num_entries
*/
#include <zipfs/zipfs.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>

using namespace zipfs;

int main(int argc, char** argv) {

	//§0 first let's acquire the archive created in zipfs_tutorial_1
	size_t sz = std::filesystem::file_size("../zipfs_tutorial_1/result.zip");
	std::vector<char> buf(sz);

	std::ifstream ifs{ "../zipfs_tutorial_1/result.zip", std::ios::binary };
	ifs.read(buf.data(), sz);
	ifs.close();

	zipfs_error_t ze;
	zipfs_t zfs(buf.data(), buf.size(), ze);
	if (!ze) goto error;

	//§1 cat; retrieves file data
	{
		std::vector<char> buf;
		zipfs_path_t file = u8"/nested 🐦/sub1/sub2/😎.txt";
		ze = zfs.cat(file, buf, false);//read-compressed = false by default
		if (!ze) goto error;

		std::cout << "contents of '" << file << "':" << std::endl << " ";
		std::cout << std::string{ buf.data(), buf.size() } << std::endl;

		file = "/hello/hello_renamed.txt";
		ze = zfs.cat(file, buf);
		if (!ze) goto error;

		std::cout << "contents of '" << file << "':" << std::endl << " ";
		std::cout << std::string{ buf.data(), buf.size() } << std::endl;

		file = "/hello_renamed.bin";
		ze = zfs.cat(file, buf);
		if (!ze) goto error;

		std::cout << "contents of '" << file << "':" << std::endl << " ";
		std::cout << std::string{ buf.data(), buf.size() } << std::endl;
	}

	//§2 ls; the zipfs_path_t argument to ls 'must' be a dir, i.e. end with '/'
	{
		std::vector<zipfs_path_t> ls_;

		std::string dir = "/";
		std::cout << std::endl << "ls from '" << dir << "'" << std::endl;

		ze = zfs.ls(dir, ls_);
		if (!ze) goto error;
		for (const auto& path : ls_)
			std::cout << " " << path << std::endl;

		dir = u8"/nested 🐦/";
		std::cout << std::endl << "ls from '" << dir << "'" << std::endl;

		ze = zfs.ls(dir, ls_);
		if (!ze) goto error;
		for (const auto& path : ls_)
			std::cout << " " << path << std::endl;
	}

	//§3 stat; retrieves the libzip info for an entry
	//https://libzip.org/documentation/zip_stat.html
	{
		zipfs_zip_stat_t stat_;

		zipfs_path_t entry = u8"/nested 🐦/sub1/sub2/😎.txt";//file
		std::cout << std::endl << "libzip info for: " << entry << std::endl;
		{
			ze = zfs.stat(entry, stat_);
			if (!ze) goto error;
			std::cout << " comp_method       : " << stat_.comp_method << std::endl;
			std::cout << " comp_size         : " << stat_.comp_size << std::endl;
			std::cout << " crc               : " << stat_.crc << std::endl;
			std::cout << " encryption_method : " << stat_.encryption_method << std::endl;
			std::cout << " flags             : " << stat_.flags << std::endl;
			std::cout << " index             : " << stat_.index << std::endl;
			std::cout << " mtime             : " << stat_.mtime << std::endl;
			std::cout << " name              : " << stat_.name << std::endl;
			std::cout << " size              : " << stat_.size << std::endl;
			std::cout << " valid             : " << stat_.valid << std::endl;
		}

		entry = u8"/🦄/";//directory
		std::cout << std::endl << "libzip info for: " << entry << std::endl;
		{
			ze = zfs.stat(entry, stat_);
			if (!ze) goto error;
			std::cout << " comp_method       : " << stat_.comp_method << std::endl;
			std::cout << " comp_size         : " << stat_.comp_size << std::endl;
			std::cout << " crc               : " << stat_.crc << std::endl;
			std::cout << " encryption_method : " << stat_.encryption_method << std::endl;
			std::cout << " flags             : " << stat_.flags << std::endl;
			std::cout << " index             : " << stat_.index << std::endl;
			std::cout << " mtime             : " << stat_.mtime << std::endl;
			std::cout << " name              : " << stat_.name << std::endl;
			std::cout << " size              : " << stat_.size << std::endl;
			std::cout << " valid             : " << stat_.valid << std::endl;
		}

		entry = u8"/🦄";//<- file '/🦄' doesn't exist
		ze = zfs.stat(entry, stat_);
		std::cout << std::endl << ze << std::endl;
	}

	//§4 index; returns -1 if the name can't be found, can be used to check if an entry exists
	{
		std::cout << std::endl;

		zip_int64_t index_;

		std::string entry = u8"/nested 🐦/sub1/sub2/😎.txt";//file
		ze = zfs.index(entry, index_);
		if (!ze) goto error;

		std::cout << "index of '" << entry << "' is: " << index_ << std::endl;

		entry = u8"/🦄/";//directory
		ze = zfs.index(entry, index_);
		if (!ze) goto error;

		std::cout << "index of '" << entry << "' is: " << index_ << std::endl;

		entry = "/";//not an entry
		ze = zfs.index(entry, index_);
		if (!ze) goto error;

		std::cout << "index of '" << entry << "' is: " << index_ << std::endl;

		entry = "/not-an-entry.txt";//not an entry
		ze = zfs.index(entry, index_);
		if (!ze) goto error;

		std::cout << "index of '" << entry << "' is: " << index_ << std::endl;
	}

	//§5 num_entries; a shortcut to zip_get_num_entries(); use ls() to count inside a folder
	{
		zip_int64_t num_entries_;
		ze = zfs.num_entries(num_entries_);
		std::cout << std::endl << "total entries count: " << num_entries_ << std::endl;

		std::vector<zipfs_path_t> ls_;
		zfs.ls("/", ls_);
		assert(ls_.size() == num_entries_);
	}

	//end of sample
	goto end;

error:
	{
		std::cout << ze << std::endl;
		return -1;
	}

end:
	return 0;
}