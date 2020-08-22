#include "stdafx.h"
#include "xiyue_xy_re_vm.h"
#include "xiyue_xy_re_process.h"
#include "xiyue_xy_re_match_builder.h"
#include "xiyue_xy_re_const_string_buffer.h"

using namespace std;
using namespace xiyue;

XyReVM::XyReVM(const XyReProgram* program)
{
	m_program = program;
	m_isIgnoreCase = false;
	m_isMultiLineMode = false;
	m_isDotMatchNewLine = false;
	m_isUnicodeMode = false;
}

bool XyReVM::match(const ConstString& str, XyReMatch* matchesOut)
{
	XyReProcess process(m_program);
	XyReConstStringBuffer strBuf = str;
	process.setStringBuffer(&strBuf);
	process.setIgnoreCase(m_isIgnoreCase);
	process.setMultiLineMode(m_isMultiLineMode);
	process.setDotMatchNewLine(m_isDotMatchNewLine);
	process.setUnicodeMode(m_isUnicodeMode);
	process.match();

	XyReMatchBuilder builder(str, m_program);
	if (process.isSucceeded())
		builder.makeSucceededMatch(matchesOut, 0, process.matchedLength(), process.matchedThread());
	else
		builder.makeFailedMatch(matchesOut);

	return process.isSucceeded();
}

bool XyReVM::searchOne(const ConstString& str, uint32_t startIndex, XyReMatch* matchesOut)
{
	XyReProcess process(m_program);
	XyReConstStringBuffer strBuf = str;
	process.setStringBuffer(&strBuf);
	process.setIgnoreCase(m_isIgnoreCase);
	process.setMultiLineMode(m_isMultiLineMode);
	process.setDotMatchNewLine(m_isDotMatchNewLine);
	process.setUnicodeMode(m_isUnicodeMode);

	while (startIndex < (uint32_t)str.length())
	{
		process.start(startIndex);
		if (process.isSucceeded())
			break;

		// 优化，fast fail
		if(process.isNoNeedTestMore())
			break;

		++startIndex;
	}

	XyReMatchBuilder builder(str, m_program);
	if (process.isSucceeded())
		builder.makeSucceededMatch(matchesOut, 0, process.matchedLength(), process.matchedThread());
	else
		builder.makeFailedMatch(matchesOut);

	return process.isSucceeded();
}

bool XyReVM::searchGlobal(const ConstString& str, bool globalMode, std::vector<XyReMatch>& matchesOut)
{
	matchesOut.clear();

	XyReProcess process(m_program);
	XyReConstStringBuffer strBuf = str;
	process.setStringBuffer(&strBuf);
	process.setIgnoreCase(m_isIgnoreCase);
	process.setMultiLineMode(m_isMultiLineMode);
	process.setDotMatchNewLine(m_isDotMatchNewLine);
	process.setUnicodeMode(m_isUnicodeMode);

	XyReMatchBuilder builder(str, m_program);

	process.start(0u);

	while (!process.isNoNeedTestMore())
	{
		if (process.isSucceeded())
		{
			matchesOut.emplace_back();
			builder.makeSucceededMatch(&matchesOut.back(), process.startMatchIndex(), process.matchedLength(), process.matchedThread());
			if (!globalMode)
				break;
		}

		process.start();
	}

	return !matchesOut.empty();
}

bool XyReVM::test(const ConstString& str, uint32_t startIndex, bool matchWhole)
{
	XyReProcess process(m_program);
	XyReConstStringBuffer strBuf = str;
	process.setStringBuffer(&strBuf);
	process.setIgnoreCase(m_isIgnoreCase);
	process.setMultiLineMode(m_isMultiLineMode);
	process.setDotMatchNewLine(m_isDotMatchNewLine);
	process.setUnicodeMode(m_isUnicodeMode);
	process.setJustTestMatch(true);

	if (matchWhole)
	{
		process.match();
		return process.isSucceeded();
	}

	while (startIndex < (uint32_t)str.length())
	{
		process.start(startIndex);
		if (process.isSucceeded())
			return true;

		// 优化，fast fail
		if (process.isNoNeedTestMore())
			return false;

		++startIndex;
	}

	return false;
}
