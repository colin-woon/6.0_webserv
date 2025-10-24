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

static std::string getUploadPath(Router &router)
{
	std::string uploadStore = router.locationConfig->upload_store;
	if (uploadStore.empty())
		throw Http500InternalServerErrorException();

	std::string finalPath;
	// if (uploadStore[0] == '/')
	finalPath = uploadStore;
	// else
	// {
	// 	std::string root;
	// 	if (!router.locationConfig->root.empty())
	// 		root = router.locationConfig->root;
	// 	else
	// 		root = router.locationConfig->getServerRoot();

	// 	finalPath = root + uploadStore;
	// }

	struct stat fileStat;
	if (stat(finalPath.c_str(), &fileStat) < 0 || !S_ISDIR(fileStat.st_mode))
		throw Http500InternalServerErrorException();

	if (finalPath[finalPath.size() - 1] != '/')
		finalPath += "/";

	return finalPath;
}

void FileHandler::uploadFile(std::string &hashedFilename, std::string &fileContent, Router &router)
{
	std::string uploadPath = getUploadPath(router);
	const std::string &filePath = uploadPath + hashedFilename;

	std::ofstream out(filePath.c_str(), std::ios::binary);
	if (!out.is_open())
		throw Http500InternalServerErrorException();
	out.write(fileContent.c_str(), fileContent.size());
	out.close();
}

void FileHandler::deleteFile(std::string &hashedFilename, Router &router, const std::string &sessionId)
{
	std::string uploadPath = getUploadPath(router);
	const std::string &filePath = uploadPath + hashedFilename;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) < 0)
		throw Http404NotFoundException();
	if (std::remove(filePath.c_str()) == 0)
	{
		deleteFileMetaData(hashedFilename);
		Cookie::removeHashedFileFromSession(sessionId, hashedFilename);
		return;
	}
}

std::string FileHandler::downloadFile(std::string &hashedFilename, Router &router)
{
	std::string uploadPath = getUploadPath(router);
	const std::string &filePath = uploadPath + hashedFilename;

	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) < 0)
		throw Http404NotFoundException();

	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw Http404NotFoundException();

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	return content;
}

void FileHandler::validateFileOwnership(const std::string &sessionId, const std::string &hashedFilename)
{
	Cookie::validateSession(sessionId);

	std::vector<std::string> &userFiles = Cookie::sessionMetadata[sessionId];
	for (size_t i = 0; i < userFiles.size(); ++i)
	{
		if (userFiles[i] == hashedFilename)
			return;
	}

	throw Http404NotFoundException();
}
