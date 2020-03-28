#include "stdafx.h"
#include "xiyue_buffered_file_reader.h"

using namespace std;
using namespace xiyue;

BufferedFileReader::BufferedFileReader()
{
	m_buffer = nullptr;
	m_bufferLength = 0;
}

bool BufferedFileReader::readFile(ConstString fileName, size_t appendZeroSize)
{
	File file;
	if (!file.open(fileName, FileOpenFlag_openForRead, FileCreateMode_failedIfNotExist))
		return false;

	free(m_buffer);
	m_bufferLength = file.getFileLength();
	m_buffer = (byte*)malloc(sizeof(byte) * (m_bufferLength + appendZeroSize));
	m_bufferLength = file.readBytes(m_buffer, m_bufferLength, m_bufferLength);
	memset(m_buffer + m_bufferLength, 0, appendZeroSize);
	m_bufferLength += appendZeroSize;

	file.close();

	return true;
}

void BufferedFileReader::swapBuffer(byte** buffer, size_t* bufferSize)
{
	assert(buffer && bufferSize);
	std::swap(*buffer, m_buffer);
	std::swap(*bufferSize, m_bufferLength);
}

BufferedFileReader::~BufferedFileReader()
{
	free(m_buffer);
}
