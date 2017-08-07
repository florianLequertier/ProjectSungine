

#include <sstream>

#include "dirent.h"
#include "FileHandler.h"

namespace FileHandler {

Path::Path()
	:m_data("")
{
	format();
}

Path::Path(const std::string& path)
	: m_data(path)
{
	format();
}

Path::Path(const Path & path, const std::string & appendFolder)
{
	m_data = path.toString() + "/" + appendFolder;

	format();
}

Path::Path(const CompletePath& completePath)
	:Path(completePath.getPath())
{
	format();
}

void Path::push_back(const std::string& folderName)
{
	size_t offset = 0;
	if (folderName[0] == '/' || folderName[0] == '\\')
	{
		offset = 1;
	}

	m_data.append(folderName, offset, folderName.size() - offset);

	format();
}

void Path::push_back(const Path& subPath)
{
	push_back(subPath.m_data);
}

void Path::pop_back()
{
	assert(m_data.back() == '/');

	size_t cutIdx = m_data.find_last_of('/', m_data.size() - 2);
	m_data = m_data.substr(0, cutIdx + 1);
	format();
}

void Path::pop_front()
{
	size_t cutIdx = m_data.find_first_of('/', m_data.size() - 1);
	m_data = m_data.substr(cutIdx + 1);
	format();
}

std::string Path::operator[](size_t idx) const
{
	size_t cutPos01 = 0; // m_data.find_first_of("/\\");
	size_t cutPos02 = m_data.find_first_of("/\\" /*, cutPos01 + 1*/);

	for (int i = 0; i < idx; i++)
	{
		cutPos01 = cutPos02 + 1;
		cutPos02 = m_data.find_first_of("/\\", cutPos01 + 1);
	}

	size_t firstPos = cutPos01;
	size_t endPos = (cutPos02 - cutPos01);
	return m_data.substr(firstPos, endPos);
}

size_t Path::size() const
{
	size_t size = -1;
	size_t cutPos = 0; // m_data.find_first_of("/\\");

	while (cutPos != std::string::npos)
	{
		cutPos = m_data.find_first_of("/\\", cutPos + 1);
		size++;
	}

	return size;
}

size_t Path::getPathFolderNames(std::vector<std::string>& folderNames) const
{
	size_t size = 0;
	size_t cutPos01 = m_data.find_first_of("/\\");
	size_t cutPos02 = m_data.find_first_of("/\\", cutPos01 + 1);

	for (int i = 1; i < m_data.size(); i++)
	{
		cutPos01 = m_data.find_first_of("/\\", cutPos02 + 1);
		cutPos02 = m_data.find_first_of("/\\", cutPos01 + 1);

		folderNames.push_back(m_data.substr(cutPos01 + 1, (cutPos02 - cutPos01) - 1));

		size++;
	}

	return size;
}

void Path::format()
{
	if (m_data.empty())
		return;

	m_data.shrink_to_fit();

	size_t foundPos = m_data.find_first_of('\\');
	while (foundPos != std::string::npos)
	{
		m_data[foundPos] = '/';
		foundPos = m_data.find_first_of('\\', foundPos + 1);
	}

	foundPos = m_data.find("//");
	while (foundPos != std::string::npos)
	{
		m_data[foundPos] = '/';
		for (int i = foundPos + 1; i < m_data.size() - 1; i++)
		{
			m_data[i] = m_data[i + 1];
		}
		m_data.pop_back();
		foundPos = m_data.find("//", foundPos + 1);
	}

	foundPos = m_data.find_last_of('/');
	if (foundPos != m_data.size() - 1)
	{
		m_data.push_back('/'); //%NOCOMMIT%
	}
}

bool Path::empty() const
{
	return m_data.empty();
}

std::string Path::back(int idx) const
{
	assert(m_data.back() == '/');

	size_t cutPos = m_data.find_last_of("/\\", m_data.size() - 2);
	for (int i = 0; i < idx - 1; i++)
	{
		cutPos = m_data.find_last_of("/\\", cutPos - 1);
	}
	size_t length = 0;
	if (cutPos == std::string::npos)
	{
		cutPos = 0;
		length = m_data.size() - 2;
	}
	else
	{
		cutPos++;
		length = (m_data.size() - cutPos) - 1;
	}
	return m_data.substr(cutPos, length);
}

Path Path::getSubPath(int begin, int count) const
{
	size_t cutPos01 = 0; // m_data.find_first_of("/\\");
	size_t cutPos02 = m_data.find_first_of("/\\" /*, cutPos01 + 1*/);
	size_t cutPos03 = 0;

	for (int i = 0; i < begin; i++)
	{
		cutPos01 = cutPos02 + 1;
		cutPos02 = m_data.find_first_of("/\\", cutPos01 + 1);
	}

	for (int i = 0; i < count - 1; i++)
	{
		cutPos03 = cutPos02 + 1;
		cutPos02 = m_data.find_first_of("/\\", cutPos03 + 1);
	}

	size_t firstPos = cutPos01;
	size_t endPos = (cutPos02 - cutPos01) + 1;
	return Path(m_data.substr(firstPos, endPos));
}

const std::string& Path::toString() const
{
	return m_data;
}

std::ofstream& operator<<(std::ofstream& os, const Path& path)
{
	os << path.c_str();
	return os;
}

//////////////////////////////

CompletePath::CompletePath()
	: m_fileName("")
	, m_extention("")
	, m_complete(false)
	, m_subFileName("")
	, m_data("")
{
	format();
}

CompletePath::CompletePath(const std::string & completePath, const std::string* subFileName)
	: m_fileName("")
	, m_extention("")
	, m_complete(false)
	, m_subFileName("")
	, m_data("")
{
	std::string path = "";
	splitPathFileNameExtention(completePath, path, m_fileName, m_extention);
	if (path != "")
	{
		m_path = Path(path);
	}

	if (subFileName != nullptr)
	{
		m_subFileName = *subFileName;
	}

	format();

	assert(hasValidExtention() && hasValidFileName());
}

CompletePath::CompletePath(Path path, const std::string& fileName, const std::string& extention, const std::string* subFileName)
	: m_path(path)
	, m_fileName(fileName)
	, m_extention(extention)
	, m_complete(true)
	, m_subFileName("")
	, m_data("")
{
	if (subFileName != nullptr)
	{
		m_subFileName = *subFileName;
	}

	format();

	assert(hasValidExtention() && hasValidFileName());
}

CompletePath::CompletePath(Path path, const std::string& fileNameAndExtention, const std::string* subFileName)
	: m_path(path)
	, m_fileName("")
	, m_extention("")
	, m_subFileName("")
	, m_data("")
{
	if (!getFileNameAndExtentionFromExtendedFilename(fileNameAndExtention, m_fileName, m_extention) != std::string::npos)
	{
		m_complete = false;
	}

	if (subFileName != nullptr)
	{
		m_subFileName = *subFileName;
	}

	format();

	assert(hasValidExtention() && hasValidFileName());
}

const std::string& CompletePath::toString() const
{
	return m_data;
}

std::string CompletePath::operator[](size_t idx) const
{
	const std::string& folderPath = m_path.toString();

	size_t cutPos01 = 0; // folderPath.find_first_of("/\\");
	size_t cutPos02 = folderPath.find_first_of("/\\" /*, cutPos01 + 1*/);

	for (int i = 0; i < idx; i++)
	{
		cutPos01 = cutPos02 + 1;
		cutPos02 = folderPath.find_first_of("/\\", cutPos01 + 1);
	}

	size_t firstPos = cutPos01;
	size_t endPos = (cutPos02 - cutPos01);
	return m_data.substr(firstPos, endPos);
}

size_t CompletePath::size() const
{
	return m_path.size() + 1;
}

const Path& CompletePath::getPath() const
{
	return m_path;
}

const std::string& CompletePath::getFilename() const
{
	return m_fileName;
}

std::string CompletePath::getFilenameWithExtention() const
{
	return m_fileName + m_extention;
}

const std::string& CompletePath::getExtention() const
{
	return m_extention;
}

const std::string & CompletePath::getSubFileName() const
{
	return m_subFileName;
}

FileType CompletePath::getFileType() const
{
	return getFileTypeFromExtention(getExtention());
}

bool CompletePath::operator<(const CompletePath & other) const
{
	return m_data < other.m_data;
}

bool CompletePath::operator==(const CompletePath & other) const
{
	return m_data == other.m_data;
}

bool CompletePath::hasValidFileName() const
{
	return (m_fileName != ""
		&& m_fileName.find_first_of("/\\.-'&") == std::string::npos
		);
}

void CompletePath::format()
{
	//format path
	m_path.format();

	m_fileName.shrink_to_fit();
	m_extention.shrink_to_fit();
	m_fileName = m_fileName.substr(0, m_fileName.find_first_of('\0'));
	m_extention = m_extention.substr(0, m_extention.find_first_of('\0'));

	if (!m_fileName.empty())
	{
		//remove /filename patern
		if (m_fileName.find_first_of('/\\') == 0)
		{
			m_fileName.erase(0);
		}
		//remove filename. patern
		if (m_fileName.find_last_of('.') != std::string::npos)
		{
			m_fileName.pop_back();
		}
	}
	if (!m_extention.empty())
	{
		//remove extension patern (-> .extention)
		if (m_extention.find_first_of('.') != 0)
		{
			m_extention.insert(m_extention.begin(), '.');
		}
	}

	m_data += m_path.toString();
	m_data += m_fileName;
	m_data += m_extention;
	if (!m_subFileName.empty())
		m_data += ("/" + m_subFileName);
}

bool CompletePath::empty() const
{
	return m_path.empty() && m_extention.empty() && m_fileName.empty();
}

void CompletePath::replaceExtension(const std::string& newExtension)
{
	m_extention = newExtension;
	format();
}

bool CompletePath::hasValidExtention() const 
{
	return (m_extention != ""
		&& m_extention.find_first_of("/\\-'&") == std::string::npos
		&& m_extention.find_first_of('.') == 0
		);
};

std::ofstream& operator<<(std::ofstream& os, const CompletePath& path)
{
	os << path.c_str();
	return os;
}

//////////////////////////////

size_t getExtentionFromExtendedFilename(const std::string& filename, std::string& outExtention)
{
	size_t cutPos = filename.find_last_of('.');
	if (cutPos == std::string::npos)
	{
		outExtention = "";
		return cutPos;
	}
	else
	{
		outExtention = filename.substr(cutPos);
		return cutPos;
	}
}

size_t getFileNameAndExtentionFromExtendedFilename(const std::string& extendedFileName, std::string& filename, std::string& outExtention)
{
	size_t cutPos = extendedFileName.find_last_of('.');
	if (cutPos == std::string::npos)
		return cutPos;
	else
	{
		filename = extendedFileName.substr(0, cutPos);
		outExtention = extendedFileName.substr(cutPos);
		return cutPos;
	}
}

FileType getFileTypeFromExtention(const std::string& extention)
{
	assert(extention.find_first_of("/\\,;") == std::string::npos);

	if (extention == ".jpg" || extention == ".jpeg" || extention == ".png" || extention == ".bmp" || extention == ".tga")
	{
		return FileType::IMAGE;
	}
	else if (extention == ".obj" || extention == ".fbx")
	{
		return FileType::MESH;
	}
	else if (extention == ".mp3" || extention == ".ogg")
	{
		return FileType::SOUND;
	}
	else if (extention == ".glProg")
	{
		return FileType::SHADER_PROGRAM;
	}
	else if (extention == ".mat")
	{
		return FileType::MATERIAL;
	}
	else if (extention == ".ctx")
	{
		return FileType::CUBE_TEXTURE;
	}
	else if (extention == ".asm")
	{
		return FileType::ANIMATION_STATE_MACHINE;
	}
	else
	{
		PRINT_WARNING("L'extention : " + extention + " n'est pas prix en charge.");
		//assert(false && "Wrong extention");
		return FileType::NONE;
	}
}

void getAllDirNames(const Path& path, std::vector<std::string>& outDirNames)
{
	assert(outDirNames.size() == 0);

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type == DT_DIR && std::strcmp("..", ent->d_name) != 0 && std::strcmp( ".", ent->d_name) != 0)
				outDirNames.push_back(ent->d_name);
		}
		closedir(dir);
	}
	else {
		//could not open directory
		std::cout << "error, can't open directory at path : " << path.toString() << std::endl;
	}
}

