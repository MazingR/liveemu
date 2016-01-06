#pragma once

#include "fenetwork.h"

HEADER_TOP

#define I_MAX_CMD_PARAM_COUNT 16

typedef enum feEComand
{
	FECommandGameSession_Reset,
	FECommandGameSession_Pause,
	FECommandGameSession_Play,
	FECommandGameSession_Stop,
	FECommandGameSession_Quicksave,
	FECommandGameSession_Quickload,
	FECommandGameSession_AutoSave,
	FECommandGameSession_AutoLoad,

	FECommandGameSession_SetVolume,
	FECommandGameSession_SetInputBindings,

	FECommandUnknown
} feEComand;

typedef enum feECommandParamType
{
	CmdParam_Unknown=0,
	CmdParam_Integer,
	CmdParam_String
} feECommandParamType;
typedef struct feSCommandParam
{
	union
	{
		int iValue;
		char* szValue;
	};

	feECommandParamType type;
} feSCommandParam;
typedef struct feSCommand
{
	feEComand			m_eCmdType;
	int					m_iParamsCount;
	feSCommandParam		m_pParams[I_MAX_CMD_PARAM_COUNT];

} feSCommand;

typedef feresult(*feOnExecuteCommand)(const feSCommand* _pCmd);

typedef void(*feCmdCallback)();

typedef void(*feCmdCallback_1str)(const char* szParam1);
typedef void(*feCmdCallback_1str_2str)(const char* szParam1, const char* szParam2);

typedef void(*feCmdCallback_1int)(int iParam1);

typedef struct feSCommandListener
{
	feOnExecuteCommand	m_onExecuteCmd;

} feSCommandListener;

typedef struct feSCommandServer
{
	int							m_bKeepRunnning;
	feSCommandListener*			m_pListener;
	feTcpServerCallbackParam	m_threadCallbackParam;
	//std::mutex				m_stackMutex;
	feStack						m_commandsStack;

	int							m_iPort;
	SOCKET						m_iSocket;
} feSCommandServer;

feresult feCommandServerInit(feSCommandServer* _pServer);
feresult feCommandServerRelease(feSCommandServer* _pServer);
feresult feCommandServerStart(feSCommandServer* _pServer, int iPort);
feresult feCommandServerStop(feSCommandServer* _pServer);

void feCommandServerPushCommand(struct feSCommandServer* _pServer, struct feSCommand* pCmd);
feresult feCommandServerExecuteStack(struct feSCommandServer* _pServer);


feresult feCommandInit(struct feSCommand* _cmd);
feresult feCommandInitEx(struct feSCommand* _cmd, int iParamsCount, feEComand eCmdTyp);
feresult feCommandRelease(struct feSCommand* _cmd);

//feresult feCommandGetParamInt(const struct feSCommand* _cmd, int*iParam, int iParamIdx);
//feresult feCommandGetParamString(const struct feSCommand* _cmd, char**szParam, int iParamIdx);

HEADER_BOTTOM