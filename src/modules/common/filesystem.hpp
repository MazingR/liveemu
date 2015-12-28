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
};