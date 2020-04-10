#include "stdafx.h"
#include "xiyue_json_parser.h"
#include "xiyue_string_format.h"
#include "xiyue_json_exception.h"

using namespace std;
using namespace xiyue;

/**
	0.  json            = json_object EOF
	1.  json_object     = '{' members '}'
	2.  members         = key_value extra_key_value
	3.  members         = EPSILON
	4.  key_value       = STRING ':' object
	5.  extra_key_value = ',' key_value extra_key_value
	6.  extra_key_value = EPSILON
	7.  json_list       = '[' object_list ']'
	8.  object_list     = object extra_object
	9.  object_list     = EPSILON
	10. extra_object    = ',' object extra_object
	11. extra_object    = EPSILON
	12. object          = INT
	13. object          = REAL
	14. object          = STRING
	15. object          = BOOLEAN
	16. object          = json_object
	17. object          = json_list
	18. object          = NULL
*/

static int g_verbNum[19] = {
	2, 3, 2, 0, 3, 3, 0, 3, 2, 0, 3, 0, 1, 1, 1, 1, 1, 1, 1
};

static int g_reduceToSymbol[19] = {
	0, 1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8
};

#define G 0
#define S 1
#define R 2
#define E 3
namespace xiyue
{
	struct JsonAction
	{
		uint8_t action : 2;
		uint8_t data : 6;
	};
}

static const JsonAction g_jsonActions[28][12] = {
	{ {S, 2}, {E, 0}, {E, 1}, {E, 0}, {E, 0}, {E, 0}, {E, 2}, {E, 3}, {E, 4}, {E, 5}, {E, 6}, {E, 16} },
	{ {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {E, 7}, {G, 0} },
	{ {E, 8}, {R, 3}, {E, 9}, {E, 0}, {E, 10}, {E, 11}, {E, 12}, {E, 13}, {S, 5}, {E, 14}, {E, 15}, {E, 16} },
	{ {E, 17}, {S, 6}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 17}, {E, 16} },
	{ {E, 18}, {R, 6}, {E, 18}, {E, 18}, {S, 8}, {E, 18}, {E, 18}, {E, 18}, {E, 18}, {E, 18}, {E, 18}, {E, 16} },
	{ {E, 19}, {E, 19}, {E, 19}, {E, 19}, {E, 19}, {S, 9}, {E, 19}, {E, 19}, {E, 19}, {E, 19}, {E, 19}, {E, 16} },
	{ {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1}, {R, 1} },
	{ {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2}, {R, 2} },
	{ {E, 8}, {E, 20}, {E, 9}, {E, 0}, {E, 10}, {E, 11}, {E, 12}, {E, 13}, {S, 5}, {E, 14}, {E, 15}, {E, 16} },
	{ {S, 2}, {E, 21}, {S, 19}, {E, 0}, {E, 21}, {E, 0}, {S, 12}, {S, 13}, {S, 14}, {S, 15}, {S, 18}, {E, 16} },
	{ {E, 0}, {R, 6}, {E, 0}, {E, 0}, {S, 8}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 16} },
	{ {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {R, 4}, {E, 16} },
	{ {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12}, {R, 12} },
	{ {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13}, {R, 13} },
	{ {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14}, {R, 14} },
	{ {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15}, {R, 15} },
	{ {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16}, {R, 16} },
	{ {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17}, {R, 17} },
	{ {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18}, {R, 18} },
	{ {S, 2}, {E, 21}, {S, 19}, {R, 9}, {E, 21}, {E, 0}, {S, 12}, {S, 13}, {S, 14}, {S, 15}, {S, 18}, {E, 16} },
	{ {E, 0}, {E, 0}, {E, 0}, {S, 22}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 16} },
	{ {E, 0}, {E, 0}, {E, 0}, {R, 11}, {S, 24}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 16} },
	{ {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 7}, {R, 8} },
	{ {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8}, {R, 8} },
	{ {S, 2}, {E, 0}, {S, 19}, {E, 0}, {E, 0}, {E, 0}, {S, 12}, {S, 13}, {S, 14}, {S, 15}, {S, 18}, {E, 16} },
	{ {E, 0}, {E, 0}, {E, 0}, {R, 11}, {S, 24}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 0}, {E, 16} },
	{ {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10}, {R, 10} },
	{ {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5}, {R, 5} }
};

