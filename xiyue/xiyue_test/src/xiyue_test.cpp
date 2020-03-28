#include "pch.h"
using namespace std;

void setCurrentDirForTestExplorer()
{
	wchar_t buf[MAX_PATH];

	GetModuleFileName(NULL, buf, MAX_PATH);

	wstring path(buf);
	path = path.substr(0, path.find_last_of('\\'));
	path.append(L"\\..\\test-working\\");

	SetCurrentDirectory(path.c_str());
	GetCurrentDirectory(sizeof(buf) / sizeof(wchar_t), buf);
}

int main(int argc, char** argv)
{
	setlocale(LC_CTYPE, "");

	setCurrentDirForTestExplorer();

	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

	return result;
}
