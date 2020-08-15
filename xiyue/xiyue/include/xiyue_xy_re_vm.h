#pragma once
#include "xiyue_xy_re.h"
#include "xiyue_xy_re_program.h"

namespace xiyue
{
	class XyReVM
	{
	public:
		explicit XyReVM(const XyReProgram* program);

	public:
		inline void setIgnoreCase(bool on) { m_isIgnoreCase = on; }
		inline void setMultiLineMode(bool on) { m_isMultiLineMode = on; }
		inline void setDotMatchNewLine(bool on) { m_isDotMatchNewLine = on; }
		inline void setUnicodeMode(bool on) { m_isUnicodeMode = on; }

	public:
		bool match(const ConstString& str, XyReMatch* matchesOut);
		bool searchOne(const ConstString& str, uint32_t startIndex, XyReMatch* matchesOut);
		bool searchGlobal(const ConstString& str, bool globalMode, std::vector<XyReMatch>& matchesOut);
		bool test(const ConstString& str, uint32_t startIndex, bool matchWhole);

	private:
		const XyReProgram* m_program;
		bool m_isIgnoreCase;
		bool m_isMultiLineMode;
		bool m_isDotMatchNewLine;
		bool m_isUnicodeMode;
	};
}
