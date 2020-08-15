#pragma once
#include "xiyue_xy_re_lexer.h"
#include "xiyue_xy_re_ast.h"

namespace xiyue
{
	class XyReParser
	{
	public:
		XyReParser() = default;

	public:
		std::shared_ptr<XyReAst> parse(const wchar_t* begin, const wchar_t* end);
		inline std::shared_ptr<XyReAst> parse(const wchar_t* str) { return parse(str, str + wcslen(str)); }
		inline std::shared_ptr<XyReAst> parse(const ConstString& str) { return parse(str.begin(), str.end()); }

	private:
		void parseRegex();
		void processRule(int sym);
		void matchToken(int sym);
		void doAction();
		void parseRegAtom(const XyReToken* token);

		XyReAstNode* allocAstNode(const XyReToken* token);
		XyReAstNode* makeRegexNode(const XyReToken* token);
		XyReAstNode* makeRegSeqNode(const XyReToken* token);
		XyReAstNode* makeRegItemNode(const XyReToken* token);
		XyReAstNode* makeQuantifierNode(const XyReToken* token);
		XyReAstNode* makeCharNode(const XyReToken* token);
		XyReAstNode* makeCharRangeNode(const XyReToken* token);
		XyReAstNode* makeFlagNode(const XyReToken* token, bool on);
		XyReAstNode* makeEscapedCharNode(const XyReToken* token);
		XyReAstNode* makeBackReferenceNode(const XyReToken* token);
		XyReAstNode* makeMetaTypeNode(const XyReToken* token);
		XyReAstNode* makeCharClassNode(const XyReToken* token);
		XyReAstNode* makeGroupNode(const XyReToken* token);

	private:
#ifdef _DEBUG
		// 定义此结构，是为了在调试的时候，可以通过 natvis 观察到对应的枚举值
		struct XyReSymbol
		{
			int value;

			inline operator int() const { return value; }
			inline XyReSymbol(int _val) : value(_val) {}
		};
#else
		typedef int XyReSymbol;
#endif

		const wchar_t* m_start;
		const wchar_t* m_end;
		std::shared_ptr<XyReAst> m_ast;
		XyReLexer* m_lexer;
		bool m_isLooseMode;
		std::vector<XyReSymbol> m_symbols;
		std::vector<XyReAstNode*> m_nodes;
	};
}
