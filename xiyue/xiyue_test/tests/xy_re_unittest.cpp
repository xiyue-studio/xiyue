#include "pch.h"
#include "xiyue_xy_re.h"

using namespace std;
using namespace xiyue;

TEST(XyReTest, matchBasicTest)
{
	XyRe re = _XR((a|b)*abb);
	EXPECT_TRUE(re.match(L"abb"));
	EXPECT_TRUE(re.match(L"abbabb"));
	EXPECT_TRUE(re.match(L"aabbaababb"));
	EXPECT_FALSE(re.match(L"aabba"));
	EXPECT_FALSE(re.match(L"abbab"));
}

TEST(XyReTest, sequenceTest)
{
	XyRe re = _XR(abc);
	EXPECT_TRUE(re.match(L"abc"));
	EXPECT_FALSE(re.match(L"abcd"));
	EXPECT_FALSE(re.match(L"abd"));
	EXPECT_FALSE(re.match(L"ab"));
}

TEST(XyReTest, branchTest)
{
	XyRe re = _XR(ab|abcd|efg);
	EXPECT_TRUE(re.match(L"abcd"));
	EXPECT_TRUE(re.match(L"ab"));
	EXPECT_TRUE(re.match(L"efg"));
	EXPECT_FALSE(re.match(L"abc"));
	EXPECT_FALSE(re.match(L"abefg"));
}

TEST(XyReTest, starClosureTest)
{
	XyRe re = _XR(a*);
	EXPECT_TRUE(re.match(L""));
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_TRUE(re.match(L"aaaaaaaaaaaaaaaa"));
	EXPECT_FALSE(re.match(L"aaaaaaaaab"));
	EXPECT_FALSE(re.match(L"baaaaaaaaa"));
}

TEST(XyReTest, plusClosureTest)
{
	XyRe re = _XR(a+);
	EXPECT_FALSE(re.match(L""));
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_TRUE(re.match(L"aaaaaaaaaaaaaaaa"));
	EXPECT_FALSE(re.match(L"aaaaaaaaab"));
	EXPECT_FALSE(re.match(L"baaaaaaaaa"));
}

TEST(XyReTest, questionClosureTest)
{
	XyRe re = _XR(a?);
	EXPECT_TRUE(re.match(L""));
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_FALSE(re.match(L"aa"));
	EXPECT_FALSE(re.match(L"aaaaaaa"));
	EXPECT_FALSE(re.match(L"ba"));
}

TEST(XyReTest, repeatClosureTest)
{
	XyRe re = _XR(a{3});
	EXPECT_TRUE(re.match(L"aaa"));
	EXPECT_FALSE(re.match(L"a"));
	EXPECT_FALSE(re.match(L"aa"));
	EXPECT_FALSE(re.match(L"aaaa"));
	EXPECT_FALSE(re.match(L"aabaaa"));
}

TEST(XyReTest, rangeClosureTest)
{
	XyRe re = _XR(a{1,3});
	EXPECT_TRUE(re.match(L"aaa"));
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_TRUE(re.match(L"aa"));
	EXPECT_FALSE(re.match(L"aaaa"));
	EXPECT_FALSE(re.match(L""));
	EXPECT_FALSE(re.match(L"aabaaa"));
}

TEST(XyReTest, charClassTest)
{
	XyRe re = _XR([a-z]);
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_TRUE(re.match(L"z"));
	EXPECT_TRUE(re.match(L"x"));
	EXPECT_FALSE(re.match(L"az"));
	EXPECT_FALSE(re.match(L""));
	EXPECT_FALSE(re.match(L"B"));
}

TEST(XyReTest, invCharClassTest)
{
	XyRe re =_XR([^a-z]);
	EXPECT_FALSE(re.match(L"a"));
	EXPECT_FALSE(re.match(L"z"));
	EXPECT_FALSE(re.match(L"x"));
	EXPECT_FALSE(re.match(L"az"));
	EXPECT_FALSE(re.match(L""));
	EXPECT_TRUE(re.match(L"B"));
}

TEST(XyReTest, charClassTest2)
{
	XyRe re = _XR([a-z.*+-]);
	EXPECT_TRUE(re.match(L"a"));
	EXPECT_TRUE(re.match(L"z"));
	EXPECT_TRUE(re.match(L"x"));
	EXPECT_TRUE(re.match(L"."));
	EXPECT_TRUE(re.match(L"*"));
	EXPECT_TRUE(re.match(L"+"));
	EXPECT_TRUE(re.match(L"-"));
	EXPECT_FALSE(re.match(L"az"));
	EXPECT_FALSE(re.match(L""));
	EXPECT_FALSE(re.match(L"B"));
}

TEST(XyReTest, possessiveTest)
{
	XyRe re = _XR(\d*+.0);
	EXPECT_FALSE(re.match(L"1230"));
	EXPECT_TRUE(re.match(L"123.0"));
	EXPECT_TRUE(re.match(L"123d0"));
	EXPECT_FALSE(re.match(L"0"));
}

TEST(XyReTest, groupTest)
{
	XyRe re = _XR((ab)c);
	XyReMatch m;
	EXPECT_TRUE(re.match(L"abc", &m));
	EXPECT_EQ(m.getGroupCount(), 1);
	EXPECT_CONST_STRING_EQ(m[0].getMatchedString(), _CS(L"abc"));
	EXPECT_CONST_STRING_EQ(m[1].getMatchedString(), _CS(L"ab"));
}

TEST(XyReTest, backReferenceTest)
{
	XyRe re = _XR((ab)\1);
	EXPECT_TRUE(re.match(L"abab"));
	EXPECT_FALSE(re.match(L"ab"));
	EXPECT_FALSE(re.match(L"abcd"));
	EXPECT_FALSE(re.match(L"ababab"));
}

TEST(XyReTest, lookBehindTest)
{
	XyRe re = _XR((a(?=\d{1,3}).*));
	EXPECT_TRUE(re.match(L"a01b"));
	EXPECT_TRUE(re.match(L"a9"));
	EXPECT_TRUE(re.match(L"a3333"));
	EXPECT_FALSE(re.match(L"abc"));
	EXPECT_FALSE(re.match(L"a"));
}

TEST(XyReTest, fixedLookAheadTest)
{
	XyRe re = _XR(.*(?<=\d\s).*);
	EXPECT_TRUE(re.match(L"a0 a"));
	EXPECT_TRUE(re.match(L"1 "));
	EXPECT_TRUE(re.match(L"abc4 4 a"));
	EXPECT_FALSE(re.match(L"abc 0"));
}

TEST(XyReTest, searchTest_1)
{
	XyRe re = _XR(\d+);
	re.setGlobalSearchMode(true);
	auto matches = re.search(L"abc123def456");
	EXPECT_EQ(matches.size(), 2u);
	EXPECT_CONST_STRING_EQ(matches[0].getMatchedString(), _CS(L"123"));
	EXPECT_CONST_STRING_EQ(matches[1].getMatchedString(), _CS(L"456"));
}

TEST(XyReTest, replaceTest_1)
{
	XyRe re = _XR(case (\w+));
	re.setGlobalSearchMode(true);
	ConstString result = re.replace(L"switch(x){case HELLO: break; case WORLD: break; default: break;}",
		L"case enum_$L$1$E");
	ConstString resultCmp = L"switch(x){case enum_hello: break; case enum_world: break; default: break;}";
	EXPECT_CONST_STRING_EQ(result, resultCmp);
}

TEST(XyReTest, identifierTest)
{
	XyRe re = _XR([a-zA-Z_]\w+);
	EXPECT_TRUE(re.match(L"container"));
}
