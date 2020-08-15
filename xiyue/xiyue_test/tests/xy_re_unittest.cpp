#include "pch.h"
#include "xiyue_xy_re.h"

using namespace std;
using namespace xiyue;

TEST(XyReTest, matchBasicTest)
{
	XyRe re = _XR((a|b)*abb);
// 	EXPECT_TRUE(re.match(L"abb"));
// 	EXPECT_TRUE(re.match(L"abbabb"));
// 	EXPECT_TRUE(re.match(L"aabbaababb"));
// 	EXPECT_FALSE(re.match(L"aabba"));
// 	EXPECT_FALSE(re.match(L"abbab"));
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
