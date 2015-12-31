#include <filesystem.hpp>
#include <windows.h>
#include <queue>
#include "string.hpp"


#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[FileSystem] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[FileSystem] "fmt, __VA_ARGS__) 


namespace FeFileSystem
{
	uint32 DoListFiles(const char* szPath, const char* szFilter, FeTArray<FeFile>& files, bool bRecusrive)
	{
		struct HandledDir
		{
			HANDLE	Handle;
			char	Path[COMMON_PATH_SIZE];
		};
		

		FeTArray<HandledDir> dirs;
		HandledDir& rootDir = dirs.Add();
		sprintf_s(rootDir.Path, szPath);

		while (dirs.GetSize() > 0)
		{
			WIN32_FIND_DATA findData;
			HANDLE hFind;
			char szTmpPath[COMMON_PATH_SIZE];

			HandledDir& dir = dirs.Back();
			dirs.PopBack();

			sprintf_s(szTmpPath, "%s/%s", dir.Path, szFilter);
			hFind = FindFirstFileEx(szTmpPath, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);
			dir.Handle = hFind;

			while (FindNextFile(dir.Handle, &findData))
			{
				const char* szFilePath = findData.cFileName;
					
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					bool bIsVirtual = strcmp(szFilePath, ".") == 0 || strcmp(szFilePath, "..") == 0;

					if (!bIsVirtual)
					{
						HandledDir& subDir = dirs.Add();
						sprintf_s(subDir.Path, "%s/%s", dir.Path, findData.cFileName);
					}
				}
				else
				{
					FeFile& addedFile = files.Add();
					size_t iPathLen = strlen(dir.Path) + strlen(findData.cFileName) + 2;
					FE_ASSERT(iPathLen < COMMON_PATH_SIZE, "file path is too long ! : '%s'", findData.cFileName);
					sprintf_s(addedFile.Path, "%s/%s", dir.Path, findData.cFileName);
				}
			}

			FindClose(hFind);
		}
		return FeEReturnCode::Success;
	}

	uint32 ListFiles(const char* szPath, const char* szFilter, FeTArray<FeFile>& files)
	{
		return DoListFiles(szPath, szFilter, files, false);
	}
	uint32 ListFilesRecursive(const char* szPath, const char* szFilter, FeTArray<FeFile>& files)
	{
		return DoListFiles(szPath, szFilter, files, true);
	}

	uint32 ReadTextFile(const FeFile& file, char** ppOutput)
	{
		return FeEReturnCode::Success;
	}
	uint32 ReadTextFile(const char* szPath, char** ppOutput)
	{
		return FeEReturnCode::Success;
	}

	uint32 ReadBinaryFile(const FeFile& file, void** ppOutput)
	{
		return FeEReturnCode::Success;
	}
	uint32 ReadBinaryFile(const char* szPath, void** ppOutput)
	{
		return FeEReturnCode::Success;
	}

	void GetFullPathWithoutExtension(FeFile& output, const FeFile& input)
	{
		GetFullPathWithoutExtension(output, input.Path);
	}
	void GetFileNameWithoutExtension(FeFile& output, const FeFile& input)
	{
		GetFileNameWithoutExtension(output, input.Path);
	}
	void GetFileName(FeFile& output, const FeFile& input)
	{
		GetFileName(output, input.Path);
	}
	void GetDirectoryName(FeFile& output, const FeFile& input)
	{
		GetDirectoryName(output, input.Path);
	}

	void GetFullPathChangeExtension(FeFile& output, const char* input, const char* szExt)
	{
		GetFullPathWithoutExtension(output, input);
		strcat(output.Path, ".");
		strcat(output.Path, szExt);
	}
	void GetFullPathWithoutExtension(FeFile& output, const char* input)
	{
		size_t iPoint = FeStringTools::LastIndexOf(input, '.');
		sprintf_s(output.Path, input);
		output.Path[iPoint] = 0;
	}
	void GetFileNameWithoutExtension(FeFile& output, const char* input)
	{
		FE_ASSERT(false, "todo");

		size_t iLastSlash = FeStringTools::LastIndexOf(input, '/');
		size_t iLastBSlash = FeStringTools::LastIndexOf(input, '\\');
		iLastSlash = iLastSlash > iLastBSlash ? iLastSlash : iLastBSlash;

		sprintf_s(output.Path, strlen(input) - iLastSlash, input + iLastSlash);
	}
	void GetFileName(FeFile& output, const char* input)
	{
		FE_ASSERT(false, "todo");
	}
	void GetDirectoryName(FeFile& output, const char* input)
	{
		FE_ASSERT(false, "todo");
	}
	void FormatPath(FeFile& file)
	{
		FeStringTools::Replace(file.Path, '\\', '/');
	}

	bool FileExists(const FeFile& file)
	{
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFileEx(file.Path, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);
		return hFind != ((HANDLE)-1);
	}
};