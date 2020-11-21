#include "stdafx.h"
#include "xiyue_xy_re_process.h"
#include "xiyue_encoding.h"

using namespace std;
using namespace xiyue;

#pragma region Action Codes
static force_inline uint16_t makeUint16(uint8_t arg1, uint8_t arg2) {
	return ((uint16_t)arg1 << 8) + arg2;
}

static force_inline void doNoop(XyReThread* t) {
	t->pcInc();
}

static force_inline void doChar(const XyReInstruction* ins, XyReThread* t, wchar_t ch) {
	if (ins->subDirective == 0)
	{
		if (ch != ((ins->arg1 << 8) + ins->arg2))
			t->markFailed();
		else
			t->pcInc();
	}
	else if (ins->subDirective == 1)
	{
		t->pcInc();
		uint32_t insCh = *(const uint32_t*)t->pc;
		if (ch != insCh)
			t->markFailed();
		else
			t->pcInc();
	}
	else
	{
		throw exception();
	}

	t->pause();
}

static force_inline void doSplt(const XyReInstruction* ins, XyReThread* t, XyReThreadPool* pool, list<XyReThread*>& workingThreads) {
	XyReThread* nt = pool->splitThread(t);
	int offset1;
	int offset2;

	switch (ins->subDirective)
	{
	case 0:
		offset1 = (int8_t)ins->arg1;
		offset2 = (int8_t)ins->arg2;
		break;
	case 1:
		offset1 = (int16_t)makeUint16(ins->arg1, ins->arg2);
		offset2 = *(int*)(t->pc + 1);
		break;
	case 2:
		offset2 = *(int*)(t->pc + 1);
		offset1 = (int16_t)makeUint16(ins->arg1, ins->arg2);
		break;
	default:
		throw exception();
	}

	t->pc += offset1;
	nt->pc += offset2;

	workingThreads.push_front(nt);
}

static force_inline void doJump(const XyReInstruction* ins, XyReThread* t) {
	switch (ins->subDirective)
	{
	case 0:
		t->pc += (int16_t)makeUint16(ins->arg1, ins->arg2);
		break;
	case 1:
		t->pc += *(int*)(t->pc + 1);
		break;
	default:
		throw exception();
	}
}

static force_inline void doSucc(XyReProcess* process, XyReThread* &t) {
	t->markSucceeded();
	if (t->returnThread != nullptr)
	{
		XyReThread* nt = t->returnThread;
		process->removeThread(t);
		t = nt;
	}
}

static force_inline void doDely(XyReProcess* process, XyReThread* t, list<XyReThread*>& delayedThreads) {
	// 比较有意思的是，全串测试的时候，非贪婪匹配需要退化成贪婪匹配
	if (!process->isMatchWholeString())
	{
		t->markDelayedAndPause();
		delayedThreads.push_back(t);
	}
	else
	{
		t->pcInc();
	}
}

static force_inline void doAban(XyReProcess* process, const XyReInstruction* ins, XyReThread* t, map<uint32_t, XyReProcess::MatchedThreadState>& winners, uint32_t matchedLength) {
	uint32_t abanID;
	if (ins->subDirective == 0)
	{
		abanID = makeUint16(ins->arg1, ins->arg2);
	}
	else
	{
		t->pcInc();
		abanID = *(uint32_t*)t->pc;
	}

	// 查找 winners，如果获取了最长匹配，则竞争成功，否则竞争失败
	auto it = winners.find(abanID);
	if (it == winners.end())
	{
		winners.insert(make_pair(abanID, XyReProcess::MatchedThreadState{ matchedLength, t }));
	}
	else
	{
		if (matchedLength > it->second.matchedLength)
		{
			process->removeThread(it->second.thread);
			winners.insert(make_pair(abanID, XyReProcess::MatchedThreadState{ matchedLength, t }));
		}
		else
		{
			// 竞争失败，移除自身
			t->markFailed();
			return;
		}
	}

	t->pcInc();
}

