#pragma once

typedef unsigned int feresult;
typedef unsigned int uint;
typedef char bool;

#ifdef __cplusplus
	#define HEADER_TOP extern "C" {
	#define HEADER_BOTTOM }
#else
	#define HEADER_TOP
	#define HEADER_BOTTOM
#endif

#define true 1
#define false 0

#define FE_RESULT_OK 0
#define FE_RESULT_ERROR 1

#define FE_RESULT_ISERROR(_X_) (_X_!=FE_RESULT_OK)
#define FE_RESULT_ISOK(_X_) (_X_==FE_RESULT_OK)

#define FE_ERROR_NETSOCKET				10
#define FE_ERROR_NETSOCKETACCEPT		11
#define FE_ERROR_NETTHREAD				12

#define FE_ERROR_INPUT				20

#define FE_ERROR_COMMAND			30

#define FE_PRINT(a,...) { printf("[frontend] "); printf(a, ##__VA_ARGS__); printf("\n"); }
#define FE_PRINT_ERROR(a,...) { printf("[frontend] [error] "); printf(a, ##__VA_ARGS__); printf("\n"); }

#define FE_MEM_NEW(size) malloc(sizeof(size));
#define FE_MEM_NEW_ARRAY_TYPE(type,count) malloc(sizeof(type)*count);
#define FE_MEM_NEW_ARRAY(size,count) malloc(size*count);
#define FE_MEM_FREE(ptr) {if (ptr!=0) free(ptr); }

HEADER_TOP

// ****************************************************************************************************************
#define FE_STACK_DEFAULT_SIZE 16
typedef struct feStack
{
	size_t	m_iStride;
	size_t	m_iCursor;
	size_t	m_iSize;
	void*	m_pData;

} feStack;

void feStackRelease(struct feStack* _pStack);
int feStackIsEmpty(const struct feStack* _pStack);
void feStackInit(struct feStack* _pStack, size_t iStride);

void feStackTop(struct feStack* _pStack, void* pOutput);
void feStackPop(struct feStack* _pStack, void* pOutput);
void feStackPush(struct feStack* _pStack, const void* pDataEntry);

HEADER_BOTTOM