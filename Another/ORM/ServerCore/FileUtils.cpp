#include "pch.h"
#include "FileUtils.h"
#include <filesystem>
#include <fstream>

/*-----------------
	FileUtils
------------------*/

namespace fs = std::filesystem;

Vector<BYTE> FileUtils::ReadFile(const WCHAR* path)
{
	Vector<BYTE> ret; // 리턴

	fs::path filePath{ path };// 파일 패스 저장

	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath)); // 파일의 사이즈
	ret.resize(fileSize); // 리사이즈 

	basic_ifstream<BYTE> inputStream{ filePath }; // basic_ifstream으로 파일 패스를 전달해서 read하기.
	inputStream.read(&ret[0], fileSize); // 벡터에 파일 읽어 들이기 전달받을 배열과 사이즈를 전달하면된다.

	return ret;
}

String FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size()); // 전달받은 파일내용 사이즈

	String ret;
	if (srcLen == 0)
		return ret;

	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
