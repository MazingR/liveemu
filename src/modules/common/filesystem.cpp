#include <filesystem.hpp>
#include <windows.h>
#include <queue>


#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[FileSystem] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[FileSystem] "fmt, __VA_ARGS__) 


namespace FeFileSystem
{
	uint32 DoListFiles(const char* szPath, FeTArray<FeFile>& files, bool bRecusrive)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;

		hFind = FindFirstFileEx(szPath, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
		

		if (hFind == INVALID_HANDLE_VALUE)
		{
			FE_LOG("FindFirstFileEx failed (%d)\n", GetLastError());
			return FeEReturnCode::FileSystem_Error;
		}
		else
		{

			std::queue<HANDLE> handlesQueue;
			handlesQueue.push(hFind);

			while (handlesQueue.size() > 0)
			{
				WIN32_FIND_DATA findData;
				HANDLE handle = handlesQueue.front();
				handlesQueue.pop();

				if (FindNextFile(handle, &findData))
				{
					if (findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
					{
						FeFile& addedFiles = files.Add();
						sprintf_s(addedFiles.Path, findData.cFileName);
					}
					else if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						char szRecursiveSearchPath[COMMON_PATH_SIZE];
						sprintf_s(szRecursiveSearchPath, "%s/*", findData.cFileName);
						hFind = FindFirstFileEx(szRecursiveSearchPath, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
						handlesQueue.push(hFind);
					}
				}
			}

			FE_LOG(TEXT("The first file found is %s\n"), FindFileData.cFileName);
			FindClose(hFind);
		}
		return FeEReturnCode::Success;
	}

	uint32 ListFiles(const char* szPath, FeTArray<FeFile>& files)
	{
		return DoListFiles(szPath, files, false);
	}
	uint32 ListFilesRecursive(const char* szPath, FeTArray<FeFile>& files)
	{
		return DoListFiles(szPath, files, true);
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