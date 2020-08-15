#include "pch.h"
#include "xiyue_xy_re.h"
#include "xiyue_xy_re_parser.h"

using namespace std;
using namespace xiyue;

TEST(XyReParserTest, branchParse)
{
	XyReParser parser;
	auto ast = parser.parse(_XR(ab|cde|f));
	const wchar_t* astJson = LR"({"type":"regex","start":0,"length":8,"children":)"
		LR"([{"type":"branch","start":0,"length":2,"children":[{"type":"item","start":0,"length":1,"children":)"
		LR"([{"type":"char","start":0,"length":1,"char":"a"}]},{"type":"item","start":1,"length":1,"children":)"
		LR"([{"type":"char","start":1,"length":1,"char":"b"}]}]},{"type":"branch","start":3,"length":3,"childr)"
		LR"(en":[{"type":"item","start":3,"length":1,"children":[{"type":"char","start":3,"length":1,"char":"c)"
		LR"("}]},{"type":"item","start":4,"length":1,"children":[{"type":"char","start":4,"length":1,"char":"d")"
		LR"(}]},{"type":"item","start":5,"length":1,"children":[{"type":"char","start":5,"length":1,"char":"e"})"
		LR"(]}]},{"type":"branch","start":7,"length":1,"children":[{"type":"item","start":7,"length":1,"childre)"
		LR"(n":[{"type":"char","start":7,"length":1,"char":"f"}]}]}]})";
	EXPECT_STREQ(astJson, ast->toJsonString().c_str());
}

TEST(XyReParserTest, starClosureParse)
{
	XyReParser parser;
	auto ast = parser.parse(_XR(a*));
	const wchar_t* astJson = LR"({"type":"regex","start":0,"length":2,"children":[{"type":"branch","start":0,"l)"
		LR"(ength":2,"children":[{"type":"item","start":0,"length":2,"children":[{"type":"char","start":0,"leng)"
		LR"(th":1,"char":"a"},{"type":"star","start":1,"length":1}]}]}]})";
	EXPECT_STREQ(astJson, ast->toJsonString().c_str());
}

TEST(XyReParserTest, groupedClosureParse)
{
	XyReParser parser;
	auto ast = parser.parse(_XR((a|b)*?));
	const wchar_t* astJson = LR"({"type":"regex","start":0,"length":7,"children":[{"type":"branch","start":0,"l)"
		LR"(ength":7,"children":[{"type":"item","start":0,"length":7,"children":[{"type":"group","start":0,"len)"
		LR"(gth":4,"groupId":1,"children":[{"type":"regex","start":1,"length":3,"children":[{"type":"branch","s)"
		LR"(tart":1,"length":1,"children":[{"type":"item","start":1,"length":1,"children":[{"type":"char","star)"
		LR"(t":1,"length":1,"char":"a"}]}]},{"type":"branch","start":3,"length":1,"children":[{"type":"item","s)"
		LR"(tart":3,"length":1,"children":[{"type":"char","start":3,"length":1,"char":"b"}]}]}]}]},{"type":"sta)"
		LR"(rQuestion","start":5,"length":2}]}]}]})";
	EXPECT_STREQ(astJson, ast->toJsonString().c_str());
}

TEST(XyReParserTest, rangeParse)
{
	XyReParser parser;
	auto ast = parser.parse(_XR(a{3,19}+));
	const wchar_t* astJson = LR"({"type":"regex","start":0,"length":8,"children":[{"type":"branch","start":0,"le)"
		LR"(ngth":8,"children":[{"type":"item","start":0,"length":8,"children":[{"type":"char","start":0,"length)"
		LR"(":1,"char":"a"},{"type":"rangePlus","start":1,"length":7,"rangeStart":3,"rangeEnd":19}]}]}]})";
	EXPECT_STREQ(astJson, ast->toJsonString().c_str());
}

TEST(XyReParserTest, escapedParse)
{
	XyReParser parser;
	auto ast = parser.parse(_XR(\s));
	const wchar_t* astJson = LR"({"type":"regex","start":0,"length":2,"children":[{"type":"branch","start":0,"len)"
		LR"(gth":2,"children":[{"type":"item","start":0,"length":2,"children":[{"type":"space","start":0,"length")"
		LR"(:2}]}]}]})";
	EXPECT_STREQ(astJson, ast->toJsonString().c_str());
}
