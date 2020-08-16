#include "stdafx.h"
#include "xiyue_xy_re_program_builder.h"

using namespace std;
using namespace xiyue;

int XyReProgramBuilder::countNNoopNode(XyReAstNode* node)
{
	return node->arg1;
}

void XyReProgramBuilder::buildNNoopNode(XyReAstNode* node)
{
	for (int i = 0; i < node->arg1; ++i)
		generateInstruction(NOOP, 0, 0, 0);
}

void XyReProgramBuilder::buildRegexNode(XyReAstNode* node)
{
	/*
		生成所有的分支，代码如下：

		SPLT, L1, L2
		L1: XXXX
		JUMP L3
		L2: SPLT, L4, L5
		L4: XXXX
		JUMP L3
		L5: XXXX
		L3: XXXX
	*/
	XyReAstNode* branch = node->children;
	if (branch == nullptr)
		return;

	auto it = m_caches.find(node);
	if (it == m_caches.end())
	{
		countRegexNode(node);
		it = m_caches.find(node);
	}
	assert(it != m_caches.end());

	vector<BranchCodeLenStat>* branchCodeLens = (vector<BranchCodeLenStat>*)it->second;
	int l3 = branchCodeLens->back().l3;
	int codeStartSize = (int)m_codes.size();

	int index = 0;
	while (branch)
	{
		if (branch == node->getLastChild())
		{
			buildNode(branch);
			break;
		}

		auto& bcs = branchCodeLens->at(index++);
		int l2 = bcs.branchLen + bcs.jumpLen + bcs.spltLen;
		if (bcs.spltLen > 1)
		{
			generateInstruction(SPLT, 1, 2);
			generateData(l2);
		}
		else
		{
			generateInstruction(SPLT, 0, 1, (uint8_t)l2);
		}

		buildNode(branch);

		int jumpPos = l3 - (int)m_codes.size() + codeStartSize;
		assert(jumpPos != 0);
		if (bcs.jumpLen > 1)
		{
			generateInstruction(JUMP, 1, 0, 0);
			generateData(jumpPos);
		}
		else
		{
			generateInstruction(JUMP, 0, (uint16_t)jumpPos);
		}

		branch = branch->next;
	}
	
	// 删除缓存
	delete branchCodeLens;
#ifdef _DEBUG
	m_caches[node] = nullptr;
#endif
}

int XyReProgramBuilder::countRegexNode(XyReAstNode* node)
{
	// 检查 cache
	vector<BranchCodeLenStat>* branchCodeLens;
	auto it = m_caches.find(node);
	if (it != m_caches.end())
	{
		branchCodeLens = (vector<BranchCodeLenStat>*)it->second;
		return branchCodeLens->back().l3;
	}

	// 快速计算，只有一个 branch 时，直接返回
	if (node->children == nullptr)
		return 0;

	branchCodeLens = new vector<BranchCodeLenStat>;
	m_caches.insert(make_pair(node, branchCodeLens));
	int l3 = 0;
	if (node->children == node->getLastChild())
	{
		branchCodeLens->resize(1);
		l3 = branchCodeLens->back().l3 = countNode(node->children);
		return l3;
	}
	
	XyReAstNode* branch = node->children;
	// 首先，假设 JUMP 和 SPLT 是单个指令长度
	while (branch)
	{
		if (branch == node->getLastChild())
		{
			int lastBlockLen = countNode(branch);
			l3 += lastBlockLen;
			branchCodeLens->emplace_back(BranchCodeLenStat{ lastBlockLen, 0, 0 });
			break;
		}

		int bc = countNode(branch);
		branchCodeLens->emplace_back(BranchCodeLenStat{ bc, 1, 1 });
		l3 += bc + 2;
		branch = branch->next;
	}

	// 重新计算所有的 SPLT 指令
	for (size_t i = 0; i + 1 < branchCodeLens->size(); ++i)
	{
		auto& bcs = branchCodeLens->at(i);
		int l2 = bcs.branchLen + 2;
		if (l2 <= INT8_MAX)
			continue;

		bcs.spltLen = 2;
		++l3;
	}

	if (l3 <= INT16_MAX)
	{
		branchCodeLens->back().l3 = l3;
		return l3;
	}

	// 更新 JUMP 指令长度
	int jumpPos = 0;
	for (size_t i = 0; i + 1 < branchCodeLens->size(); ++i)
	{
		auto& bcs = branchCodeLens->at(i);
		jumpPos += bcs.spltLen + bcs.branchLen;
		if (l3 - jumpPos < INT16_MAX)
			break;

		bcs.jumpLen = 2;
		++l3;
		jumpPos += bcs.jumpLen;
		if (bcs.spltLen > 1)
			continue;

		int l2 = bcs.branchLen + 1 + 2;
		if (l2 <= INT8_MAX)
			continue;

		bcs.spltLen = 2;
		++l3;
	}

	branchCodeLens->back().l3 = l3;
	return l3;
}

