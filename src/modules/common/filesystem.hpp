#pragma once

#define MAX_HEAP_COUNT 64

#include <common.hpp>
#include <tarray.hpp>
#include <unordered_map>

struct FeFile
{
	char Path[COMMON_PATH_SIZE];
};

namespace FeFileSystem
{
	uint32 ListFiles(const char* szPath, const char* szFilter, FeTArray<FeFile>& files);
	uint32 ListFilesRecursive(const char* szPath, const char* szFilter, FeTArray<FeFile>& files);
	
	uint32 ReadTextFile(const FeFile& file, char** ppOutput);
	uint32 ReadTextFile(const char* szPath, char** ppOutput);

	uint32 ReadBinaryFile(const FeFile& file, void** ppOutput);
	uint32 ReadBinaryFile(const char* szPath, void** ppOutput);

	void GetFullPathWithoutExtension(FeFile&, const FeFile&);
	void GetFileNameWithoutExtension(FeFile&, const FeFile&);
	void GetFileName(FeFile&, const FeFile&);
	void GetDirectoryName(FeFile&, const FeFile&);

	void GetFullPathWithoutExtension(FeFile&, const char*);
	void GetFileNameWithoutExtension(FeFile&, const char*);
	void GetFileName(FeFile&, const char*);
	void GetDirectoryName(FeFile&, const char*);
	void FormatPath(FeFile&);

	void GetFullPathChangeExtension(FeFile&, const char*, const char*);

	bool FileExists(const FeFile&);
};