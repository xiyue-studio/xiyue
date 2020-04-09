#pragma once

namespace xiyue
{
	typedef int json_int_t;
	class JsonObject;

	enum JsonObjectType
	{
		Json_null,
		Json_string,
		Json_int,
		Json_real,
		Json_boolean,
		Json_list,
		Json_object,
		Json_undefined,
	};

	enum JsonStringType
	{
		JsonString_wcharString,          ///< 宽字符字符串，需要 JSON 自己释放
		JsonString_refWcharString,       ///< 引用的宽字符字符串，外部负责释放
	};

	struct JsonStringData
	{
		const wchar_t* stringValue;
		json_int_t strLen;
	};

	static inline bool operator==(const JsonStringData& l, const JsonStringData& r) {
		if (&l == &r)
			return true;

		if (l.strLen != r.strLen)
			return false;

		if (l.stringValue == r.stringValue)
			return true;

		return wcsncmp(l.stringValue, r.stringValue, l.strLen) == 0;
	}
	
	static inline bool operator<(const JsonStringData& l, const JsonStringData& r) {
		if (&l == &r)
			return false;

		int res = wcsncmp(l.stringValue, r.stringValue, l.strLen < r.strLen ? l.strLen : r.strLen);
		if (res == 0)
			return l.strLen < r.strLen;
		return res < 0;
	}

	struct JsonListData
	{
		JsonObject** values;
		uint32_t valueSize;
		uint32_t reservedSize;
	};

	struct JsonMember
	{
		JsonStringData key;
		JsonObject* value;
	};

	struct JsonMemberData
	{
		JsonMember* members;
		uint32_t valueSize;
		uint32_t reservedSize;
	};

	struct JsonData
	{
		JsonObjectType jsonType : 8;
		JsonStringType jsonStringType : 8;
		std::atomic_int refCount;
		union {
			json_int_t intValue;
			double realValue;
			bool boolValue;
			JsonListData listValue;
			JsonMemberData memberValue;
			JsonStringData stringValue;
		};
	};
}

/*
	JsonStringData 的哈希函数，参考 STL 的 string 的哈希函数
*/
template <>
struct std::hash<xiyue::JsonStringData>
{
	std::size_t operator()(const xiyue::JsonStringData &key) const
	{
#if defined(_WIN64)
		constexpr size_t baseOffset = 14695981039346656037ULL;
		constexpr size_t prime = 1099511628211ULL;
#else
		constexpr size_t baseOffset = 2166136261U;
		constexpr size_t prime = 16777619U;
#endif
		std::size_t val = baseOffset;

		for (int index = 0; index < key.strLen; ++index)
		{
			val ^= static_cast<size_t>(key.stringValue[index]);
			val *= prime;
		}

		return val;
	}
};