void getAllFileNames(const Path & path, std::vector<std::string>& outFileNames)
{
	assert(outFileNames.size() == 0);

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type != DT_DIR)
				outFileNames.push_back(ent->d_name);
		}
		closedir(dir);
	}
	else {
		//could not open directory
		std::cout << "error, can't open directory at path : " << path.toString() << std::endl;
	}
}

std::vector<std::string> getAllFileAndDirNames(const Path& path)
{
	std::vector<std::string> fileAndDirNames;

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			fileAndDirNames.push_back(ent->d_name);
		}
		closedir(dir);
	}
	else {
		//could not open directory
		std::cout << "error, can't open directory at path : " << path.toString() << std::endl;
	}

	return fileAndDirNames;
}


bool directoryExists(const Path& path, const std::string& name)
{
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type == DT_DIR && std::string(ent->d_name) == name)
				return true;
		}
		closedir(dir);
	}
	else {
		//could not open directory
		std::cout << "error, can't open directory at path : " << path.toString() << std::endl;
	}

	return false;
}

bool directoryExists(const Path& path)
{
	return (opendir(path.c_str()) != NULL);
}

bool fileExists(const CompletePath& completePath)
{
	if (!completePath.hasValidFileName())
		return false;

	const std::string& path = completePath.getPath().toString();
	const std::string& filename = completePath.getFilenameWithExtention();

	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_type != DT_DIR && std::string(ent->d_name) == filename)
				return true;
		}
		closedir(dir);
	}
	return false;
}

