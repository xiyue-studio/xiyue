#pragma once
#include "xiyue_const_string.h"

namespace xiyue
{
	enum JsonError
	{
		JsonError_unexpectedToken = 0,
		JsonError_topElementNotList = 1,
		JsonError_topElementNotInt = 2,
		JsonError_topElementNotReal = 3,
		JsonError_topElementNotString = 4,   // ´íÎó»Ö¸´£¬Ç°²å {
		JsonError_topElementNotBoolean = 5,
		JsonError_topElementNotNull = 6,
		JsonError_topElementOnlyOnce = 7,
		JsonError_keyNotObject = 8,
		JsonError_keyNotList = 9,
		JsonError_missingKeyValue = 10,
		JsonError_missingKey = 11,
		JsonError_keyNotInt = 12,
		JsonError_keyNotReal = 13,
		JsonError_keyNotBoolean = 14,
		JsonError_keyNotNull = 15,
		JsonError_unexpectedFileEnd = 16,
		JsonError_missingRBrace = 17,
		JsonError_missingCommaOrRBrace = 18,
		JsonError_missingColon = 19,
		JsonError_redundantComma = 20,    // ´íÎó»Ö¸´£¬ºöÂÔÕâ¸ö¶ººÅ
		JsonError_missingObject = 21,
		JsonError_missingRBracket = 22,
		JsonError_missingTopRBrace = 23,    // ´íÎó»Ö¸´£¬×´Ì¬Ìø 1 »Ö¸´
		JsonError_unclosedString,
		JsonError_unclosedComment
	};

	ConstString JsonError_toString(JsonError errNum, ConstString tokenString);
}
