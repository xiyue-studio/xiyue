#include "stdafx.h"
#include "xiyue_string_file_reader.h"

using namespace std;
using namespace xiyue;

bool StringFileReader::readFile(ConstString fileName, StringEncoding encoding /*= StringEncoding_unspecifiedEncoding*/)
{
	m_encoding = encoding;
	BufferedFileReader reader;
	if (!reader.readFile(fileName, sizeof(wchar_t)))
		return false;

	// ¼ì²âÎÄ¼þ±àÂë
	if (m_encoding == StringEncoding_unspecifiedEncoding && reader.getBufferSize() >= 2)
		m_encoding = xiyue_getEncodingFromBOM(reader.getBuffer(), std::min(reader.getBufferSize(), 3u));

	if (m_encoding == StringEncoding_unspecifiedEncoding)
		m_encoding = StringEncoding_utf8;

	size_t length = xiyue_transformStringEncoding(reader.getBuffer(), nullptr, 0, m_encoding, StringEncoding_utf16LE);
	m_text = ConstString::makeByReservedSize(length / sizeof(wchar_t));
	ConstStringData* data = m_text._getStringData();
	xiyue_transformStringEncoding(reader.getBuffer(), data->stringData(), length, m_encoding, StringEncoding_utf16LE);
	m_text._resetLength((int)(length / sizeof(wchar_t)) - 1);

	return true;
}