static force_inline void doCall(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	XyReProgramPC newPC = t->pc;
	XyReProgramPC returnPC = t->pc;
	bool isLookBehind;
	bool isNegtive;
	uint32_t minMatchLength;
	uint32_t maxMatchLength;
	XyReProcess* subProcess = nullptr;

	switch (ins->subDirective)
	{
	case 0:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = true;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		break;
	case 1:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = true;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		break;
	case 2:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = false;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		maxMatchLength = minMatchLength;
		break;
	case 3:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = false;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		maxMatchLength = minMatchLength;
		break;
	case 4:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = false;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		maxMatchLength = *(uint32_t*)(t->pc + 2);
		break;
	case 5:
		newPC += (int8_t)ins->arg1;
		returnPC += (int8_t)ins->arg2;
		isLookBehind = false;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 1);
		maxMatchLength = *(uint32_t*)(t->pc + 2);
		break;
	case 6:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = true;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		break;
	case 7:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = true;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		break;
	case 8:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = false;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		maxMatchLength = minMatchLength;
		break;
	case 9:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = false;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		maxMatchLength = minMatchLength;
		break;
	case 10:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = false;
		isNegtive = false;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		maxMatchLength = *(uint32_t*)(t->pc + 4);
		break;
	case 11:
		newPC += *(int32_t*)(t->pc + 1);
		returnPC += *(int32_t*)(t->pc + 2);
		isLookBehind = false;
		isNegtive = true;
		minMatchLength = *(uint32_t*)(t->pc + 3);
		maxMatchLength = *(uint32_t*)(t->pc + 4);
		break;
	default:
		throw exception("");
	}

	XyReProcess::LookAroundCache* cache;
	XyReProcess::LookAroundCache _cache;
	uint32_t spos = process->stringPosition() - process->stringBegin();
	if (isLookBehind)
	{
		// 后向界定搜索
		// 查询缓存
		cache = process->findLookBehindCache(newPC, process->stringPosition());
		if (cache == nullptr)
		{
			// 未被缓存，则启动新线程匹配
			subProcess = process->clone();
			subProcess->startAt(newPC, process->stringPosition());
			process->getStringBuffer()->setSP(spos);
			_cache = {
					subProcess->isSucceeded(),
					process->stringPosition() + subProcess->matchedLength(),
					subProcess->isSucceeded() ? process->threadPool()->splitThread(subProcess->matchedThread()) : nullptr
				};
			process->lookBehindCache().insert(make_pair(make_pair(newPC, process->stringPosition()), _cache));
			cache = &_cache;
			delete subProcess;
		}
	}
	else
	{
		// 前向界定搜索
		cache = process->findLookAheadCache(newPC, process->stringPosition());
		if (cache == nullptr)
		{
			maxMatchLength = std::min<uint32_t>(maxMatchLength, process->stringPosition() - process->stringBegin());
			for (uint32_t len = maxMatchLength; len >= minMatchLength; --len)
			{
				subProcess = process->clone();
				subProcess->startAt(newPC, process->stringPosition() - len, process->stringPosition());
				if (subProcess->isSucceeded())
					break;

				delete subProcess;
				subProcess = nullptr;
			}
			process->getStringBuffer()->setSP(spos);
			if (subProcess == nullptr)
				_cache = {
					false,
					process->stringPosition(),
					nullptr
				};
			else
				_cache = {
					true,
					process->stringPosition() - subProcess->matchedLength(),
					process->threadPool()->splitThread(subProcess->matchedThread())
				};
			process->lookAheadCache().insert(make_pair(make_pair(newPC, process->stringPosition()), _cache));
			delete subProcess;
			cache = &_cache;
		}
	}

	if (isNegtive && cache->succeeded || !isNegtive && !cache->succeeded)
	{
		t->markFailed();
		return;
	}
	// 环视成功的情况
	if (cache->succeededThread)
		t->copyGroupsFromThread(process->getProgram(), cache->succeededThread);
	t->pc = returnPC;
}

static force_inline void doAllm(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	if (ch == 0
		|| !t->isMultiLineMode && isLineBreak(ch)
		|| !t->isUnicodeMode && !isAscii(ch)
		)
	{
		t->markFailed();
		return;
	}

	t->pcIncAndPause();
}

