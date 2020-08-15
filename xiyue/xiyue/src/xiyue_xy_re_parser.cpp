#include "stdafx.h"
#include "xiyue_xy_re_parser.h"

using namespace std;
using namespace xiyue;

/*
	regex:
		EMPTY
		reg_seq reg_seqs

	reg_seqs:
		EMPTY
		'|' reg_seq req_seqs

	reg_seq:
		reg_item reg_items

	reg_items:
		EMPTY
		reg_item reg_items

	reg_item:
		reg_atom reg_quantifier
		ON_SWITCH
		OFF_SWITCH

	reg_quantifier:
		EMPTY
		'*'
		'+'
		'?'
		'*?'
		'+?'
		'??'
		'*+'
		'++'
		'?+'
		REPEAT
		RANGE
		RANGE_QUESTION
		RANGE_PLUS

	reg_atom:
		CHAR
		escaped_seq
		meta_type
		char_class
		inv_char_class
		capture_group
		fixed_group
		non_capture_group
		look_ahead_group
		look_behind_group
		named_capture_group
		embedded_switch_group

	escaped_seq:
		'\s'
		'\S'
		'\b'
		'\B'
		'\d'
		'\D'
		'\w'
		'\W'
		'\G'
		NUM_BACK_REFERENCE
		NAME_BACK_REFERENCE
		UNICODE_PROPERTY

	meta_type:
		'.'
		'^'
		'$'

	char_class:
		'[' class_chars ']'

	class_chars:
		CHAR_RANGE class_chars
		CHAR class_chars
		EMPTY

	inv_char_class:
		'[^' class_chars ']'

	capture_group:
		'(' regex ')'

	fixed_group:
		'(?>' regex ')'

	non_capture_group:
		'(?:' regex ')'

	look_ahead_group:
		'(?=' regex ')'
		'(?!' regex ')'

	look_behind_group:
		'(?<=' regex ')'
		'(?<!' regex ')'

	named_capture_group:
		L_NAME_CAPTURE_PAREN regex ')'

	embedded_switch_group:
		L_EMBEDDED_CAPTURE_PAREN regex ')'

	first sets:
		regex					: E CHAR ON_SWITCH OFF_SWITCH '\s' '\S' '\b' '\B' '\d' '\D' '\w' '\W' '\G' NUM_BACK_REFERENCE NAME_BACK_REFERNCE UNICODE_PROPERTY '.' '^' '$' '[' '[^'
								  '(' '(?>' '(?:' '(?=' '(?!' '(?<=' '(?<!' L_NAME_CAPTURE_PAREN L_EMBEDDED_CAPTURE_PAREN
		reg_seq					: CHAR ON_SWITCH OFF_SWITCH '\s' '\S' '\b' '\B' '\d' '\D' '\w' '\W' '\G' NUM_BACK_REFERENCE NAME_BACK_REFERNCE UNICODE_PROPERTY '.' '^' '$' '[' '[^'
								  '(' '(?>' '(?:' '(?=' '(?!' '(?<=' '(?<!' L_NAME_CAPTURE_PAREN L_EMBEDDED_CAPTURE_PAREN
		reg_seqs				: E '|'
		reg_item				: CHAR ON_SWITCH OFF_SWITCH '\s' '\S' '\b' '\B' '\d' '\D' '\w' '\W' '\G' NUM_BACK_REFERENCE NAME_BACK_REFERNCE UNICODE_PROPERTY '.' '^' '$' '[' '[^'
								  '(' '(?>' '(?:' '(?=' '(?!' '(?<=' '(?<!' L_NAME_CAPTURE_PAREN L_EMBEDDED_CAPTURE_PAREN
		reg_quantifier			: '*' '+' '?' '*?' '+?' '??' '*+' '++' '?+' REPEAT RANGE RANGE_QUESTION RANGE_PLUS
		reg_atom				: CHAR ON_SWITCH OFF_SWITCH '\s' '\S' '\b' '\B' '\d' '\D' '\w' '\W' '\G' NUM_BACK_REFERENCE NAME_BACK_REFERNCE UNICODE_PROPERTY '.' '^' '$' '[' '[^'
								  '(' '(?>' '(?:' '(?=' '(?!' '(?<=' '(?<!' L_NAME_CAPTURE_PAREN L_EMBEDDED_CAPTURE_PAREN
		escaped_seq				: '\s' '\S' '\b' '\B' '\d' '\D' '\w' '\W' '\G' NUM_BACK_REFERENCE NAME_BACK_REFERNCE UNICODE_PROPERTY
		meta_type				: '.' '^' '$'
		char_class				: '['
		inv_char_class			: '[^'
		capture_group			: '('
		fixed_group				: '(?>'
		non_capture_group		: '(?:'
		look_ahead_group		: '(?=' '(?!'
		look_behind_group		: '(?<=' '(?<!'
		named_capture_group		: L_NAME_CAPTURE_PAREN
		embedded_switch_group	: L_EMBEDDED_CAPTURE_PAREN
*/