int XyReProgramBuilder::countBranchNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	XyReAstNode* seq = node->children;
	if (seq == nullptr)
		return 0;

	int total = 0;
	while (seq)
	{
		total += countNode(seq);
		seq = seq->next;
	}

	m_caches.insert(make_pair(node, reinterpret_cast<void*>(total)));
	return total;
}

void XyReProgramBuilder::buildBranchNode(XyReAstNode* node)
{
	XyReAstNode* seq = node->children;
	while (seq)
	{
		buildNode(seq);
		seq = seq->next;
	}
}

void XyReProgramBuilder::buildItemNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it == m_caches.end())
	{
		countItemNode(node);
		it = m_caches.find(node);
	}
	assert(it != m_caches.end());
	ClosureCodeLenStat* lenStat = (ClosureCodeLenStat*)it->second;

	XyReAstNode* lastNode = node->getLastChild();

	switch (lastNode->type)
	{
	case XyReAst_star:
	case XyReAst_starPlus:
	case XyReAst_starQuestion:
		buildStar(lenStat, node, false, 0, 0);
		break;
	case XyReAst_question:
	case XyReAst_questionPlus:
	case XyReAst_questionQuestion:
		buildQuestion(lenStat, node, false, 0, 0);
		break;
	case XyReAst_plus:
		/**
			Plus 生成的代码如下：

			L1: XXXX
			SPLT, L1, L2
			L2: XXXX
		*/
		buildNode(node->children);
		if (lenStat->spltLen == 2)
		{
			generateInstruction(SPLT, 2, 1);
			generateData(2 - lenStat->totalLen);
		}
		else
		{
			generateInstruction(SPLT, 0, (uint8_t)(1 - lenStat->totalLen), 1);
		}
		break;
	case XyReAst_plusPlus:
		/**
			PlusPlus 生成的代码如下：

			L1: XXXX
			SPLT, L1, L2
			L2: ABAN, ID
			XXXX
		*/
		buildNode(node->children);
		if (lenStat->spltLen == 2)
		{
			generateInstruction(SPLT, 2, 1);
			generateData(2 + lenStat->abanLen - lenStat->totalLen);
		}
		else
		{
			generateInstruction(SPLT, 0, (uint8_t)(1 + lenStat->abanLen - lenStat->totalLen), 1);
		}
		if (lenStat->abanLen == 2)
		{
			uint32_t abanID = getAbanID(node);
			generateInstruction(ABAN, 1, 0);
			generateData(abanID);
		}
		else
		{
			uint32_t abanID = getAbanID(node);
			generateInstruction(ABAN, 0, (uint16_t)abanID);
		}
		break;
	case XyReAst_plusQuestion:
		/**
			PlusQuestion 生成的代码如下：

			L1: XXXX
			SPLT, L2, L3
			L3: DELY
			JUMP L1
			L2: XXXX
		*/
		buildNode(node->children);
		generateInstruction(SPLT, 0, (uint8_t)(1 + 1 + lenStat->jumpLen), 1);
		generateInstruction(DELY, 0, 0, 0);
		if (lenStat->jumpLen == 2)
		{
			generateInstruction(JUMP, 1, 0);
			generateData(2 - lenStat->totalLen);
		}
		else
		{
			generateInstruction(JUMP, 0, (uint16_t)(1 - lenStat->totalLen));
		}
		break;
	case XyReAst_repeat:
		buildRange(lenStat, node, lastNode->arg1, lastNode->arg1);
		break;
	case XyReAst_range:
	case XyReAst_rangePlus:
	case XyReAst_rangeQuestion:
		buildRange(lenStat, node, lastNode->arg1, lastNode->arg2);
		break;
	default:
		buildNode(lastNode);
		break;
	}

	delete lenStat;
#ifdef _DEBUG
	m_caches[node] = nullptr;
#endif
}

void XyReProgramBuilder::calcStarLen(ClosureCodeLenStat* lenStat)
{
	// 如果 jumpLen == 2, 则 atomLen 一定远大于 128，所以，假设 jumpLen == 1
	if (lenStat->atomLen + 1 + lenStat->delyLen > INT8_MAX)
		lenStat->spltLen = 2;
	else
		lenStat->spltLen = 1;
	// 计算 jumpLen
	if (lenStat->atomLen + lenStat->spltLen + lenStat->delyLen > INT16_MAX)
	{
		assert(lenStat->spltLen == 2);
		lenStat->jumpLen = 2;
	}
	else
	{
		lenStat->jumpLen = 1;
	}

	lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->jumpLen + lenStat->abanLen + lenStat->delyLen;
}

void XyReProgramBuilder::calcQuestionLen(ClosureCodeLenStat* lenStat)
{
	if (lenStat->atomLen + lenStat->delyLen > INT8_MAX)
		lenStat->spltLen = 2;
	else
		lenStat->spltLen = 1;

	lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->jumpLen + lenStat->abanLen + lenStat->delyLen;
}

