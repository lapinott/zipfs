/*
	zipfs_tutorial_5 - encryption & decryption

	We will use the files created in zipfs_tutorial_3

	- §1 setting an encryption and decryption function
	- §2 pulling some files
	- §3 extracting encrypted files
	- §4 extracting decrypted files
	- §5 write archive to HDD
	- §6 cat()'ing a file as encrypted or decrypted
	- §7 extra
*/
#include <zipfs/zipfs.h>
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace zipfs;

/*
	the encryption and decryption functions must have this signature:

	void(*f)(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len);
*/

//let's define an encryption function first; this function will just invert all bits of the input data, it is reversible (not using filename here)
void encrypt_func(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len) {
	uint8_t* encrypted = new uint8_t[len];
	for (size_t b = 0; b < len; b++) {
		encrypted[b] = buf[b] ^ 0xff;
	}

	*ret_buf = encrypted;
	*ret_len = len;
}

//our encrypt_func is reversible, we can use the same function for our decrypt_func
auto decrypt_func = encrypt_func;

int main(int argc, char** argv) {

	//§0 first let's create the zip filesystem!
	zipfs_error_t ze;
	zipfs_t zfs(ze);
	if (!ze) goto error;

	//§1 setting an encryption and decryption function
	{
		zfs.set_file_encrypt_func(encrypt_func);
		zfs.set_file_decrypt_func(decrypt_func);

		//set encryption to active
		zfs.set_file_encrypt(true);//all files added to the archive will now be encrypted using our custom function
		zfs.set_file_decrypt(false);//default
	}

	//§2 pulling some files
	{
		//let's pull a directory
		//all files will be encrypted prior to be added to the archive
		ze = zfs.dir_pull("/", "../zipfs_tutorial_3/dir-extract");
		if (!ze) goto error;
	}

	//§3 extracting encrypted files
	{
		//let's extract as 'not' decrypted to see the results (set_file_decrypt() was set to false)
		std::filesystem::remove_all("dir-extract-encrypted");
		std::filesystem::create_directory("dir-extract-encrypted");

		zipfs_query_result_t qr;
		ze = zfs.dir_extract_query("/", "dir-extract-encrypted", qr);
		if (!ze) goto error;
		std::cout << qr << std::endl;

		ze = zfs.dir_extract("/", "dir-extract-encrypted");
		if (!ze) goto error;
	}

	//§4 extracting decrypted files
	{
		zfs.set_file_decrypt(true);//all files will be decrypted prior to being written to the filesystem

		std::filesystem::remove_all("dir-extract-decrypted");
		std::filesystem::create_directory("dir-extract-decrypted");

		zipfs_query_result_t qr;
		ze = zfs.dir_extract_query("/", "dir-extract-decrypted", qr);
		if (!ze) goto error;
		std::cout << qr << std::endl;

		ze = zfs.dir_extract("/", "dir-extract-decrypted");
		if (!ze) goto error;
	}

	//§5 write archive to HDD
	{
		//files are encrypted inside the archive
		std::vector<char> source;
		ze = zfs.get_source(source);
		if (!ze) goto error;

		std::ofstream ofs{ "result.zip", std::ios::binary };
		ofs.write(source.data(), source.size());
		ofs.close();
	}

	//§6 cat()'ing a file as encrypted or decrypted
	{
		std::vector<char> buf;

		//let's get some encrypted data
		zfs.set_file_decrypt(false);
		ze = zfs.cat("/hello/hello_renamed.txt", buf);
		if (!ze) goto error;
		std::cout << "encrypted data: " << std::string(buf.data(), buf.size()) << std::endl;

		//let's get some decrypted data
		zfs.set_file_decrypt(true);
		ze = zfs.cat("/hello/hello_renamed.txt", buf);
		if (!ze) goto error;
		std::cout << "decrypted data: " << std::string(buf.data(), buf.size()) << std::endl;
	}

	//§7 extra; let's pull our encrypted data from HDD with encryption set to false; the encryption function is reversible so we end up with the same data
	{
		zipfs_t zfs2(ze);
		zfs2.dir_pull("/", "dir-extract-encrypted");//encryption not set.
		zfs2.set_file_decrypt(true);
		zfs2.set_file_decrypt_func(decrypt_func);
		std::filesystem::remove_all("should-be-decrypted");
		std::filesystem::create_directory("should-be-decrypted");
		zfs2.dir_extract("/", "should-be-decrypted");
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