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
		�ļ��Ĵ���ģʽ�����ļ���д��ʽ�򿪵�ʱ�򣬿���ָ������
		�ļ�����ģʽ��
	*/
	enum FileCreateMode
	{
		FileCreateMode_createIfNotExist,			///< ����ļ��������򴴽�������ֱ�Ӵ�
		FileCreateMode_alwaysCreate,				///< �����ļ��Ƿ���ڣ������´���
		FileCreateMode_failedIfNotExist,			///< ����ļ������ھʹ�ʧ��
		FileCreateMode_failedIfExist				///< ����ļ����ھʹ��Ƿ񣬷��򴴽��µ��ļ�
	};

	enum FileState
	{
		FileState_justInitialized,
		FileState_openFailed,
		FileState_openSucceeded,
		FileState_closed
	};
}