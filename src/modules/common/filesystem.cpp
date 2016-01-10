#include <filesystem.hpp>
#include <windows.h>
#include <queue>
#include "string.hpp"


#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[FileSystem] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[FileSystem] "fmt, __VA_ARGS__) 
#define FILE_CHANGE_FLAGS				\
	FILE_NOTIFY_CHANGE_FILE_NAME |		\
	FILE_NOTIFY_CHANGE_DIR_NAME |		\
	FILE_NOTIFY_CHANGE_ATTRIBUTES |		\
	FILE_NOTIFY_CHANGE_SIZE |			\
	FILE_NOTIFY_CHANGE_LAST_WRITE |		\
	FILE_NOTIFY_CHANGE_LAST_ACCESS |	\
	FILE_NOTIFY_CHANGE_CREATION |		\
	FILE_NOTIFY_CHANGE_SECURITY

void FePath::Set(const char* str)
{
	sprintf_s(Value, str);
	FeFileTools::FormatPath(*this);
}

uint32 FeModuleFilesManager::Load(const FeModuleInit*)
{
	return FeEReturnCode::Success;
}
uint32 FeModuleFilesManager::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleFilesManager::Update(const FeDt& fDt)
{
	const size_t iMaxFileChange = 512;

	const size_t bufLen = sizeof(FILE_NOTIFY_INFORMATION) * iMaxFileChange;
	static FILE_NOTIFY_INFORMATION buffer[iMaxFileChange];
	
	DWORD bytesRet = 0;

	for (auto watchedDir : WatchedDirs)
	{

		DWORD dwWaitStatus = WaitForMultipleObjects(1, &watchedDir.WatchHandle, TRUE, 0);
		if (dwWaitStatus == WAIT_OBJECT_0)
		{
			if (ReadDirectoryChangesW(watchedDir.WatchHandle,
				buffer,
				bufLen,
				true,
				FILE_CHANGE_FLAGS,
				&bytesRet,
				NULL,
				NULL))
			{
				size_t iFileChangedCount = bytesRet / sizeof(FILE_NOTIFY_INFORMATION);
				for (size_t i = 0; i < iFileChangedCount-1; ++i)
				{
					FePath changedPath;
					size_t iConvertedCount;
					char szFileName[128];

					wcstombs_s(&iConvertedCount, szFileName, buffer[i].FileName, 128);
					sprintf_s(changedPath.Value, COMMON_PATH_SIZE, "%s/%s", watchedDir.Path.Value, szFileName);
					FeFileTools::FormatPath(changedPath);

					watchedDir.OnFileChangeEvent(FeEFileChangeType::Changed, changedPath.Value, watchedDir.FileEventUserData);
				}
			}

			FindNextChangeNotification(watchedDir.WatchHandle);
		}



		//DWORD  dwWaitStatus = WaitForSingleObject(watchedDir.WatchHandle, 0);

		//if (dwWaitStatus == WAIT_OBJECT_0)
		//{
		//	FILE_NOTIFY_INFORMATION notifyInfos;
		//	ZeroMemory(&notifyInfos, sizeof(FILE_NOTIFY_INFORMATION));

		//	DWORD iBytesReturned;
		//	BOOL bResult = ReadDirectoryChangesW(watchedDir.Handle, &notifyInfos, sizeof(notifyInfos), false, FILE_CHANGE_FLAGS, &iBytesReturned, NULL, NULL);
		//	
		//	if (bResult)
		//	{
		//		FePath changedPath;
		//		size_t iConvertedCount;
		//		wcstombs_s(&iConvertedCount, changedPath.Value, notifyInfos.FileName, COMMON_PATH_SIZE);
		//		watchedDir.OnFileChangeEvent(FeEFileChangeType::Changed, changedPath.Value, watchedDir.FileEventUserData);

		//		FindNextChangeNotification(watchedDir.WatchHandle);
		//	}
		//	else
		//	{
		//		watchedDir.OnFileChangeEvent(FeEFileChangeType::Changed, "", watchedDir.FileEventUserData);
		//	}
		//}
	}
	return FeEReturnCode::Success;
}

