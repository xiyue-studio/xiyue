#pragma once

namespace xiyue
{
	enum XyReMatchMode
	{
		/**
			忽略大小写匹配。
		*/
		XyReMatch_ignoreCase = 'i',
		/**
			全局搜索模式，一次性匹配所有可能匹配的结果，并替换。
			\G 在此模式下可用。

			此选项不能在匹配中使用。
		*/
		XyReMatch_globalSearch = 'g',
		/**
			松散排版模式，对实际匹配没有什么影响。只影响书写。
		*/
		XyReMatch_looseMode = 'x',
		/**
			多行匹配模式，开启后，^ 和 $ 匹配的就是行首和行尾。
		*/
		XyReMatch_multiLine = 'm',
		/**
			指定 . 是否能匹配换行符。
		*/
		XyReMatch_dotMatchNewLine = 's',
		/**
			Unicode 模式，主要影响大小写匹配，\w \s \d 等匹配
			的结果。
		*/
		XyReMatch_unicode = 'u',
		/**
			禁用捕获组，在此模式下，括号对以及命名分组均被视为
			非捕获组，反向引用也将失去作用。

			此选项对于生成 DFA 有好处。不能在匹配中应用。
		*/
		XyReMatch_noCaptureGroup = 'C'
	};

	enum XyReDirective : uint8_t
	{
		/**
			什么也不做。
		*/
		NOOP = 0,
		/**
			CHAR(CHARacter match)

			匹配一个字符。
			subDirective = 0，则 arg1 和 arg2 合并为 16 位
			待匹配字符。
			subDirective = 1，则后跟一个 uint32，表示待匹配
			字符的 unicode 编码。
		*/
		CHAR = 1,
		/**
			SPLT(SPLiT)

			分裂线程。线程运行到此处的时候，分裂成两个状态
			相同的线程，并在指定的位置继续执行。
			subDirective = 0，则 arg1 表示线程 1 的地址偏移，
			arg2 表示线程 2 的地址偏移(int8_t)。
			subDirective = 1，则 arg1 和 arg2 合成 int16 表
			示线程 1 的偏移地址，后跟一个 int 表示线程 2 的
			偏移地址。
			subDirective = 2，则 arg1 和 arg2 合成 int16 表
			示线程 2 的偏移地址，后跟一个 int 表示线程 1 的
			偏移地址。

			注：如果无特殊说明，偏移地址基准都是该指令的地址。
			而不是加上后跟数据之后的地址。
		*/
		SPLT = 2,
		/**
			JUMP

			无条件跳转指令。
			subDirective = 0，则 arg1 和 arg2 合并 int16_t
			表示跳转的偏移量。
			subDirective = 1，则 arg1 和 arg2 均为 0，且
			后跟指令为 int，表示跳转的偏移值。
		*/
		JUMP = 3,
		/**
			SUCC(SUCCeeded)

			匹配成功。将当前线程标记为成功匹配的状态。
		*/
		SUCC = 4,
		/**
			DELY(DELaY)

			延迟当前线程，用于贪婪匹配。遇到这个指令之后，
			就将当前线程加入到延迟线程列表中。
			延迟线程列表中，线程依然跟着推进，直到工作线程
			全都失败了，就从延迟线程列表栈顶中选一个放入
			工作线程列表中。
			延迟线程列表是个栈。
		*/
		DELY = 5,
		/**
			ABAN(ABANdon candidates)

			候选抛弃指令，用于实现固化分组和占有优先。
			遇到这个指令之后，就会删除所有同一个 id 的
			候选抛弃线程，并将自己加入候选抛弃线程。
			subDirective = 0，则 arg1 和 arg2 合起来
			表示 ID，16 位。
			subDirective = 1，则 arg1 和 arg2 均为 0，
			后跟指令为 uint，表示更大的 ID。
		*/
		ABAN = 6,
		/**
			CALL

			调用子程序。用于环视。线程运行这里之后，
			就会启动新的 Process，在指定的位置上匹配，
			匹配成功或者失败的结果返回。
			subDirective = 0，表示是正后向环视，期待
			从当前位置匹配子程序，匹配成功。arg1 表示
			子程序起始位置的偏移值，arg2 表示子程序
			返回之后继续执行位置的偏移值。
			后跟 uint32，表示匹配的最小长度。
			subDirective = 1，表示是负后向环视，期待
			从当前位置匹配子程序，匹配失败。arg1 和
			arg2 的含义与上相同。
			后跟 uint32，表示匹配的最小长度。
			subDirective = 2，表示定长正前向环视。后跟
			指令为 uint，表示前向移动的长度。
			subDirective = 3，表示定长负前向环视。后跟
			指令为 uint，表示前向移动的长度。
			subDirective = 4，表示不定长正前向环视，
			后跟两个 uint，表示前向移动的长度区间。
			subDirective = 5，表示不定长负前向环视。
			subDirective = 6，表示长地址的正后向环视，
			后跟两个 int，表示偏移值。
			subDirective = 7,8,9,10,11 含义一致。
			均表示之前的长地址形式。
		*/
		CALL = 7,
		/**
			ALLM(ALL character Match)

			匹配任意字符。字符编码统一设定。
			这个指令不能匹配 \0 字符，不能匹配非多行模式
			下的换行符，不能匹配非 unicode 模式下的非
			ASCII 字符。
		*/
		ALLM = 8,
		/**
			SPCE(SPaCE match)

			匹配空白字符。
		*/
		SPCE = 9,
		/**
			NSPC(Non-SPaCe match)

			匹配非空白字符。参数含义与 SPCE 一致。
		*/
		NSPC = 10,
		/**
			DGIT(DiGIT match)

			匹配数字字符。参数不再赘述，参见 SPCE，下同。
		*/
		DGIT = 11,
		/**
			NDGT(Non-DiGiT match)

			匹配非数字字符。
		*/
		NDGT = 12,
		/**
			WORD(WORD match)

			匹配单词字符。在普通模式下，等同于 [0-9a-zA-Z_]。
			扩展到 unicode 模式下，会算上所有的字母数字。
		*/
		WORD = 13,
		/**
			NWOD(Non-WOrD match)

			匹配非单词字符。
		*/
		NWOD = 14,
		/**
			HEAD(line HEAD match)

			匹配多行模式下的行首，以及单行模式的字符串开始位置。
			行首的判定方式为：
			上一个字符是行首或者换行符。
		*/
		HEAD = 15,
		/**
			TAIL(line TAIL match)

			匹配多行模式下的行尾，以及单行模式的字符串结束位置。
			行尾的判定方式为：
			当前字符是 EOF 或者换行符。
		*/
		TAIL = 16,
		/**
			BOND(BOuND match)

			匹配单词边界。
			subDirective = 0，匹配单词双边界。
			subDirective = 1，匹配单词左边界。
			subDirective = 2，匹配单词右边界。
			判定的方法为，左侧是空白，右侧是 WORD，或者相反。
		*/
		BOND = 17,
		/**
			NBND(Non-BouND match)

			匹配非单词边界。
			subDirective = 0，匹配双侧非边界。
			subDirective = 1，匹配左侧非边界。
			subDirective = 2，匹配右侧非边界。
		*/
		NBND = 18,
		/**
			LSPM(LaSt Position Match)

			匹配上次匹配结束的位置。用于全局搜索替换的模式。
			如果是第一次搜索，则指令无效，等价于 NOOP。
		*/
		LSPM = 19,
		/**
			CLSM(CLaSs Match)

			匹配一个字符集合，比如[a-z]。
			subDirective = 0，匹配正集合，即类似 [a-z]。
			subDirective = 1，匹配负集合，即类似 [^a-z]。
			arg1 表示后跟 range 的长度，arg2 表示后跟 char
			的长度。如果 subDirective = 2 或 3，则后跟两个
			uint 分别表示 range 的长度和 char 的长度。

			首先后跟的是 range，用两个 int 表示起点和终点。
			后跟数据表示需要匹配的字符，排过序。用 32 位
			表示原字符编码的字符数值。UTF8 编码因为会超过
			32 位字符，所以匹配该指令时，会转换成 UTF32。

			匹配时，先去 range 中查找匹配，然后再到字符列表
			中进行二分查找。
			查找结果得出之后，跳过数据部分，继续执行后续指令。
		*/
		CLSM = 20,
		/**
			BREF(Back REFerence)

			反向引用匹配。
			subDirective = 0，arg1 和 arg2 联合表示反向引用的
			组号。
			subDirective = 1，后跟 uint 表示反向引用的组号。

			进行反向引用的时候，线程查找已经匹配上的引用组，
			然后，每次 step 的时候，就根据引用组号和反向引用
			计数器计算要匹配的字符。
		*/
		BREF = 21,
		/**
			WBRF(Wait Back ReFerence)

			等待反向引用结束。
			每次进来一个字符，就会被拆成多个字节，在逐字节比对
			的时候，反向引用计数不断后移。
			此外，捕获组捕获的长度，记录的是字节数。
			直到所有的字节都比对成功之后，WBRF 指令才算完成，
			指令指针才会后移。
		*/
		WBRF = 22,
		/**
			SWCH(SWitCH)

			运行时模式切换指令。
			subDirective = 0，表示打开开关。
			subDirective = 1，表示关闭开关。
			arg1 表示对应开关的 ASCII 值。
		*/
		SWCH = 23,
		/**
			SVST(SaVe STart position)

			记录当前的字节位置为分组的起点。同时，将分组的终点
			记为无效，表示这是一个无效的区间。用于分组捕获。
			subDirective = 0，则 arg1 和 arg2 联合表示组号。
			subDirective = 1，则后跟 int 表示组号。
		*/
		SVST = 24,
		/**
			SVED(SaVe EnD position)

			记录当前的字节位置为分组的终点。参数与 SVST 一致。
		*/
		SVED = 25,
		/**
			DFAM(DFA Match)

			DFA 匹配专用节点。只有在 DFA 匹配模式下才会出现这个
			指令，且不再会出现别的指令。
			subDirective = 1，表示这个节点是接受状态。

			后跟 sparse set 结构，结构具体如下：
			64 个 uint，表示 256 个 uint8 索引位置，其中的值表示
			后面跟着的位置索引。
			后面根据实际情况，跟上任意个 int，表示跳转到的状态
			的偏移地址。跳转位置一定是个 DFAM 指令。
		*/
		DFAM = 26,
		/**
			LAFG(Look Around FlaG)

			这个指令指定了环视的时候的一些优化措施。
			arg1 和 arg2 组成 16 位 bool 标志。依次为：
			
			1. 环视结构中，不包含捕获组则为 1。这意味着：
			   * 后向正环视，只需要最短匹配，且不需要保留线程。
			     占有优先匹配成功依然不算匹配成功。
			   * 前向正环视，不需要保留线程。

			这个指令暂未实现。
		*/
		LAFG = 27,
	};

	struct XyReInstruction
	{
		XyReDirective directive;
		uint8_t subDirective;
		uint8_t arg1;
		uint8_t arg2;
	};
}
