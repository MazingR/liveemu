
#include "fepch.h"
#include "fecommands.h"
#include <pthread.h>

#define I_MAX_STRING_SIZE		1024*5

#define IF_ENUM_PARSE(sValue, eValue, eTo)\
if (strcmp(#eValue, sValue) == 0) \
{ eTo = FECommand##eValue; } \

static pthread_mutex_t g_cmdsStackMutex = PTHREAD_MUTEX_INITIALIZER;
static int g_bStartedServer = false;
static int g_bStopServer = false;
static int g_bStoppedServer = false;

#define SAFE_CALLBACK(a,...) if (a) a(__VA_ARGS__);

feresult feCommandInit(struct feSCommand* _cmd)
{
	_cmd->m_eCmdType = FECommandUnknown;
	_cmd->m_iParamsCount = 0;

	memset(_cmd->m_pParams, 0, I_MAX_CMD_PARAM_COUNT*sizeof(feSCommandParam));

	return FE_RESULT_OK;
}
feresult feCommandInitEx(struct feSCommand* _cmd, int iParamsCount, feEComand eCmdTyp)
{
	_cmd->m_eCmdType = eCmdTyp;
	_cmd->m_iParamsCount = iParamsCount;
	memset(_cmd->m_pParams, 0, I_MAX_CMD_PARAM_COUNT*sizeof(feSCommandParam));

	return FE_RESULT_OK;
}
feresult feCommandRelease(struct feSCommand* _cmd)
{
	for (int i = 0; i < _cmd->m_iParamsCount; ++i)
	{
		if (_cmd->m_pParams[i].type == CmdParam_String)
		{
			FE_MEM_FREE(_cmd->m_pParams[i].szValue);
		}
	}
	_cmd->m_iParamsCount = 0;

	return FE_RESULT_OK;
}
feEComand feEComandParse(const char* _szValue)
{
	feEComand eValue = FECommandUnknown;

			IF_ENUM_PARSE(_szValue, GameSession_Reset,		eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_Pause,		eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_Play,		eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_Stop,		eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_Quicksave,	eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_Quickload,	eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_AutoSave,	eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_AutoLoad,	eValue)
	else	IF_ENUM_PARSE(_szValue, GameSession_SetInputBindings, eValue)
	else
	{
		char szMsg[128] = "Eror : unknown command";
		MessageBox(NULL, L"Error", _szValue, 0);
	}

	return eValue;
}
feresult feCommandServerInit(feSCommandServer* _pServer)
{
	_pServer->m_bKeepRunnning = false;
	_pServer->m_pListener = 0;
	
	feStackInit(&_pServer->m_commandsStack, sizeof(feSCommand));

	return FE_RESULT_OK;
}
feresult feCommandClientFetchParamInt(SOCKET sock, char* szReadBuffer, feSCommand* pCmd, int iParamIdx)
{
	int iValue = 0;
	fenet_readInt(sock, &iValue);

	pCmd->m_pParams[iParamIdx].type = CmdParam_Integer;
	pCmd->m_pParams[iParamIdx].iValue = iValue;
}

feresult feCommandClientFetchParamString(SOCKET sock, char* szReadBuffer, feSCommand* pCmd, int iParamIdx)
{
	int iParamSize = fenet_readString(sock, &szReadBuffer, I_MAX_STRING_SIZE);

	if (iParamSize != SOCKET_ERROR)
	{
		char* szFilePath = FE_MEM_NEW_ARRAY_TYPE(char, iParamSize + 1);

		memcpy(szFilePath, szReadBuffer, iParamSize + 1);

		pCmd->m_pParams[iParamIdx].type = CmdParam_String;
		pCmd->m_pParams[iParamIdx].szValue = szFilePath;
		
	}
}

DWORD WINAPI feCommandClientThreadCallback(LPVOID lpParam)
{
	char*						szBuff = FE_MEM_NEW_ARRAY_TYPE(char, I_MAX_STRING_SIZE);
	feTcpServerCallbackParam*	callbackParams = (feTcpServerCallbackParam*)lpParam;
	SOCKET						sock = callbackParams->m_clientSocket;
	feSCommandServer*			pCmdServer = (feSCommandServer*)callbackParams->m_pUserParam;

	g_bStartedServer = true;

	while (!g_bStopServer)
	{
		if (fenet_readString(sock, &szBuff, I_MAX_STRING_SIZE) != SOCKET_ERROR)
		{
			// Process keep alive
			if (strlen(szBuff) == 5 && strcmp(szBuff, "alive") == 0)
				continue;

			// compute message type enum value
			feEComand eCmd = feEComandParse(szBuff);

			FE_PRINT("Command %s", szBuff);
			
			if (eCmd != FECommandUnknown)
			{
				feSCommand cmd;

				feCommandInitEx(&cmd, 1, eCmd);

				switch (eCmd)
				{
				//case FECommandGameSession_Reset:
				//case FECommandGameSession_Pause:
				//case FECommandGameSession_Play:
				//case FECommandGameSession_Stop:

				case FECommandGameSession_Quicksave:
				case FECommandGameSession_Quickload:
				case FECommandGameSession_AutoSave:
				case FECommandGameSession_AutoLoad:
				{
					cmd.m_iParamsCount= 1;
					feCommandClientFetchParamString(sock, szBuff, &cmd, 0); // file path
				} break;

				case FECommandGameSession_SetInputBindings:
				{
					cmd.m_iParamsCount= 2;
					feCommandClientFetchParamInt(sock, szBuff, &cmd, 0); // player index
					feCommandClientFetchParamString(sock, szBuff, &cmd, 1); // player config json
				} break;

				default:
					break;
				}
				
				feStackPush(&pCmdServer->m_commandsStack, &cmd);
			}
		}
	}

	FE_MEM_FREE(szBuff);

	g_bStoppedServer = true;
	return 0;
}
feresult feCommandServerStop(feSCommandServer* _pServer)
{
	feresult iRes = FE_RESULT_OK;
	g_bStopServer = true;

	fenet_closeSocket(_pServer->m_iSocket);

	if (g_bStartedServer)
	{
		while (!g_bStoppedServer)
		{
			Sleep(100);
		}
	}
	return iRes;
}
feresult feCommandServerStart(feSCommandServer* _pServer, int iPort)
{
	feresult iRes = FE_RESULT_OK;
	FE_PRINT("starting server...")

	_pServer->m_bKeepRunnning = true;
	_pServer->m_threadCallbackParam.m_pUserParam = _pServer;
	
	iRes = fenet_createSocketListener(iPort, &_pServer->m_iSocket);
	
	
	if (FE_RESULT_ISOK(iRes))
	{
		FE_PRINT("\t\twaiting for client...");
		iRes = fenet_waitForClientConnection(_pServer->m_iSocket, feCommandClientThreadCallback, &_pServer->m_threadCallbackParam);
	}
	return iRes;
}
void feCommandServerPushCommand(struct feSCommandServer* _pServer, struct feSCommand* pCmd)
{
	pthread_mutex_lock(&g_cmdsStackMutex);
	feStackPush(&_pServer->m_commandsStack, pCmd);
	pthread_mutex_unlock(&g_cmdsStackMutex);
}
feresult feCommandServerExecuteStack(struct feSCommandServer* _pServer)
{
	feresult iResult = FE_RESULT_OK;

	if (_pServer)
	{
		feStack tmpCmdsStack;
		feStackInit(&tmpCmdsStack, sizeof(feSCommand));
		

		// recover cmds stack content
		pthread_mutex_lock(&g_cmdsStackMutex);
		while (!feStackIsEmpty(&_pServer->m_commandsStack))
		{
			feSCommand tmpCmd;

			feStackPop(&_pServer->m_commandsStack, &tmpCmd);
			feStackPush(&tmpCmdsStack, &tmpCmd);
		}
		pthread_mutex_unlock(&g_cmdsStackMutex);
		

		while (!feStackIsEmpty(&tmpCmdsStack))
		{
			feSCommand cmd;
			feStackPop(&tmpCmdsStack, &cmd);
			
			if (_pServer->m_pListener)
			{
				SAFE_CALLBACK(_pServer->m_pListener->m_onExecuteCmd, &cmd);
			}

			feCommandRelease(&cmd);
		}

		feStackRelease(&tmpCmdsStack);
	}

	return iResult;
}

//feresult feCommandGetParamInt(const struct feSCommand* _cmd, int*iParam, int iParamIdx)
//{
//	if (_cmd->m_iParamsCount > iParamIdx)
//	{
//		*iParam = atoi(_cmd->m_pParams[iParamIdx]);
//		return FE_RESULT_OK;
//	}
//
//
//	return FE_RESULT_ERROR;
//}
//feresult feCommandGetParamString(const struct feSCommand* _cmd, char**szParam, int iParamIdx)
//{
//	if (_cmd->m_iParamsCount > iParamIdx)
//	{
//		*szParam = _cmd->m_pParams[iParamIdx];
//		return FE_RESULT_OK;
//	}
//	return FE_RESULT_ERROR; 
//}