int XyReProgramBuilder::countItemNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	ClosureCodeLenStat* lenStat = nullptr;
	if (it != m_caches.end())
	{
		lenStat = (ClosureCodeLenStat*)it->second;
		return lenStat->totalLen;
	}

	lenStat = new ClosureCodeLenStat{ 0 };
	m_caches.insert(make_pair(node, lenStat));
	XyReAstNode* lastNode = node->getLastChild();
	lenStat->atomLen = countNode(node->children);

	switch (lastNode->type)
	{
	case XyReAst_star:
		/**
			Star 生成的代码如下：

			L3: SPLT, L1, L2
			L1: XXXX
			JUMP, L3
			L2: XXXX

			长度 = SPLT + CODE + JUMP
		*/
		calcStarLen(lenStat);
		break;
	case XyReAst_starPlus:
		/**
			StarPlus 生成的代码如下：

			L3: SPLT, L1, L2
			L1: XXXX
			JUMP, L3
			L2: ABAN, ID
			XXXX

			长度 = SPLT + CODE + JUMP + ABAN
		*/
		lenStat->abanLen = getAbanID(node) > UINT16_MAX ? 2 : 1;
		calcStarLen(lenStat);
		break;
	case XyReAst_starQuestion:
		/**
			StarQuestion 生成的代码如下：

			L3: SPLT, L2, L1
			L1: DELY
			XXXX
			JUMP, L3
			L2: XXXX

			长度 = SPLT + CODE + JUMP + DELY(1)
		*/
		lenStat->delyLen = 1;
		calcStarLen(lenStat);
		break;
	case XyReAst_question:
		/**
			Question 生成的代码如下：

			SPLT, L1, L2
			L1: XXXX
			L2: XXXX

			长度 = SPLT + CODE
		*/
		calcQuestionLen(lenStat);
		break;
	case XyReAst_questionPlus:
		/**
			QuestionPlus 生成的代码如下：

			SPLT, L1, L2
			L1: XXXX
			L2: ABAN, ID
			XXXX

			长度 = SPLT + CODE + ABAN
		*/
		lenStat->abanLen = getAbanID(node) > UINT16_MAX ? 2 : 1;
		calcQuestionLen(lenStat);
		break;
	case XyReAst_questionQuestion:
		/**
			QuestionQuestion 生成的代码如下：

			SPLT, L2, L1
			L1: DELY
			XXXX
			L2: XXXX

			长度 = SPLT + CODE + DELY
		*/
		lenStat->delyLen = 1;
		calcQuestionLen(lenStat);
		break;
	case XyReAst_plus:
		/**
			Plus 生成的代码如下：

			L1: XXXX
			SPLT, L1, L2
			L2: XXXX

			长度 = SPLT + CODE
		*/
		lenStat->spltLen = lenStat->atomLen > INT8_MAX ? 2 : 1;
		lenStat->totalLen = lenStat->atomLen + lenStat->spltLen;
		break;
	case XyReAst_plusPlus:
		/**
			PlusPlus 生成的代码如下：

			L1: XXXX
			SPLT, L1, L2
			L2: ABAN, ID
			XXXX

			长度 = SPLT + CODE + ABAN
		*/
		lenStat->spltLen = lenStat->atomLen > INT8_MAX ? 2 : 1;
		lenStat->abanLen = getAbanID(node) > UINT16_MAX ? 2 : 1;
		lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->abanLen;
		break;
	case XyReAst_plusQuestion:
		/**
			PlusQuestion 生成的代码如下：

			L1: XXXX
			SPLT, L2, L3
			L3: DELY
			JUMP L1
			L2: XXXX

			长度 = SPLT + CODE + DELY + JUMP
		*/
		lenStat->spltLen = 1;
		lenStat->delyLen = 1;
		lenStat->jumpLen = lenStat->atomLen + 1 + 1 > INT16_MAX ? 2 : 1;
		lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->delyLen + lenStat->jumpLen;
		break;
	case XyReAst_repeat:
		/**
			Repeat 生成的代码直接复制 n 次。

			长度 = CODE * n
		*/
		lenStat->totalLen = lenStat->atomLen * lastNode->arg1;
		break;
	case XyReAst_range:
		/**
			Range 按照 a{n}a?...a? 生成。
			如果 range 右开，则右边生成 a*。
			下同。

			长度 = CODE * n + CODE * m + SPLT * m
			长度(a*) = CODE * n + SPLT + CODE + JUMP
		*/
		if (lastNode->arg2 < 0)
		{
			calcStarLen(lenStat);
			lenStat->totalLen += lenStat->atomLen * lastNode->arg1;
		}
		else
		{
			calcQuestionLen(lenStat);
			lenStat->totalLen = lenStat->atomLen * lastNode->arg1 + lenStat->totalLen * (lastNode->arg2 - lastNode->arg1);
		}
		break;
	case XyReAst_rangePlus:
		/**
			a{n}a?+...a?+

			长度 = CODE * n + (SPLT + CODE + ABAN) * m
			长度(a*+) = CODE * n + SPLT + CODE + JUMP + ABAN
		*/
		lenStat->abanLen = getAbanID(node) > UINT16_MAX ? 2 : 1;
		if (lastNode->arg2 < 0)
		{
			calcStarLen(lenStat);
			lenStat->totalLen += lenStat->atomLen * lastNode->arg1;
		}
		else
		{
			calcQuestionLen(lenStat);
			lenStat->totalLen = lenStat->atomLen * lastNode->arg1 + lenStat->totalLen * (lastNode->arg2 - lastNode->arg1);
		}
		break;
	case XyReAst_rangeQuestion:
		/**
			a{n}a??...a??

			长度 = CODE * n + (SPLT + CODE + DELY) * m
			长度(a*?) = CODE * n + (SPLT + CODE + JUMP + DELY(1)) * m
		*/
		lenStat->delyLen = 1;
		if (lastNode->arg2 < 0)
		{
			calcStarLen(lenStat);
			lenStat->totalLen += lenStat->atomLen * lastNode->arg1;
		}
		else
		{
			calcQuestionLen(lenStat);
			lenStat->totalLen = lenStat->atomLen * lastNode->arg1 + lenStat->totalLen * (lastNode->arg2 - lastNode->arg1);
		}
		break;
	default:
		assert(lastNode == node->children);
		lenStat->totalLen = lenStat->atomLen;
		break;
	}

	return lenStat->totalLen;
}

