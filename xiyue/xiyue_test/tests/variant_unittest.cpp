#include "pch.h"
#include "xiyue_variant_type.h"

using namespace std;
using namespace xiyue;

TEST(VariantTest, basicTest)
{
	Variant<int, double, bool, wstring, vector<int>> v;

	static_assert(v.bufferSize == sizeof(wstring), "Variant size incorrect.");

	EXPECT_TRUE(v.isNull());
	v = 1;
	EXPECT_TRUE(v.is<int>());
	EXPECT_EQ(v.get<int>(), 1);
	v = 3.14159;
	EXPECT_TRUE(v.is<double>());
	EXPECT_FALSE(v.is<int>());
	EXPECT_NEAR(v.get<double>(), 3.14159, 0.00001);
	v = false;
	EXPECT_TRUE(v.is<bool>());
	EXPECT_EQ(v.get<bool>(), false);
	v = wstring(L"Hello world!");
	EXPECT_TRUE(v.is<wstring>());
	EXPECT_EQ(v.get<wstring>(), L"Hello world!");

	v = vector<int>({ 1, 2, 3 });
	EXPECT_TRUE(v.is<vector<int>>());
	EXPECT_EQ(v.get<vector<int>>().size(), 3u);

	EXPECT_THROW(v.get<int>(), std::bad_cast);
	double val = 0.0;
	EXPECT_THROW(val = v, std::bad_cast);
}
