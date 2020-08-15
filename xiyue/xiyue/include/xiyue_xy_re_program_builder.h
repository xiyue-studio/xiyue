#pragma once
#include "xiyue_xy_re_ast.h"
#include "xiyue_xy_re_program.h"
#include "xiyue_xy_re_match_length_calculator.h"

namespace xiyue
{
	class XyReProgramBuilder
	{
	public:
		uint32_t* build(XyReAst* ast);

	private:
		typedef std::vector<uint32_t> CodeBlock;
		struct BranchCodeLenStat
		{
			int branchLen;
			union {
				struct {
					int16_t spltLen;
					int16_t jumpLen;
				};
				int l3;
			};
		};

		struct ClosureCodeLenStat
		{
			uint32_t atomLen;
			uint32_t spltLen : 2;
			uint32_t jumpLen : 2;
			uint32_t abanLen : 2;
			uint32_t delyLen : 2;
			uint32_t reserved : 24;
			uint32_t totalLen;
		};

		struct CharClassLenStat
		{
			std::set<int> chars;
			std::set<std::pair<int, int>> ranges;
			int totalLen;
		};

	private:
		void generateInstruction(XyReDirective d, uint8_t s, uint8_t a1, uint8_t a2);
		void generateInstruction(XyReDirective d, uint8_t s, uint16_t a);
		void generateData(uint32_t d);
		void generateByClone(size_t start, size_t end);

		void buildNode(XyReAstNode* node);
		int countNode(XyReAstNode* node);

		void buildRegexNode(XyReAstNode* node);
		int countRegexNode(XyReAstNode* node);
		void buildBranchNode(XyReAstNode* node);
		int countBranchNode(XyReAstNode* node);
		void buildItemNode(XyReAstNode* node);
		int countItemNode(XyReAstNode* node);
		void buildCharNode(XyReAstNode* node);
		int countCharNode(XyReAstNode* node);
		void buildSpaceNode(XyReAstNode* node);
		int countSpaceNode(XyReAstNode* node);
		void buildNonSpaceNode(XyReAstNode* node);
		int countNonSpaceNode(XyReAstNode* node);
		void buildDigitNode(XyReAstNode* node);
		int countDigitNode(XyReAstNode* node);
		void buildNonDigitNode(XyReAstNode* node);
		int countNonDigitNode(XyReAstNode* node);
		void buildWordNode(XyReAstNode* node);
		int countWordNode(XyReAstNode* node);
		void buildNonWordNode(XyReAstNode* node);
		int countNonWordNode(XyReAstNode* node);
		void buildBoundNode(XyReAstNode* node);
		int countBoundNode(XyReAstNode* node);
		void buildNonBoundNode(XyReAstNode* node);
		int countNonBoundNode(XyReAstNode* node);
		void buildLastPositionNode(XyReAstNode* node);
		int countLastPositionNode(XyReAstNode* node);
		void buildBackReferenceNode(XyReAstNode* node);
		int countBackReferenceNode(XyReAstNode* node);
		void buildDotNode(XyReAstNode* node);
		int countDotNode(XyReAstNode* node);
		void buildCaretNode(XyReAstNode* node);
		int countCaretNode(XyReAstNode* node);
		void buildDollarNode(XyReAstNode* node);
		int countDollarNode(XyReAstNode* node);
		void buildCharClassNode(XyReAstNode* node);
		int countCharClassNode(XyReAstNode* node);
		void buildGroupNode(XyReAstNode* node);
		int countGroupNode(XyReAstNode* node);
		void buildFixedGroupNode(XyReAstNode* node);
		int countFixedGroupNode(XyReAstNode* node);
		void buildNonCaptureGroupNode(XyReAstNode* node);
		int countNonCaptureGroupNode(XyReAstNode* node);
		void buildLookBehindNode(XyReAstNode* node);
		int countLookBehindNode(XyReAstNode* node);
		void buildLookAheadNode(XyReAstNode* node);
		int countLookAheadNode(XyReAstNode* node);
		void buildEmbeddedSwitchNode(XyReAstNode* node);
		int countEmbeddedSwitchNode(XyReAstNode* node);
		void buildFlagOnSwitchNode(XyReAstNode* node);
		int countFlagOnSwitchNode(XyReAstNode* node);
		void buildFlagOffSwitchNode(XyReAstNode* node);
		int countFlagOffSwitchNode(XyReAstNode* node);

		void buildNNoopNode(XyReAstNode* node);
		int countNNoopNode(XyReAstNode* node);

		uint32_t getAbanID(XyReAstNode* node);

		void calcStarLen(ClosureCodeLenStat* lenStat);
		void calcQuestionLen(ClosureCodeLenStat* lenStat);
		void buildStar(ClosureCodeLenStat* lenStat, XyReAstNode* node, bool clone, size_t cloneStart, size_t cloneEnd);
		void buildQuestion(ClosureCodeLenStat* lenStat, XyReAstNode* node, bool clone, size_t cloneStart, size_t cloneEnd);
		void buildRange(ClosureCodeLenStat* lenStat, XyReAstNode* node, int start, int end);

	private:
		XyReMatchLengthCalculator m_lengthCalculator;
		std::vector<uint32_t> m_codes;
		std::map<XyReAstNode*, void*> m_caches;
		std::map<XyReAstNode*, uint32_t> m_abanIDs;
	};

	typedef std::unique_ptr<XyReProgram, decltype(free)*> XyReProgramPtr;
}
