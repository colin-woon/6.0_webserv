#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <map>
#include <string>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include "HttpExceptions.hpp"

typedef std::map<std::string, std::string> Headers;

class FileHandler
{
private:
	FileHandler();
	FileHandler(const FileHandler &other);
	FileHandler &operator=(const FileHandler &other);
	~FileHandler();

	static std::map<std::string, Headers> _hashedFileNames;

public:
	static Headers &getFileMetaData(std::string &hashKey);
	static std::map<std::string, Headers> getAllFileMetaData();
	static void addNewFileMetaData(std::string &hashKey, Headers &fileMetaData);
	static void deleteFileMetaData(std::string &hashKey);
	static void uploadFile(std::string &hashedFilename, std::string &fileContent);
	static void deleteFile(std::string &hashedFilename);
};

#endif