static const wchar_t* g_asciiSpaceChars = L" \t\n\r\f\v";
static const wchar_t* g_unicodeSpaceChars = g_asciiSpaceChars;
static force_inline void doSpce(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	bool matched;
	if (t->isUnicodeMode)
		matched = wcschr(g_unicodeSpaceChars, ch) != nullptr;
	else
		matched = wcschr(g_asciiSpaceChars, ch) != nullptr;

	if (matched)
		t->pcIncAndPause();
	else
		t->markFailed();
}

static force_inline void doNspc(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	bool matched;
	if (t->isUnicodeMode)
		matched = wcschr(g_unicodeSpaceChars, ch) != nullptr;
	else
		matched = wcschr(g_asciiSpaceChars, ch) != nullptr;

	if (matched)
		t->markFailed();
	else
		t->pcIncAndPause();
}

static force_inline void doDgit(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	if (isDigit(ch))
		t->pcIncAndPause();
	else
		t->markFailed();
}

static force_inline void doNdgt(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	if (isDigit(ch))
		t->markFailed();
	else
		t->pcIncAndPause();
}

static force_inline void doWord(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	if (isWordChar(ch))
		t->pcIncAndPause();
	else
		t->markFailed();
}

static force_inline void doNwod(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->cch(process->stringPosition());
	if (isWordChar(ch))
		t->markFailed();
	else
		t->pcIncAndPause();
}

static force_inline void doHead(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->stringPosition() == process->stringBegin() ? process->formerChar() : *(process->stringPosition() - 1);
	bool matched = t->isMultiLineMode ? ch == 0 || isLineBreak(ch) : ch == 0;
	if (matched)
		t->pcInc();
	else
		t->markFailed();
}

static force_inline void doTail(XyReProcess* process, XyReThread* t) {
	wchar_t ch = process->stringPosition() == process->stringEnd() ? process->latterChar() : *process->stringPosition();
	bool matched = t->isMultiLineMode ? ch == 0 || isLineBreak(ch) : ch == 0;
	if (matched)
		t->pcInc();
	else
		t->markFailed();
}

static force_inline void doBond(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	wchar_t fc = process->stringPosition() == process->stringBegin() ? process->formerChar() : *(process->stringPosition() - 1);
	wchar_t lc = process->stringPosition() == process->stringEnd() ? process->latterChar() : *process->stringPosition();
	bool matched = false;
	switch (ins->subDirective)
	{
	case 0:
		matched = ((fc == 0 || isSpace(fc)) && isWordChar(lc))
			|| ((lc == 0 || isSpace(lc)) && isWordChar(fc));
		break;
	case 1:
		matched = (fc == 0 || isSpace(fc)) && isWordChar(lc);
		break;
	case 2:
		matched = (lc == 0 || isSpace(lc)) && isWordChar(fc);
		break;
	}

	if (matched)
		t->pcInc();
	else
		t->markFailed();
}

static force_inline void doNbnd(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	wchar_t fc = process->stringPosition() == process->stringBegin() ? process->formerChar() : *(process->stringPosition() - 1);
	wchar_t lc = process->stringPosition() == process->stringEnd() ? process->latterChar() : *process->stringPosition();
	bool matched = false;
	switch (ins->subDirective)
	{
	case 0:
		matched = ((fc == 0 || isSpace(fc)) && isWordChar(lc))
			|| ((lc == 0 || isSpace(lc)) && isWordChar(fc));
		break;
	case 1:
		matched = (fc == 0 || isSpace(fc)) && isWordChar(lc);
		break;
	case 2:
		matched = (lc == 0 || isSpace(lc)) && isWordChar(fc);
		break;
	}

	if (matched)
		t->markFailed();
	else
		t->pcInc();
}

static force_inline void doLspm(XyReProcess* process, XyReThread* t) {
	if (process->lastSearchStoppedPosition() != nullptr && process->lastSearchStoppedPosition() != process->stringPosition())
		t->markFailed();
	t->pcInc();
}

static inline bool isChInSet(wchar_t ch, const int* rangeStart, uint32_t rangeLen, const int* charStart, uint32_t charLen) {
	// 先比对是否满足某个 range
	for (uint32_t i = 0; i < rangeLen; ++i)
	{
		wchar_t startChar = (wchar_t)rangeStart[2 * i];
		wchar_t endChar = (wchar_t)rangeStart[2 * i + 1];
		if (ch >= startChar && ch <= endChar)
			return true;
	}

	// 再查找是否落在列举的字符列表中
	auto it = lower_bound(charStart, charStart + charLen, (int)ch);
	return it != charStart + charLen && *it == (int)ch;
}

