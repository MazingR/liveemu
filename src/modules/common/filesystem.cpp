#include <filesystem.hpp>
#include <windows.h>
#include <queue>


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
					FeFile& addedFiles = files.Add();
					sprintf_s(addedFiles.Path, "%s/%s", dir.Path, findData.cFileName);
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
};