#pragma once

HEADER_TOP

#if defined(_WIN64)
	#define SOCKET unsigned __int64
#else
	#define SOCKET unsigned int
#endif

typedef struct feTcpServerCallbackParam
{
	SOCKET	m_clientSocket;
	void*	m_pUserParam;
} feTcpServerCallbackParam;

int fenet_readInt(SOCKET sock, int* pOuput);
int fenet_readUint(SOCKET sock, unsigned int* pOuput);
int fenet_readString(SOCKET sock, char** pOuput, unsigned int iMaxSize);

feresult fenet_createSocketListener(int iPort, SOCKET* _pSocket);
feresult fenet_closeSocketListener(SOCKET* _pSocket);

void fenet_closeSocket();
feresult fenet_waitForClientConnection(SOCKET _socket, void* _callback, struct feTcpServerCallbackParam* _pCallbackParam);


HEADER_BOTTOM