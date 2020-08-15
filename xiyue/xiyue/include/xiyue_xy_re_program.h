#pragma once
#include "xiyue_xy_re_instruction.h"

namespace xiyue
{
	struct XyReProgram
	{
		/**
			ָ�������������ָ instructions �ĸ�����
		*/
		uint32_t instructionCount;
		/**
			�����������������ʵ�ʱ�Ŵ� -1 �� -namedGroupCount��

			���ű����Ƶ���ŵ�ӳ�䣩���� XyReProcess �С�
		*/
		uint32_t namedGroupCount;
		/**
			�������������������ʵ�ʱ�Ŵ� 1 �� numberGroupCount��
		*/
		uint32_t numberGroupCount;
		/**
			������������ʽ�ܳɹ�ƥ��Ļ���������Ҫ�����ַ���
			����һ���Ż�������������ַ������Ȳ���������ֱ��ʧ�ܡ�
		*/
		uint32_t leastMatchedLength;
		/**
			������������ʽ�ܳɹ�ƥ��Ļ��������Ҫ�����ַ���
			��Ҳ��һ���Ż��������ȫƥ�䣬����ƥ�俪ͷ����ĩβ��ʱ��
			������ȳ�����Ҳ����ֱ��ʧ�ܡ�
		*/
		uint32_t mostMatchedLength;
		/**
			�����ʾʽ�ǲ����� ^ ��ͷ�ġ�
			���Խ���ƥ���Ż����Ƕ���ģʽ�£������ַ�����ʼ��λ��������
			����ģʽ�£���������������
		*/
		uint32_t startHeaderMatch : 1;
		/**
			������ʽ�ǲ����� $ ��β�ġ�
		*/
		uint32_t endTailMatch : 1;
		/**
			�����������Ƶ��ַ��Ƕ����ֽڵġ�
			0 - ���ֽڡ�
			1 - ���ֽڡ�
		*/
		uint32_t referenceNameCharType : 1;
		uint32_t reserved : 29;

		const XyReInstruction* instructions() const { return (const XyReInstruction*)(this + 1); }
		const wchar_t* referenceNames() const { return (const wchar_t*)(instructions() + instructionCount); }
	};

	typedef const int* XyReProgramPC;
}