static force_inline void doClsm(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	bool isNegtive = false;
	uint32_t rangeLength = 0;
	uint32_t charLength = 0;
	const int* rangeStart = nullptr;
	const int* charStart = nullptr;
	XyReProgramPC returnPos = t->pc + 1;
	switch (ins->subDirective)
	{
	case 0:
		isNegtive = false;
		rangeLength = ins->arg1;
		charLength = ins->arg2;
		rangeStart = t->pc + 1;
		break;
	case 1:
		isNegtive = true;
		rangeLength = ins->arg1;
		charLength = ins->arg2;
		rangeStart = t->pc + 1;
		break;
	case 2:
		isNegtive = false;
		rangeLength = *(uint32_t*)(t->pc + 1);
		charLength = *(uint32_t*)(t->pc + 2);
		rangeStart = t->pc + 3;
		break;
	case 3:
		isNegtive = false;
		rangeLength = *(uint32_t*)(t->pc + 1);
		charLength = *(uint32_t*)(t->pc + 2);
		rangeStart = t->pc + 3;
		break;
	default:
		throw exception();
	}

	charStart = rangeStart + rangeLength * 2;
	returnPos = charStart + charLength;

	wchar_t ch = process->cch(process->stringPosition());
	bool matched = isChInSet(ch, rangeStart, rangeLength, charStart, charLength);
	if (t->isIgnoreCase && !matched)
	{
		wchar_t lowerChar = (wchar_t)tolower(ch);
		wchar_t upperChar = (wchar_t)toupper(ch);
		if (lowerChar != ch)
			matched = isChInSet(lowerChar, rangeStart, rangeLength, charStart, charLength);
		else if (upperChar != ch)
			matched = isChInSet(upperChar, rangeStart, rangeLength, charStart, charLength);
	}

	if (!(isNegtive ^ matched))
		t->markFailed();

	t->pc = returnPos;
	t->pause();
}

static force_inline void doBref(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	assert(!t->isMatchingBackReference);
	int referenceId;
	if (ins->subDirective == 0)
	{
		referenceId = (int16_t)makeUint16(ins->arg1, ins->arg2);
	}
	else
	{
		referenceId = *(int*)(t->pc + 1);
		t->pcInc();
	}
	t->isMatchingBackReference = true;
	t->backReferenceGroupID = t->getGroupIndexById(process->getProgram(), referenceId);
	t->backReferenceIndex = 0;
	t->pcInc();
}

static force_inline void doWbrf(XyReProcess* process, XyReThread* t) {
	wchar_t ch = t->getBackReferencedChar(process->stringBegin());
	if (ch == 0)
	{
		t->isMatchingBackReference = false;
		t->pcInc();
		return;
	}

	if (process->stringPosition() >= process->stringEnd()
		||  *process->stringPosition() != ch)
	{
		t->markFailed();
		return;
	}
	++t->backReferenceIndex;
	t->pause();
}

static force_inline void doSwch(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	bool switchOn = ins->subDirective == 0;
	switch (ins->arg1)
	{
	case XyReMatch_ignoreCase:
		t->isIgnoreCase = switchOn ^ process->isIgnoreCase();
		break;
	case XyReMatch_multiLine:
		t->isMultiLineMode = switchOn ^ process->isMultiLineMode();
		break;
	case XyReMatch_dotMatchNewLine:
		t->isDotMatchNewLine = switchOn ^ process->isDotMatchNewLine();
		break;
	case XyReMatch_unicode:
		t->isUnicodeMode = switchOn ^ process->isUnicodeMode();
		break;
	}

	t->pcInc();
}

static force_inline void doSvst(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	int groupId;
	if (ins->subDirective == 0)
	{
		groupId = (int16_t)makeUint16(ins->arg1, ins->arg2);
	}
	else
	{
		groupId = *(int*)(t->pc + 1);
		t->pcInc();
	}
	
	t->saveGroupStart(process->getProgram(), groupId, process->stringPosition() - process->stringBegin());
	t->pcInc();
}

