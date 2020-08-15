#include "stdafx.h"
#include "xiyue_xy_re_match_length_calculator.h"

using namespace std;
using namespace xiyue;

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcRegexNode(XyReAstNode* node)
{
	XyReAstNode* branch = node->children;
	if (branch == nullptr)
		return LengthPair(0, 0);

	LengthPair p = calcNode(branch);
	branch = branch->next;
	while (branch)
	{
		LengthPair tp = calcNode(branch);
		p.first = std::min(p.first, tp.first);
		p.second = std::max(p.second, tp.second);
		branch = branch->next;
	}

	return p;
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcBranchNode(XyReAstNode* node)
{
	LengthPair p(0, 0);

	XyReAstNode* seq = node->children;
	while (seq)
	{
		LengthPair tp = calcNode(seq);
		if (tp.first == UINT32_MAX || p.first == UINT32_MAX)
			p.first = UINT32_MAX;
		else
			p.first += tp.first;

		if (tp.second == UINT32_MAX || p.second == UINT32_MAX)
			p.second = UINT32_MAX;
		else
			p.second += tp.second;

		seq = seq->next;
	}

	return p;
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcItemNode(XyReAstNode* node)
{
	XyReAstNode* lastNode = node->getLastChild();

	switch (lastNode->type)
	{
	case XyReAst_star:
	case XyReAst_starPlus:
	case XyReAst_starQuestion:
		calcNode(node->children);
		return LengthPair(0, UINT32_MAX);
	case XyReAst_question:
	case XyReAst_questionPlus:
	case XyReAst_questionQuestion:
		return LengthPair(0, calcNode(node->children).second);
	case XyReAst_plus:
	case XyReAst_plusPlus:
	case XyReAst_plusQuestion:
		return LengthPair(calcNode(node->children).first, UINT32_MAX);
	case XyReAst_repeat:
	{
		LengthPair p = calcNode(node->children);
		p.first = p.first == UINT32_MAX ? p.first : p.first * lastNode->arg1;
		p.second = p.second == UINT32_MAX ? p.second : p.second * lastNode->arg1;
		return p;
	}
	case XyReAst_range:
	case XyReAst_rangePlus:
	case XyReAst_rangeQuestion:
	{
		int start = lastNode->arg1;
		int end = lastNode->arg2;
		LengthPair p = calcNode(node->children);
		p.first = p.first == UINT32_MAX ? p.first : p.first * start;
		if (end < 0)
			p.second = UINT32_MAX;
		else
			p.second = p.second == UINT32_MAX ? p.second : p.second * end;
		return p;
	}
	default:
		return calcNode(lastNode);
	}
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcCharNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcSpaceNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNonSpaceNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcDigitNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNonDigitNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcWordNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNonWordNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcBoundNode(XyReAstNode* /*node*/)
{
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNonBoundNode(XyReAstNode* /*node*/)
{
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcLastPositionNode(XyReAstNode* /*node*/)
{
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcBackReferenceNode(XyReAstNode* node)
{
	auto it = m_matchLengths.find(m_ast->getNodeByGroupId(node->arg1));
	assert(it != m_matchLengths.end());
	return it->second;
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcDotNode(XyReAstNode* /*node*/)
{
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcCaretNode(XyReAstNode* /*node*/)
{
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcDollarNode(XyReAstNode* /*node*/)
{
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcCharClassNode(XyReAstNode* node)
{
	XyReAstNode* child = node->children;
	while (child)
	{
		calcNode(child);
		child = child->next;
	}
	return LengthPair(1, 1);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcGroupNode(XyReAstNode* node)
{
	return calcNode(node->getLastChild());
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcFixedGroupNode(XyReAstNode* node)
{
	return calcNode(node->getLastChild());
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNonCaptureGroupNode(XyReAstNode* node)
{
	return calcNode(node->getLastChild());
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcLookAroudNode(XyReAstNode* node)
{
	calcNode(node->children);
	return LengthPair(0, 0);
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::calcNode(XyReAstNode* node)
{
	LengthPair p(0, 0);
	switch (node->type)
	{
	case XyReAst_regex:
		p = calcRegexNode(node);
		break;
	case XyReAst_branch:
		p = calcBranchNode(node);
		break;
	case XyReAst_item:
		p = calcItemNode(node);
		break;
	case XyReAst_star:
	case XyReAst_starPlus:
	case XyReAst_starQuestion:
	case XyReAst_question:
	case XyReAst_questionPlus:
	case XyReAst_questionQuestion:
	case XyReAst_plus:
	case XyReAst_plusPlus:
	case XyReAst_plusQuestion:
	case XyReAst_repeat:
	case XyReAst_range:
	case XyReAst_rangePlus:
	case XyReAst_rangeQuestion:
		break;
	case XyReAst_char:
	case XyReAst_charRange:
		p = calcCharNode(node);
		break;
	case XyReAst_flagOn:
	case XyReAst_flagOff:
		break;
	case XyReAst_space:
		p = calcSpaceNode(node);
		break;
	case XyReAst_nonSpace:
		p = calcNonSpaceNode(node);
		break;
	case XyReAst_boundary:
		p = calcBoundNode(node);
		break;
	case XyReAst_nonBoundary:
		p = calcNonBoundNode(node);
		break;
	case XyReAst_digit:
		p = calcDigitNode(node);
		break;
	case XyReAst_nonDigit:
		p = calcNonDigitNode(node);
		break;
	case XyReAst_word:
		p = calcWordNode(node);
		break;
	case XyReAst_nonWord:
		p = calcNonWordNode(node);
		break;
	case XyReAst_lastPosMatch:
		p = calcLastPositionNode(node);
		break;
	case XyReAst_backReference:
		p = calcBackReferenceNode(node);
		break;
	case XyReAst_dot:
		p = calcDotNode(node);
		break;
	case XyReAst_caret:
		p = calcCaretNode(node);
		break;
	case XyReAst_dollar:
		p = calcDollarNode(node);
		break;
	case XyReAst_charClass:
	case XyReAst_invCharClass:
		p = calcCharClassNode(node);
		break;
	case XyReAst_group:
		p = calcGroupNode(node);
		break;
	case XyReAst_groupIndex:
		break;
	case XyReAst_fixedGroup:
		p = calcFixedGroupNode(node);
		break;
	case XyReAst_nonCaptureGroup:
		p = calcNonCaptureGroupNode(node);
		break;
	case XyReAst_lookAhead:
	case XyReAst_negLookAhead:
	case XyReAst_lookBehind:
	case XyReAst_negLookBehind:
		p = calcLookAroudNode(node);
		break;
	case XyReAst_embeddedSwitch:
		p = calcGroupNode(node);
		break;
	case XyReAst_switchName:
	case XyReAst_nNoop:
		break;
	}

	m_matchLengths.insert(make_pair(node, p));
	return p;
}

void XyReMatchLengthCalculator::calculate(XyReAst* ast)
{
	m_ast = ast;
	calcNode(ast->getRootNode());
}

XyReMatchLengthCalculator::LengthPair XyReMatchLengthCalculator::getNodeMatchLength(XyReAstNode* node)
{
	auto it = m_matchLengths.find(node);
	assert(it != m_matchLengths.end());
	return it->second;
}
