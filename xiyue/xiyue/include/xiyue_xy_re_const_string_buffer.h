#pragma once
#include "xiyue_xy_re_string_buffer.h"

namespace xiyue
{
	class XyReConstStringBuffer : public XyReStringBuffer
	{
	public:
		XyReConstStringBuffer(ConstString str);

	public:
		virtual bool loadMoreCharacters() { return false; }

	private:
		ConstString m_string;
	};
}