enum XyReRule
{
	XyReRule_regex = 1000,
	XyReRule_regSeq,
	XyReRule_regSeqs,
	XyReRule_regItem,
	XyReRule_regItems,
	XyReRule_regQuantifier,
	XyReRule_regAtom,
	XyReRule_escapedSeq,
	XyReRule_metaType,
	XyReRule_charClass,
	XyReRule_invCharClass,
	XyReRule_captureGroup,
	XyReRule_fixedGroup,
	XyReRule_nonCaptureGroup,
	XyReRule_lookAheadGroup,
	XyReRule_lookBehindGroup,
	XyReRule_namedCaptureGroup,
	XyReRule_embeddedSwitchGroup,
	XyReRule_classChars
};

enum XyReAction
{
	XyReAction_pop = 2000
};

static const unordered_set<XyReTokenType> g_regex1SelectSet = { XyReToken_EOF, XyReToken_rParen };
static const unordered_set<XyReTokenType> g_regex2SelectSet = {
	XyReToken_char, XyReToken_lSwitchOnParen, XyReToken_lSwitchOffParen, XyReToken_space, XyReToken_nonSpace,
	XyReToken_boundary, XyReToken_nonBoundary, XyReToken_digit, XyReToken_nonDigit, XyReToken_word,
	XyReToken_nonWord, XyReToken_lastPosMatch, XyReToken_backReference, XyReToken_nameReference, XyReToken_unicodeProperty,
	XyReToken_dot, XyReToken_caret, XyReToken_dollar, XyReToken_lBracket, XyReToken_lBracketCaret,
	XyReToken_lParen, XyReToken_lFixedParen, XyReToken_lNonCaptureParen, XyReToken_lLookAheadParen, XyReToken_lLookAheadNegParen,
	XyReToken_lLookBehindParen, XyReToken_lLookBehindNegParen, XyReToken_lNamedCaptureParen, XyReToken_lEmbeddedSwitchParen
};
static const unordered_set<XyReTokenType>& g_regSeqSelectSet = g_regex2SelectSet;
static const unordered_set<XyReTokenType>& g_regSeqs1SelectSet = g_regex1SelectSet;

static const unordered_set<XyReTokenType>& g_regItems1SelectSet = {
	XyReToken_EOF, XyReToken_rParen, XyReToken_pipe
};
static const unordered_set<XyReTokenType>& g_regItems2SelectSet = g_regex2SelectSet;
static const unordered_set<XyReTokenType>& g_regItem1SelectSet = {
	XyReToken_boundary, XyReToken_nonBoundary, XyReToken_digit, XyReToken_nonDigit, XyReToken_word,
	XyReToken_nonWord, XyReToken_lastPosMatch, XyReToken_backReference, XyReToken_nameReference, XyReToken_unicodeProperty,
	XyReToken_dot, XyReToken_caret, XyReToken_dollar, XyReToken_lBracket, XyReToken_lBracketCaret,
	XyReToken_lParen, XyReToken_lFixedParen, XyReToken_lNonCaptureParen, XyReToken_lLookAheadParen, XyReToken_lLookAheadNegParen,
	XyReToken_lLookBehindParen, XyReToken_lLookBehindNegParen, XyReToken_lNamedCaptureParen, XyReToken_lEmbeddedSwitchParen,
	XyReToken_char, XyReToken_space, XyReToken_nonSpace,
};