void addDirectories(const Path& path)
{
	if (directoryExists(path))
		return;

	std::vector<std::string> dirNames;
	if(path.getPathFolderNames(dirNames) == 0)
		return;

	Path currentPath;

	for (int i = 0; i < dirNames.size(); i++)
	{
		if (!directoryExists(currentPath, dirNames[i]))
		{
			addDirectory(dirNames[i], currentPath);
		}
		currentPath.push_back(dirNames[i]);
	}
}

void addDirectory(const std::string& name, const Path& path)
{
#ifdef _WIN32
		CreateDirectory((path.toString() + name).c_str(), NULL);
#endif // _WIN32

#ifdef linux
	std::cerr << "error : the addDirectory function isn't support on linux platform yet." << std::endl;
#endif // linux


	//TODO add linux support.
}

void removeDirectory(const Path& path)
{
#ifdef _WIN32
	RemoveDirectory((path.toString()).c_str());
#endif // _WIN32

#ifdef linux
	std::cerr << "error : the addDirectory function isn't support on linux platform yet." << std::endl;
#endif // linux


	//TODO add linux support.
}

std::vector<std::string> splitString(const std::string& s, char delim)
{
	std::vector<std::string> elements;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elements.push_back(item);
	}
	return elements;
}

std::vector<std::string> splitString(const std::string& s, char delim01, char delim02)
{
	std::vector<std::string> tmpElements;
	std::vector<std::string> elements;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim01)) {
		tmpElements.push_back(item);
	}

	for (int i = 0; i < tmpElements.size(); i++)
	{
		std::stringstream ss2(tmpElements[i]);

		while (std::getline(ss2, item, delim02)) {
			elements.push_back(item);
		}
	}

	return elements;
}

