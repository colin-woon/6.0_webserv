#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <map>
#include <string>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include "HttpExceptions.hpp"

class FileHandler
{
private:
	FileHandler();
	FileHandler(const FileHandler &other);
	FileHandler &operator=(const FileHandler &other);
	~FileHandler();

	static std::map<std::string, std::map<std::string, std::string>> _hashedFileNames;

public:
	static std::map<std::string, std::string> &getFileMetaData(std::string &hashKey);
	static std::map<std::string, std::map<std::string, std::string>> getAllFileMetaData();
	static void addNewFileMetaData(std::string &hashKey, std::map<std::string, std::string> &fileMetaData);
	static void deleteFileMetaData(std::string &hashKey);
	static void uploadFile(std::string &hashedFilename, std::string &fileContent);
	static void deleteFile(std::string &hashedFilename);
};

#endif
