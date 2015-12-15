#pragma once

#include <common.hpp>
#include <tarray.hpp>

#include	<map>

namespace FeCommon
{
	class FeMemoryManager
	{
	public:
		enum ChunkType
		{
			FREE_CHUNK,
			SHADER_CHUNK,
			INDEX_CHUNK,
			VERTEX_CHUNK,
			TEXTURE_CHUNK,
			COLORBUFFER_CHUNK,
			DEPTHBUFFER_CHUNK,
			RENDERTARGET_CHUNK,
			EDGE_CHUNK,
			LOCK_CHUNK,
			OTHER_CHUNK,
			UNKNOWN_CHUNK,

			NUM_CHUNK_TYPE
		};


		FeMemoryManager();
		~FeMemoryManager();

		void*		Allocate(const uint64& _size, const uint64& _alignmemnt, ChunkType _type);
		void*		Free(void* _ptr);
		
		uint64		GetLocalTotalAllocatedSize() const	{ return TotalAllocatedSize; }
		uint64		GetLocalPeakAllocatedSize() const	{ return PeakAllocatedSize; }

		void		Init(const uint64& _baseAddress, const uint64& _localSize);
		void		GetLocalMemoryHeapAddress(uint64** _baseAddress);

		void		Defrag();

	private:
		uint64		GetTotalFree() const;
		uint64		GetLocalTotalSize()	const	{ return LocalTotalSize; }

	private:
		typedef struct MemoryChunk
		{
			uint64		address;
			uint64		alignAddress;
			uint64		chunkSize;
			ChunkType	type;

			MemoryChunk()
				: address(0)
				, alignAddress(0)
				, chunkSize(0)
				, type(FREE_CHUNK)
			{

			}
		} MemoryChunk;

		typedef struct FreeMemoryChunk
		{
			uint64		address;
			uint64		chunkSize;
		} FreeMemoryChunk;

	protected:
		typedef std::map<uint64, uint64>		MapFreeChunk;
		typedef MapFreeChunk::iterator			MapFreeChunkIt;
		typedef MapFreeChunk::const_iterator	MapFreeChunkConstIt;

		MapFreeChunk							MapFreeChunks;
		FeTArray<MemoryChunk, uint64>			AllocatedChunks;
		uint64									AllocationStats[NUM_CHUNK_TYPE];

		uint64			LocalBaseAdress;
		uint64			LocalTotalSize;
		uint64			TotalAllocatedSize;
		uint64			PeakAllocatedSize;
		uint64			ChunckAllocatedSize[NUM_CHUNK_TYPE];
	};
}