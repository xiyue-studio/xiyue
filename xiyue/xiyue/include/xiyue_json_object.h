#pragma once
#include "xiyue_const_string.h"
#include "xiyue_json_data.h"
#include "xiyue_json_data_allocator.h"

namespace xiyue
{
	class JsonObject final
	{
	public:
		// 类型创建
		JsonObject(); // json_null
		JsonObject(json_int_t val);
		JsonObject(uint32_t val);
		JsonObject(bool val);
		JsonObject(const wchar_t* val, bool shouldFree = true);
		JsonObject(ConstString val, bool shouldFree = true);
		JsonObject(double val);
		JsonObject(const JsonObject*) = delete;

		static JsonObject list(size_t reservedSize = 8);
		static JsonObject list(const std::initializer_list<JsonObject>& ilist);
		static JsonObject object(size_t reservedSize = 8);
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

		/*
			运算规则：
			1. 字符串与任何类型运算，都会转成字符串然后进行拼接；
			2. 其它类型相加，都会尝试转成 real 进行相加，无法转换则为 NaN；
			3. -, *, / 都会尝试将两端类型转换成 real 进行运算，无法运算则结果为 NaN。

			转换方式：
			null		0
			true		1
			false		0
			undefined	NaN
			list		NaN
			object		NaN
			string		可以解析则转换，否则为 NaN

			特别的，字符串和非负整数相乘，结果是这个字符串的倍数。否则，为 NaN。
		*/
		JsonObject operator+(const JsonObject& r) const;
		JsonObject operator-(const JsonObject& r) const;
		JsonObject operator*(const JsonObject& r) const;
		JsonObject operator/(const JsonObject& r) const;

	public:
		uint32_t getMemberCount() const;
		void append(const JsonObject& o);
		void setMember(ConstString key, const JsonObject& val);
		void setMember(const wchar_t* key, const JsonObject& val);
		ConstString getKeyAtIndex(uint32_t index);
		ConstString getMemberAtIndex(uint32_t index, JsonObject& member);
		uint32_t findMember(ConstString key);
		uint32_t findMember(const wchar_t* key);
		inline bool containsMember(ConstString key) { return findMember(key) != UINT32_MAX; }
		inline bool containsMember(const wchar_t* key) { return findMember(key) != UINT32_MAX; }

		JsonObjectType getType() const { return m_data->jsonType; }
		bool is(JsonObjectType type) const;
		bool isInt() const { return is(Json_int); }
		bool isBoolean() const { return is(Json_boolean); }
		bool isReal() const { return is(Json_real); }
		bool isNull() const { return is(Json_null); }
		bool isString() const { return is(Json_string); }
		bool isList() const { return is(Json_list); }
		bool isObject() const { return is(Json_object); }

		ConstString toString() const;
		json_int_t toInteger() const;
		double toReal() const;

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
		static JsonDataAllocator* m_allocator;
		JsonData* m_data;
	};
}
