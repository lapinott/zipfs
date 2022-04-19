/*
	zipfs_tutorial_3 - *read-only* filesystem operations

	We will use the archive created in zipfs_tutorial_1

	- §1 file_extract
	- §2 file_extract_replace
	- §3 dir_extract_query & dir_extract
*/
#include <zipfs/zipfs.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>

using namespace zipfs;

bool commit_ask() {
	std::cout << "commit? [y]es [n]o : "; std::string choice; std::cin >> choice;
	if (choice == "y")
		return true;
	else
		return false;
}

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

	//§1 file_extract
	{
		std::filesystem::remove_all("file-extract");
		std::filesystem::create_directory("file-extract");

		ze = zfs.file_extract("/hello/hello_renamed.txt", "file-extract/hello_renamed.txt");
		if (!ze) goto error;

		ze = zfs.file_extract(u8"/nested 🐦/sub1/sub2/😎.txt", u8"file-extract/😎.txt");
		if (!ze) goto error;

		ze = zfs.file_extract(u8"/nested 🐦/sub1/sub2/😎.txt", "file-extract/hello_renamed.txt");//<-already exists
		std::cout << ze << std::endl;
	}

	//§2 file_extract_replace
	{
		ze = zfs.file_extract_replace(u8"/nested 🐦/sub1/sub2/😎.txt", "file-extract/hello_renamed.txt");
		if (!ze) goto error;

		ze = zfs.file_extract_replace("/hello/hello_renamed.txt", u8"file-extract/😎.txt");
		if (!ze) goto error;

		ze = zfs.file_extract_replace("/hello/hello_renamed.txt", "file-extract/doesn't exist.txt");//<-doesn't exist
		std::cout << ze << std::endl;
	}

	//§3 dir_extract_query& dir_extract; the query will show us what will take place before we decide to commit
	{
		std::filesystem::remove_all("dir-extract");
		std::filesystem::create_directory("dir-extract");

		zipfs_query_results_t qr;
		ze = zfs.dir_extract_query("/", "dir-extract", qr, zipfs::OVERWRITE::NEVER);
		if (!ze) goto error;
		std::cout << std::endl << "query results:" << std::endl;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_extract("/", "dir-extract", zipfs::OVERWRITE::NEVER);
			if (!ze) goto error;
		}

		//now let's run a query with a different overwrite flag
		ze = zfs.dir_extract_query("/", "dir-extract", qr, zipfs::OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH);//this should produce no overwrites
		if (!ze) goto error;
		std::cout << std::endl << "query results:" << std::endl;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_extract("/", "dir-extract", zipfs::OVERWRITE::IF_DATE_OLDER_AND_SIZE_MISMATCH);
			if (!ze) goto error;
		}

		//this query will mark everything to be overwritten
		ze = zfs.dir_extract_query("/", "dir-extract", qr, zipfs::OVERWRITE::ALWAYS);//overwrite everything
		if (!ze) goto error;
		std::cout << std::endl << "query results:" << std::endl;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_extract("/", "dir-extract", zipfs::OVERWRITE::ALWAYS);
			if (!ze) goto error;
		}

		//now lets modify a file and run a query
		ze = zfs.file_add_replace(u8"/nested 🐦/sub1/sub2/😎.txt", "new contents");
		std::cout << std::endl << u8"we modified /nested 🐦/sub1/sub2/😎.txt" << std::endl;
		ze = zfs.dir_extract_query("/", "dir-extract", qr, zipfs::OVERWRITE::IF_SIZE_MISMATCH);//should mark the modified file to be overwritten
		if (!ze) goto error;
		std::cout << std::endl << "query results:" << std::endl;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_extract("/", "dir-extract", zipfs::OVERWRITE::IF_SIZE_MISMATCH);
			if (!ze) goto error;
		}
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