#pragma once

#define MAX_HEAP_COUNT 64

#include <common.hpp>
#include <tarray.hpp>
#include <unordered_map>

#include <application.hpp>

namespace FeEFileChangeType
{
	enum Type
	{
		Deleted,
		Created,
		Changed
	};
}
typedef void(*FeFileChangeEvent) (FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData);

class FeModuleFilesManager : public FeModule
{
public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;

	void WatchDirectory(const char* szPath, FeFileChangeEvent onFileChanged, void* pUserData);

private:
	struct FileChangeEventParam
	{
		FeEFileChangeType::Type Type;
		FePath					Path;
	};
	struct WatchedDirectory
	{
		FePath							Path;
		FePath							Filter;
		void*							Handle;
		void*							WatchHandle;
		//FeTArray<FileChangeEventParam>	OccuredEvents;
		FeFileChangeEvent				OnFileChangeEvent;
		void*							FileEventUserData;
	};
	FeTArray<WatchedDirectory> WatchedDirs;
};

namespace FeFileTools
{
	void SetRootDir(const char* szPath);
	const FePath& GetRootDir();

	uint32 ListFiles(FeTArray<FePath>& files, const char* szPath, const char* szFilter = nullptr);
	uint32 ListFilesRecursive(FeTArray<FePath>& files, const char* szPath, const char* szFilter=nullptr);

	uint32 GetFileSize(const char* szPath, size_t* iSize);
	
	uint32 ReadTextFile(const FePath& file, char** ppOutput, size_t* iFileSize);
	uint32 ReadTextFile(const char* szPath, char** ppOutput, size_t* iFileSize);

	uint32 ReadBinaryFile(const FePath& file, void** ppOutput, size_t* iFileSize);
	uint32 ReadBinaryFile(const char* szPath, void** ppOutput, size_t* iFileSize);

	uint32 WriteTextFile(const FePath& file, const char* pInput);
	uint32 WriteTextFile(const char* szPath, const char* pInput);

	void GetFullPathWithoutExtension(FePath&, const FePath&);
	void GetFileNameWithoutExtension(FePath&, const FePath&);
	void GetFileName(FePath&, const FePath&);
	void GetDirectoryName(FePath&, const FePath&);

	void GetFullPathWithoutExtension(FePath&, const char*);
	void GetFileNameWithoutExtension(FePath&, const char*);
	void GetFileName(FePath&, const char*);
	void GetDirectoryName(FePath&, const char*);
	void FormatPath(FePath& path,bool bIsDir=false);

	void GetFullPathChangeExtension(FePath&, const char*, const char*);

	bool FileExists(const FePath&);
};