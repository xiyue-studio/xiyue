#pragma once
#include "xiyue_file.h"

namespace xiyue
{
	/*
		一次性将整个文件读入内存
	*/
	class BufferedFileReader
	{
	public:
		BufferedFileReader();
		virtual ~BufferedFileReader();

	public:
		virtual bool readFile(ConstString fileName, size_t appendZeroSize = 0);

		byte* getBuffer() const { return m_buffer; }
		size_t getBufferSize() const { return m_bufferLength; }

		void swapBuffer(byte** buffer, size_t* bufferSize);

	protected:
		byte* m_buffer;
		size_t m_bufferLength;
	};
}