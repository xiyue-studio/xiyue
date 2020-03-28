#include "stdafx.h"
#include "xiyue_string_file_writer.h"

using namespace std;
using namespace xiyue;

StringFileWriter::StringFileWriter(size_t bufferSize /*= 4096u*/)
{
	m_bufferSize = bufferSize - 1;
	m_file = nullptr;
	m_buffer.reserve(bufferSize);
}

StringFileWriter::~StringFileWriter()
{
	close();
}

bool StringFileWriter::open(ConstString fileName, StringEncoding encoding /*= StringEncoding_utf8*/)
{
	m_file = new File();
	if (!m_file->open(fileName, FileOpenFlag_openForWrite, FileCreateMode_alwaysCreate))
	{
		delete m_file;
		m_file = nullptr;
		return false;
	}

	// 处理文件的 BOM 头
	if (encoding == StringEncoding_utf8WithBOM)
	{
		m_file->writeBytes((const byte*)UTF8_BOM, sizeof(UTF8_BOM) - 1);
		m_encoding = StringEncoding_utf8;
	}
	else if (encoding == StringEncoding_utf16LEWithBOM)
	{
		m_file->writeBytes((const byte*)UTF16_LE_BOM, sizeof(UTF16_LE_BOM) - 1);
		m_encoding = StringEncoding_utf16LE;
	}
	else if (encoding == StringEncoding_utf16BEWithBOM)
	{
		m_file->writeBytes((const byte*)UTF16_BE_BOM, sizeof(UTF16_BE_BOM) - 1);
		m_encoding = StringEncoding_utf16BE;
	}
	else
	{
		m_encoding = encoding;
	}

	return true;
}

bool StringFileWriter::writeString(const wchar_t* str, size_t length)
{
	if (m_file == nullptr)
		return false;

	writeToBuffer(str, length);
	return true;
}

bool StringFileWriter::writeString(const ConstString& str)
{
	return writeString(str.data(), str.length());
}

bool StringFileWriter::writeString(const wchar_t* str)
{
	return writeString(str, wcslen(str));
}

bool StringFileWriter::writeString(const wstring& str)
{
	return writeString(str.c_str(), str.size());
}

void StringFileWriter::writeToBuffer(const wchar_t* str, size_t length)
{
	assert(m_bufferSize >= m_buffer.size());
	while (length > 0)
	{
		size_t bufferLeftSize = m_bufferSize - m_buffer.size();
		size_t appendSize = std::min(length, bufferLeftSize);
		m_buffer.append(str, str + appendSize);
		str += appendSize;
		bufferLeftSize -= appendSize;
		length -= appendSize;
		if (bufferLeftSize == 0)
			flush();
	}
}

bool StringFileWriter::flush()
{
	if (m_file == nullptr)
		return false;

	if (m_buffer.size() == 0)
		return true;

	if (m_encoding == StringEncoding_utf16LE)
	{
		m_file->writeBytes((const byte*)m_buffer.c_str(), sizeof(wchar_t) * m_buffer.size());
		m_buffer.clear();
		return true;
	}

	size_t length = xiyue_transformStringEncoding(m_buffer.c_str(), nullptr, 0, StringEncoding_utf16LE, m_encoding);
	m_writeBuffer.resize(length);
	xiyue_transformStringEncoding(m_buffer.c_str(), &*m_writeBuffer.begin(), length, StringEncoding_utf16LE, m_encoding);
	while (!m_writeBuffer.empty() && m_writeBuffer.back() == 0)
		m_writeBuffer.pop_back();
	m_file->writeBytes((const byte*)&*m_writeBuffer.begin(), m_writeBuffer.size());
	m_buffer.clear();
	return true;
}

void StringFileWriter::close()
{
	if (m_file == nullptr)
		return;

	flush();
	m_file->close();
	delete m_file;
	m_file = nullptr;
}
