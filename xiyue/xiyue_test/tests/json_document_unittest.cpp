#include "pch.h"
#include "xiyue_json_document.h"

using namespace std;
using namespace xiyue;

TEST(JsonDocumentTest, basicParseTest)
{
	ConstString jsonText = L"{ \"name\": \"tecyle\", \"age\": 18 }"_cs;
	JsonDocument doc;
	ASSERT_TRUE(doc.parse(jsonText));

	JsonObject root = doc.getRootObject();
	EXPECT_TRUE(root.isObject());
	EXPECT_EQ(root.getMemberCount(), 2);
	EXPECT_EQ(root[L"name"], L"tecyle");
	EXPECT_EQ(root[L"age"], 18);
}

TEST(JsonDocumentTest, nestingObjectTest)
{
	ConstString jsonText = L"{ 'name':'genius', 'age':18, 'hobby': ['sleep', 'eat', { 'name':'programming', 'language':'C++', 'exp':10}] }"_cs;
	JsonDocument doc;
	ASSERT_TRUE(doc.parse(jsonText));

	JsonObject root = doc.getRootObject();
	EXPECT_TRUE(root.isObject());
	EXPECT_EQ(root.getMemberCount(), 3);
	EXPECT_EQ(root[L"name"], L"genius");
	EXPECT_EQ(root[L"age"], 18);
	EXPECT_TRUE(root[L"hobby"].isList());
	EXPECT_EQ(root[L"hobby"].getMemberCount(), 3);
	EXPECT_EQ(root[L"hobby"][0], L"sleep");
	EXPECT_EQ(root[L"hobby"][1], L"eat");
	EXPECT_TRUE(root[L"hobby"][2].isObject());
	EXPECT_EQ(root[L"hobby"][2][L"name"], L"programming");
	EXPECT_EQ(root[L"hobby"][2][L"language"], L"C++");
	EXPECT_EQ(root[L"hobby"][2][L"exp"], 10);
}
