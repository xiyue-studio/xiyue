#pragma once

namespace xiyue
{
	template<typename T>
	class Array
	{
	public:
		explicit Array(size_t size)
			: m_data(new T[size](), std::default_delete<T[]>())
			, m_dataSize(size)
		{
		}

		Array(const Array<T>& r)
			: m_dataSize(r.m_dataSize)
			, m_data(r.m_data)
		{
		}

		virtual ~Array() = default;

	public:
		T& operator[](int index) {
			assert(index >= 0 && (size_t)index < m_dataSize);
			return *(m_data.get() + index);
		}

		Array<T>& operator=(const Array<T>& r) {
			m_dataSize = r.m_dataSize;
			m_data = r.m_dataSize;
		}

		size_t size() const { return m_dataSize; }

		operator std::remove_reference_t<T>*() const { return m_data.get(); }

	public:
		std::remove_reference_t<T>* begin() const { return m_data.get(); }
		std::remove_reference_t<T>* end() const { return m_data.get() + m_dataSize; }

	protected:
		size_t m_dataSize;
		std::shared_ptr<T> m_data;
	};
}
