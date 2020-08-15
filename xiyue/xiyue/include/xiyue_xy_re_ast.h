#pragma once
#pragma warning(disable: 4201)
#include "xiyue_single_object_mem_pool.h"

namespace xiyue
{
	enum XyReAstType
	{
		XyReAst_regex,
		XyReAst_branch,
		XyReAst_item,
		XyReAst_star,
		XyReAst_starPlus,
		XyReAst_starQuestion,
		XyReAst_question,
		XyReAst_questionPlus,
		XyReAst_questionQuestion,
		XyReAst_plus,
		XyReAst_plusPlus,
		XyReAst_plusQuestion,
		XyReAst_repeat,
		XyReAst_range,
		XyReAst_rangePlus,
		XyReAst_rangeQuestion,
		XyReAst_char,
		XyReAst_charRange,
		XyReAst_flagOn,
		XyReAst_flagOff,
		XyReAst_space,
		XyReAst_nonSpace,
		XyReAst_boundary,
		XyReAst_nonBoundary,
		XyReAst_digit,
		XyReAst_nonDigit,
		XyReAst_word,
		XyReAst_nonWord,
		XyReAst_lastPosMatch,
		XyReAst_backReference,
		XyReAst_dot,
		XyReAst_caret,
		XyReAst_dollar,
		XyReAst_charClass,
		XyReAst_invCharClass,
		XyReAst_group,
		XyReAst_groupIndex,
		XyReAst_fixedGroup,
		XyReAst_nonCaptureGroup,
		XyReAst_lookAhead,
		XyReAst_negLookAhead,
		XyReAst_lookBehind,
		XyReAst_negLookBehind,
		XyReAst_embeddedSwitch,
		XyReAst_switchName,

		XyReAst_nNoop = 255				// 为了构建测试而存在的一个 node
	};

	const wchar_t* XyReAstType_toString(XyReAstType type);

	struct TextRange
	{
		int start;
		int length;
	};

	struct XyReAstNode : TextRange
	{
		void addChild(XyReAstNode* child);
		XyReAstNode* getLastChild() const { return lastChild; }

		bool isLeaf() const;

		void toJsonString(std::wstring& strBuf);
		void buildChildrenJsonString(std::wstring& strBuf);

		XyReAstType type;
		union {
			struct {
				XyReAstNode* children;
				XyReAstNode* lastChild;
			};
			struct {
				int arg1;
				int arg2;
			};
		};
		XyReAstNode* next;
	};

	class XyReAst
	{
	public:
		XyReAst();

	public:
		XyReAstNode* allocRootNode();
		XyReAstNode* allocNode();

		inline XyReAstNode* getRootNode() const { return m_rootNode; }

		XyReAstNode* getNodeByGroupId(int id) const;
		XyReAstNode* getNodeByReferenceName(const ConstString& name) const;
		int getGroupIdByReferenceName(const ConstString& name) const;
		uint32_t getNamedGroupCount() const;
		uint32_t getUnnamedGroupCount() const;

		int makeNodeGroupId(XyReAstNode* node, const ConstString& name = ConstString());

		std::list<ConstString>& getReferencedNames() { return m_referenceNames; }

		std::wstring toJsonString();

	private:
		XyReAstNode* m_rootNode;
		std::map<ConstString, int> m_groupNameMap;
		std::vector<XyReAstNode*> m_unnamedGroups;
		std::vector<XyReAstNode*> m_namedGroups;
		std::list<ConstString> m_referenceNames;
		SingleObjectMemPool<XyReAstNode> m_pool;
	};
}
