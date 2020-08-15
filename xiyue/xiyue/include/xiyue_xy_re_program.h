#pragma once
#include "xiyue_xy_re_instruction.h"

namespace xiyue
{
	struct XyReProgram
	{
		/**
			指令的数量，具体指 instructions 的个数。
		*/
		uint32_t instructionCount;
		/**
			命名捕获组的数量，实际编号从 -1 到 -namedGroupCount。

			符号表（名称到组号的映射）存在 XyReProcess 中。
		*/
		uint32_t namedGroupCount;
		/**
			非命名捕获组的数量，实际编号从 1 到 numberGroupCount。
		*/
		uint32_t numberGroupCount;
		/**
			如果这个正则表达式能成功匹配的话，至少需要多少字符。
			这是一个优化，如果给定的字符串长度不够，可以直接失败。
		*/
		uint32_t leastMatchedLength;
		/**
			如果这个正则表达式能成功匹配的话，最多需要多少字符。
			这也是一个优化，如果是全匹配，后者匹配开头或者末尾的时候，
			如果长度超长，也可以直接失败。
		*/
		uint32_t mostMatchedLength;
		/**
			正则表示式是不是以 ^ 开头的。
			可以进行匹配优化，非多行模式下，仅在字符串开始的位置搜索。
			多行模式下，仅在行首搜索。
		*/
		uint32_t startHeaderMatch : 1;
		/**
			正则表达式是不是以 $ 结尾的。
		*/
		uint32_t endTailMatch : 1;
		/**
			保存引用名称的字符是多少字节的。
			0 - 两字节。
			1 - 四字节。
		*/
		uint32_t referenceNameCharType : 1;
		uint32_t reserved : 29;

		const XyReInstruction* instructions() const { return (const XyReInstruction*)(this + 1); }
		const wchar_t* referenceNames() const { return (const wchar_t*)(instructions() + instructionCount); }
	};

	typedef const int* XyReProgramPC;
}
