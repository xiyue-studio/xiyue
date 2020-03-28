#pragma once
#include "xiyue_json_object.h"
#include "xiyue_encoding.h"

namespace xiyue
{
	/*
		����һ�� JSON �ļ���
	*/
	class JsonDocument
	{
	public:
		JsonDocument();

	public:
		/*
			������˱���ģʽ����������᳢���޸� jsonStr ���ַ����ڴ����ݡ�
			��� jsonStr ��ϣ�����޸ģ��벻Ҫ�򿪱���ģʽ��
		*/
		bool parse(const ConstString& jsonStr);
		bool parseJsonFile(ConstString fileName, StringEncoding encoding = StringEncoding_utf8);

	public:
		/*
			���ñ���ģʽ��

			�������ģʽ�򿪣������������ JSON �ַ�������������� Document ���ڲ�
			�洢�ռ䣬�ڻ���Ҫʹ�ý��������� JSON �����ʱ�򣬲����ͷ� Document��
			���ǽ����ٶȻ��ߡ�
			������˱���ģʽ�Ļ���ʹ�� parse ʱ�����޸� jsonStr �����ݡ�
			
			���û�д򿪱���ģʽ�����������������ǽ��������� JsonObject ��������
			Document ʹ�á�
		*/
		bool setRetainMode(bool retainModeOn) { m_isRetainMode = retainModeOn; }
		bool setRedundantCommaIgnored(bool ignored) { m_ignoreRedundantComma = ignored; }
		bool setTopBraceIgnored(bool ignored) { m_ignoreTopBrace = ignored; }

		JsonObject getRootObject() const { return m_rootObject; }

	private:
		bool m_isRetainMode;
		bool m_ignoreRedundantComma;
		bool m_ignoreTopBrace;
		ConstString m_jsonText;
		JsonObject m_rootObject;
	};
}
