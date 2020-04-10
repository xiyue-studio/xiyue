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
				// ��������ڴ���� free list
				if (oldSize > newSize)
					addFreeList(p + newSize, oldSize - newSize);
				return p;
			}

			// ȥ block �в����Ƿ���Ժϲ�
			if (m_blockSize - m_dispatchedSize >= newSize - oldSize
				&& p + oldSize == (T*)(m_blocks + 1) + m_dispatchedSize)
			{
				m_dispatchedSize += newSize - oldSize;
				return p;
			}

			// ���վ��ڴ棬�������ڴ�
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
			// ������������ֱ�ӳ����� blockSize����ֱ�ӷ����µ�
			if (count >= m_blockSize)
			{
				return (T*)(allocBlockAndInsertSecond(count) + 1);
			}

			// ȷ����ǰû������
			if (m_dispatchedSize >= m_blockSize)
			{
				allocBlockAndInsertHead(m_blockSize);
				m_dispatchedSize = 0;
			}

			// ����������������δ�����������ֱ�ӷ���
			if (count + m_dispatchedSize <= m_blockSize)
			{
				T* result = (T*)(m_blocks + 1) + m_dispatchedSize;
				m_dispatchedSize += count;
				return result;
			}

			// ����������������δ�������������δ������ǲ��ּ��� freeList
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
			// ����ӽ��Ĵ�С�� free list �в���
			int index = getFreeListIndex(count);
			while (index < XIYUE_ARRAY_COUNT(m_freeList))
			{
				Node* pPrev = &m_freeList[index];
				Node* p = pPrev->next;
				while (p != nullptr)
				{
					if (p->capacity >= count)
					{
						// �ҵ����ʴ�С���ڴ�飬ֱ�ӷ���
						T* result = p->addr;
						p->addr += count;
						p->capacity -= count;
						// ժ������ڵ�
						pPrev->next = p->next;
						if (p->capacity == 0)
						{
							// p û��ʣ��ռ���
							m_nodeAllocator.free(p);
						}
						else
						{
							// �� p �ҵ����ʵ�λ��
							index = getFreeListIndex(p->capacity);
							p->next = m_freeList[index].next;
							m_freeList[index].next = p;
						}

						return result;
					}

					pPrev = p;
					p = p->next;
				}
				// ��ǰ��������û�к��ʵĿ飬�ͳ���ȥ����Ŀ�����
				++index;
			}
			// free list ��û�к��ʵĿ飬�����ʧ��
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
