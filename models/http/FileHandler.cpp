#include "FileHandler.hpp"

std::map<std::string, std::string> FileHandler::_hashedFileNames;

FileHandler::FileHandler() {}

FileHandler::FileHandler(const FileHandler &other)
{
	(void)other;
}

FileHandler &FileHandler::operator=(const FileHandler &other)
{
	(void)other;
	return *this;
}

FileHandler::~FileHandler() {}

std::string &FileHandler::getRealFileName(std::string hashKey)
{
	return (_hashedFileNames[hashKey]);
}

void FileHandler::addNewFileName(std::string hashKey, std::string nameValue)
{
	_hashedFileNames[hashKey] = nameValue;
}

void FileHandler::deleteFileName(std::string hashKey)
{
	_hashedFileNames.erase(hashKey);
}

void FileHandler::uploadFile(std::string hashedFilename, std::string &fileContent)
{
	std::string TEMP_root = "/home/colin/42_core_program/6.0_webserv/var/www";

	std::ofstream out(TEMP_root + "/uploads/" + hashedFilename, std::ios::binary);
	out.write(fileContent.c_str(), fileContent.size());
	out.close();
}