static const uint8_t g_gotoMap[28][9] = {
	{ 0, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 3, 4, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 7, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 10, 0, 0, 0, 0, 0 },
	{ 0, 16, 0, 0, 0, 17, 0, 0, 11 },
	{ 0, 0, 0, 0, 27, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 16, 0, 0, 0, 17, 20, 0, 21 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 23, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 16, 0, 0, 0, 17, 0, 0, 25 },
	{ 0, 0, 0, 0, 0, 0, 0, 26, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

enum JsonRuleType
{
	JsonRule_json = 100,
	JsonRule_jsonObject,
	JsonRule_members,
	JsonRule_keyValue,
	JsonRule_extraKeyValue,
	JsonRule_jsonList,
	JsonRule_objectList,
	JsonRule_extraObject,
	JsonRule_object
};

static inline bool tokenStringNeeded(JsonTokenType type)
{
	switch (type)
	{
	case JsonToken_int:
	case JsonToken_real:
	case JsonToken_string:
	case JsonToken_boolean:
		return true;
	}

	return false;
}

template<typename T>
static inline void popVectorBackN(vector<T>& v, int n) {
	if (n == 0)
		return;
	v.resize(v.size() - n);
}

JsonParser::JsonParser()
{
	m_ignoreRedundantComma = true;
	m_ignoreTopBrace = true;
	m_firstBraceNotGiven = false;
	m_stringNeedCreate = false;
	m_lexer = nullptr;
}

JsonParser::~JsonParser()
{
	delete m_lexer;
}

void JsonParser::parse()
{
	m_stateStack.clear();
	m_symbolStack.clear();
	m_tokenStack.clear();
	m_rootObjects.clear();

	m_symbolStack.push_back(JsonToken_EOF);
	m_stateStack.push_back(0);

	for (;;)
	{
		const JsonToken& token = m_lexer->peekToken();
		uint8_t state = m_stateStack.back();
		JsonAction action = g_jsonActions[state][token.tokenType - 1];
		switch (action.action)
		{
		case S:
			m_stateStack.push_back(action.data);
			m_symbolStack.push_back((uint8_t)token.tokenType);
			if (tokenStringNeeded(token.tokenType))
				m_tokenStack.push_back(token);
			
			if (token.tokenType == JsonToken_lBrace)
				m_rootObjects.push_back(JsonObject::object(8));
			else if (token.tokenType == JsonToken_lBracket)
				m_rootObjects.push_back(JsonObject::list(8));

			m_lexer->consumeToken();
			break;
		case R:
			callReduce(action.data);
			popVectorBackN(m_symbolStack, g_verbNum[action.data]);
			popVectorBackN(m_stateStack, g_verbNum[action.data]);
			m_symbolStack.push_back(static_cast<uint8_t>(100 + g_reduceToSymbol[action.data]));
			state = g_gotoMap[m_stateStack.back()][g_reduceToSymbol[action.data]];
			assert(state != 0);
			m_stateStack.push_back(state);
			assert(m_stateStack.size() == m_symbolStack.size());
			break;
		case E:
			handleError(action.data);
			break;
		case G:
			return;
		}
	}
}

void JsonParser::callReduce(uint8_t id)
{
	switch (id)
	{
	case 4:
		reduceKeyValue();
		break;
	case 12:
		reduceIntObject();
		break;
	case 13:
		reduceRealObject();
		break;
	case 14:
		reduceStringObject();
		break;
	case 15:
		reduceBooleanObject();
		break;
	case 16:
		reduceObjectObject();
		break;
	case 17:
		reduceListObject();
		break;
	case 18:
		reduceNullObject();
		break;
	}
}

/*
	4.  key_value       = STRING ':' object
*/
void JsonParser::reduceKeyValue()
{
	const JsonToken& token = m_tokenStack.back();
	ConstString key = ConstString::makeUnmanagedString(m_jsonText.data() + token.offset + 1, token.length - 2);
	bool isEscaped = token.isEscapedString;
	if (m_stringNeedCreate)
		key = key.duplicate();

	if (isEscaped)
		key._resetLength(xiyue_unescapeCppStyleInplace(const_cast<wchar_t*>(key.data()), key.length()));
	m_tokenStack.pop_back();

	JsonObject obj = m_rootObjects.back();
	m_rootObjects.pop_back();

	m_rootObjects.back().setMember(key, obj);
}

static json_int_t csToJsonInt(const ConstString& str)
{
	json_int_t result = 0;
	bool isNegtive;
	const wchar_t* p = str.begin();
	const wchar_t* pEnd = p + str.length();

	// 决定符号
	switch (*p)
	{
	case '-':
		isNegtive = true;
		++p;
		break;
	case '+':
		++p;
	default:
		isNegtive = false;
	}

	// 解析数字
	while (p < pEnd)
	{
		result = result * 10 + (*p - '0');
		++p;
	}

	return isNegtive ? -result : result;
}

/*
	12. object          = INT
*/
void JsonParser::reduceIntObject()
{
	const JsonToken& token = m_tokenStack.back();
	ConstString intStr = m_jsonText.substr(token.offset, token.length);
	JsonObject intObj = csToJsonInt(intStr);
	m_tokenStack.pop_back();

	emplaceObject(intObj);
}

/*
	13. object          = REAL
*/
void JsonParser::reduceRealObject()
{
	const JsonToken& token = m_tokenStack.back();
	ConstString realStr = m_jsonText.substr(token.offset, token.length);
	JsonObject realObj = realStr.toDouble();
	m_tokenStack.pop_back();

	emplaceObject(realObj);
}

/*
	14. object          = STRING
*/
void JsonParser::reduceStringObject()
{
	const JsonToken& token = m_tokenStack.back();
	ConstString str = ConstString::makeUnmanagedString(m_jsonText.data() + token.offset + 1, token.length - 2);
	bool isEscaped = token.isEscapedString;
	if (m_stringNeedCreate)
		str = str.duplicate();

	m_tokenStack.pop_back();

	if (isEscaped)
		str._resetLength(xiyue_unescapeCppStyleInplace(const_cast<wchar_t*>(str.data()), str.length()));
	JsonObject strObj(str, m_stringNeedCreate);

	emplaceObject(strObj);
}

/*
	15. object          = BOOLEAN
*/
void JsonParser::reduceBooleanObject()
{
	JsonObject boolObj = m_tokenStack.back().isTrueString;
	m_tokenStack.pop_back();

	emplaceObject(boolObj);
}

/*
	16. object          = json_object
*/
void JsonParser::reduceObjectObject()
{
	if (m_rootObjects.size() == 1)
		return;

	JsonObject& obj = m_rootObjects[m_rootObjects.size() - 2];
	if (obj.isList())
	{
		obj.append(m_rootObjects.back());
		m_rootObjects.pop_back();
	}
}

void JsonParser::emplaceObject(const JsonObject& obj)
{
	if (m_rootObjects.back().isList())
		m_rootObjects.back().append(obj);
	else
		m_rootObjects.push_back(obj);
}

/*
	17. object          = json_list
*/
void JsonParser::reduceListObject()
{
	JsonObject& obj = m_rootObjects[m_rootObjects.size() - 2];
	if (obj.isList())
	{
		obj.append(m_rootObjects.back());
		m_rootObjects.pop_back();
	}
}

/*
	18. object          = NULL
*/
void JsonParser::reduceNullObject()
{
	emplaceObject(JsonObject());
}

void JsonParser::handleError(int errorNum)
{
	if (m_ignoreRedundantComma && errorNum == 20)
	{
		// 需要忽略多余的逗号，则将多余的逗号出栈即可
		m_stateStack.pop_back();
		m_symbolStack.pop_back();
		return;
	}
	
	if (m_ignoreTopBrace)
	{
		if (errorNum == 4)
		{
			// 没有遇到 { 就直接遇到了 STRING(作为 key)，则模拟遇到了 {
			m_symbolStack.push_back(JsonToken_lBrace);
			m_stateStack.push_back(2);
			m_firstBraceNotGiven = true;
			return;
		}
		
		if (errorNum == 23 && m_firstBraceNotGiven)
		{
			// 在期待 } 的时候遇到了文件尾，并且头上的 { 没有给出
			if (m_symbolStack.size() == 3)
			{
				m_symbolStack.push_back(JsonToken_rBrace);
				m_stateStack.push_back(6);
				return;
			}
		}
	}

	// 报告这个错误，并抛出异常
	throw JsonException((JsonError)errorNum, m_lexer->peekToken().offset, m_lexer->peekToken().length);
}

JsonObject JsonParser::parseJson(const ConstString& jsonText)
{
	m_jsonText = jsonText;
	if (m_lexer != nullptr)
		delete m_lexer;
	m_lexer = new JsonLexer(m_jsonText);
	parse();
	assert(m_rootObjects.size() == 1);
	JsonObject result = m_rootObjects.back();
	m_stateStack.clear();
	m_symbolStack.clear();
	m_tokenStack.clear();
	m_rootObjects.clear();
	delete m_lexer;
	m_lexer = nullptr;

	return result;
}