void XyReProgramBuilder::buildStar(ClosureCodeLenStat* lenStat, XyReAstNode* node, bool clone, size_t cloneStart, size_t cloneEnd)
{
	int starTotal = lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->jumpLen + lenStat->delyLen;
	if (lenStat->spltLen == 2)
	{
		generateInstruction(SPLT, (uint8_t)(lenStat->delyLen + 1), 1);
		generateData(starTotal);
	}
	else
	{
		if (lenStat->delyLen == 1)
			generateInstruction(SPLT, 0, (uint8_t)starTotal, 1);
		else
			generateInstruction(SPLT, 0, 1, (uint8_t)starTotal);
	}
	if (lenStat->delyLen == 1)
		generateInstruction(DELY, 0, 0, 0);

	if (clone)
		generateByClone(cloneStart, cloneEnd);
	else
		buildNode(node->children);

	int offset = lenStat->jumpLen - starTotal;
	if (lenStat->jumpLen == 2)
	{
		generateInstruction(JUMP, 1, 0);
		generateData((uint32_t)offset);
	}
	else
	{
		generateInstruction(JUMP, 0, (uint16_t)offset);
	}
	if (lenStat->abanLen == 2)
	{
		uint32_t abanID = getAbanID(node);
		generateInstruction(ABAN, 1, 0, 0);
		generateData(abanID);
	}
	else if (lenStat->abanLen == 1)
	{
		uint32_t abanID = getAbanID(node);
		generateInstruction(ABAN, 0, (uint16_t)abanID);
	}
}

void XyReProgramBuilder::buildQuestion(ClosureCodeLenStat* lenStat, XyReAstNode* node, bool clone, size_t cloneStart, size_t cloneEnd)
{
	int questionTotal = lenStat->totalLen = lenStat->atomLen + lenStat->spltLen + lenStat->jumpLen + lenStat->delyLen;

	if (lenStat->spltLen == 2)
	{
		generateInstruction(SPLT, uint8_t(1 + lenStat->delyLen), 1);
		generateData(questionTotal);
	}
	else if (lenStat->delyLen == 1)
	{
		generateInstruction(SPLT, 0, (uint8_t)questionTotal, 1);
	}
	else
	{
		generateInstruction(SPLT, 0, 1, (uint8_t)questionTotal);
	}

	if (lenStat->delyLen == 1)
		generateInstruction(DELY, 0, 0, 0);

	if (clone)
		generateByClone(cloneStart, cloneEnd);
	else
		buildNode(node->children);

	if (lenStat->abanLen == 0)
		return;

	uint32_t abanID = getAbanID(node);
	if (lenStat->abanLen == 2)
	{
		generateInstruction(ABAN, 1, 0);
		generateData(abanID);
	}
	else
	{
		generateInstruction(ABAN, 0, (uint16_t)abanID);
	}
}

void XyReProgramBuilder::buildRange(ClosureCodeLenStat* lenStat, XyReAstNode* node, int start, int end)
{
	if (start == end && end == 0)
		return;

	if (start == end && end == 1)
	{
		buildNode(node->children);
		return;
	}

	int repeatNum = start;
	int closureNum = end < 0 ? end : end - start;

	size_t cloneStart = m_codes.size();
	size_t cloneEnd = cloneStart;
	bool codeIsGenerated = false;
	if (repeatNum > 0)
	{
		buildNode(node->children);
		cloneEnd = m_codes.size();
		for (int i = 1; i < repeatNum; ++i)
		{
			generateByClone(cloneStart, cloneEnd);
		}
		codeIsGenerated = true;
	}

	if (closureNum == 0)
		return;

	if (end < 0)
	{
		buildStar(lenStat, node, codeIsGenerated, cloneStart, cloneEnd);
		return;
	}

	assert(closureNum > 0);
	size_t newCloneStart = m_codes.size();
	size_t newCloneEnd = newCloneStart;
	buildQuestion(lenStat, node, codeIsGenerated, cloneStart, cloneEnd);
	newCloneEnd = m_codes.size();

	for (int i = 1; i < closureNum; ++i)
	{
		generateByClone(newCloneStart, newCloneEnd);
	}
}

