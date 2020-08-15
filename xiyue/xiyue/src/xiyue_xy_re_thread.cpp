#include "stdafx.h"
#include "xiyue_xy_re_thread.h"

using namespace std;
using namespace xiyue;

static_assert(is_pod<XyReThread>::value, "XyReThread must be POD");

XyReGroupPosition* XyReThread::getGroups()
{
	return reinterpret_cast<XyReGroupPosition*>(this + 1);
}

void XyReThread::copyGroupsFromThread(const XyReProgram* program, XyReThread* t)
{
	size_t groupCount = program->namedGroupCount + program->numberGroupCount;
	if (groupCount == 0)
		return;

	memcpy(getGroups(), t->getGroups(), sizeof(XyReGroupPosition) * groupCount);
}

void XyReThread::saveGroupStart(const XyReProgram* program, int groupID, uint32_t startPos)
{
	uint32_t index = getGroupIndexById(program, groupID);

	XyReGroupPosition* p = getGroups() + index;
	p->startPos = startPos;
	p->endPos = UINT32_MAX;
}

void XyReThread::saveGroupEnd(const XyReProgram* program, int groupID, uint32_t endPos)
{
	uint32_t index = getGroupIndexById(program, groupID);

	XyReGroupPosition* p = getGroups() + index;
	p->endPos = endPos;
}

wchar_t XyReThread::getBackReferencedChar(const wchar_t* srcStr)
{
	if (!isMatchingBackReference)
		return 0;

	XyReGroupPosition* p = getGroups() + backReferenceGroupID;
	if (p->startPos + backReferenceIndex >= p->endPos)
		return 0;

	return srcStr[p->startPos + backReferenceIndex];
}

uint32_t XyReThread::getGroupIndexById(const XyReProgram* program, int groupID)
{
	uint32_t index = groupID < 0 ? -groupID : program->namedGroupCount + groupID;
	assert(index != 0);
	assert(groupID > 0 || (uint32_t)(-groupID) <= program->namedGroupCount);
	assert(groupID < 0 || (uint32_t)groupID <= program->numberGroupCount);
	--index;

	return index;
}
