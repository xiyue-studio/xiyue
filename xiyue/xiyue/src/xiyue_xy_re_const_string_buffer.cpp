#include "stdafx.h"
#include "xiyue_xy_re_const_string_buffer.h"

using namespace std;
using namespace xiyue;

XyReConstStringBuffer::XyReConstStringBuffer(ConstString str)
	: m_string(str)
{
	m_strBegin = m_string.begin();
	m_strEnd = m_string.end();
	m_sp= m_strBegin;
}
