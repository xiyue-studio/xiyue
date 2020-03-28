#pragma once

namespace xiyue
{
	class Int64Pair
	{
	public:
		Int64Pair() = default;
		Int64Pair(uint32_t l, uint32_t r)
			: m_value((((uint64_t)l) << 32ull) + r)
		{
		}
		Int64Pair(const Int64Pair& r) : m_value(r.m_value) {}

		inline uint32_t first() const { return (uint32_t)(m_value >> 32); }
		inline uint32_t second() const { return (uint32_t)(m_value & 0xffffffff); }

		Int64Pair& operator=(const Int64Pair& r) { m_value = r.m_value; return *this; }
		operator uint64_t() const { return m_value; }
		bool operator<(const Int64Pair& r) const { return m_value < r.m_value; }

	protected:
		uint64_t m_value = 0;
	};

#define XIYUE_UNION_KEY(name, t1, t2)								\
	class name : public ::xiyue::Int64Pair							\
	{																\
	public:															\
		name() = default;											\
		name(uint32_t l, uint32_t r) : ::xiyue::Int64Pair(l, r) {}	\
		inline uint32_t t1() const { return first(); }				\
		inline uint32_t t2() const { return second(); }				\
	}
}