int XyReProgramBuilder::countCharNode(XyReAstNode* node)
{
	if (std::abs(node->arg1) > UINT16_MAX)
		return 2;
	return 1;
}

void XyReProgramBuilder::buildCharNode(XyReAstNode* node)
{
	if (std::abs(node->arg1) <= UINT16_MAX)
	{
		generateInstruction(CHAR, 0, (uint16_t)node->arg1);
		return;
	}

	generateInstruction(CHAR, 1, 0, 0);
	generateData(node->arg1);
}

int XyReProgramBuilder::countSpaceNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildSpaceNode(XyReAstNode* /*node*/)
{
	generateInstruction(SPCE, 0, 0, 0);
}

int XyReProgramBuilder::countNonSpaceNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildNonSpaceNode(XyReAstNode* /*node*/)
{
	generateInstruction(NSPC, 0, 0, 0);
}

int XyReProgramBuilder::countDigitNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildDigitNode(XyReAstNode* /*node*/)
{
	generateInstruction(DGIT, 0, 0, 0);
}

int XyReProgramBuilder::countNonDigitNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildNonDigitNode(XyReAstNode* /*node*/)
{
	generateInstruction(NDGT, 0, 0, 0);
}

int XyReProgramBuilder::countWordNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildWordNode(XyReAstNode* /*node*/)
{
	generateInstruction(WORD, 0, 0, 0);
}

int XyReProgramBuilder::countNonWordNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildNonWordNode(XyReAstNode* /*node*/)
{
	generateInstruction(NWOD, 0, 0, 0);
}

int XyReProgramBuilder::countBoundNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildBoundNode(XyReAstNode* /*node*/)
{
	generateInstruction(BOND, 0, 0, 0);
}

int XyReProgramBuilder::countNonBoundNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildNonBoundNode(XyReAstNode* /*node*/)
{
	generateInstruction(NBND, 0, 0, 0);
}

int XyReProgramBuilder::countLastPositionNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildLastPositionNode(XyReAstNode* /*node*/)
{
	generateInstruction(LSPM, 0, 0, 0);
}

int XyReProgramBuilder::countBackReferenceNode(XyReAstNode* node)
{
	int refGroup = node->arg1;
	if (std::abs(refGroup) > INT16_MAX)
		return 2 + 1;
	return 2;
}

void XyReProgramBuilder::buildBackReferenceNode(XyReAstNode* node)
{
	int refGroup = node->arg1;
	if (std::abs(refGroup) > INT16_MAX)
	{
		generateInstruction(BREF, 1, 0);
		generateData(refGroup);
	}
	else
	{
		generateInstruction(BREF, 0, (uint16_t)(int16_t)refGroup);
	}
	generateInstruction(WBRF, 0, 0, 0);
}

int XyReProgramBuilder::countDotNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildDotNode(XyReAstNode* /*node*/)
{
	generateInstruction(ALLM, 0, 0, 0);
}

int XyReProgramBuilder::countCaretNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildCaretNode(XyReAstNode* /*node*/)
{
	generateInstruction(HEAD, 0, 0, 0);
}

int XyReProgramBuilder::countDollarNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildDollarNode(XyReAstNode* /*node*/)
{
	generateInstruction(TAIL, 0, 0, 0);
}

int XyReProgramBuilder::countCharClassNode(XyReAstNode* node)
{
	CharClassLenStat* lenStat = nullptr;
	auto it = m_caches.find(node);
	if (it != m_caches.end())
	{
		lenStat = (CharClassLenStat*)it->second;
		return lenStat->totalLen;
	}

	lenStat = new CharClassLenStat;
	lenStat->totalLen = 1;
	XyReAstNode* subNode = node->children;
	while (subNode)
	{
		if (subNode->type == XyReAst_char)
			lenStat->chars.insert(subNode->arg1);
		else
			lenStat->ranges.insert(make_pair(subNode->arg1, subNode->arg2));

		subNode = subNode->next;
	}

	if (lenStat->chars.size() > UINT8_MAX || lenStat->ranges.size() > UINT8_MAX)
		lenStat->totalLen = 3;

	lenStat->totalLen += (int)lenStat->chars.size() + (int)lenStat->ranges.size() * 2;
	m_caches.insert(make_pair(node, lenStat));
	return lenStat->totalLen;
}

