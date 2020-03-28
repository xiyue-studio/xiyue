#pragma once
#include "xiyue_json_data.h"

namespace xiyue
{
	class JsonDataAllocator
	{
	public:
		virtual JsonData* allocIntData(json_int_t val) = 0;
		virtual JsonData* allocRealData(double val) = 0;
		virtual JsonData* allocBoolData(bool val) = 0;
		virtual JsonData* allocStringData(const wchar_t* str, json_int_t len, bool managed) = 0;
		virtual JsonData* allocListData(size_t reservedSize) = 0;
		virtual JsonData* allocObjectData(size_t reservedSize) = 0;
		virtual JsonData* allocNullData() = 0;

		virtual JsonObject* allocJsonObject(const JsonObject& obj) = 0;

		virtual JsonStringData allocKeyString(const wchar_t* str, json_int_t len, bool managed) = 0;

		virtual void reallocList(JsonData* listData, size_t newSize) = 0;
		virtual void reallocObjectMembers(JsonData* objectData, size_t newSize) = 0;

		virtual void freeJsonData(JsonData* data) = 0;
		virtual void freeJsonObjectData(JsonObject* obj) = 0;

		virtual void clearMemory() = 0;
	};

	static inline JsonData* addJsonDataReference(JsonData* data) {
		if (data == nullptr)
			return nullptr;

		++data->refCount;
		return data;
	}

	void releaseJsonDataReference(JsonData* data, JsonDataAllocator* allocator);
}