static const unordered_set<XyReTokenType> g_regQuantifierSelectSet = {
	XyReToken_star, XyReToken_question, XyReToken_plus,
	XyReToken_starPlus, XyReToken_questionPlus, XyReToken_plusPlus,
	XyReToken_starQuestion, XyReToken_questionQuestion, XyReToken_plusQuestion,
	XyReToken_repeat, XyReToken_range, XyReToken_rangeQuestion, XyReToken_rangePlus
};

static inline bool isInSelectSet(const unordered_set<XyReTokenType>& s, const XyReToken* t) {
	return s.find(t->type) != s.end();
}

//////////////////////////////////////////////////////////////////////////


shared_ptr<XyReAst> XyReParser::parse(const wchar_t* begin, const wchar_t* end)
{
	m_start = begin;
	m_end = end;
	m_ast = make_shared<XyReAst>();
	m_lexer = new XyReLexer(begin, end);
	m_symbols.clear();
	m_nodes.clear();

	parseRegex();

	delete m_lexer;
	auto result = move(m_ast);
	return result;
}

void XyReParser::parseRegex()
{
	m_symbols.push_back(XyReRule_regSeqs);
	m_symbols.push_back(XyReRule_regSeq);

	XyReAstNode* rootNode = m_ast->allocRootNode();
	rootNode->start = 0;
	rootNode->length = m_end - m_start;
	m_nodes.push_back(rootNode);

	while (!m_symbols.empty())
	{
		int sym = m_symbols.back();
		m_symbols.pop_back();

		if (sym >= XyReRule_regex && sym < XyReAction_pop)
			processRule(sym);
		else if (sym < XyReRule_regex)
			matchToken(sym);
		else
			doAction();
	}

	const XyReToken* token = m_lexer->peekToken();
	if (token->type != XyReToken_EOF)
	{
		throw exception();
	}
}

