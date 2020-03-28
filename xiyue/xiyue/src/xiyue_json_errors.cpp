#include "stdafx.h"
#include "xiyue_json_errors.h"

using namespace std;
using namespace xiyue;

ConstString xiyue::JsonError_toString(JsonError errNum, ConstString tokenString)
{
	switch (errNum)
	{
	case JsonError_unexpectedToken:
		return ConstString::makeByFormat(L"遇到了意外的符号 `%s`", tokenString.cstr());
	case JsonError_topElementNotList:
		return L"顶层元素不能是一个列表"_cs;
	case JsonError_topElementNotInt:
	case JsonError_topElementNotReal:
		return L"顶层元素不能是一个数字"_cs;
	case JsonError_topElementNotString:
		return L"顶层元素不能是一个字符串"_cs;
	case JsonError_topElementNotBoolean:
		return L"顶层元素不能是一个布尔值"_cs;
	case JsonError_topElementNotNull:
		return L"顶层元素不能是一个 null 值"_cs;
	case JsonError_topElementOnlyOnce:
		return ConstString::makeByFormat(L"顶层元素只能有一个，结束的位置遇到了意外的符号 `%s`", tokenString.cstr());
	case JsonError_keyNotObject:
		return L"键只能是字符串，不能是一个 JSON 对象"_cs;
	case JsonError_keyNotList:
		return L"键只能是字符串，不能是一个列表"_cs;
	case JsonError_missingKeyValue:
		return L"键只能是字符串，不能是一个 JSON 对象"_cs;
	case JsonError_missingKey:
		return L"`,` 前面缺少键值成员定义"_cs;
	case JsonError_keyNotInt:
	case JsonError_keyNotReal:
		return L"键只能是字符串，不能是一个数字"_cs;
	case JsonError_keyNotBoolean:
		return L"键只能是字符串，不能是一个布尔类型"_cs;
	case JsonError_keyNotNull:
		return L"键只能是字符串，不能是 null"_cs;
	case JsonError_unexpectedFileEnd:
		return L"遇到了意外的文件尾，JSON 字符串不完整"_cs;
	case JsonError_missingRBrace:
		return L"缺少 `}`"_cs;
	case JsonError_missingCommaOrRBrace:
		return L"缺少 `,` 或 `}`"_cs;
	case JsonError_missingColon:
	case JsonError_missingTopRBrace:
		return L"缺少 `:`"_cs;
	case JsonError_redundantComma:
		return L"多余的 `,`"_cs;
	case JsonError_missingObject:
		return L"缺少 JSON 元素"_cs;
	case JsonError_missingRBracket:
		return L"缺少 `]`"_cs;
	case JsonError_unclosedString:
		return L"未闭合的字符串"_cs;
	case JsonError_unclosedComment:
		return L"未闭合的块注释"_cs;
	default:
		return L"发生了未知错误"_cs;
	}
}
