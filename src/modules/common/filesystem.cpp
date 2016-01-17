#include <filesystem.hpp>
#include <windows.h>
#include <queue>
#include <regex>
#include "string.hpp"

#include <SDL.h>


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

	for (auto& watchedDir : WatchedDirs)
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
				
				if (iFileChangedCount == 0)
				{
					watchedDir.OnFileChangeEvent(FeEFileChangeType::Changed, "", watchedDir.FileEventUserData);
				}
				else
				{
					for (size_t i = 0; i < iFileChangedCount; ++i)
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
			}

			FindNextChangeNotification(watchedDir.WatchHandle);
		}
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

uint32 sdl_file_write(const char* filename, const char* szContent)
{
	SDL_RWops *rw = SDL_RWFromFile(filename, "w");

	if (rw == NULL)
		return FeEReturnCode::File_OpenFailed;

	if (rw != NULL) {
		const char *str = "Hello World";
		size_t len = SDL_strlen(szContent);
		size_t iWritten = SDL_RWwrite(rw, str, 1, len);

		FE_ASSERT(iWritten == len, "Failed writting to file %s", filename);

		SDL_RWclose(rw);
	}

	return FeEReturnCode::Success;
}
uint32 sdl_file_read(const char* filename, void** outputBuff, size_t* pFileSize) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "r");

	if (rw == NULL)
		return FeEReturnCode::File_OpenFailed;

	size_t res_size = (size_t)SDL_RWsize(rw);
	*outputBuff = (char*)FE_ALLOCATE(res_size + 1, 1);

	size_t nb_read_total = 0, nb_read = 1;
	char* buf = (char*)*outputBuff;

	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	*pFileSize = nb_read_total;

	if (nb_read_total != res_size) {
		free(*outputBuff);
		return FeEReturnCode::File_ReadFailed;
	}

	((char*)*outputBuff)[nb_read_total] = '\0';

	return FeEReturnCode::Success;
}
namespace FeFileTools
{
	FePath RootPath;

	uint32 DoListFiles(const char* szPath, const char* szFilter, FeTArray<FePath>& files, bool bRecusrive)
	{
		std::regex filterRegex(szFilter);

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

			sprintf_s(szTmpPath, "%s%s/%s", GetRootDir().Value, dir.Path, "*");
			hFind = FindFirstFileEx(szTmpPath, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);

			dir.Handle = hFind;

			while (FindNextFile(dir.Handle, &findData))
			{
				const char* szFilePath = findData.cFileName;
					
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					bool bIsVirtual = strcmp(szFilePath, ".") == 0 || strcmp(szFilePath, "..") == 0;

					if (!bIsVirtual && bRecusrive)
					{
						HandledDir& subDir = dirs.Add();
						sprintf_s(subDir.Path, "%s/%s", dir.Path, findData.cFileName);
					}
				}
				else
				{
					bool bSatisfiesFilter = szFilter ? std::regex_match(findData.cFileName, filterRegex) : true;

					if (bSatisfiesFilter)
					{
						FePath& addedFile = files.Add();
						size_t iPathLen = strlen(dir.Path) + strlen(findData.cFileName) + 2;
						FE_ASSERT(iPathLen < COMMON_PATH_SIZE, "file path is too long ! : '%s'", findData.cFileName);
						sprintf_s(addedFile.Value, "%s/%s", dir.Path, findData.cFileName);
						FeFileTools::FormatPath(addedFile);
					}
				}
			}

			FindClose(hFind);
		}
		return FeEReturnCode::Success;
	}

	uint32 ListFiles(FeTArray<FePath>& files, const char* szPath, const char* szFilter)
	{
		return DoListFiles(szPath, szFilter, files, false);
	}
	uint32 ListFilesRecursive(FeTArray<FePath>& files, const char* szPath, const char* szFilter)
	{
		return DoListFiles(szPath, szFilter, files, true);
	}

	uint32 ReadTextFile(const FePath& file, char** ppOutput, size_t* iFileSize)
	{
		return ReadTextFile(file.Value, ppOutput, iFileSize);
	}
	uint32 ReadTextFile(const char* szPath, char** ppOutput, size_t* iFileSize)
	{
		char szTmpPath[1024];
		sprintf_s(szTmpPath, "%s%s", RootPath.Value, szPath);

		return sdl_file_read(szTmpPath, (void**)ppOutput, iFileSize);
	}
	
	uint32 WriteTextFile(const FePath& file, const char* pInput)
	{
		return WriteTextFile(file.Value, pInput);
	}
	uint32 WriteTextFile(const char* szPath, const char* pInput)
	{
		char szTmpPath[1024];
		sprintf_s(szTmpPath, "%s%s", RootPath.Value, szPath);

		return sdl_file_write(szTmpPath, pInput);
	}


	uint32 ReadBinaryFile(const FePath& file, void** ppOutput, size_t* iFileSize)
	{
		return ReadBinaryFile(file.Value, ppOutput, iFileSize);
	}
	uint32 ReadBinaryFile(const char* szPath, void** ppOutput, size_t* iFileSize)
	{
		char szTmpPath[1024];
		sprintf_s(szTmpPath, "%s%s", RootPath.Value, szPath);

		return sdl_file_read(szTmpPath, ppOutput, iFileSize);
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
	void FormatPath(FePath& file, bool bIsDir)
	{
		FeStringTools::Replace(file.Value, '\\', '/');
		

		if (bIsDir)
		{
			FeStringTools::TrimEnd(file.Value, '/');
			strcat_s(file.Value, "/");
		}
	}

	bool FileExists(const FePath& file)
	{
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFileEx(file.Value, FindExInfoStandard, &findData, FindExSearchNameMatch, NULL, 0);
		return hFind != ((HANDLE)-1);
	}
	uint32 GetFileSize(const char* szPath, size_t* iSize)
	{
		SDL_RWops *rw = SDL_RWFromFile(szPath, "r");

		if (rw == NULL)
			return FeEReturnCode::File_OpenFailed;

		*iSize = (size_t)SDL_RWsize(rw);

		SDL_RWclose(rw);

		return FeEReturnCode::Success;
	}

	void SetRootDir(const char* szPath)
	{
		RootPath.Set(szPath);
		FormatPath(RootPath, true);
	}
	const FePath& GetRootDir()
	{
		return RootPath;
	}
};