void XyReParser::processRule(int sym)
{
	const XyReToken* token = m_lexer->peekToken();
	switch (sym)
	{
	case XyReRule_regex:
		if (isInSelectSet(g_regex1SelectSet, token))
			break;
		if (isInSelectSet(g_regex2SelectSet, token))
		{
			m_nodes.back()->addChild(makeRegexNode(token));
			m_nodes.push_back(m_nodes.back()->lastChild);
			m_symbols.push_back(XyReAction_pop);
			m_symbols.push_back(XyReRule_regSeqs);
			m_symbols.push_back(XyReRule_regSeq);
		}
		else
		{
			throw exception("regex");
		}
		break;
	case XyReRule_regSeq:
		if (isInSelectSet(g_regSeqSelectSet, token))
		{
			m_nodes.back()->addChild(makeRegSeqNode(token));
			m_nodes.push_back(m_nodes.back()->lastChild);
			m_symbols.push_back(XyReAction_pop);
			m_symbols.push_back(XyReRule_regItems);
			m_symbols.push_back(XyReRule_regItem);
		}
		else
		{
			throw exception("reg_seq");
		}
		break;
	case XyReRule_regSeqs:
		if (isInSelectSet(g_regSeqs1SelectSet, token))
			break;
		if (token->type == XyReToken_pipe)
		{
			m_lexer->consume();
			m_symbols.push_back(XyReRule_regSeqs);
			m_symbols.push_back(XyReRule_regSeq);
		}
		else
		{
			throw exception("reg seqs");
		}
		break;
	case XyReRule_regItems:
		if (isInSelectSet(g_regItems1SelectSet, token))
			break;
		if (isInSelectSet(g_regItems2SelectSet, token))
		{
			m_symbols.push_back(XyReRule_regItems);
			m_symbols.push_back(XyReRule_regItem);
		}
		else
		{
			throw exception("");
		}
		break;
	case XyReRule_regItem:
		if (isInSelectSet(g_regItem1SelectSet, token))
		{
			m_nodes.back()->addChild(makeRegItemNode(token));
			m_nodes.push_back(m_nodes.back()->getLastChild());
			m_symbols.push_back(XyReAction_pop);
			m_symbols.push_back(XyReRule_regQuantifier);
			m_symbols.push_back(XyReRule_regAtom);
		}
		else if (token->type == XyReToken_lSwitchOffParen || token->type == XyReToken_lSwitchOnParen)
		{
			if (token->arg1 == 'x')
				m_lexer->setLooseMode(token->type == XyReToken_lSwitchOnParen ? !m_isLooseMode : m_isLooseMode);
			m_nodes.back()->addChild(makeFlagNode(token, token->type == XyReToken_lSwitchOnParen));
			m_lexer->consume();
		}
		else
		{
			throw exception("");
		}
		break;
	case XyReRule_regQuantifier:
		if (isInSelectSet(g_regQuantifierSelectSet, token))
		{
			m_nodes.back()->addChild(makeQuantifierNode(token));
			m_lexer->consume();
		}
		break;
	case XyReRule_regAtom:
		parseRegAtom(token);
		break;
	case XyReRule_classChars:
		if (token->type == XyReToken_char)
		{
			m_nodes.back()->addChild(makeCharNode(token));
			m_lexer->consume();
			m_symbols.push_back(XyReRule_classChars);
		}
		else if (token->type == XyReToken_charRange)
		{
			m_nodes.back()->addChild(makeCharRangeNode(token));
			m_lexer->consume();
			m_symbols.push_back(XyReRule_classChars);
		}
		else if (token->type != XyReToken_rBracket)
		{
			throw exception("");
		}
		break;
	default:
		throw exception("");
	}
}

void XyReParser::parseRegAtom(const XyReToken* token)
{
	switch (token->type)
	{
	case XyReToken_char:
		m_nodes.back()->addChild(makeCharNode(token));
		m_lexer->consume();
		break;
	case XyReToken_space:
	case XyReToken_nonSpace:
	case XyReToken_boundary:
	case XyReToken_nonBoundary:
	case XyReToken_digit:
	case XyReToken_nonDigit:
	case XyReToken_word:
	case XyReToken_nonWord:
	case XyReToken_lastPosMatch:
		m_nodes.back()->addChild(makeEscapedCharNode(token));
		m_lexer->consume();
		break;
	case XyReToken_backReference:
	case XyReToken_nameReference:
		m_nodes.back()->addChild(makeBackReferenceNode(token));
		m_lexer->consume();
		break;
	case XyReToken_unicodeProperty:
		throw exception("");
		break;
	case XyReToken_dot:
	case XyReToken_caret:
	case XyReToken_dollar:
		m_nodes.back()->addChild(makeMetaTypeNode(token));
		m_lexer->consume();
		break;
	case XyReToken_lBracket:
	case XyReToken_lBracketCaret:
		m_nodes.back()->addChild(makeCharClassNode(token));
		m_nodes.push_back(m_nodes.back()->getLastChild());
		m_symbols.push_back(XyReAction_pop);
		m_symbols.push_back(XyReToken_rBracket);
		m_symbols.push_back(XyReRule_classChars);
		m_lexer->consume();
		break;
	case XyReToken_lParen:
	case XyReToken_lFixedParen:
	case XyReToken_lNonCaptureParen:
	case XyReToken_lLookAheadParen:
	case XyReToken_lLookAheadNegParen:
	case XyReToken_lLookBehindParen:
	case XyReToken_lLookBehindNegParen:
	case XyReToken_lNamedCaptureParen:
	case XyReToken_lEmbeddedSwitchParen:
		m_nodes.back()->addChild(makeGroupNode(token));
		m_nodes.push_back(m_nodes.back()->getLastChild());
		m_lexer->consume();
		m_symbols.push_back(XyReAction_pop);
		m_symbols.push_back(XyReToken_rParen);
		m_symbols.push_back(XyReRule_regex);
		break;
	default:
		throw exception("");
	}
}

