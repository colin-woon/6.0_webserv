#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <map>
#include <string>
#include <fstream>

class FileHandler
{
private:
	FileHandler();
	FileHandler(const FileHandler &other);
	FileHandler &operator=(const FileHandler &other);
	~FileHandler();

	static std::map<std::string, std::string> _hashedFileNames;

public:
	static std::string &getRealFileName(std::string hashKey);
	static void addNewFileName(std::string hashKey, std::string nameValue);
	static void deleteFileName(std::string hashKey);
	static void uploadFile(std::string hashedFilename, std::string &fileContent);
};

#endif
