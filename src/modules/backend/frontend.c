
#include "fepch.h"
#include "frontend.h"

feSFrontEndInstance* g_pFrontEndInstance = 0;

typedef enum feECmdLineLabel
{
	FECmdLineLabel_ScreenWidth,
	FECmdLineLabel_ScreenHeight,
	FECmdLineLabel_SharedRenderTarget,
	FECmdLineLabel_WindowHwnd,
	FECmdLineLabel_CommandsServerPort,

	FECmdLineLabel_Count,
	FECmdLineLabel_Unknown
} feEComand;

char sz_CmdLineLabels[FECmdLineLabel_Count][64] =
{
	"-ScreenWidth",
	"-ScreenHeight",
	"-SharedRenderTarget",
	"-WindowHwnd",
	"-CommandsServerPort",
};
int parseCmdLineLabel(char* szLabel)
{
	int iRes = -1;

	for (int i = 0; i < FECmdLineLabel_Count; ++i)
	{
		if (strcmp(sz_CmdLineLabels[i], szLabel) == 0)
		{
			iRes = i;
			break;
		}
	}
	return iRes;
}

feSFrontEndInstance* feFrontEndInstance()
{
	return g_pFrontEndInstance;
}
feresult feFrontEndInstanceInit(feSFrontEndInstance* _pInst)
{
	feRendererInit(&_pInst->m_renderer);
	feCommandServerInit(&_pInst->m_commandServer);

	_pInst->m_iServerListenPort = 0;
	_pInst->m_iWindowHandle		= 0;

	g_pFrontEndInstance = _pInst;

	return FE_RESULT_OK;
}
feresult feFrontEndInstanceRelease(feSFrontEndInstance* _pInst)
{
	// TODO : release

	return FE_RESULT_OK;
}
feresult feFrontEndInstanceParseCmdLineW(feSFrontEndInstance* _pInst, int* argc, wchar_t** argv)
{
	// Convert strings from widechar to char
	int iArgc = *argc;

	char **szArgv = malloc(sizeof(char*)*iArgc);
	const iMaxCharLen = 128;

	for (int i = 0; i < *argc; ++i)
	{
		szArgv[i] = malloc(iMaxCharLen);

		wchar_t* szWide = argv[i];
		char * szChar = szArgv[i];

		int iLen = lstrlenW(szWide);
		
		int iCharIdx = 0;

		for (int j = 0; j < iLen; ++j)
		{
			int iCharCount = 0;
			wctomb_s(&iCharCount, szChar + iCharIdx, iMaxCharLen - iCharIdx, szWide[j]);
			iCharIdx += iCharCount;

		}

		szChar[iCharIdx] = '\0';
	}

	feresult res = feFrontEndInstanceParseCmdLine(_pInst, argc, szArgv);

	// free 
	for (int i = 0; i < iArgc; ++i)
		free(szArgv[i]);
	free(szArgv);

	return res;
}
feresult feFrontEndInstanceParseCmdLine(feSFrontEndInstance* _pInst, int* argc, char** argv)
{
	FE_PRINT("Initialize command line...");

	int iParamsFound = 0;

	for (int i = 0; i < *argc; ++i)
	{
		char* szParamLabel = argv[i];
		char* szParamValue = NULL;

		int iParam = parseCmdLineLabel(szParamLabel);

		switch (iParam)
		{
			case FECmdLineLabel_ScreenWidth:
			{
				_pInst->m_renderer.m_iSharedTargetWidth = atol(argv[++i]);
				FE_PRINT("\t\tScreenWidth : %d", _pInst->m_renderer.m_iSharedTargetWidth);
				iParamsFound+=2;
			} break;
			case FECmdLineLabel_ScreenHeight:
			{
				_pInst->m_renderer.m_iSharedTargetHeight = atol(argv[++i]);
				FE_PRINT("\t\tScreenHeight : %d", _pInst->m_renderer.m_iSharedTargetHeight);
				iParamsFound += 2;
			} break;
			case FECmdLineLabel_SharedRenderTarget:
			{
				_pInst->m_renderer.m_iSharedTargetHandle = (HANDLE)_atoi64(argv[++i]);
				iParamsFound += 2;
			} break;
			case FECmdLineLabel_WindowHwnd:
			{
				_pInst->m_iWindowHandle = (HWND)_atoi64(argv[++i]);
				iParamsFound += 2;
			} break;
			case FECmdLineLabel_CommandsServerPort:
			{
				_pInst->m_iServerListenPort = atol(argv[++i]);
				FE_PRINT("\t\tCommandsServerPort : %d", _pInst->m_iServerListenPort);
				iParamsFound += 2;
			} break;
			
		default:
			break;
		}
	}

	*argc = (*argc) - iParamsFound;

	return FE_RESULT_OK;
}
feresult feFrontEndExecuteCommands(feSFrontEndInstance* _pInst)
{
	return feCommandServerExecuteStack(&_pInst->m_commandServer);
}
