#include <pch.hpp>

uint32 file_read(const char* filename, void** outputBuff) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "r");
	
	if (rw == NULL) 
		return EFeReturnCode::File_OpenFailed;

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
	if (nb_read_total != res_size) {
		free(*outputBuff);
		return EFeReturnCode::File_ReadFailed;
	}

	((char*)*outputBuff)[nb_read_total] = '\0';

	return EFeReturnCode::Success;
}

struct TestValue
{
	char Name[32];
	TestValue()
	{
		FE_LOG("Created !!");
	}
	~TestValue()
	{
		FE_LOG("Destroyed !!");
	}
};

#define PERF_RESET(time) time = std::clock();

#define PERF_PRINT_SET(time, msg) {\
	double ___duration = (std::clock() - time) / (double)CLOCKS_PER_SEC; \
	FE_LOG("\tPerf (%s) = %4.8f (s)", msg, ___duration); \
	PERF_RESET(time) }

#include <map>

void test1()
{
	srand(time(NULL));
	std::clock_t time;
	
	uint32 iValuesCount = 2 << 22;
	FE_LOG("Unit test map with %d elements", iValuesCount);

	PERF_RESET(time);
	
	uint32* testKeys = new uint32[iValuesCount];
	TestValue* testValues = new TestValue[iValuesCount];

	for (uint32 i = 0; i < iValuesCount; ++i)
	{
		uint32 iKey = rand() % 1000;

		testKeys[i] = iKey;
		sprintf_s(testValues[i].Name, "value_%d", iKey);
	}
	uint32 iIdxOfFind = 2 << 16;
	uint32 iFindTimes = 2 << 16;

	uint32 iKeyToFind = testKeys[iIdxOfFind];
	TestValue* valueToFind = &testValues[iIdxOfFind];
	
	PERF_PRINT_SET(time, "Generate data");

	{
		FeCommon::FeTMap<uint32, TestValue*> map;

		map.Reserve(iValuesCount);
		for (uint32 i = 0; i < iValuesCount; ++i)
		{
			map.AddNoSort(testKeys[i], &testValues[i]);
		}
		
		FE_LOG("FeTMap");
		PERF_PRINT_SET(time, "inject data");
		uint32 iFoundIdx = 0;
		for (uint32 i = 0; i<iFindTimes; ++i)
			iFoundIdx = map.Find(iKeyToFind);
		PERF_PRINT_SET(time, "find");
		FE_LOG("\tFound value '%s' ?= %s ", valueToFind->Name, map.GetValueAt(iFoundIdx)->Name);
	}
	{
		std::map<uint32, TestValue*> map;
		
		for (uint32 i = 0; i < iValuesCount; ++i)
		{
			map[testKeys[i]] = &testValues[i];
		}
		
		FE_LOG("StdMap");

		PERF_PRINT_SET(time, "inject data");
		std::map<uint32, TestValue*>::iterator it;
			
		for (uint32 i = 0; i < iFindTimes; ++i)
			it = map.find(iKeyToFind);
		PERF_PRINT_SET(time, "find");
		FE_LOG("\tFound value '%s' ?= %s ", valueToFind->Name, it->second->Name);
	}
	
}


void unitTest()
{
	//TestValue* testV =FE_NEW_ARRAYD(TestValue,3);
	//FE_DELETE_ARRAYD(TestValue, testV, 3);

	//const int iCount = 50;

	////std::string szPaths[4096];
	//char* szBasePath = SDL_GetBasePath();

	//void* ptrs[iCount];

	//for (int i = 0; i < iCount; ++i)
	//{
		void* ptr;
		std::string szPath = "../data/image.jpg";
		file_read(szPath.c_str(), &ptr);
	//}

}
