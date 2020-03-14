#include "FileHelper.h"
#include "LoggerAPI.h"
#include <fstream>
#include <string>
#include <cassert>

using std::vector;
using std::string;
using std::fstream;
using std::ifstream;

vector<char> FileHelper::readFileByte(const string & path)
{

	ifstream file(path, std::ios::ate | std::ios::binary);

	assert(file.is_open());

	const auto fileSize = static_cast<size_t>(file.tellg());
	auto result = vector<char>(fileSize);
	file.seekg(0);
	file.read(result.data(), fileSize);
	file.close();

	return result;
}

std::vector<string> FileHelper::readFileLines(const string & path)
{
	ifstream file(path);

	if (!file.is_open())
	{
		LoggerAPI::getLogger()->logError("Could not open file " + path);
		return vector<string>();
	}

	auto result = vector<string>();

	for (string line; std::getline(file, line);)
	{
		result.push_back(line);
	}

	return result;
}
