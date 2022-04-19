/*
	zipfs_tutorial_4 - *write* filesystem operations

	We will use the files created in zipfs_tutorial_3

	- §1 file_pull
	- §2 file_pull_replace
	- §3 dir_pull_query & dir_pull
	- §4 write archive to HDD
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

	//§0 first let's create the zip filesystem!
	zipfs_error_t ze;
	zipfs_t zfs(ze);
	if (!ze) goto error;

	//§1 file_pull
	{
		ze = zfs.file_pull("/let's/pull/here/hello.txt", "../zipfs_tutorial_3/file-extract/hello_renamed.txt");//<-will create tree
		if (!ze) goto error;

		ze = zfs.file_pull(u8"/let's/pull/here/😎😎.txt", u8"../zipfs_tutorial_3/file-extract/😎.txt");
		if (!ze) goto error;

		std::vector<zipfs_path_t> ls_;
		ze = zfs.ls("/", ls_);
		std::cout << "ls: " << std::endl;
		for (const auto& p : ls_)
			std::cout << " " << p << std::endl;
	}

	//§2 file_pull_replace
	{
		ze = zfs.file_pull_replace("/there/is/no/file/here.txt", "../zipfs_tutorial_3/file-extract/hello_renamed.txt");
		std::cout << ze << std::endl;

		ze = zfs.file_pull_replace("/let's/pull/here/hello.txt", "../neither/here.txt");
		std::cout << ze << std::endl;

		ze = zfs.file_pull_replace("/let's/pull/here/hello.txt", u8"../zipfs_tutorial_3/file-extract/😎.txt");
		if (!ze) goto error;

		ze = zfs.file_pull_replace(u8"/let's/pull/here/😎😎.txt", "../zipfs_tutorial_3/file-extract/hello_renamed.txt");
		if (!ze) goto error;
	}

	//§3 dir_pull_query & dir_pull; the query will show us what will take place before we decide to commit
	{
		zipfs_query_results_t qr;
		ze = zfs.dir_pull_query("/dir-pull/", "../zipfs_tutorial_3/dir-extract", qr, zipfs::OVERWRITE::NEVER, zipfs::ORPHAN::KEEP);
		if (!ze) goto error;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_pull("/dir-pull/", "../zipfs_tutorial_3/dir-extract", zipfs::OVERWRITE::NEVER, zipfs::ORPHAN::KEEP);
			if (!ze) goto error;
		}

		//let's run a query with a different orphan flag (and pulling to the root). we set orphans to be deleted.
		ze = zfs.dir_pull_query("/", "../zipfs_tutorial_3/dir-extract", qr, zipfs::OVERWRITE::NEVER, zipfs::ORPHAN::DELETE_);
		if (!ze) goto error;
		std::cout << std::endl << "query results:" << std::endl;
		std::cout << qr << std::endl;

		if (commit_ask()) {
			//commit with the same parameters
			ze = zfs.dir_pull("/", "../zipfs_tutorial_3/dir-extract", zipfs::OVERWRITE::NEVER, zipfs::ORPHAN::DELETE_);
			if (!ze) goto error;
		}
	}

	//§8 write archive to HDD
	{
		std::vector<char> source;
		ze = zfs.get_source(source);
		if (!ze) goto error;

		std::ofstream ofs{ "result.zip", std::ios::binary };
		ofs.write(source.data(), source.size());
		ofs.close();
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