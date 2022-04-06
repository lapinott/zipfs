/*
	zipfs_tutorial_0

	zipfs_path_t & filesystem_path_t
*/
#include <zipfs/zipfs_path_t.h>
#include <zipfs/zipfs_filesystem_path_t.h>

#include <fstream>
#include <filesystem>

using namespace zipfs;

void print_path_info(const zipfs_path_t& p) {
	std::cout << "path      : " << p.c_str() << std::endl;
	std::cout << "is_root() : " << p.is_root() << std::endl;
	std::cout << "is_dir()  : " << p.is_dir() << std::endl;
	std::cout << "is_file() : " << p.is_file() << std::endl;
	std::cout << "tree()    :" << std::endl;
	for (const auto& t : p.tree())
		std::cout << " " << t << std::endl;
}

int main(int argc, char** argv) {

	//§1 zipfs_path_t
	{
		/*
			The 2 rules to remember about zipfs_path_t:
				- a zipfs_path_t is 'always' absolute
				- a zipfs_path_t 'must' start with '/'

			- a zipfs_path_t can only represent a 'dir' or a 'file'
			- a zipfs_path_t represents a 'dir' if it ends with '/', or a file if it doesnt
		*/

		try {
			zipfs_path_t try_a_path = "/";

			print_path_info(try_a_path);
		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}

		/*
			given the previous, you can't append a zipfs_path_t to a zipfs_path_t, but you can append a c-string or std::string to a zipfs_path_t
			(although you probably won't need to do that anyway)
		*/
		try {
			zipfs_path_t try_a_path = zipfs_path_t("/x/") + "hello.txt";

			std::cout << std::endl;
			print_path_info(try_a_path);
		}
		catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}

		/*
			if your path contains >non-ascii< characters, prefix it with u8"..."
		*/
		zipfs_path_t fancy_path = u8"/fancy/path/🐦/🦄.txt";
		std::cout << std::endl;
		print_path_info(fancy_path);

		zipfs_path_t french_path_wrong = "/répertoire-1/éléphant.txt";//<-not ASCII
		std::cout << std::endl;
		print_path_info(french_path_wrong);

		zipfs_path_t french_path_correct = u8"/répertoire-1/éléphant.txt";//<-UTF8
		std::cout << std::endl;
		print_path_info(french_path_correct);
	}

	//§2 filesystem_path_t
	{
		/*
			There is only 1 rule to remember about filesystem_path_t:
				- if your string argument contains >non-ascii< characters, prefix it with u8"..." (or alternatively L"..." on windows)
		*/

		filesystem_path_t fancy_path = u8"/fancy/path/🐦/🦄.txt";
		std::cout << std::endl << fancy_path.u8path() << std::endl;

		filesystem_path_t french_path_wrong = "/répertoire-1/éléphant.txt";//<-not ASCII
		std::cout << std::endl << french_path_wrong.u8path() << std::endl;

		filesystem_path_t french_path_correct = u8"/répertoire-1/éléphant.txt";//<-UTF8
		std::cout << std::endl << french_path_correct.u8path() << std::endl;

		/*
			platform_path() returns a:
				std::wstring / utf16 encoded on windows
				std::string / ut8 encoded on other platforms

			this is mostly used internally by zipfs.
		*/
#ifdef _MSC_VER
		std::wcout << fancy_path.platform_path() << std::endl;
#else
		std::cout << fancy_path.platform_path() << std::endl;
#endif
	}

	return 0;
}