std::size_t splitPathFileName(const std::string& pathAndFileName, std::string& path, std::string& filename)
{
	std::size_t pathLength = pathAndFileName.find_last_of("/\\");
	if (pathLength == std::string::npos) {
		pathLength = 0;
		path = "";
		filename = pathAndFileName;
	}
	else {
		path = pathAndFileName.substr(0, pathLength + 1);
		filename = pathAndFileName.substr(pathLength + 1);
	}
	return pathLength;
}

std::size_t splitFileNameExtention(const std::string fileNameAndExtension, std::string& fileName, std::string& extention)
{
	std::size_t fileNameLength = fileNameAndExtension.find_last_of(".");
	if (fileNameLength == std::string::npos) {
		//No extention found
		fileNameLength = fileNameAndExtension.size();
		fileName = fileNameAndExtension;
		extention = "";
	}
	else {
		fileName = fileNameAndExtension.substr(0, fileNameLength);
		extention = fileNameAndExtension.substr(fileNameLength + 1);
	}
	return fileNameLength;
}

std::size_t splitPathFileNameExtention(const std::string& pathAndFileNameAndExtention, std::string& path, std::string& filename, std::string& extention)
{
	std::string filenameAndExtention;

	splitPathFileName(pathAndFileNameAndExtention, path, filenameAndExtention);
	return splitFileNameExtention(filenameAndExtention, filename, extention);
}

