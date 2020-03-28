#pragma once
#include "xiyue_buffered_file_reader.h"
#include "xiyue_encoding.h"

namespace xiyue
{
	/*
		按照指定的编码一次性读入整个文本文件。
	*/
	class StringFileReader
	{
	public:

	public:
		StringFileReader() = default;
		virtual ~StringFileReader() = default;

	public:
		virtual bool readFile(ConstString fileName, StringEncoding encoding = StringEncoding_unspecifiedEncoding);

		StringEncoding getFileEncoding() const { return m_encoding; }

		ConstString getText() const {
			return m_text;
		}

		size_t getTextLength() const {
			return m_text.length();
		}

	protected:
		StringEncoding m_encoding = StringEncoding_unspecifiedEncoding;
		ConstString m_text;
	};
}