XyReAstNode* XyReParser::allocAstNode(const XyReToken* token)
{
	XyReAstNode* node = m_ast->allocNode();
	node->start = token->cursor - m_start;
	node->length = token->length;
	node->next = nullptr;
	return node;
}

XyReAstNode* XyReParser::makeRegexNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = XyReAst_regex;
	node->children = node->lastChild = nullptr;
	return node;
}

XyReAstNode* XyReParser::makeRegSeqNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = XyReAst_branch;
	node->children = node->lastChild = nullptr;
	return node;
}

XyReAstNode* XyReParser::makeRegItemNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = XyReAst_item;
	node->children = node->lastChild = nullptr;
	return node;
}

XyReAstNode* XyReParser::makeQuantifierNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	switch (token->type)
	{
	case XyReToken_star:
		node->type = XyReAst_star;
		break;
	case XyReToken_plus:
		node->type = XyReAst_plus;
		break;
	case XyReToken_question:
		node->type = XyReAst_question;
		break;
	case XyReToken_starQuestion:
		node->type = XyReAst_starQuestion;
		break;
	case XyReToken_plusQuestion:
		node->type = XyReAst_plusQuestion;
		break;
	case XyReToken_questionQuestion:
		node->type = XyReAst_questionQuestion;
		break;
	case XyReToken_starPlus:
		node->type = XyReAst_starPlus;
		break;
	case XyReToken_plusPlus:
		node->type = XyReAst_plusPlus;
		break;
	case XyReToken_questionPlus:
		node->type = XyReAst_questionPlus;
		break;
	case XyReToken_repeat:
		node->type = XyReAst_repeat;
		break;
	case XyReToken_range:
		node->type = XyReAst_range;
		break;
	case XyReToken_rangeQuestion:
		node->type = XyReAst_rangeQuestion;
		break;
	case XyReToken_rangePlus:
		node->type = XyReAst_rangePlus;
		break;
	}
	node->arg1 = token->arg1;
	node->arg2 = token->arg2;

	return node;
}

XyReAstNode* XyReParser::makeCharNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = XyReAst_char;
	node->arg1 = token->arg1;
	return node;
}

XyReAstNode* XyReParser::makeCharRangeNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = XyReAst_charRange;
	node->arg1 = token->arg1;
	node->arg2 = token->arg2;
	return node;
}

XyReAstNode* XyReParser::makeFlagNode(const XyReToken* token, bool on)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = on ? XyReAst_flagOn : XyReAst_flagOff;
	node->arg1 = token->arg1;
	return node;
}

XyReAstNode* XyReParser::makeEscapedCharNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	switch (token->type)
	{
	case XyReToken_space:
		node->type = XyReAst_space;
		break;
	case XyReToken_nonSpace:
		node->type = XyReAst_nonSpace;
		break;
	case XyReToken_boundary:
		node->type = XyReAst_boundary;
		break;
	case XyReToken_nonBoundary:
		node->type = XyReAst_nonBoundary;
		break;
	case XyReToken_digit:
		node->type = XyReAst_digit;
		break;
	case XyReToken_nonDigit:
		node->type = XyReAst_nonDigit;
		break;
	case XyReToken_word:
		node->type = XyReAst_word;
		break;
	case XyReToken_nonWord:
		node->type = XyReAst_nonWord;
		break;
	case XyReToken_lastPosMatch:
		node->type = XyReAst_lastPosMatch;
		break;
	}

	return node;
}

