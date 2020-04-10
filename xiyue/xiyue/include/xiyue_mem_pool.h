#pragma once
#include "xiyue_single_object_mem_pool.h"

namespace xiyue
{
	template<typename T>
	class MemPool
	{
	private:
		struct Block {
			Block* next;
		}*m_blocks;

		struct Node {
			T* addr;
			Node* next;
			size_t capacity;
		};

		size_t m_blockSize;
		size_t m_dispatchedSize;
		/*
			0 - 1
			1 - 2
			2 - 4
			3 - 8
			4 - 16
			5 - 32
			6 - 64
			7 - 128
			8 - 256
			9 - 512
			10 - 1024
			11 - 2048
			12 - 4096
			13 - 8192
			14 - 16384
			15 - larger
		*/
		Node m_freeList[16];
		SingleObjectMemPool<Node> m_nodeAllocator;

	public:
		explicit MemPool(size_t blockSize = 4096u)
			: m_blocks(nullptr)
			, m_blockSize(blockSize)
			, m_dispatchedSize(blockSize)
		{
			for (int i = 0; i < XIYUE_ARRAY_COUNT(m_freeList); ++i)
				m_freeList[i].next = nullptr;
		}

		virtual ~MemPool() {
			clear();
		}

	public:
		T* alloc() {
			return allocArray(1u);
		}

		void free(T* p) {
			return freeArray(p, 1u);
		}

		T* allocArray(size_t count) {
			T* result;
			if ((result = allocArrayFromFreeList(count)) != nullptr)
				return result;
			return allocArrayFromBlock(count);
		}

		void freeArray(T* p, size_t count) {
			addFreeList(p, count);
		}

		T* reallocArray(T* p, size_t oldSize, size_t newSize) {
			if (p == nullptr)
				return allocArray(newSize);

			if (oldSize >= newSize)
			{
				// 将多余的内存加入 free list
				if (oldSize > newSize)
					addFreeList(p + newSize, oldSize - newSize);
				return p;
			}

			// 去 block 中查找是否可以合并
			if (m_blockSize - m_dispatchedSize >= newSize - oldSize
				&& p + oldSize == (T*)(m_blocks + 1) + m_dispatchedSize)
			{
				m_dispatchedSize += newSize - oldSize;
				return p;
			}

			// 回收旧内存，分配新内存
			T* result = allocArray(newSize);
			std::copy(p, p + oldSize, result);
			freeArray(p, oldSize);
			return result;
		}

		void clear() {
			while (m_blocks)
			{
				Block* b = m_blocks->next;
				::free(m_blocks);
				m_blocks = b;
			}

			for (int i = 0; i < XIYUE_ARRAY_COUNT(m_freeList); ++i)
			{
				m_freeList[i].next = nullptr;
			}

			m_nodeAllocator.clear();
		}

	private:
		T* allocArrayFromBlock(size_t count)
		{
			// 如果申请的数量直接超过了 blockSize，则直接分配新的
			if (count >= m_blockSize)
			{
				return (T*)(allocBlockAndInsertSecond(count) + 1);
			}

			// 确保当前没有满载
			if (m_dispatchedSize >= m_blockSize)
			{
				allocBlockAndInsertHead(m_blockSize);
				m_dispatchedSize = 0;
			}

			// 如果分配的数量少于未分配的数量，直接分配
			if (count + m_dispatchedSize <= m_blockSize)
			{
				T* result = (T*)(m_blocks + 1) + m_dispatchedSize;
				m_dispatchedSize += count;
				return result;
			}

			// 如果分配的数量多于未分配的数量，则将未分配的那部分加入 freeList
			addFreeList((T*)(m_blocks + 1) + m_dispatchedSize, m_blockSize - m_dispatchedSize);
			m_dispatchedSize = count;
			return (T*)(allocBlockAndInsertHead(m_blockSize) + 1);
		}

		void addFreeList(T* p, size_t count)
		{
			int index = getFreeListIndex(count);
			Node* node = m_nodeAllocator.alloc();
			node->capacity = count;
			node->addr = p;
			node->next = m_freeList[index].next;
			m_freeList[index].next = node;
		}

		T* allocArrayFromFreeList(size_t count)
		{
			// 从最接近的大小的 free list 中查找
			int index = getFreeListIndex(count);
			while (index < XIYUE_ARRAY_COUNT(m_freeList))
			{
				Node* pPrev = &m_freeList[index];
				Node* p = pPrev->next;
				while (p != nullptr)
				{
					if (p->capacity >= count)
					{
						// 找到合适大小的内存块，直接分配
						T* result = p->addr;
						p->addr += count;
						p->capacity -= count;
						// 摘下这个节点
						pPrev->next = p->next;
						if (p->capacity == 0)
						{
							// p 没有剩余空间了
							m_nodeAllocator.free(p);
						}
						else
						{
							// 将 p 挂到合适的位置
							index = getFreeListIndex(p->capacity);
							p->next = m_freeList[index].next;
							m_freeList[index].next = p;
						}

						return result;
					}

					pPrev = p;
					p = p->next;
				}
				// 当前的容量中没有合适的块，就尝试去更大的块中找
				++index;
			}
			// free list 中没有合适的块，则分配失败
			return nullptr;
		}

		Block* allocBlockAndInsertHead(size_t count) {
			Block* b = (Block*)malloc(sizeof(Block) + sizeof(T) * count);
			b->next = m_blocks;
			m_blocks = b;
			return b;
		}

		Block* allocBlockAndInsertSecond(size_t count) {
			Block* b = (Block*)malloc(sizeof(Block) + sizeof(T) * count);
			Block* &insertPlace = m_blocks == nullptr ? m_blocks : m_blocks->next;
			b->next = insertPlace;
			insertPlace = b;
			return b;
		}

		constexpr int getFreeListIndex(size_t size) {
			static const size_t sizeArray[] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384 };
			// fast return
			if (size == 1)
				return 0;

			auto index = std::lower_bound(sizeArray, std::end(sizeArray), size);
			return (int)(index - sizeArray);
		}
	};
}
