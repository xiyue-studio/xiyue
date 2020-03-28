#pragma once
#include "xiyue_file.h"
#include "xiyue_encoding.h"

namespace xiyue
{
	class StringFileWriter
	{
	public:
		explicit StringFileWriter(size_t bufferSize = 4096u);
		virtual ~StringFileWriter();

	public:
		bool open(ConstString fileName, StringEncoding encoding = StringEncoding_utf8);
		void close();

	public:
		bool flush();
		bool writeString(const ConstString& str);
		bool writeString(const wchar_t* str);
		bool writeString(const std::wstring& str);
		bool writeString(const wchar_t* str, size_t length);

	protected:
		void writeToBuffer(const wchar_t* str, size_t length);

	protected:
		size_t m_bufferSize;
		StringEncoding m_encoding;
		File* m_file;
		std::wstring m_buffer;
		std::vector<char> m_writeBuffer;
	};
}
