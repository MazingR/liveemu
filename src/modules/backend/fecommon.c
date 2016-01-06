#include "fepch.h"

void feStackRelease(struct feStack* _pStack)
{
	FE_MEM_FREE(_pStack->m_pData);
	memset(_pStack, 0, sizeof(feStack));
}
int feStackIsEmpty(const struct feStack* _pStack)
{
	return _pStack->m_iCursor == 0;
}
void feStackInit(struct feStack* _pStack, size_t iStride)
{
	memset(_pStack, 0, sizeof(feStack));

	_pStack->m_iStride = iStride;
	_pStack->m_iSize = FE_STACK_DEFAULT_SIZE;
	_pStack->m_pData = FE_MEM_NEW_ARRAY(_pStack->m_iStride, _pStack->m_iSize);
}
void feStackTop(struct feStack* _pStack, void* pOutput)
{
	size_t iCursor = _pStack->m_iCursor - _pStack->m_iStride;

	if (_pStack->m_iCursor < _pStack->m_iStride)
		return;

	memcpy(pOutput, (char*)_pStack->m_pData + iCursor, _pStack->m_iStride);
}
void feStackPop(struct feStack* _pStack, void* pOutput)
{
	if (_pStack->m_iCursor < _pStack->m_iStride)
		return;

	_pStack->m_iCursor -= _pStack->m_iStride;

	memcpy(pOutput, (char*)_pStack->m_pData + _pStack->m_iCursor, _pStack->m_iStride);
}
void feStackPush(struct feStack* _pStack, const void* pDataEntry)
{
	// realloc data buffer if cursor is out of range
	if (_pStack->m_iCursor >= (_pStack->m_iStride*_pStack->m_iSize))
	{
		size_t iNewSize = _pStack->m_iSize + FE_STACK_DEFAULT_SIZE;
		size_t iOldSize = _pStack->m_iStride*_pStack->m_iSize;

		void* pData = malloc(_pStack->m_iStride*iNewSize);
		void* pPreviousData = _pStack->m_pData;

		memcpy(pData, _pStack->m_pData, iOldSize); // copy old data in new buffer
		FE_MEM_FREE(pPreviousData); // delete old buffer

		_pStack->m_iSize = iNewSize;
		_pStack->m_pData = pData;
	}

	void* pDst = ((char*)_pStack->m_pData + _pStack->m_iCursor);
	memcpy(pDst, pDataEntry, _pStack->m_iStride);
	_pStack->m_iCursor += _pStack->m_iStride;
}