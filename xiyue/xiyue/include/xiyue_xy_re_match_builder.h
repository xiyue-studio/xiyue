#pragma once
#include "xiyue_xy_re_thread.h"

namespace xiyue
{
	class XyReMatch;

	class XyReMatchBuilder
	{
	public:
		XyReMatchBuilder(const ConstString& str, const XyReProgram* program);

	public:
		void makeSucceededMatch(XyReMatch* matchOut, uint32_t matchStart, uint32_t matchLength, XyReThread* matchThread);
		void makeFailedMatch(XyReMatch* matchOut);

	private:
		ConstString m_originalString;
		const XyReProgram* m_program;
	};
}
