#include "stdafx.h"
#include "xiyue_xy_re_ast.h"
#include "xiyue_string_format.h"

using namespace std;
using namespace xiyue;

XyReAst::XyReAst()
{
	m_rootNode = nullptr;
}

XyReAstNode* XyReAst::allocNode()
{
	return m_pool.alloc();
}

XyReAstNode* XyReAst::allocRootNode()
{
	assert(m_rootNode == nullptr);
	m_rootNode = allocNode();
	m_rootNode->type = XyReAst_regex;
	m_rootNode->children = m_rootNode->lastChild = m_rootNode->next = nullptr;
	return m_rootNode;
}

XyReAstNode* XyReAst::getNodeByGroupId(int id) const
{
	if (id > 0 && (size_t)id <= m_unnamedGroups.size())
		return m_unnamedGroups[id - 1];
	if (id < 0 && (size_t)-id <= m_namedGroups.size())
		return m_namedGroups[-id - 1];

	return nullptr;
}

XyReAstNode* XyReAst::getNodeByReferenceName(const ConstString& name) const
{
	auto it = m_groupNameMap.find(name);
	if (it == m_groupNameMap.end())
		return nullptr;

	return m_namedGroups[-it->second];
}

int XyReAst::getGroupIdByReferenceName(const ConstString& name) const
{
	auto it = m_groupNameMap.find(name);
	return it == m_groupNameMap.end() ? 0 : it->second;
}

uint32_t XyReAst::getNamedGroupCount() const
{
	return (uint32_t)m_namedGroups.size();
}

uint32_t XyReAst::getUnnamedGroupCount() const
{
	return (uint32_t)m_unnamedGroups.size();
}

int XyReAst::makeNodeGroupId(XyReAstNode* node, const ConstString& name /*= L""_cs*/)
{
	if (name.isEmpty())
	{
		m_unnamedGroups.push_back(node);
		int id = (int)m_unnamedGroups.size();
		return id;
	}
	else
	{
		assert(getGroupIdByReferenceName(name) == 0);
		m_referenceNames.push_back(name);
		int id = -(int)m_referenceNames.size();
		m_groupNameMap.insert(make_pair(name, id));
		m_namedGroups.push_back(node);
		return id;
	}
}

wstring XyReAst::toJsonString()
{
	wstring result;
	m_rootNode->toJsonString(result);
	return move(result);
}

void XyReAstNode::addChild(XyReAstNode* child)
{
	assert(!isLeaf());
	assert(child->next == nullptr);
	if (children == nullptr)
		children = lastChild = child;
	else
		lastChild = lastChild->next = child;
}

bool XyReAstNode::isLeaf() const
{
	static const unordered_set<XyReAstType> leafNodeSet = {
		XyReAst_star, XyReAst_starPlus, XyReAst_starQuestion, XyReAst_question, XyReAst_questionPlus, XyReAst_questionQuestion,
		XyReAst_plus, XyReAst_plusPlus, XyReAst_plusQuestion, XyReAst_repeat, XyReAst_range, XyReAst_rangePlus,
		XyReAst_rangeQuestion, XyReAst_char, XyReAst_charRange, XyReAst_flagOn, XyReAst_flagOff, XyReAst_space,
		XyReAst_nonSpace, XyReAst_boundary, XyReAst_nonBoundary, XyReAst_digit, XyReAst_nonDigit, XyReAst_word,
		XyReAst_nonWord, XyReAst_lastPosMatch, XyReAst_backReference, XyReAst_dot, XyReAst_caret,
		XyReAst_dollar, XyReAst_groupIndex, XyReAst_switchName,
		XyReAst_nNoop
	};

	return leafNodeSet.find(type) != leafNodeSet.end();
}

