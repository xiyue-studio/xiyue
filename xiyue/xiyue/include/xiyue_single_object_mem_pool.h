#pragma once

namespace xiyue
{
	template<typename T>
	class SingleObjectMemPool
	{
	public:
		explicit SingleObjectMemPool(size_t blockSize = 4096u)
			: m_blockSize(blockSize)
			, m_dispatchedSize(blockSize)
			, m_freeList(nullptr)
			, m_blocks(nullptr)
		{
		}

		virtual ~SingleObjectMemPool() {
			clear();
		}

	public:
		T* alloc() {
			T* result = allocFromFreeList();
			if (result)
				return result;

			return allocFromBlock();
		}

		void free(T* p) {
			if (p == nullptr)
				return;

			Object* o = (Object*)p;
			o->next = m_freeList;
			m_freeList = o;
		}

		void clear() {
			while (m_blocks != nullptr)
			{
				Block* b = m_blocks->next;
				::free(m_blocks);
				m_blocks = b;
			}

			m_freeList = nullptr;
			m_dispatchedSize = m_blockSize;
		}

	protected:
		T* allocFromFreeList() {
			if (m_freeList == nullptr)
				return nullptr;

			Object* result = m_freeList;
			m_freeList = result->next;
			return &result->obj;
		}

		T* allocFromBlock() {
			if (m_dispatchedSize >= m_blockSize) {
				Block* b = (Block*)malloc(sizeof(Block) + sizeof(Object) * m_blockSize);
				b->next = m_blocks;
				m_blocks = b;
				m_dispatchedSize = 0;
			}

			return &((Object*)(m_blocks + 1) + m_dispatchedSize++)->obj;
		}

	private:
		size_t m_blockSize;
		size_t m_dispatchedSize;
		struct Object
		{
			T obj;
			Object* next;
		} *m_freeList;
		struct Block
		{
			Block* next;
		} *m_blocks;
	};
}
