#include "pch.h"
#include "xiyue_const_string.h"

using namespace std;
using namespace xiyue;

TEST(ConstStringTest, substrTest)
{
	ConstString str = L"This is a test."_cs;
	str = str.substr(2, 7);
	EXPECT_CONST_STRING_EQ(str, L"is is a"_cs);
}