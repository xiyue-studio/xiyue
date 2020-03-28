#include "pch.h"
#include "xiyue_json_object.h"

using namespace std;
using namespace xiyue;

TEST(JsonObjectTest, initializeTest)
{
	JsonObject intObj = 4;
	JsonObject realObj = 3.1415926;
	JsonObject boolObj = false;
	JsonObject str1Obj = L"Hello world!"_cs;
	JsonObject str2Obj = L"This is test.";
	JsonObject listObj = JsonObject::list({ 3, true, L"test" });
	JsonObject mapObj = JsonObject::object({
		{L"name", L"tecyle"},
		{L"age", 18},
		{L"hobby", JsonObject::list({L"sleep", L"eat"})}
	});

	EXPECT_TRUE(intObj.isInt());
	EXPECT_EQ(intObj.intValue(), 4);
	intObj = L"Change type";
	EXPECT_TRUE(intObj.isString());
	EXPECT_CONST_STRING_EQ(intObj.stringValue(), L"Change type"_cs);
	EXPECT_TRUE(realObj.isReal());
	EXPECT_NEAR(realObj.realValue(), 3.1415926, 1e-7);
	EXPECT_TRUE(boolObj.isBoolean());
	EXPECT_EQ(boolObj.booleanValue(), false);
	EXPECT_TRUE(str1Obj.isString());
	EXPECT_CONST_STRING_EQ(str1Obj.stringValue(), L"Hello world!"_cs);
	EXPECT_TRUE(str2Obj.isString());
	EXPECT_CONST_STRING_EQ(str2Obj.stringValue(), L"This is test."_cs);
	EXPECT_TRUE(listObj.isList());
	EXPECT_EQ((ConstString)listObj, L"[ size = 3 ]");
	EXPECT_EQ(listObj.getMemberCount(), 3);
	EXPECT_EQ(listObj[0].intValue(), 3);
	EXPECT_EQ(listObj[1].booleanValue(), true);
	EXPECT_EQ(listObj[2].stringValue(), L"test");
	EXPECT_TRUE(mapObj.isObject());
	EXPECT_EQ((ConstString)mapObj, L"{ size = 3 }");
	EXPECT_EQ(mapObj.getMemberCount(), 3);
	EXPECT_EQ(mapObj[L"name"].stringValue(), L"tecyle");
	EXPECT_EQ(mapObj[L"age"].intValue(), 18);
	EXPECT_TRUE(mapObj[L"hobby"].isList());
	EXPECT_EQ(mapObj[L"hobby"][0].stringValue(), L"sleep");
	EXPECT_EQ(mapObj[L"hobby"][1].stringValue(), L"eat");
	mapObj[L"hobby"] = str1Obj;
	EXPECT_TRUE(mapObj[L"hobby"].isString());
	EXPECT_EQ(mapObj[L"hobby"].stringValue(), L"Hello world!");

	JsonObject::getAllocator()->clearMemory();
}
