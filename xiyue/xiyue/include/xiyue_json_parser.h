#pragma once
#include "xiyue_json_lexer.h"
#include "xiyue_json_object.h"

namespace xiyue
{
	/*
		基于 SLR 分析法的 Json 解析器。
	*/
	class JsonParser
	{
	public:
		JsonParser();
		virtual ~JsonParser();

	public:
		JsonObject parseJson(const ConstString& jsonText);

		/*
			当出现类似 [ 1, 2, ] 这样的情况，是否忽略多余的逗号，
			不忽略的话，就会报错。
		*/
		void setRedundantCommaIgnored(bool ignored) {
			m_ignoreRedundantComma = ignored;
		}

		/*
			决定最外层的 {} 是不是必须出现的。
		*/
		void setTopBraceIgnored(bool ignored) {
			m_ignoreTopBrace = ignored;
		}

		void setStringNeedCreate(bool needCreate) {
			m_stringNeedCreate = needCreate;
		}

	protected:
		void parse();
		void callReduce(uint8_t id);
		void reduceKeyValue();
		void reduceIntObject();
		void reduceRealObject();
		void reduceStringObject();
		void reduceBooleanObject();
		void reduceObjectObject();
		void reduceListObject();
		void reduceNullObject();

		void emplaceObject(const JsonObject& obj);

		void handleError(int errorNum);

	private:
		bool m_stringNeedCreate;
		bool m_ignoreRedundantComma;
		bool m_ignoreTopBrace;
		bool m_firstBraceNotGiven;
		ConstString m_jsonText;
		JsonLexer* m_lexer;
		std::vector<uint8_t> m_symbolStack;
		std::vector<uint8_t> m_stateStack;
		std::vector<JsonToken> m_tokenStack;
		std::vector<JsonObject> m_rootObjects;
	};
}
