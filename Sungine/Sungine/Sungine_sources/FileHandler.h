#pragma once

#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

#include "ErrorHandler.h"

/**
* FileHandler system :
* A Path is a path toward a directory, with format : "toto/tutu/"
* A CompletePath is a path toward a file, composed by a Path, a fileName and a file extention
* A CompletePath toward : "toto/tutu/titi.txt" will give us :
* CompletePath::Path = "toto/tutu/",
* CompletePath::FileName = "titi",
* CompletePath::Extention = ".txt",
* The toString() methode will return : "toto/tutu/titi.txt"
*/
namespace FileHandler {

enum FileType
{
	NONE,
	IMAGE,
	MESH,
	SOUND,
	MATERIAL,
	MATERIAL_INSTANCE,
	CUBE_TEXTURE,
	ANIMATION_STATE_MACHINE,
};

class CompletePath;
class ResourceFile;

class Path
{
private:
	std::string m_data;

public:
	Path();
	Path(const std::string& path);
	Path(const Path& path, const std::string& appendFolder);
	Path(const CompletePath& completePath);

	const std::string& toString() const;
	const char* c_str() const { return toString().c_str(); }
	//operator std::string() const { return toString(); }

	std::string operator[](size_t idx) const;
	size_t size() const;
	size_t getPathFolderNames(std::vector<std::string>& folderNames) const;

	void push_back(const std::string& folderName);
	void push_back(const Path& subPath);
	void pop_back();
	void pop_front();
	void format();
	bool empty() const;
	std::string back(int idx = 1) const;

	Path getSubPath(int begin, int count) const;

	bool operator==(const Path& other) const
	{
		return m_data == other.m_data;
	}
};

std::ofstream& operator<<(std::ofstream& os, const Path& path);

///////////////////

class CompletePath
{
private:
	std::string m_data;
	Path m_path;
	std::string m_fileName;
	std::string m_extention;
	bool m_complete;
	std::string m_subFileName; //Allow a complete path to represent a "sub-resource"

public:
	CompletePath();
	CompletePath(const std::string& completePath, const std::string* subFileName = nullptr);
	CompletePath(Path path, const std::string& fileName, const std::string& extention, const std::string* subFileName = nullptr);
	CompletePath(Path path, const std::string& fileNameAndExtention, const std::string* subFileName = nullptr);

	const std::string& toString() const;
	const char* c_str() const { return toString().c_str(); }
	//operator std::string() const { return toString(); }

	std::string operator[](size_t idx) const;
	size_t size() const;

	const Path& getPath() const;
	const std::string& getFilename() const;
	std::string getFilenameWithExtention() const;
	const std::string& getExtention() const;
	bool hasValidFileName() const;
	bool hasValidExtention() const;
	void format();
	bool empty() const;
	void replaceExtension(const std::string& newExtension);

	const std::string& getSubFileName() const;

	FileType getFileType() const;

	bool operator<(const CompletePath& other) const;
	bool operator==(const CompletePath& other) const;
};

//bool operator<(const CompletePath& a, const CompletePath& b)
//{
//	return a.m_data < b.m_data;
//}


std::ofstream& operator<<(std::ofstream& os, const CompletePath& path);


///////////////////

size_t getExtentionFromExtendedFilename(const std::string& filename, std::string& outExtention);
size_t getFileNameAndExtentionFromExtendedFilename(const std::string& extendedFileName, std::string& filename, std::string& outExtention);

FileType getFileTypeFromExtention(const std::string& extention);

//fill a vector with all directories's names in the directory at given path.
void getAllDirNames(const Path& path, std::vector<std::string>& outDirNames);
//fill a vector with all file's names in the directory at given path.
void getAllFileNames(const Path& path, std::vector<std::string>& outFileNames);
//fill a vector with all files' and directories' names in the directory at given path.
std::vector<std::string> getAllFileAndDirNames(const Path& path);

bool directoryExists(const Path& path);
bool directoryExists(const Path& path, const std::string& name);
bool fileExists(const CompletePath& completePath);

void addDirectories(const Path& path);
void addDirectory(const std::string& name, const Path& path);
void removeDirectory(const Path& path);

//string helper
std::vector<std::string> splitString(const std::string& s, char delim);
std::vector<std::string> splitString(const std::string& s, char delim01, char delim02);
std::size_t splitPathFileName(const std::string& pathAndFileName, std::string& path, std::string& filename);
std::size_t splitFileNameExtention(const std::string fileNameAndExtension, std::string& fileName, std::string& extention);
std::size_t splitPathFileNameExtention(const std::string& pathAndFileNameAndExtention, std::string& path, std::string& filename, std::string& extention);

bool createFile(const CompletePath &path, const std::string& content);
bool createFileOverride(const CompletePath &path, const std::string& content);
void copyPastFile(const CompletePath &from, const Path &to);
void deleteFile(const CompletePath& completePath);
void moveFile(const CompletePath &from, const Path &to);

void renameFile(const CompletePath &filePath, const std::string& newFileName);
void renameDirectory(const Path &directoryPath, const std::string& newDirectoryName);

bool isValidFileOrDirectoryName(const std::string& name);

std::string getFileExtention(const std::string& filePath);

}