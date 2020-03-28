#pragma once
#include "xiyue_file_types.h"

namespace xiyue
{
	class File
	{
	public:
		File();
		virtual ~File();

	public:
		virtual bool open(ConstString fileName, FileOpenFlag flag, FileCreateMode createMode);
		virtual void close();

		virtual bool seek(long offset, FileSeekMode seekMode);
		virtual size_t tell() const;

		void reset();

		size_t getFileLength();
		ConstString getFileName() const { return m_fileName.c_str(); }
		FileState getFileState() const { return m_state; }

		size_t readBytes(byte* buffer, size_t readCount, size_t bufferSize);

		template <size_t bufferSize>
		inline size_t readBytes(byte (&buffer)[bufferSize], size_t readCount) {
			return readBytes(buffer, readCount, bufferSize);
		}

		size_t writeBytes(const byte* buffer, size_t writeCount);

		template <size_t bufferSize>
		inline size_t writeBytes(const byte (&buffer)[bufferSize]) {
			return writeBytes(buffer, bufferSize);
		}

	protected:
		FILE* m_fp;
		FileOpenFlag m_openFlag;
		FileCreateMode m_createMode;
		size_t m_fileLength;
		std::wstring m_fileName;
		FileState m_state;
	};
}
