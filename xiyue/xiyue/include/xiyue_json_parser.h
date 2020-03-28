#pragma once
#include "xiyue_json_lexer.h"
#include "xiyue_json_object.h"

namespace xiyue
{
	/*
		���� SLR �������� Json ��������
	*/
	class JsonParser
	{
	public:
		JsonParser();
		virtual ~JsonParser();

	public:
		JsonObject parseJson(const ConstString& jsonText);

		/*
			���������� [ 1, 2, ] ������������Ƿ���Զ���Ķ��ţ�
			�����ԵĻ����ͻᱨ��
		*/
		void setRedundantCommaIgnored(bool ignored) {
			m_ignoreRedundantComma = ignored;
		}

		/*
			���������� {} �ǲ��Ǳ�����ֵġ�
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
