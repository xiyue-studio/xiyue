#pragma once

namespace xiyue
{
	enum FileSeekMode
	{
		FileSeekMode_fromFileBegin = SEEK_SET,
		FileSeekMode_fromCurrentPosition = SEEK_CUR,
		FileSeekMode_fromFileEnd = SEEK_END
	};

	enum FileOpenFlag
	{
		FileOpenFlag_openForRead,
		FileOpenFlag_openForWrite,
		FileOpenFlag_openForAppend,
		FileOpenFlag_openForReadWrite
	};

	/*
		文件的创建模式，当文件以写方式打开的时候，可以指定以下
		文件创建模式。
	*/
	enum FileCreateMode
	{
		FileCreateMode_createIfNotExist,			///< 如果文件不存在则创建，否则直接打开
		FileCreateMode_alwaysCreate,				///< 不管文件是否存在，都重新创建
		FileCreateMode_failedIfNotExist,			///< 如果文件不存在就打开失败
		FileCreateMode_failedIfExist				///< 如果文件存在就打开是否，否则创建新的文件
	};

	enum FileState
	{
		FileState_justInitialized,
		FileState_openFailed,
		FileState_openSucceeded,
		FileState_closed
	};
}