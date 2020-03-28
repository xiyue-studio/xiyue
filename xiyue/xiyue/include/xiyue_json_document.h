#pragma once
#include "xiyue_json_object.h"
#include "xiyue_encoding.h"

namespace xiyue
{
	/*
		解析一个 JSON 文件。
	*/
	class JsonDocument
	{
	public:
		JsonDocument();

	public:
		/*
			如果打开了保持模式，则解析器会尝试修改 jsonStr 的字符串内存内容。
			如果 jsonStr 不希望被修改，请不要打开保持模式。
		*/
		bool parse(const ConstString& jsonStr);
		bool parseJsonFile(ConstString fileName, StringEncoding encoding = StringEncoding_utf8);

	public:
		/*
			设置保持模式。

			如果保持模式打开，则解析出来的 JSON 字符串都会引用这个 Document 的内部
			存储空间，在还需要使用解析出来的 JSON 对象的时候，不能释放 Document。
			但是解析速度会变高。
			如果打开了保持模式的话，使用 parse 时，会修改 jsonStr 的内容。
			
			如果没有打开保持模式，则解析会变慢，但是解析出来的 JsonObject 可以脱离
			Document 使用。
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
