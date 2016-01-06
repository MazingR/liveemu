#pragma once

#include "fecommon.h"
#include "ferenderer.h"
#include "fecommands.h"

HEADER_TOP

typedef struct feSFrontEndInstance
{
	feSRenderer					m_renderer;
	int							m_iServerListenPort;
	feSCommandServer			m_commandServer;
	void*						m_iWindowHandle;

} feSFrontEndInstance;

feSFrontEndInstance* feFrontEndInstance();
feresult feFrontEndInstanceInit(feSFrontEndInstance* _pInst);
feresult feFrontEndInstanceRelease(feSFrontEndInstance* _pInst);
feresult feFrontEndInstanceParseCmdLine(feSFrontEndInstance* _pInst, int* argc, char** argv);
feresult feFrontEndInstanceParseCmdLineW(feSFrontEndInstance* _pInst, int* argc, wchar_t** argv);
feresult feFrontEndExecuteCommands(feSFrontEndInstance* _pInst);

HEADER_BOTTOM