void XyReProgramBuilder::buildCharClassNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it == m_caches.end())
	{
		countCharClassNode(node);
		it = m_caches.find(node);
	}
	assert(it != m_caches.end());
	CharClassLenStat* lenStat = (CharClassLenStat*)it->second;

	if (lenStat->chars.size() > UINT8_MAX || lenStat->ranges.size() > UINT8_MAX)
	{
		generateInstruction(CLSM, node->type == XyReAst_charClass ? 2 : 3, 0);
		generateData((uint32_t)lenStat->ranges.size());
		generateData((uint32_t)lenStat->chars.size());
	}
	else
	{
		generateInstruction(CLSM, node->type == XyReAst_charClass ? 0 : 1, (uint8_t)lenStat->ranges.size(), (uint8_t)lenStat->chars.size());
	}

	for (auto p : lenStat->ranges)
	{
		generateData(p.first);
		generateData(p.second);
	}

	for (int c : lenStat->chars)
	{
		generateData(c);
	}

	delete lenStat;
#ifdef _DEBUG
	m_caches[node] = nullptr;
#endif // _DEBUG
}

int XyReProgramBuilder::countGroupNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	int groupNum = node->children->arg1;
	int groupLen = std::abs(groupNum) > INT16_MAX ? 2 : 1;
	int codeLen = countNode(node->getLastChild());
	int total = codeLen + groupLen * 2;
	m_caches.insert(make_pair(node, reinterpret_cast<void*>(total)));
	return total;
}

void XyReProgramBuilder::buildGroupNode(XyReAstNode* node)
{
	int groupNum = node->children->arg1;
	int groupLen = std::abs(groupNum) > INT16_MAX ? 2 : 1;

	if (groupLen == 2)
	{
		generateInstruction(SVST, 1, 0);
		generateData(groupNum);
	}
	else
	{
		generateInstruction(SVST, 0, (uint16_t)groupNum);
	}

	buildNode(node->getLastChild());

	if (groupLen == 2)
	{
		generateInstruction(SVED, 1, 0);
		generateData(groupNum);
	}
	else
	{
		generateInstruction(SVED, 0, (uint16_t)groupNum);
	}
}

int XyReProgramBuilder::countFixedGroupNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	uint32_t abanID = getAbanID(node);
	int len = countNode(node->children);
	len += abanID > UINT16_MAX ? 2 : 1;
	m_caches.insert(make_pair(node, reinterpret_cast<void*>(len)));
	return len;
}

void XyReProgramBuilder::buildFixedGroupNode(XyReAstNode* node)
{
	uint32_t abanID = getAbanID(node);
	buildNode(node->children);
	if (abanID > UINT16_MAX)
	{
		generateInstruction(ABAN, 1, 0);
		generateData(abanID);
	}
	else
	{
		generateInstruction(ABAN, 0, (uint16_t)abanID);
	}
}

int XyReProgramBuilder::countNonCaptureGroupNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	int len = countNode(node->children);
	m_caches.insert(make_pair(node, reinterpret_cast<void*>(len)));
	return len;
}

void XyReProgramBuilder::buildNonCaptureGroupNode(XyReAstNode* node)
{
	buildNode(node->children);
}

int XyReProgramBuilder::countLookBehindNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	int codeLen = countNode(node->children) + 1 /* SUCC */ + 1 /* minMatchLen */;
	int returnOffsetLen = codeLen >= INT8_MAX ? 3 : 1;
	int len = codeLen + returnOffsetLen;
	m_caches.insert(make_pair(node, reinterpret_cast<void*>(len)));
	return len;
}

void XyReProgramBuilder::buildLookBehindNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	int totalLen = it == m_caches.end() ? countLookBehindNode(node) : reinterpret_cast<int>(it->second);
	bool isNegtive = node->type == XyReAst_negLookBehind;
	if (totalLen <= INT8_MAX)
	{
		generateInstruction(CALL, isNegtive ? 1 : 0, 2, (uint8_t)totalLen);
	}
	else
	{
		generateInstruction(CALL, isNegtive ? 7 : 6, 0);
		generateData(4);
		generateData(totalLen);
	}

	uint32_t minMatchedLen = m_lengthCalculator.getNodeMatchLength(node->children).first;
	generateData(minMatchedLen);

	buildNode(node->children);

	generateInstruction(SUCC, 0, 0, 0);
}

int XyReProgramBuilder::countLookAheadNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	int len = countNode(node->children) + 1 /* SUCC */ + 1 /* CALL */;
	auto p = m_lengthCalculator.getNodeMatchLength(node->children);
	if (p.first == p.second && p.first != UINT32_MAX)
	{
		// 定长
		len += 1;
	}
	else
	{
		// 不定长
		len += 2;
	}
	if (len > INT8_MAX)
		len += 2;

	m_caches.insert(make_pair(node, reinterpret_cast<void*>(len)));
	return len;
}

