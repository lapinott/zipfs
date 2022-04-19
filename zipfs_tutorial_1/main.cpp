/*
	zipfs_tutorial_1 - *write* memory operations

	- §1 file_add
	- §2 file_add replace
	- §3 file_delete
	- §4 file_rename
	- §5 dir_add
	- §6 dir_delete
	- §7 dir_rename
	- §8 write archive to HDD
*/
#include <zipfs/zipfs.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>

using namespace zipfs;

int main(int argc, char** argv) {

	//§0 first let's create the zip filesystem!
	zipfs_error_t ze;
	zipfs_t zfs(ze);
	if (!ze) goto error;

	//§1 file_add
	{
		//from string
		ze = zfs.file_add("/hello.txt", "world");
		if (!ze) goto error;

		//from data (random bytes from random.org)
		const uint8_t data[] = {
			0x8a, 0x13, 0xa6, 0xb7, 0x67, 0x0c, 0x3c, 0x6a, 0xb3, 0x66, 0x5a, 0x71, 0x5c, 0x53, 0x1a, 0x7f,
			0x1b, 0xd6, 0x68, 0x72, 0xf1, 0x56, 0xa9, 0xc4, 0x90, 0xeb, 0x14, 0x15, 0x40, 0x61, 0x4a, 0x46,
			0x94, 0xd4, 0x44, 0xcf, 0xc5, 0x42, 0xfe, 0xfb, 0xb7, 0xb5, 0x66, 0xb8, 0x84, 0x4a, 0x48, 0x50,
			0x99, 0x9d, 0x3c, 0x98, 0x85, 0xea, 0x97, 0xc5, 0xe4, 0x8d, 0x3d, 0xbb, 0x0b, 0x24, 0xb9, 0x69,
		};
		std::vector<char> buf{ data, data + sizeof(data) };
		ze = zfs.file_add("/hello.bin", buf);
		if (!ze) goto error;

		//nested + utf8
		ze = zfs.file_add(u8"/nested/sub1/sub2/😎.txt", u8"👍👍");//<-will create tree
		if (!ze) goto error;

		//already exists
		ze = zfs.file_add("/hello.txt", "hello hello");
		std::cout << ze << std::endl;
	}

	//§2 file_add replace
	{
		//from string
		ze = zfs.file_add("/hello.txt", "hello world", OVERWRITE::ALWAYS);
		if (!ze) goto error;

		//from data
		const uint8_t data[] = {
			0x99, 0x9d, 0x3c, 0x98, 0x85, 0xea, 0x97, 0xc5, 0xe4, 0x8d, 0x3d, 0xbb, 0x0b, 0x24, 0xb9, 0x69,
			0x94, 0xd4, 0x44, 0xcf, 0xc5, 0x42, 0xfe, 0xfb, 0xb7, 0xb5, 0x66, 0xb8, 0x84, 0x4a, 0x48, 0x50,
		};
		std::vector<char> buf{ data, data + sizeof(data) };
		ze = zfs.file_add("/hello.bin", buf, OVERWRITE::ALWAYS);
		if (!ze) goto error;

		//nested + utf8
		ze = zfs.file_add(u8"/nested/sub1/sub2/😎.txt", u8"😏😏", OVERWRITE::ALWAYS);
		if (!ze) goto error;
	}

	//&3 file_delete (with source image backup and restore)
	{
		zip_int64_t ne;
		zfs.num_entries(ne);
		assert(ne == 6);//<-files + dirs

		zfs.zipfs_image_update();//update image
		{
			ze = zfs.file_delete("/hello.txt");
			if (!ze) goto error;
			ze = zfs.file_delete("/hello.bin");
			if (!ze) goto error;

			zfs.num_entries(ne);
			assert(ne == 4);
		}
		zfs.zipfs_revert_to_image();//revert to image

		zfs.num_entries(ne);
		assert(ne == 6);
	}

	//§4 file_rename
	{
		ze = zfs.file_rename("/hello.txt", "/hello/hello_renamed.txt");//<-we can rename across folders similar to *nix mv; will create /hello/
		if (!ze) goto error;
		ze = zfs.file_rename("/hello.bin", "/hello_renamed.bin");
		if (!ze) goto error;

		//doesn't exist
		ze = zfs.file_rename("/hello?.bin", "/hello?_renamed.bin");
		std::cout << ze << std::endl;
	}

	//§5 dir_add (=mkdir)
	{
		ze = zfs.dir_add(u8"/🦄/");
		if (!ze) goto error;
		ze = zfs.dir_add(u8"/répertoire français/sub1/sub2/");//<-will create tree
		if (!ze) goto error;
		ze = zfs.dir_add(u8"/répertoire français 2/sub1/sub2/");//<-will create tree
		if (!ze) goto error;
	}

	//§6 dir_delete
	{
		size_t count;
		ze = zfs.dir_delete(u8"/répertoire français 2/", count);//<-will remove all contents
		if (!ze) goto error;

		assert(count == 3);
		std::cout << "dir_delete count: " << count << std::endl;
	}

	//§7 dir_rename
	{
		ze = zfs.dir_rename("/nested/", u8"/nested 🐦/");
		if (!ze) goto error;
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
	std::cout << ze << std::endl;
	return 0;
}