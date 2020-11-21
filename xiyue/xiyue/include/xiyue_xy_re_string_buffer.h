#pragma once

namespace xiyue
{
	class XyReStringBuffer
	{
	public:
		XyReStringBuffer() = default;
		virtual ~XyReStringBuffer() = default;

	public:
		virtual bool loadMoreCharacters() = 0;

	public:
		inline const wchar_t* stringBegin() const { return m_strBegin; }
		inline const wchar_t* stringEnd() const { return m_strEnd; }
		inline const wchar_t* stringPosition() const { return m_sp; }

		inline uint32_t sp() const { return m_sp - m_strBegin; }
		inline void setSP(uint32_t offset) { m_sp = m_strBegin + offset; }
		inline void spInc() { ++m_sp; }

		/*
			在前向界定中使用，临时更改字符串的结束位置。
		*/
		inline void resetStringEnd(const wchar_t* ep) { m_strEnd = ep; }

	protected:
		const wchar_t* m_strBegin;
		const wchar_t* m_strEnd;
		const wchar_t* m_sp;
	};
}