static force_inline void doSved(XyReProcess* process, const XyReInstruction* ins, XyReThread* t) {
	int groupId;
	if (ins->subDirective == 0)
	{
		groupId = (int16_t)makeUint16(ins->arg1, ins->arg2);
	}
	else
	{
		groupId = *(int*)(t->pc + 1);
		t->pcInc();
	}

	t->saveGroupEnd(process->getProgram(), groupId, process->stringPosition() - process->stringBegin());
	t->pcInc();
}

static force_inline XyReProgramPC matchDfa(XyReProgramPC ins, char ch) {
	assert(((const XyReInstruction*)ins)->directive == DFAM);
	int index = reinterpret_cast<const char*>(ins + 1)[ch];
	if (index == 0)
		return nullptr;

	index = *(int*)(ins + 64 + index - 1);
	return ins + index;
}

static force_inline void doDfam(XyReProcess* process, XyReThread* t) {
/**
	DFAM(DFA Match)

	DFA 匹配专用节点。只有在 DFA 匹配模式下才会出现这个
	指令，且不再会出现别的指令。
	subDirective = 1，表示这个节点是接受状态。

	后跟 sparse set 结构，结构具体如下：
	64 个 uint，表示 256 个 uint8 索引位置，其中的值表示
	后面跟着的位置索引。
	后面根据实际情况，跟上任意个 int，表示跳转到的状态
	的偏移地址。跳转位置一定是个 DFAM 指令。
*/
	XyReProgramPC nextJump = t->pc;
	for (int i = 0; i < XyReProcess::CHAR_SIZE; ++i)
	{
		char ch = reinterpret_cast<const char*>(process->stringPosition())[i];
		nextJump = matchDfa(nextJump, ch);
		if (nextJump == nullptr)
			break;
	}

	if (nextJump == nullptr)
	{
		t->markFailed();
		return;
	}

	t->pc = nextJump;
	const XyReInstruction* ins = (const XyReInstruction*)t->pc;
	if (ins->subDirective == 1)
		t->markSucceeded();
	else
		t->pause();
}

static force_inline void doXrpc(XyReXrpcCallback* callback, XyReThread* &t, XyReThreadPool* pool) {
	const XyReInstruction* inst = (const XyReInstruction*)t->pc;
	t->pcInc();
	XyReThread* nt = pool->allocThread();
	nt->returnThread = t;
	nt->pc = callback->xrpc(inst);
	t = nt;
}
#pragma endregion

//////////////////////////////////////////////////////////////////////////

XyReProcess::XyReProcess(const XyReProgram* program)
	: m_program(program)
	, m_succThread{ 0, nullptr }
{
	m_stringBuffer = nullptr;
	m_lastSearchStoppedPos = nullptr;

	m_isIgnoreCase = false;
	m_isMultiLineMode = false;
	m_isDotMatchNewLine = false;
	m_isUnicodeMode = false;
	m_isSucceeded = false;
	m_isFailed = false;
	m_isMatchWholeString = false;
	m_isJustTestMatch = false;
	m_isNoNeedTestMore = false;

	m_isThreadPoolManaged = true;

	m_lengthCheckEnabled = true;

	m_formerChar = 0;
	m_latterChar = 0;
	m_startIndex = 0;

	m_threadPool = nullptr;

	m_xrpcCallback = nullptr;
}

XyReProcess::~XyReProcess()
{
	if (m_isThreadPoolManaged)
		delete m_threadPool;
}

