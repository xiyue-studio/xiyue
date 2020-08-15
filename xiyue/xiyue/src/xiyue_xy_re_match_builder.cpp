#include "stdafx.h"
#include "xiyue_xy_re.h"
#include "xiyue_xy_re_match_builder.h"

using namespace std;
using namespace xiyue;

XyReMatchBuilder::XyReMatchBuilder(const ConstString& str, const XyReProgram* program)
	: m_originalString(str)
	, m_program(program)
{
}

void XyReMatchBuilder::makeFailedMatch(XyReMatch* matchOut)
{
	if (matchOut == nullptr)
		return;

	matchOut->m_state = 2;
	matchOut->m_matchedPosition = UINT32_MAX;
	matchOut->m_matchedLength = UINT32_MAX;
	matchOut->m_originalString = m_originalString;
	matchOut->m_unnamedGroups.clear();
	matchOut->m_namedGroups.clear();
}

void XyReMatchBuilder::makeSucceededMatch(XyReMatch* matchOut, uint32_t matchStart, uint32_t matchLength, XyReThread* matchThread)
{
	if (matchOut == nullptr)
		return;

	matchOut->m_state = 1;
	matchOut->m_matchedPosition = matchStart;
	matchOut->m_matchedLength = matchLength;
	matchOut->m_originalString = m_originalString;

	uint32_t namesCount = m_program->namedGroupCount;
	uint32_t unnamesCount = m_program->numberGroupCount;

	const wchar_t* nameStart = m_program->referenceNames();
	const wchar_t* nameEnd = nameStart;
	auto& namedGroups = matchOut->m_namedGroups;
	auto& unnamedGroups = matchOut->m_unnamedGroups;
	auto groupPositions = matchThread->getGroups();
	namedGroups.clear();
	unnamedGroups.clear();

	for (size_t i = 0; i < namesCount; ++i)
	{
		while (*nameEnd)
			++nameEnd;

		ConstString key(nameStart, nameEnd - nameStart);
		nameStart = nameEnd + 1;
		nameEnd = nameStart;

		auto group = groupPositions + i;
		ConstString matchedString = m_originalString.substr(group->startPos, group->endPos - group->startPos);
		namedGroups.emplace(key, XyReSubMatch(matchedString, group->startPos));
	}
	assert(namesCount == namedGroups.size());

	groupPositions += namesCount;
	// 插入 \0 所代表的整个被匹配的字符串
	unnamedGroups.emplace_back(m_originalString.substr(matchStart, matchLength), matchStart);
	for (size_t i = 0; i < unnamesCount; ++i)
	{
		auto group = groupPositions + i;
		ConstString matchedString = m_originalString.substr(group->startPos, group->endPos - group->startPos);
		unnamedGroups.emplace_back(matchedString, group->startPos);
	}
	assert(unnamesCount == unnamedGroups.size() - 1);
}