bool createFile(const CompletePath &path, const std::string& content)
{
	if (fileExists(path))
		return false;
	else
	{
		std::ofstream s;
		s.open(path.toString());
		if (s.is_open())
		{
			s << content.c_str();
			s.close();
		}
	}
}

bool createFileOverride(const CompletePath &path, const std::string& content)
{
	std::ofstream s;
	s.open(path.toString(), std::ios::trunc);
	if (s.is_open())
	{
		s << content.c_str();
		s.close();
	}
}

void copyPastFile(const CompletePath& from, const Path& to)
{
	// WARNING : not safe TODO

	assert(directoryExists(to));
	assert(fileExists(from));
	if (!directoryExists(to) || !fileExists(from))
		return;

	CompletePath toFile(to, from.getFilenameWithExtention());

	std::ifstream src(from.toString(), std::ios::binary);
	std::ofstream dst(toFile.toString(), std::ios::binary);

	dst << src.rdbuf();
}

void deleteFile(const CompletePath& completePath)
{
	// WARNING : not safe TODO

	assert(fileExists(completePath));

	if(fileExists(completePath))
		std::remove(completePath.c_str());
}

void moveFile(const CompletePath &from, const Path &to)
{
	copyPastFile(from, to);
	deleteFile(from);
}

void renameFile(const CompletePath &filePath, const std::string& newFileName)
{
	assert(fileExists(filePath));

	CompletePath newFilePath(filePath.getPath(), newFileName, filePath.getExtention());

	assert(!fileExists(newFilePath));
	if (fileExists(newFilePath))
		return;

	std::rename(filePath.toString().data(), newFilePath.toString().data());
}

void renameDirectory(const Path &directoryPath, const std::string& newDirectoryName)
{
	assert(directoryExists(directoryPath));

	Path newDirectoryPath = directoryPath;
	newDirectoryPath.pop_back();
	newDirectoryPath.push_back(newDirectoryName);

	assert(!directoryExists(newDirectoryPath));
	if (directoryExists(newDirectoryPath))
		return;

	std::rename(directoryPath.toString().data(), newDirectoryPath.toString().data());
}

bool isValidFileOrDirectoryName(const std::string& name)
{
	for (int i = 0; i < name.size(); i++)
	{
		const char& currentChar = name.at(i);
		if (currentChar < 48
			//Allow numbers
			|| (currentChar > 57 && currentChar < 65)
			//Allow A->Z
			|| (currentChar > 90 && currentChar < 95)
			//Allow '_'
			|| (currentChar > 95 && currentChar < 97)
			//Allow a->z
			|| currentChar > 122)
		{
			return false;
		}
		else
			return true;
	}
}

std::string getFileExtention(const std::string& filePath)
{
	return filePath.substr(filePath.find_last_of('.'));
}

}
