#pragma once
#include <vector>
#include <string>

class FileHelper
{
public:
	static std::vector<char> readFileByte(const std::string &path);
	static std::vector<std::string> readFileLines(const std::string &path);

};

