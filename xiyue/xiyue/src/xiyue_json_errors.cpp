#include "stdafx.h"
#include "xiyue_json_errors.h"

using namespace std;
using namespace xiyue;

ConstString xiyue::JsonError_toString(JsonError errNum, ConstString tokenString)
{
	switch (errNum)
	{
	case JsonError_unexpectedToken:
		return ConstString::makeByFormat(L"����������ķ��� `%s`", tokenString.cstr());
	case JsonError_topElementNotList:
		return L"����Ԫ�ز�����һ���б�"_cs;
	case JsonError_topElementNotInt:
	case JsonError_topElementNotReal:
		return L"����Ԫ�ز�����һ������"_cs;
	case JsonError_topElementNotString:
		return L"����Ԫ�ز�����һ���ַ���"_cs;
	case JsonError_topElementNotBoolean:
		return L"����Ԫ�ز�����һ������ֵ"_cs;
	case JsonError_topElementNotNull:
		return L"����Ԫ�ز�����һ�� null ֵ"_cs;
	case JsonError_topElementOnlyOnce:
		return ConstString::makeByFormat(L"����Ԫ��ֻ����һ����������λ������������ķ��� `%s`", tokenString.cstr());
	case JsonError_keyNotObject:
		return L"��ֻ�����ַ�����������һ�� JSON ����"_cs;
	case JsonError_keyNotList:
		return L"��ֻ�����ַ�����������һ���б�"_cs;
	case JsonError_missingKeyValue:
		return L"��ֻ�����ַ�����������һ�� JSON ����"_cs;
	case JsonError_missingKey:
		return L"`,` ǰ��ȱ�ټ�ֵ��Ա����"_cs;
	case JsonError_keyNotInt:
	case JsonError_keyNotReal:
		return L"��ֻ�����ַ�����������һ������"_cs;
	case JsonError_keyNotBoolean:
		return L"��ֻ�����ַ�����������һ����������"_cs;
	case JsonError_keyNotNull:
		return L"��ֻ�����ַ����������� null"_cs;
	case JsonError_unexpectedFileEnd:
		return L"������������ļ�β��JSON �ַ���������"_cs;
	case JsonError_missingRBrace:
		return L"ȱ�� `}`"_cs;
	case JsonError_missingCommaOrRBrace:
		return L"ȱ�� `,` �� `}`"_cs;
	case JsonError_missingColon:
	case JsonError_missingTopRBrace:
		return L"ȱ�� `:`"_cs;
	case JsonError_redundantComma:
		return L"����� `,`"_cs;
	case JsonError_missingObject:
		return L"ȱ�� JSON Ԫ��"_cs;
	case JsonError_missingRBracket:
		return L"ȱ�� `]`"_cs;
	case JsonError_unclosedString:
		return L"δ�պϵ��ַ���"_cs;
	case JsonError_unclosedComment:
		return L"δ�պϵĿ�ע��"_cs;
	default:
		return L"������δ֪����"_cs;
	}
}