void XyReProgramBuilder::buildLookAheadNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	int totalLen = it == m_caches.end() ? countLookBehindNode(node) : reinterpret_cast<int>(it->second);
	bool isNegtive = node->type == XyReAst_negLookAhead;
	auto p = m_lengthCalculator.getNodeMatchLength(node->children);
	bool isFixed = p.first == p.second && p.first != UINT32_MAX;
	if (totalLen <= INT8_MAX)
	{
		uint8_t sd = isFixed ? (isNegtive ? 3 : 2) : (isNegtive ? 5 : 4);
		generateInstruction(CALL, sd, isFixed ? 2 : 3, (uint8_t)totalLen);
	}
	else
	{
		uint8_t sd = isFixed ? (isNegtive ? 9 : 8) : (isNegtive ? 11 : 10);
		generateInstruction(CALL, sd, 0);
		generateData(isFixed ? 4 : 5);
		generateData(totalLen);
	}

	generateData(p.first);
	if (!isFixed)
		generateData(p.second);

	buildNode(node->children);

	generateInstruction(SUCC, 0, 0, 0);
}

int XyReProgramBuilder::countEmbeddedSwitchNode(XyReAstNode* node)
{
	auto it = m_caches.find(node);
	if (it != m_caches.end())
		return reinterpret_cast<int>(it->second);

	int len = countNode(node->getLastChild()) + 2;
	m_caches.insert(make_pair(node, reinterpret_cast<void*>(len)));
	return len;
}

void XyReProgramBuilder::buildEmbeddedSwitchNode(XyReAstNode* node)
{
	generateInstruction(SWCH, 0, (uint8_t)node->children->arg1, 0);

	buildNode(node->getLastChild());

	generateInstruction(SWCH, 1, (uint8_t)node->children->arg1, 0);
}

int XyReProgramBuilder::countFlagOnSwitchNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildFlagOnSwitchNode(XyReAstNode* node)
{
	generateInstruction(SWCH, 0, (uint8_t)node->arg1, 0);
}

int XyReProgramBuilder::countFlagOffSwitchNode(XyReAstNode* /*node*/)
{
	return 1;
}

void XyReProgramBuilder::buildFlagOffSwitchNode(XyReAstNode* node)
{
	generateInstruction(SWCH, 1, (uint8_t)node->arg1, 0);
}

void XyReProgramBuilder::buildNode(XyReAstNode* node)
{
	switch (node->type)
	{
	case XyReAst_regex:
		buildRegexNode(node);
		break;
	case XyReAst_branch:
		buildBranchNode(node);
		break;
	case XyReAst_item:
		buildItemNode(node);
		break;
	case XyReAst_char:
		buildCharNode(node);
		break;
	case XyReAst_flagOn:
		buildFlagOnSwitchNode(node);
		break;
	case XyReAst_flagOff:
		buildFlagOffSwitchNode(node);
		break;
	case XyReAst_space:
		buildSpaceNode(node);
		break;
	case XyReAst_nonSpace:
		buildNonSpaceNode(node);
		break;
	case XyReAst_boundary:
		buildBoundNode(node);
		break;
	case XyReAst_nonBoundary:
		buildNonBoundNode(node);
		break;
	case XyReAst_digit:
		buildDigitNode(node);
		break;
	case XyReAst_nonDigit:
		buildNonDigitNode(node);
		break;
	case XyReAst_word:
		buildWordNode(node);
		break;
	case XyReAst_nonWord:
		buildNonWordNode(node);
		break;
	case XyReAst_lastPosMatch:
		buildLastPositionNode(node);
		break;
	case XyReAst_backReference:
		buildBackReferenceNode(node);
		break;
	case XyReAst_dot:
		buildDotNode(node);
		break;
	case XyReAst_caret:
		buildCaretNode(node);
		break;
	case XyReAst_dollar:
		buildDollarNode(node);
		break;
	case XyReAst_charClass:
	case XyReAst_invCharClass:
		buildCharClassNode(node);
		break;
	case XyReAst_group:
		buildGroupNode(node);
		break;
	case XyReAst_fixedGroup:
		buildFixedGroupNode(node);
		break;
	case XyReAst_nonCaptureGroup:
		buildNonCaptureGroupNode(node);
		break;
	case XyReAst_lookAhead:
	case XyReAst_negLookAhead:
		buildLookAheadNode(node);
		break;
	case XyReAst_lookBehind:
	case XyReAst_negLookBehind:
		buildLookBehindNode(node);
		break;
	case XyReAst_embeddedSwitch:
		buildEmbeddedSwitchNode(node);
		break;
	case XyReAst_nNoop:
		buildNNoopNode(node);
		break;
	}
}

