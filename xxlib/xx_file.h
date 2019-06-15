#pragma once
#include "xx_bbuffer.h"
#include <fstream>
#include <filesystem>

namespace xx
{
	inline int ReadAllBytes(std::wstring const& path, BBuffer& bb) noexcept {
		std::ifstream f(path, std::ifstream::binary);
		if (!f) return -1;						// not found? no permission? locked?
		xx::ScopeGuard sg([&] { f.close(); });
		f.seekg(0, f.end);
		auto&& siz = f.tellg();
		if ((uint64_t)siz > std::numeric_limits<size_t>::max()) return -2;	// too big
		f.seekg(0, f.beg);
		bb.Resize(siz);
		f.read((char*)bb.buf, siz);
		if (!f) return -3;						// only f.gcount() could be read
		return 0;
	}

	inline int WriteAllBytes(std::wstring const& path, char const* const& buf, size_t const& len) noexcept {
		std::ofstream f(path, std::ios::binary | std::ios::trunc);
		if (!f) return -1;						// no create permission? exists readonly?
		xx::ScopeGuard sg([&] { f.close(); });
		f.write(buf, len);
		if (!f) return -2;						// write error
		return 0;
	}

	inline int WriteAllBytes(std::wstring const& path, BBuffer const& bb) noexcept {
		return WriteAllBytes(path, (char*)bb.buf, bb.len);
	}

	inline std::filesystem::path GetCurrentPath() {
		return std::filesystem::absolute("./");
	}
	// todo: more

	//std::cout << std::filesystem::current_path() << std::endl;
	//for (auto&& entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path())) {
	//	std::cout << entry << std::endl;
	//	std::cout << std::filesystem::absolute(entry) << std::endl;
	//	std::cout << entry.is_regular_file() << std::endl;
	//}
	//try {
	//	auto siz = std::filesystem::file_size(L"中文文件名.txt");
	//	std::cout << siz << std::endl;
	//}
	//catch (std::filesystem::filesystem_error& ex) {
	//	std::cout << ex.what() << std::endl;
	//}

}