void FeModuleFilesManager::WatchDirectory(const char* szPath, FeFileChangeEvent onFileChanged, void* pUserData)
{
	WatchedDirectory& watchedDir = WatchedDirs.Add();
	watchedDir.OnFileChangeEvent = onFileChanged;
	watchedDir.FileEventUserData = pUserData;
	watchedDir.Path.Set(szPath);
	
	watchedDir.Handle = CreateFile(
		szPath, // pointer to the directory containing the tex files
		FILE_LIST_DIRECTORY | GENERIC_READ,                // access (read-write) mode
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,  // share mode
		NULL, // security descriptor
		OPEN_EXISTING, // how to create
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, // file attributes
		NULL); // file with attributes to copy

	watchedDir.WatchHandle = FindFirstChangeNotification(watchedDir.Path.Value, TRUE, FILE_CHANGE_FLAGS);

	if (watchedDir.Handle == INVALID_HANDLE_VALUE)
	{
		FE_ASSERT(false,  "WatchDirectory function failed [%d].\n", GetLastError());
		WatchedDirs.PopBack(); // remove the element we just added
		return;
	}
}

namespace FeFileTools
{
	uint32 DoListFiles(const char* szPath, const char* szFilter, FeTArray<FePath>& files, bool bRecusrive)
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
					FePath& addedFile = files.Add();
					size_t iPathLen = strlen(dir.Path) + strlen(findData.cFileName) + 2;
					FE_ASSERT(iPathLen < COMMON_PATH_SIZE, "file path is too long ! : '%s'", findData.cFileName);
					sprintf_s(addedFile.Value, "%s/%s", dir.Path, findData.cFileName);
					FeFileTools::FormatPath(addedFile);
				}
			}

			FindClose(hFind);
		}
		return FeEReturnCode::Success;
	}

	uint32 ListFiles(const char* szPath, const char* szFilter, FeTArray<FePath>& files)
	{
		return DoListFiles(szPath, szFilter, files, false);
	}
	uint32 ListFilesRecursive(const char* szPath, const char* szFilter, FeTArray<FePath>& files)
	{
		return DoListFiles(szPath, szFilter, files, true);
	}

	uint32 ReadTextFile(const FePath& file, char** ppOutput)
	{
		return FeEReturnCode::Success;
	}
	uint32 ReadTextFile(const char* szPath, char** ppOutput)
	{
		return FeEReturnCode::Success;
	}

	uint32 ReadBinaryFile(const FePath& file, void** ppOutput)
	{
		return FeEReturnCode::Success;
	}
	uint32 ReadBinaryFile(const char* szPath, void** ppOutput)
	{
		return FeEReturnCode::Success;
	}

	void GetFullPathWithoutExtension(FePath& output, const FePath& input)
	{
		GetFullPathWithoutExtension(output, input.Value);
	}
	void GetFileNameWithoutExtension(FePath& output, const FePath& input)
	{
		GetFileNameWithoutExtension(output, input.Value);
	}
	void GetFileName(FePath& output, const FePath& input)
	{
		GetFileName(output, input.Value);
	}
	void GetDirectoryName(FePath& output, const FePath& input)
	{
		GetDirectoryName(output, input.Value);
	}

	void GetFullPathChangeExtension(FePath& output, const char* input, const char* szExt)
	{
		GetFullPathWithoutExtension(output, input);
		strcat_s(output.Value, ".");
		strcat_s(output.Value, szExt);
	}
	void GetFullPathWithoutExtension(FePath& output, const char* input)
	{
		size_t iPoint = FeStringTools::LastIndexOf(input, '.');
		sprintf_s(output.Value, input);
		output.Value[iPoint] = 0;
	}
	void GetFileNameWithoutExtension(FePath& output, const char* input)
	{
		FE_ASSERT(false, "todo");

		size_t iLastSlash = FeStringTools::LastIndexOf(input, '/');
		size_t iLastBSlash = FeStringTools::LastIndexOf(input, '\\');
		iLastSlash = iLastSlash > iLastBSlash ? iLastSlash : iLastBSlash;

		sprintf_s(output.Value, strlen(input) - iLastSlash, input + iLastSlash);
	}
	void GetFileName(FePath& output, const char* input)
	{
		FE_ASSERT(false, "todo");
	}
	void GetDirectoryName(FePath& output, const char* input)
	{
		FE_ASSERT(false, "todo");
	}
	void FormatPath(FePath& file)
	{
		FeStringTools::Replace(file.Value, '\\', '/');
	}

	bool FileExists(const FePath& file)
	{
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFileEx(file.Value, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);
		return hFind != ((HANDLE)-1);
	}
};