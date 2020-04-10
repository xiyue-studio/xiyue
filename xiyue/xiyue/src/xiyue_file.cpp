#include "stdafx.h"
#include "xiyue_file.h"

using namespace std;
using namespace xiyue;

static ConstString getOpenStringByFlag(FileOpenFlag flag)
{
	switch (flag)
	{
	case FileOpenFlag_openForRead:
		return L"rb";
	case FileOpenFlag_openForWrite:
		return L"wb";
	case FileOpenFlag_openForAppend:
		return L"ab+";
	case FileOpenFlag_openForReadWrite:
		return L"rb+";
	default:
		assert(!"Unsupported open flag!");
		return L"";
	}
}

static bool checkFileExist(const wchar_t* fileName, FileCreateMode createMode)
{
	FILE* fp = nullptr;
	bool isFileExisted = _waccess(fileName, 0) == 0;

	switch (createMode)
	{
	case FileCreateMode_createIfNotExist:
		if (isFileExisted)
			return true;
	case FileCreateMode_alwaysCreate:
		_wfopen_s(&fp, fileName, L"wb");
		if (fp == nullptr)
			return false;
		fclose(fp);
		return true;
	case FileCreateMode_failedIfExist:
		return !isFileExisted;
	case FileCreateMode_failedIfNotExist:
		return isFileExisted;
	default:
		assert(!"Unsupported FileCreateMode!");
		return false;
	}
}

File::File()
	: m_fp(nullptr)
	, m_openFlag(FileOpenFlag_openForRead)
	, m_createMode(FileCreateMode_failedIfNotExist)
	, m_fileLength(0)
	, m_state(FileState_justInitialized)
{
}

File::~File()
{
	close();
}

bool File::open(ConstString fileName, FileOpenFlag flag, FileCreateMode createMode)
{
	if (m_state != FileState_justInitialized && m_state != FileState_closed)
	{
		// TODO log error state
		return false;
	}

	m_openFlag = flag;
	m_createMode = createMode;
	m_fileName = fileName;
	ConstString openStr = getOpenStringByFlag(flag);

	if (flag != FileOpenFlag_openForRead)
	{
		if (!checkFileExist(fileName, createMode))
		{
			// TODO createFileError
			return false;
		}
	}

	m_fp = nullptr;
	_wfopen_s(&m_fp, fileName, openStr);
	if (m_fp == nullptr)
	{
		// TODO log file open failed
		m_state = FileState_openFailed;
		return false;
	}

	m_state = FileState_openSucceeded;
	
	// º∆À„≥§∂»
	size_t offset = tell();
	seek(0, FileSeekMode_fromFileEnd);
	m_fileLength = tell();
	seek(offset, FileSeekMode_fromFileBegin);

	return true;
}

void File::close()
{
	if (m_state != FileState_openSucceeded || m_fp != nullptr)
		return;

	fclose(m_fp);
	m_fp = nullptr;
	m_state = FileState_closed;
}

bool File::seek(long offset, FileSeekMode seekMode)
{
	if (m_state != FileState_openSucceeded || m_fp == nullptr)
		return false;

	return fseek(m_fp, offset, seekMode) == 0;
}

size_t File::tell() const
{
	assert(m_state == FileState_openSucceeded && m_fp != nullptr);

	if (m_state != FileState_openSucceeded || m_fp == nullptr)
		return 0;

	return static_cast<size_t>(ftell(m_fp));
}

void File::reset()
{
	close();

	m_openFlag = FileOpenFlag_openForRead;
	m_createMode = FileCreateMode_failedIfNotExist;
	m_fileLength = 0;
	m_state = FileState_justInitialized;
}

size_t File::getFileLength()
{
	if (m_openFlag == FileOpenFlag_openForRead)
		return m_fileLength;

	size_t offset = tell();
	seek(0, FileSeekMode_fromFileEnd);
	m_fileLength = tell();
	seek(offset, FileSeekMode_fromFileBegin);

	return m_fileLength;
}

size_t File::readBytes(byte* buffer, size_t readCount, size_t bufferSize)
{
	assert(m_state == FileState_openSucceeded && m_openFlag != FileOpenFlag_openForWrite);

	if (m_state != FileState_openSucceeded || m_openFlag == FileOpenFlag_openForWrite)
		return 0;

	return fread_s(buffer, bufferSize, sizeof(char), readCount, m_fp);
}

size_t File::writeBytes(const byte* buffer, size_t writeCount)
{
	assert(m_state == FileState_openSucceeded && m_openFlag != FileOpenFlag_openForRead);

	if (m_state != FileState_openSucceeded || m_openFlag == FileOpenFlag_openForRead)
		return 0;

	return fwrite(buffer, sizeof(byte), writeCount, m_fp);
}