void XyReAstNode::toJsonString(std::wstring& strBuf)
{
	wchar_t numBuf[16];
	strBuf.append(L"{\"type\":\"").append(XyReAstType_toString(type)).append(L"\"");
	strBuf.append(L",\"start\":").append(itow(numBuf, start));
	strBuf.append(L",\"length\":").append(itow(numBuf, length));
	switch (type)
	{
	case XyReAst_regex:
		buildChildrenJsonString(strBuf);
		break;
	case XyReAst_branch:
		buildChildrenJsonString(strBuf);
		break;
	case XyReAst_item:
		buildChildrenJsonString(strBuf);
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
		break;
	case XyReAst_repeat:
		strBuf.append(L",\"repeatNum\":").append(itow(numBuf, arg1));
		break;
	case XyReAst_range:
	case XyReAst_rangePlus:
	case XyReAst_rangeQuestion:
		strBuf.append(L",\"rangeStart\":").append(itow(numBuf, arg1));
		strBuf.append(L",\"rangeEnd\":").append(itow(numBuf, arg2));
		break;
	case XyReAst_char:
		strBuf.append(L",\"char\":\"");
		strBuf.append(xiyue_escapeCharToDisplay((wchar_t)arg1)).append(L"\"");
		break;
	case XyReAst_charRange:
		strBuf.append(L",\"startChar\":\"");
		strBuf.append(xiyue_escapeCharToDisplay((wchar_t)arg1)).append(L"\"");
		strBuf.append(L",\"endChar\":\"");
		strBuf.append(xiyue_escapeCharToDisplay((wchar_t)arg2)).append(L"\"");
		break;
	case XyReAst_flagOn:
	case XyReAst_flagOff:
		strBuf.append(L",\"flag\":");
		strBuf.push_back((wchar_t)arg1);
		strBuf.append(L"\"");
		break;
	case XyReAst_space:
	case XyReAst_nonSpace:
	case XyReAst_boundary:
	case XyReAst_nonBoundary:
	case XyReAst_digit:
	case XyReAst_nonDigit:
	case XyReAst_word:
	case XyReAst_nonWord:
	case XyReAst_lastPosMatch:
		break;
	case XyReAst_backReference:
		strBuf.append(L",\"referenceId\":").append(itow(numBuf, arg1)).append(L"\"");
		break;
	case XyReAst_dot:
	case XyReAst_caret:
	case XyReAst_dollar:
		break;
	case XyReAst_charClass:
	case XyReAst_invCharClass:
		buildChildrenJsonString(strBuf);
		break;
	case XyReAst_group:
		strBuf.append(L",\"groupId\":").append(itow(numBuf, children->arg1));
		strBuf.append(L",\"children\":[");
		lastChild->toJsonString(strBuf);
		strBuf.push_back(']');
		break;
	case XyReAst_groupIndex:
		break;
	case XyReAst_fixedGroup:
	case XyReAst_nonCaptureGroup:
	case XyReAst_lookAhead:
	case XyReAst_negLookAhead:
	case XyReAst_lookBehind:
	case XyReAst_negLookBehind:
		buildChildrenJsonString(strBuf);
		break;
	case XyReAst_embeddedSwitch:
		strBuf.append(L",\"switch\":\"").push_back((wchar_t)children->arg1);
		strBuf.push_back('"');
		strBuf.append(L",\"children\":[");
		lastChild->toJsonString(strBuf);
		strBuf.push_back(']');
		break;
	case XyReAst_switchName:
	case XyReAst_nNoop:
	default:
		break;
	}
	strBuf.push_back('}');
}

void XyReAstNode::buildChildrenJsonString(std::wstring& strBuf)
{
	strBuf.append(L",\"children\":[");
	XyReAstNode* child = children;
	while (child)
	{
		child->toJsonString(strBuf);
		if (child != lastChild)
			strBuf.push_back(',');

		child = child->next;
	}
	strBuf.push_back(']');
}

const wchar_t* xiyue::XyReAstType_toString(XyReAstType type)
{
	switch (type)
	{
	case XyReAst_regex:
		return L"regex";
	case XyReAst_branch:
		return L"branch";
	case XyReAst_item:
		return L"item";
	case XyReAst_star:
		return L"star";
	case XyReAst_starPlus:
		return L"starPlus";
	case XyReAst_starQuestion:
		return L"starQuestion";
	case XyReAst_question:
		return L"question";
	case XyReAst_questionPlus:
		return L"questionPlus";
	case XyReAst_questionQuestion:
		return L"questionQuestion";
	case XyReAst_plus:
		return L"plus";
	case XyReAst_plusPlus:
		return L"plusPlus";
	case XyReAst_plusQuestion:
		return L"plusQuestion";
	case XyReAst_repeat:
		return L"repeat";
	case XyReAst_range:
		return L"range";
	case XyReAst_rangePlus:
		return L"rangePlus";
	case XyReAst_rangeQuestion:
		return L"rangeQuestion";
	case XyReAst_char:
		return L"char";
	case XyReAst_charRange:
		return L"charRange";
	case XyReAst_flagOn:
		return L"flagOn";
	case XyReAst_flagOff:
		return L"flagOff";
	case XyReAst_space:
		return L"space";
	case XyReAst_nonSpace:
		return L"nonSpace";
	case XyReAst_boundary:
		return L"boundary";
	case XyReAst_nonBoundary:
		return L"nonBoundary";
	case XyReAst_digit:
		return L"digit";
	case XyReAst_nonDigit:
		return L"nonDigit";
	case XyReAst_word:
		return L"word";
	case XyReAst_nonWord:
		return L"nonWord";
	case XyReAst_lastPosMatch:
		return L"lastPosMatch";
	case XyReAst_backReference:
		return L"backReference";
	case XyReAst_dot:
		return L"dot";
	case XyReAst_caret:
		return L"caret";
	case XyReAst_dollar:
		return L"dollar";
	case XyReAst_charClass:
		return L"charClass";
	case XyReAst_invCharClass:
		return L"invCharClass";
	case XyReAst_group:
		return L"group";
	case XyReAst_groupIndex:
		return L"groupIndex";
	case XyReAst_fixedGroup:
		return L"fixedGroup";
	case XyReAst_nonCaptureGroup:
		return L"nonCaptureGroup";
	case XyReAst_lookAhead:
		return L"lookAhead";
	case XyReAst_negLookAhead:
		return L"negLookAhead";
	case XyReAst_lookBehind:
		return L"lookBehind";
	case XyReAst_negLookBehind:
		return L"negLookBehind";
	case XyReAst_embeddedSwitch:
		return L"embeddedSwitch";
	case XyReAst_switchName:
		return L"switchName";
	case XyReAst_nNoop:
		return L"nNoop";
	default:
		return L"";
	}
}
