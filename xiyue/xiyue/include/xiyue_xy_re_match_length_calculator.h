#pragma once
#include "xiyue_xy_re_ast.h"

namespace xiyue
{
	class XyReMatchLengthCalculator
	{
	public:
		XyReMatchLengthCalculator() = default;
		~XyReMatchLengthCalculator() = default;

	public:
		typedef std::pair<uint32_t, uint32_t> LengthPair;

	public:
		void calculate(XyReAst* ast);
		LengthPair getNodeMatchLength(XyReAstNode* node);

	private:
		LengthPair calcNode(XyReAstNode* node);

		LengthPair calcRegexNode(XyReAstNode* node);
		LengthPair calcBranchNode(XyReAstNode* node);
		LengthPair calcItemNode(XyReAstNode* node);
		LengthPair calcCharNode(XyReAstNode* node);
		LengthPair calcSpaceNode(XyReAstNode* node);
		LengthPair calcNonSpaceNode(XyReAstNode* node);
		LengthPair calcDigitNode(XyReAstNode* node);
		LengthPair calcNonDigitNode(XyReAstNode* node);
		LengthPair calcWordNode(XyReAstNode* node);
		LengthPair calcNonWordNode(XyReAstNode* node);
		LengthPair calcBoundNode(XyReAstNode* node);
		LengthPair calcNonBoundNode(XyReAstNode* node);
		LengthPair calcLastPositionNode(XyReAstNode* node);
		LengthPair calcBackReferenceNode(XyReAstNode* node);
		LengthPair calcDotNode(XyReAstNode* node);
		LengthPair calcCaretNode(XyReAstNode* node);
		LengthPair calcDollarNode(XyReAstNode* node);
		LengthPair calcCharClassNode(XyReAstNode* node);
		LengthPair calcGroupNode(XyReAstNode* node);
		LengthPair calcFixedGroupNode(XyReAstNode* node);
		LengthPair calcNonCaptureGroupNode(XyReAstNode* node);
		LengthPair calcLookAroudNode(XyReAstNode* node);

	private:
		XyReAst* m_ast;
		std::map<XyReAstNode*, LengthPair> m_matchLengths;
	};
}