void XyReProcess::stepThread(XyReThread* &t)
{
	const XyReInstruction* ins = (const XyReInstruction*)t->pc;
	switch (ins->directive)
	{
	case NOOP:
		doNoop(t);
		break;
	case CHAR:
		doChar(ins, t, cch(stringPosition()));
		break;
	case SPLT:
		doSplt(ins, t, m_threadPool, m_workingThreads);
		break;
	case JUMP:
		doJump(ins, t);
		break;
	case SUCC:
		doSucc(this, t);
		break;
	case DELY:
		doDely(this, t, m_delayedThreads);
		break;
	case ABAN:
		doAban(this, ins, t, m_abandonWinners, stringPosition() - stringBegin());
		break;
	case CALL:
		doCall(this, ins, t);
		break;
	case ALLM:
		doAllm(this, t);
		break;
	case SPCE:
		doSpce(this, t);
		break;
	case NSPC:
		doNspc(this, t);
		break;
	case DGIT:
		doDgit(this, t);
		break;
	case NDGT:
		doNdgt(this, t);
		break;
	case WORD:
		doWord(this, t);
		break;
	case NWOD:
		doNwod(this, t);
		break;
	case HEAD:
		doHead(this, t);
		break;
	case TAIL:
		doTail(this, t);
		break;
	case BOND:
		doBond(this, ins, t);
		break;
	case NBND:
		doNbnd(this, ins, t);
		break;
	case LSPM:
		doLspm(this, t);
		break;
	case CLSM:
		doClsm(this, ins, t);
		break;
	case BREF:
		doBref(this, ins, t);
		break;
	case WBRF:
		doWbrf(this, t);
		break;
	case SWCH:
		doSwch(this, ins, t);
		break;
	case SVST:
		doSvst(this, ins, t);
		break;
	case SVED:
		doSved(this, ins, t);
		break;
	case DFAM:
		doDfam(this, t);
		break;
	case LAFG:
		t->pcInc();
		break;
	case XRPC:
		doXrpc(m_xrpcCallback, t, m_threadPool);
		break;
	default:
		assert(!"Not supported directive.");
		t->pcInc();
		break;
	}
}

void XyReProcess::stepThreadLastMatch(XyReThread* t)
{
	const XyReInstruction* ins = (const XyReInstruction*)t->pc;
	switch (ins->directive)
	{
	case CHAR:
	case ALLM:
	case SPCE:
	case NSPC:
	case DGIT:
	case NDGT:
	case WORD:
	case NWOD:
	case CLSM:
	case DFAM:
		t->markFailed();
		break;
	default:
		stepThread(t);
		break;
	}
}

bool XyReProcess::stepChar()
{
	if(m_isSucceeded || m_isFailed)
		return false;

	if (stringPosition() == stringEnd())
	{
		// 尝试是否可以继续加载字符
		if (!m_stringBuffer->loadMoreCharacters()		// 无后续字符了
			|| stringPosition() == stringEnd())			// 有后续字符但是加载为空
		{
			// 进行最后一步尝试
			if (!m_workingThreads.empty())
				runThreads(true);
			checkMatchState();

			return false;
		}
	}

	runThreads(false);
	checkMatchState();
	return !m_isSucceeded && !m_isFailed;
}

bool XyReProcess::startAndSuspend(XyReProgramPC pc)
{
	reset();

	// fast fail
	m_isFailed = true;
	m_isNoNeedTestMore = true;
	// 开始匹配的位置越界，则失败，注意，sp == ep 的时候，表示空串，是可以正常匹配的
	if (stringPosition() > stringEnd())
	{
		if (!m_stringBuffer->loadMoreCharacters())
			return false;
	}

	// 如果正则表达式匹配 ^，且当前位置不在开头或者行首，则失败
	if (m_program->startHeaderMatch)
	{
		wchar_t fc = formerChar();
		if (fc != 0 && (!m_isMultiLineMode || !isLineBreak(fc)))
			return false;
	}
	if (m_lengthCheckEnabled)
	{
		// 如果正则表达式最短匹配长度大于字符串剩余长度，则失败
		if (m_program->leastMatchedLength > (uint32_t)(stringEnd() - stringPosition()))
			return false;
		// 全匹配模式，或者单行模式 $ 结尾的表达式，最大长度小于字符串剩余长度，则失败
		if ((m_isMatchWholeString || !m_isMultiLineMode && m_program->endTailMatch)
			&& m_program->mostMatchedLength < (uint32_t)(stringEnd() - stringPosition()))
			return false;
	}
	// TODO 其它更多的 fast fail 情况

	m_isFailed = false;
	m_isNoNeedTestMore = false;
	// 创建初始线程
	XyReThread* t = m_threadPool->allocThread();
	t->pc = pc ? pc : (XyReProgramPC)m_program->instructions();
	m_workingThreads.push_back(t);

	return true;
}

