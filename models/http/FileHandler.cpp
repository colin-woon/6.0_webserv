#include "FileHandler.hpp"

std::map<std::string, Headers> FileHandler::_hashedFileNames;

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

Headers &FileHandler::getFileMetaData(std::string &hashKey)
{
	return (_hashedFileNames[hashKey]);
}

std::map<std::string, Headers> FileHandler::getAllFileMetaData()
{
	return _hashedFileNames;
}

void FileHandler::addNewFileMetaData(std::string &hashKey, Headers &fileMetaData)
{
	_hashedFileNames[hashKey] = fileMetaData;
}

void FileHandler::deleteFileMetaData(std::string &hashKey)
{
	_hashedFileNames.erase(hashKey);
}

void FileHandler::uploadFile(std::string &hashedFilename, std::string &fileContent)
{
	std::string TEMP_root = "/home/colin/42_core_program/6.0_webserv/var/www";
	std::string filePath = TEMP_root + "/uploads/" + hashedFilename;

	std::ofstream out(filePath.c_str(), std::ios::binary);
	out.write(fileContent.c_str(), fileContent.size());
	out.close();
}

void FileHandler::deleteFile(std::string &hashedFilename)
{
	std::string TEMP_root = "/home/colin/42_core_program/6.0_webserv/var/www";

	std::string filePath = TEMP_root + "/uploads/" + hashedFilename;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) < 0)
		throw Http404NotFoundException(); // Not Found
	if (std::remove(filePath.c_str()) == 0)
	{
		deleteFileMetaData(hashedFilename);
		return;
	}
}
