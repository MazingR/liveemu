// ------------------------------------------------------------------------------------------------------------------------
//#include "fecommon.h"

#include "fepch.h"
#include "fenetwork.h"

// ------------------------------------------------------------------------------------------------------------------------
#define DEFAULT_PORT			5150
#define TEST_SOCKET_READ_RETURN(iValue) 										\
{ 																				\
	int iTestValue = iValue; 													\
	if (iTestValue == 0 || iTestValue == SOCKET_ERROR) 							\
	{																			\
		FE_PRINT("Read() failed: %d", WSAGetLastError()); return SOCKET_ERROR;	\
	} 																			\
}

// ------------------------------------------------------------------------------------------------------------------------
typedef unsigned char uchar;
 
int fenet_readInt(SOCKET sock, int* pOuput)
{
	*pOuput = 0;
	char szBuff[4];
	ZeroMemory(szBuff, 4);

	TEST_SOCKET_READ_RETURN(recv(sock, szBuff, 4, 0));
	*pOuput = (int)
		((uchar)szBuff[0] << 0) +
		((uchar)szBuff[1] << 8) +
		((uchar)szBuff[2] << 16) +
		((uchar)szBuff[3] << 24);

	return 1;
}
int fenet_readUint(SOCKET sock, unsigned int* pOuput)
{
	*pOuput = 0;
	char szBuff[4];
	ZeroMemory(szBuff, 4);

	TEST_SOCKET_READ_RETURN(recv(sock, szBuff, 4, 0));
	*pOuput = (unsigned int)
		((uchar)szBuff[0] << 0) +
		((uchar)szBuff[1] << 8) +
		((uchar)szBuff[2] << 16) +
		((uchar)szBuff[3] << 24);

	return 1;
}

int fenet_readString(SOCKET sock, char** pOuput, unsigned int iMaxSize)
{
	unsigned int iStringLength = -1;

	TEST_SOCKET_READ_RETURN(fenet_readUint(sock, &iStringLength));

	if (iMaxSize + 1 < iStringLength || iStringLength <= 0)
		return SOCKET_ERROR;

	//TEST_SOCKET_READ_RETURN( recv(sock, *pOuput, iStringLength, 0) );
	unsigned int iReadBytes = 0;
	do
	{
		iReadBytes = recv(sock, *pOuput, iStringLength, 0);

		if (iReadBytes == SOCKET_ERROR)
			break;

	} while (iReadBytes < iStringLength);

	if (iReadBytes < iStringLength)
		return (unsigned int)SOCKET_ERROR;

	(*pOuput)[iStringLength] = '\0';

	return iStringLength;
}

feresult fenet_closeSocketListener(SOCKET* _pSocket)
{
	closesocket(_pSocket);
}
feresult fenet_createSocketListener(int iPort, SOCKET* _pSocket)
{
	struct sockaddr_in	local;

	WSADATA       wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		FE_PRINT_ERROR("Failed to load Winsock library!")
		return FE_ERROR_NETSOCKET;
	}

	*_pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (*_pSocket == SOCKET_ERROR)
	{
		FE_PRINT_ERROR("\tsocket() failed: %d", WSAGetLastError())

		return FE_ERROR_NETSOCKET;
	}
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_family = AF_INET;
	local.sin_port = htons(iPort);

	if (bind(*_pSocket, (struct sockaddr *)&local,
		sizeof(local)) == SOCKET_ERROR)
	{
		FE_PRINT_ERROR("bind() failed: %d", WSAGetLastError())
		return FE_ERROR_NETSOCKET;
	}
	listen(*_pSocket, 8);

	return FE_RESULT_OK;
}

feresult fenet_waitForClientConnection(SOCKET _socket, _In_ void* _callback, struct feTcpServerCallbackParam* _pCallbackParam)
{
	int					iAddrSize;
	struct sockaddr_in	client;

	HANDLE				hThread;
	DWORD				dwThreadId;

	iAddrSize = sizeof(client);
	_pCallbackParam->m_clientSocket = accept(_socket, (struct sockaddr *)&client, &iAddrSize);

	if (_pCallbackParam->m_clientSocket == INVALID_SOCKET)
	{
		FE_PRINT_ERROR("accept() failed: %d", WSAGetLastError());
		return FE_ERROR_NETSOCKETACCEPT;
	}
	
	FE_PRINT("accepted client: %s:%d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_callback, (LPVOID)_pCallbackParam, 0, &dwThreadId);

	if (hThread == NULL)
	{
		FE_PRINT_ERROR("createThread() failed: %d", GetLastError());
		return FE_ERROR_NETTHREAD;
	}
	CloseHandle(hThread);

	return FE_RESULT_OK;
}
void fenet_closeSocket(SOCKET _socket)
{
	closesocket(_socket);
	WSACleanup();
}