void XyReProcess::startAtPc(XyReProgramPC pc)
{
	if (!startAndSuspend(pc))
		return;
	uint32_t savedSp = stringPosition() - stringBegin();
	// 逐字符推进
	while (stepChar())
	{
		if (m_isFailed)
		{
			// 预测失败，则恢复 sp 位置为之前的后一个位置
			m_stringBuffer->setSP(savedSp + 1);
			break;
		}

		m_stringBuffer->spInc();
		if (m_isSucceeded)
			break;
	}

	// 检查匹配状态
	assert(m_isSucceeded || m_isFailed);
	m_stringBuffer->spInc();
}

void XyReProcess::start()
{
	startAtPc(nullptr);
}

void XyReProcess::start(const wchar_t* sp)
{
	start(sp - stringBegin());
}

void XyReProcess::start(uint32_t index)
{
	m_stringBuffer->setSP(index);
	start();
}

void XyReProcess::checkMatchState()
{
	if (!m_workingThreads.empty())
		return;

	if (m_isSucceeded || m_isFailed)
		return;

	// working 线程全部预测结束，且有成功预测的结果，则直接返回成功
	if (m_succThread.thread != nullptr)
	{
		m_isSucceeded = true;
		m_isFailed = false;
		return;
	}

	// delay 线程中，依然存在，则从 delay 中恢复一个线程到 working 中
	if (!m_delayedThreads.empty())
	{
		m_workingThreads.push_back(m_delayedThreads.back());
		m_delayedThreads.pop_back();
		return;
	}

	// 所有线程都预测失败了，看 abandon 中有没有成功的线程
	if (m_delayedThreads.empty() && !m_abandonThreads.empty())
	{
		for (auto p : m_abandonWinners)
		{
			MatchedThreadState s = p.second;
			if (!s.thread->isSucceeded)
				continue;

			if (m_succThread.thread == nullptr || m_succThread.matchedLength < s.matchedLength)
				m_succThread = s;
		}

		m_isSucceeded = m_succThread.thread != nullptr;
		m_isFailed = !m_isSucceeded;
		return;
	}

	// 所有都检查完了，那就是预测失败了
	m_isSucceeded = false;
	m_isFailed = true;
}

void XyReProcess::runThreads(bool isFinalTest)
{
	assert(m_suspendThreads.empty());
	// 判断当前是否无法继续推进了
	checkMatchState();
	if (m_isSucceeded || m_isFailed)
		return;

	uint32_t marchedLength = stringPosition() - stringBegin() - m_startIndex;
	// 推进所有 working 线程
	while (!m_workingThreads.empty())
	{
		XyReThread* t = m_workingThreads.front();
		m_workingThreads.pop_front();

		t->markWorking();
		if (isFinalTest)
			while (t->isWorking()) stepThreadLastMatch(t);
		else
			while (t->isWorking()) stepThread(t);

		if (t->isFailed)
		{
			removeThread(t);
		}
		else if (t->isSucceeded)
		{
			// 如果是包含 aban 的线程，放任不管
			if (m_abandonThreads.find(t) != m_abandonThreads.end())
				continue;

			if (marchedLength > m_succThread.matchedLength || m_succThread.thread == nullptr)
			{
				m_succThread.matchedLength = marchedLength;
				m_succThread.thread = t;
			}
		}
		else if (t->isSuspended)
		{
			if (!t->isDelayed)
				m_suspendThreads.push_back(t);
		}
	}
	m_workingThreads.swap(m_suspendThreads);

	// 推进所有 delay 线程
	if (m_delayedThreads.empty())
		return;
	while (!m_delayedThreads.empty())
	{
		XyReThread* t = m_delayedThreads.front();
		m_delayedThreads.pop_front();

		t->markWorking();
		while (t->isWorking())
			stepThread(t);

		if (t->isFailed)
		{
			removeThread(t);
		}
		else if (t->isSucceeded)
		{
			assert(m_abandonThreads.find(t) != m_abandonThreads.end() || marchedLength <= m_succThread.matchedLength);
		}
		else if (t->isSuspended)
		{
			m_suspendThreads.push_back(t);
		}
	}
	m_delayedThreads.swap(m_suspendThreads);
}

