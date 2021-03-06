#ifndef ZIPARCHIVE_H
#define	ZIPARCHIVE_H

#include <cstdio>
#include <string>
#include <vector>
#include <Windows.h>

#include <zipconf.h>
#include "UnicodeConv.h"

struct zip;

#define DIRECTORY_SEPARATOR '/'
#define IS_DIRECTORY(str) (str.length()>0 && str[str.length()-1]==DIRECTORY_SEPARATOR)

class CZipEntry;

class CZipArchive
{
public:

	/*
	 * WRITE 添加到已有zip 或 创建新zip
	 * NEW 创建新zip 或 删除已有zip的所有数据
	 */
	enum OpenMode { NOT_OPEN, READ_ONLY, WRITE, NEW };

	enum State { ORIGINAL, CURRENT };

	CZipArchive(const std::string &zipPath, bool isUtf8 = false, const std::string &password = "");
	virtual ~CZipArchive(void);

	// 打开zip存档
	bool open(OpenMode mode = READ_ONLY, bool checkConsistency = false);

	// 返回zip路径
	std::string getPath(void) const
	{
		return path;
	}

	// 返回打开模式
	OpenMode getMode(void) const
	{
		return mode;
	}

	// 关闭zip存档
	void close(void);

	// 关闭zip存档，回滚操作
	void discard(void);

	// 删除存档
	bool unlink(void);

	// zip存档是否打开
	bool isOpen(void) const
	{
		return zipHandle != NULL;
	}

	// zip存档是否可以修改
	bool isMutable(void) const
	{
		return isOpen() && mode != NOT_OPEN && mode != READ_ONLY;
	}

	// zip存档是否要加密
	bool isEncrypted(void) const
	{
		return !password.empty();
	}

	// 定义zip存档的注释
	std::string getComment(State state = CURRENT) const;
	bool setComment(const std::string &comment) const;

	// 删除zip存档的注释
	bool removeComment(void) const
	{
		return setComment(std::string());
	}

	/**
	 * 返回zip存档中的条目数量(包含文件夹)
	 * 如果有3个条目，你删除一个条目，还是会返回3,
	 * 如果你添加一个条目，CURRENT状态将返回4，ORIGINAL将返回3。
	 *
	 * getEntries可以获取zip存档的真实条目
	 */
	zip_int64_t getNbEntries(State state = CURRENT) const;
	zip_int64_t getEntriesCount(State state = CURRENT) const
	{
		return getNbEntries(state);
	}

	// 返回所有条目
	std::vector<CZipEntry> getEntries(State state = CURRENT) const;

	// 判断条目是否存在
	bool hasEntry(const std::string &name, bool excludeDirectories = false, bool caseSensitive = true, State state = CURRENT) const;

	// 根据索引获取条目
	CZipEntry getEntry(const std::string &name, bool excludeDirectories = false, bool caseSensitive = true, State state = CURRENT) const;
	CZipEntry getEntry(zip_int64_t index, State state = CURRENT) const;

	// 定义条目的注释
	std::string getEntryComment(const CZipEntry &entry, State state = CURRENT) const;
	bool setEntryComment(const CZipEntry &entry, const std::string &comment) const;

	// 读取条目内容
	void *readEntry(const CZipEntry &zipEntry, bool asText = false, State state = CURRENT) const;
	void *readEntry(const std::string &zipEntry, bool asText = false, State state = CURRENT) const;
	std::string readString(const std::string &zipEntry, CZipArchive::State state = CZipArchive::CURRENT) const;

	// 把条目内容写入到文件
	bool writeEntry(const std::string &zipEntry, const std::string &fileName, State state = CURRENT) const;
	bool writeEntry(const CZipEntry &zipEntry, const std::string &fileName, State state = CURRENT) const;

	// 删除条目
	int deleteEntry(const CZipEntry &entry) const;
	int deleteEntry(const std::string &entry) const;

	// 重命名条目
	int renameEntry(const CZipEntry &entry, const std::string &newName) const;
	int renameEntry(const std::string &entry, const std::string &newName) const;

	// 添加文件到zip存档
	bool addFile(const std::string &entryName, const std::string &file) const;

	// 添加数据到zip存档
	bool addData(const std::string &entryName, const void *data, unsigned int length, bool freeData = false) const;

	// 添加目录条目到zip存档，entryName必须是目录(以'/'结尾)
	bool addEntry(const std::string &entryName) const;

	// UTF8编码转换
#define Utf8ToAscii(str) (isUtf8 ? ConvertUtf8ToMultiBytes(str) : str)
#define AsciiToUtf8(str) (isUtf8 ? ConvertMultiBytesToUtf8(str) : str)
#define DEFAULLT_ENC_FLAG (isUtf8 ? ZIP_FL_ENC_UTF_8 : ZIP_FL_ENC_GUESS)

	// 解压zip存档
	void extract(const std::string &folderName);

	// 添加目录到zip存档
	bool addFolder(const std::string &entryName, const std::string &folderName);

	// 合并路径
	std::string concatPath(const std::string &strDir, const std::string &strFile, char slash = '\\');

	// 创建目录
	void createFolder(const std::string &folderName);

	// 获取目录路径
	std::string getFolderPath(const std::string &filePath);

	// time_t 转成 FileTime
	void TimetToFileTime(time_t t, LPFILETIME pft) const;

private:
	std::string path;
	zip *zipHandle;
	OpenMode mode;
	std::string password;
	bool isUtf8;

	// 创建ZipEntry
	CZipEntry createEntry(struct zip_stat *stat) const;

	CZipArchive(const CZipArchive &zf);
	CZipArchive &operator=(const CZipArchive &);
};

class CZipEntry
{
	friend class CZipArchive;

public:
	CZipEntry(void) : zipFile(NULL), index(0), time(0), method(-1), size(0), sizeComp(0), crc(0)  {}
	virtual ~CZipEntry(void) {}

	std::string getName(void) const
	{
		return name;
	}

	zip_uint64_t getIndex(void) const
	{
		return index;
	}

	time_t getDate(void) const
	{
		return time;
	}

	int getMethod(void) const
	{
		return method;
	}

	// 返回文件未压缩尺寸
	zip_uint64_t getSize(void) const
	{
		return size;
	}

	// 返回文件压缩尺寸
	zip_uint64_t getInflatedSize(void) const
	{
		return sizeComp;
	}

	int getCRC(void) const
	{
		return crc;
	}

	bool isDirectory(void) const
	{
		return IS_DIRECTORY(name) || dirFlag;
	}

	bool isFile(void) const
	{
		return !isDirectory();
	}

	bool isNull(void) const
	{
		return zipFile == NULL;
	}

private:
	const CZipArchive *zipFile;
	std::string name;
	zip_uint64_t index;
	time_t time;
	int method;
	zip_uint64_t size;
	zip_uint64_t sizeComp;
	int crc;
	bool dirFlag;

	CZipEntry(
		const CZipArchive *zipFile, 
		const std::string &name, zip_uint64_t index, 
		time_t time, int method, 
		zip_uint64_t size, zip_uint64_t sizeComp, 
		int crc, bool dirFlag
	) : zipFile(zipFile),
		name(name), index(index),
		time(time), method(method),
		size(size), sizeComp(sizeComp),
		crc(crc), dirFlag(dirFlag)
	{

	}
};

#endif

