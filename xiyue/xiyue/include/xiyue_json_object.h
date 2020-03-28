#pragma once
#include "xiyue_const_string.h"
#include "xiyue_json_data.h"
#include "xiyue_json_data_allocator.h"

namespace xiyue
{
	class JsonObject
	{
	public:
		// 类型创建
		JsonObject(); // json_null
		JsonObject(json_int_t val);
		JsonObject(bool val);
		JsonObject(const wchar_t* val, bool shouldFree = true);
		JsonObject(ConstString val, bool shouldFree = true);
		JsonObject(double val);
		JsonObject(const JsonObject*) = delete;

		static JsonObject list(size_t reservedSize = 0);
		static JsonObject list(const std::initializer_list<JsonObject>& ilist);
		static JsonObject object(size_t reservedSize = 0);
		static JsonObject object(const std::initializer_list<std::pair<ConstString, JsonObject>>& ilist);

		// 引用
		JsonObject(const JsonObject& o);
		JsonObject(JsonObject&& o);
		~JsonObject();

	protected:
		explicit JsonObject(JsonData* data) { m_data = data; }

	public:
		// 运算符重载
		JsonObject& operator=(const JsonObject& o);
		JsonObject& operator[](int index);
		JsonObject& operator[](ConstString key);
		JsonObject& operator[](const wchar_t* key);
		bool operator==(const JsonObject& r) const;
		bool operator==(const ConstString& str) const;
		bool operator==(json_int_t val) const;
		bool operator==(const wchar_t* str) const;
		bool operator==(double val) const;
		bool operator==(bool val) const;
		operator ConstString() const;
		operator bool() const;

	public:
		uint32_t getMemberCount() const;
		void append(const JsonObject& o);
		void setMember(ConstString key, const JsonObject& val);
		void setMember(const wchar_t* key, const JsonObject& val);
		ConstString getKeyAtIndex(uint32_t index);
		ConstString getMemberAtIndex(uint32_t index, JsonObject& member);
		uint32_t findMember(ConstString key);
		uint32_t findMember(const wchar_t* key);

		JsonObjectType getType() const { return m_data->jsonType; }
		bool is(JsonObjectType type) const;
		bool isInt() const { return is(Json_int); }
		bool isBoolean() const { return is(Json_boolean); }
		bool isReal() const { return is(Json_real); }
		bool isNull() const { return is(Json_null); }
		bool isString() const { return is(Json_string); }
		bool isList() const { return is(Json_list); }
		bool isObject() const { return is(Json_object); }

		json_int_t intValue() const;
		double realValue() const;
		ConstString stringValue() const;
		bool booleanValue() const;

	public:
		// 快速接口，对原理不熟悉的话谨慎使用
		// 无拷贝字符串
		const wchar_t* stringData() const;
		uint32_t stringLength() const;
		// 无重分配空间追加元素
		void appendNoCheck(const JsonObject& o);
		void appendMemberNoCheck(ConstString key, const JsonObject& val);
		void appendMemberNoCheck(const wchar_t* key, const JsonObject& val);
		// 无重复检查
		void appendMember(ConstString key, const JsonObject& val);
		void appendMember(const wchar_t* key, const JsonObject& val);
		// 获取原始数据
		const JsonData* data() const { return m_data; }

		static JsonDataAllocator* selectAllocator(JsonDataAllocator* allocator);
		static JsonDataAllocator* getAllocator() {
			return m_allocator;
		}

	private:
		static thread_local JsonDataAllocator* m_allocator;
		JsonData* m_data;
	};
}