XyReAstNode* XyReParser::makeBackReferenceNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);

	node->type = XyReAst_backReference;
	if (token->type == XyReToken_backReference)
	{
		if ((size_t)token->arg1 > m_ast->getUnnamedGroupCount())
			throw exception();

		node->arg1 = token->arg1;
	}
	else
	{
		ConstString key(m_start + token->arg1, token->arg2);
		int groupId = m_ast->getGroupIdByReferenceName(key);
		if (groupId == 0)
			throw exception();

		node->arg1 = groupId;
	}

	return node;
}

XyReAstNode* XyReParser::makeMetaTypeNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	switch (token->type)
	{
	case XyReToken_dot:
		node->type = XyReAst_dot;
		break;
	case XyReToken_caret:
		node->type = XyReAst_caret;
		break;
	case XyReToken_dollar:
		node->type = XyReAst_dollar;
		break;
	}

	return node;
}

XyReAstNode* XyReParser::makeCharClassNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	node->type = token->type == XyReToken_lBracket ? XyReAst_charClass : XyReAst_invCharClass;
	node->children = node->lastChild = nullptr;
	return node;
}

XyReAstNode* XyReParser::makeGroupNode(const XyReToken* token)
{
	XyReAstNode* node = allocAstNode(token);
	XyReAstNode* prop = nullptr;
	node->children = node->lastChild = nullptr;
	switch (token->type)
	{
	case XyReToken_lParen:
		node->type = XyReAst_group;
		prop = allocAstNode(token);
		prop->type = XyReAst_groupIndex;
		prop->arg1 = m_ast->makeNodeGroupId(node);
		node->addChild(prop);
		return node;
	case XyReToken_lFixedParen:
		node->type = XyReAst_fixedGroup;
		return node;
	case XyReToken_lNonCaptureParen:
		node->type = XyReAst_nonCaptureGroup;
		return node;
	case XyReToken_lLookAheadParen:
		node->type = XyReAst_lookAhead;
		return node;
	case XyReToken_lLookAheadNegParen:
		node->type = XyReAst_negLookAhead;
		return node;
	case XyReToken_lLookBehindParen:
		node->type = XyReAst_lookBehind;
		return node;
	case XyReToken_lLookBehindNegParen:
		node->type = XyReAst_negLookBehind;
		return node;
	case XyReToken_lNamedCaptureParen:
	{
		ConstString referenceName(m_start + token->arg1, token->arg2);
		if (m_ast->getGroupIdByReferenceName(referenceName) < 0)
			throw exception();
		int groupId = m_ast->makeNodeGroupId(node, referenceName);
		node->type = XyReAst_group;
		prop = allocAstNode(token);
		prop->type = XyReAst_groupIndex;
		prop->arg1 = groupId;
		node->addChild(prop);
		return node;
	}
	case XyReToken_lEmbeddedSwitchParen:
		node->type = XyReAst_embeddedSwitch;
		prop = allocAstNode(token);
		prop->type = XyReAst_switchName;
		prop->arg1 = token->arg1;
		node->addChild(prop);
		return node;
	default:
		throw exception();
	}
}

void XyReParser::matchToken(int sym)
{
	const XyReToken* token = m_lexer->peekToken();
	if (token->type != sym)
	{
		throw exception("Token not matched.");
	}

	m_lexer->consume();
}

void XyReParser::doAction()
{
	XyReAstNode* node = m_nodes.back();
	m_nodes.pop_back();

	XyReAstNode* lastChild = node->isLeaf() ? nullptr : node->lastChild;
	if (lastChild == nullptr)
		return;

	node->length = lastChild->start + lastChild->length - node->start;
}