void XyReProcess::reset()
{
	if (m_isThreadPoolManaged)
	{
		delete m_threadPool;
		m_threadPool = new XyReThreadPool(m_program);
	}

	m_isSucceeded = false;
	m_isFailed = false;
	m_isNoNeedTestMore = false;

	m_workingThreads.clear();
	m_suspendThreads.clear();
	m_delayedThreads.clear();
	m_abandonWinners.clear();
	m_abandonThreads.clear();
	m_succThread.matchedLength = 0;
	m_succThread.thread = nullptr;

	m_lookBehindCaches.clear();
	m_lookAheadCaches.clear();

	m_startIndex = stringPosition() - stringBegin();
}

void XyReProcess::match()
{
	m_isMatchWholeString = true;
	m_stringBuffer->setSP(0);

	start();

	if (!m_isSucceeded)
		return;

	// 如果并没有匹配到字符串末尾，则认为匹配失败
	if (m_succThread.matchedLength != (uint32_t)(stringEnd() - stringBegin()))
	{
		removeThread(m_succThread.thread);
		m_isSucceeded = false;
		m_isFailed = true;
	}
}

void XyReProcess::startAt(XyReProgramPC pc, const wchar_t* sp)
{
	m_stringBuffer->setSP(sp - stringBegin());
	startAtPc(pc);
}

void XyReProcess::startAt(XyReProgramPC pc, const wchar_t* sp, const wchar_t* ep)
{
	const wchar_t* endPos = stringEnd();
	wchar_t latterChar = m_latterChar;
	m_latterChar = cch(ep);
	m_stringBuffer->resetStringEnd(ep);

	m_isMatchWholeString = true;
	startAt(pc, sp);

	m_stringBuffer->resetStringEnd(endPos);
	m_latterChar = latterChar;

	// 检查匹配长度
	if (!m_isSucceeded)
		return;

	if (m_succThread.matchedLength != (uint32_t)(ep - sp))
	{
		removeThread(m_succThread.thread);
		m_isSucceeded = false;
		m_isFailed = true;
	}
}

XyReProcess* XyReProcess::clone()
{
	XyReProcess* p = new XyReProcess(m_program);

	p->m_stringBuffer = m_stringBuffer;
	p->m_threadPool = m_threadPool;
	p->m_isThreadPoolManaged = false;

	p->m_isIgnoreCase = m_isIgnoreCase;
	p->m_isMultiLineMode = m_isMultiLineMode;
	p->m_isDotMatchNewLine = m_isDotMatchNewLine;
	p->m_isUnicodeMode = m_isUnicodeMode;
	p->m_isSucceeded = m_isSucceeded;
	p->m_isFailed = m_isFailed;
	p->m_isMatchWholeString = m_isMatchWholeString;
	p->m_isJustTestMatch = m_isJustTestMatch;
	p->m_isNoNeedTestMore = m_isNoNeedTestMore;

	p->m_formerChar = m_formerChar;
	p->m_latterChar = m_latterChar;
	p->m_startIndex = m_startIndex;

	p->m_lastSearchStoppedPos = m_lastSearchStoppedPos;

	return p;
}

void XyReProcess::removeThread(XyReThread* t)
{
	while (t != nullptr)
	{
		XyReThread* nt = t->returnThread;
		m_abandonThreads.erase(t);
		for (auto p : m_abandonWinners)
		{
			if (p.second.thread == t)
			{
				m_abandonWinners.erase(p.first);
				break;
			}
		}
		if (m_succThread.thread == t)
			m_succThread.thread = nullptr;
	
		m_threadPool->recycleThread(t);
		t = nt;
	}
}

XyReProcess::LookAroundCache* XyReProcess::findLookBehindCache(XyReProgramPC pc, const wchar_t* sp)
{
	auto it = m_lookBehindCaches.find(make_pair(pc, sp));
	if (it == m_lookBehindCaches.end())
		return nullptr;

	return &it->second;
}

XyReProcess::LookAroundCache* XyReProcess::findLookAheadCache(XyReProgramPC pc, const wchar_t* sp)
{
	auto it = m_lookAheadCaches.find(make_pair(pc, sp));
	if (it == m_lookAheadCaches.end())
		return nullptr;

	return &it->second;
}
