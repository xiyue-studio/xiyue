#include "pch.h"
#include "xiyue_xy_re_parser.h"
#include "xiyue_xy_re_match_length_calculator.h"
#include "xiyue_xy_re.h"

using namespace std;
using namespace xiyue;

void testXyReMatchLength(const wchar_t* re, uint32_t minLen, uint32_t maxLen = UINT32_MAX)
{
	XyReParser parser;
	XyReMatchLengthCalculator calculator;
	auto ast = parser.parse(ConstString::makeUnmanagedString(re));
	ASSERT_TRUE(ast != nullptr);
	calculator.calculate(ast.get());
	auto p = calculator.getNodeMatchLength(ast->getRootNode());
	EXPECT_EQ(p.first, minLen) << "With XyRe: " << re;
	EXPECT_EQ(p.second, maxLen) << "With XyRe: " << re;;
}

TEST(XyReMatchLengthCalculatorTest, branchTest)
{
	testXyReMatchLength(_XR(ab|abcd|abc), 2, 4);
}

TEST(XyReMatchLengthCalculatorTest, closureTest)
{
	testXyReMatchLength(_XR(a*), 0);
	testXyReMatchLength(_XR(a*+), 0);
	testXyReMatchLength(_XR(a*?), 0);
	testXyReMatchLength(_XR(a?), 0, 1);
	testXyReMatchLength(_XR(a??), 0, 1);
	testXyReMatchLength(_XR(a?+), 0, 1);
	testXyReMatchLength(_XR(a+), 1);
	testXyReMatchLength(_XR(a+?), 1);
	testXyReMatchLength(_XR(a++), 1);
	testXyReMatchLength(_XR((abc){2,5}), 6, 15);
	testXyReMatchLength(_XR((abc){,100}?), 0, 300);
	testXyReMatchLength(_XR((abc){4,}+), 12);
	testXyReMatchLength(_XR((abc){6}), 18, 18);

}

TEST(XyReMatchLengthCalculatorTest, lookAroundTest)
{
	testXyReMatchLength(_XR((?=abc)), 0, 0);
	testXyReMatchLength(_XR((?!\w{2,4})), 0, 0);
	testXyReMatchLength(_XR((?<=abc)), 0, 0);
	testXyReMatchLength(_XR((?<!abc)), 0, 0);
}

TEST(XyReMatchLengthCalculatorTest, charClassTest)
{
	testXyReMatchLength(_XR([a-zA-Z_\]]), 1, 1);
	testXyReMatchLength(_XR([^abcde-]), 1, 1);
}

TEST(XyReMatchLengthCalculatorTest, escapedSequenceTest)
{
	testXyReMatchLength(_XR(\s), 1, 1);
	testXyReMatchLength(_XR(\S), 1, 1);
	testXyReMatchLength(_XR(\d), 1, 1);
	testXyReMatchLength(_XR(\D), 1, 1);
	testXyReMatchLength(_XR(\w), 1, 1);
	testXyReMatchLength(_XR(\D), 1, 1);
	testXyReMatchLength(_XR(\b), 0, 0);
	testXyReMatchLength(_XR(\B), 0, 0);
	testXyReMatchLength(_XR(\G), 0, 0);
	testXyReMatchLength(_XR(\x2c), 1, 1);
	testXyReMatchLength(_XR(\u9521), 1, 1);
	testXyReMatchLength(_XR(\*), 1, 1);
}
