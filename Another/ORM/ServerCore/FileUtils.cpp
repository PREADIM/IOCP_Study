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
	Vector<BYTE> ret; // ����

	fs::path filePath{ path };// ���� �н� ����

	const uint32 fileSize = static_cast<uint32>(fs::file_size(filePath)); // ������ ������
	ret.resize(fileSize); // �������� 

	basic_ifstream<BYTE> inputStream{ filePath }; // basic_ifstream���� ���� �н��� �����ؼ� read�ϱ�.
	inputStream.read(&ret[0], fileSize); // ���Ϳ� ���� �о� ���̱� ���޹��� �迭�� ����� �����ϸ�ȴ�.

	return ret;
}

String FileUtils::Convert(string str)
{
	const int32 srcLen = static_cast<int32>(str.size()); // ���޹��� ���ϳ��� ������

	String ret;
	if (srcLen == 0)
		return ret;

	const int32 retLen = ::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, NULL, 0);
	ret.resize(retLen);
	::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(&str[0]), srcLen, &ret[0], retLen);

	return ret;
}
