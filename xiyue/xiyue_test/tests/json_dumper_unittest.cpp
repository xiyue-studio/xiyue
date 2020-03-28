#include "pch.h"
#include "xiyue_json_dumper.h"

using namespace std;
using namespace xiyue;

TEST(JsonDumperTest, basicTest)
{
	JsonObject root = JsonObject::object({
		{L"name", L"Tecyle"},
		{L"age", 18},
		{L"hobby", JsonObject::list({L"sleep", L"eat", 3.1415926535})}
	});

	JsonDumper dumper;
	dumper.setCompactMode(true);
	dumper.setRealNumberFormat(L"0.000"_cs);
	ConstString result = dumper.dumpToString(root);

	const wchar_t* expectedString = LR"({"name":"Tecyle","age":18,"hobby":["sleep","eat",3.142]})";
	EXPECT_EQ(result, expectedString);

	dumper.setCompactMode(false);
	dumper.setIndentSize(4);
	expectedString =
LR"({
    "name": "Tecyle",
    "age": 18,
    "hobby": [
        "sleep",
        "eat",
        3.142
    ]
})";
	result = dumper.dumpToString(root);
	EXPECT_EQ(result, expectedString);
}