int XyReProgramBuilder::countNode(XyReAstNode* node)
{
	switch (node->type)
	{
	case XyReAst_regex:
		return countRegexNode(node);
	case XyReAst_branch:
		return countBranchNode(node);
	case XyReAst_item:
		return countItemNode(node);
	case XyReAst_char:
		return countCharNode(node);
	case XyReAst_flagOn:
		return countFlagOnSwitchNode(node);
	case XyReAst_flagOff:
		return countFlagOffSwitchNode(node);
	case XyReAst_space:
		return countSpaceNode(node);
	case XyReAst_nonSpace:
		return countNonSpaceNode(node);
	case XyReAst_boundary:
		return countBoundNode(node);
	case XyReAst_nonBoundary:
		return countNonBoundNode(node);
	case XyReAst_digit:
		return countDigitNode(node);
	case XyReAst_nonDigit:
		return countNonDigitNode(node);
	case XyReAst_word:
		return countWordNode(node);
	case XyReAst_nonWord:
		return countNonWordNode(node);
	case XyReAst_lastPosMatch:
		return countLastPositionNode(node);
	case XyReAst_backReference:
		return countBackReferenceNode(node);
	case XyReAst_dot:
		return countDotNode(node);
	case XyReAst_caret:
		return countCaretNode(node);
	case XyReAst_dollar:
		return countDollarNode(node);
	case XyReAst_charClass:
	case XyReAst_invCharClass:
		return countCharClassNode(node);
	case XyReAst_group:
		return countGroupNode(node);
	case XyReAst_fixedGroup:
		return countFixedGroupNode(node);
	case XyReAst_nonCaptureGroup:
		return countNonCaptureGroupNode(node);
	case XyReAst_lookAhead:
	case XyReAst_negLookAhead:
		return countLookAheadNode(node);
	case XyReAst_lookBehind:
	case XyReAst_negLookBehind:
		return countLookBehindNode(node);
	case XyReAst_embeddedSwitch:
		return countEmbeddedSwitchNode(node);
	case XyReAst_nNoop:
		return countNNoopNode(node);
	default:
		return 0;
	}
}

uint32_t XyReProgramBuilder::getAbanID(XyReAstNode* node)
{
	auto it = m_abanIDs.find(node);
	if (it == m_abanIDs.end())
	{
		uint32_t id = m_abanIDs.size() + 1;
		m_abanIDs.insert(make_pair(node, id));
		return id;
	}

	return it->second;
}

void XyReProgramBuilder::generateInstruction(XyReDirective d, uint8_t s, uint8_t a1, uint8_t a2)
{
	XyReInstruction ins{ d, s, a1, a2 };
	m_codes.push_back(*reinterpret_cast<uint32_t*>(&ins));
}

void XyReProgramBuilder::generateInstruction(XyReDirective d, uint8_t s, uint16_t a)
{
	XyReInstruction ins{ d, s, uint8_t((a & 0xff00) >> 8), uint8_t(a & 0xff) };
	m_codes.push_back(*reinterpret_cast<uint32_t*>(&ins));
}

void XyReProgramBuilder::generateData(uint32_t d)
{
	m_codes.push_back(d);
}

void XyReProgramBuilder::generateByClone(size_t start, size_t end)
{
	for (size_t i = start; i < end; ++i)
	{
		m_codes.push_back(m_codes[i]);
	}
}

uint32_t* XyReProgramBuilder::build(XyReAst* ast)
{
	vector<uint32_t> metas;
	m_lengthCalculator.calculate(ast);
	m_codes.clear();
	m_caches.clear();
	m_abanIDs.clear();

	buildNode(ast->getRootNode());
	generateInstruction(SUCC, 0, 0, 0);

	metas.push_back(m_codes.size());
	metas.push_back(ast->getNamedGroupCount());
	metas.push_back(ast->getUnnamedGroupCount());
	auto p = m_lengthCalculator.getNodeMatchLength(ast->getRootNode());
	metas.push_back(p.first);
	metas.push_back(p.second);
	metas.push_back(0);

	list<ConstString>& referencedNames = ast->getReferencedNames();
	size_t namesSize = 0;
	for (auto cs : referencedNames)
		namesSize += (cs.length() + 1) * sizeof(wchar_t);
	size_t namesIntSize = (namesSize + 3) / 4;

	uint32_t* data = (uint32_t*)malloc(sizeof(uint32_t) * (metas.size() + m_codes.size() + namesIntSize));
	memcpy(data, metas.data(), sizeof(uint32_t) * metas.size());
	memcpy(data + metas.size(), m_codes.data(), sizeof(uint32_t) * m_codes.size());
	uint32_t* nameData = data + metas.size() + m_codes.size();
	wchar_t* wcsNameData = (wchar_t*)nameData;
	for (auto cs : referencedNames)
	{
		memcpy(wcsNameData, cs.begin(), cs.length() * sizeof(wchar_t));
		wcsNameData += cs.length();
		*wcsNameData = 0;
		++wcsNameData;
	}
	// 对齐内存，以备后续扩展（比如，将原始正则表达式保存下来）
	if ((wcsNameData - (wchar_t*)nameData) % 2 != 0)
		++wcsNameData;
	assert((size_t)((uint32_t*)wcsNameData - data) == metas.size() + m_codes.size() + namesIntSize);

	return data;
}
