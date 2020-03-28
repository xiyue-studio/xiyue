#pragma once
#include "xiyue_json_data_allocator.h"

namespace xiyue
{
	class DefaultJsonDataAllocator : public JsonDataAllocator
	{
	public:
		virtual JsonData* allocIntData(json_int_t val) override;
		virtual JsonData* allocRealData(double val) override;
		virtual JsonData* allocBoolData(bool val) override;
		virtual JsonData* allocStringData(const wchar_t* str, json_int_t len, bool managed) override;
		virtual JsonData* allocListData(size_t reservedSize) override;
		virtual JsonData* allocObjectData(size_t reservedSize) override;
		virtual JsonData* allocNullData() override;

		virtual JsonObject* allocJsonObject(const JsonObject& obj) override;

		virtual JsonStringData allocKeyString(const wchar_t* str, json_int_t len, bool managed) override;

		virtual void reallocList(JsonData* listData, size_t newSize) override;
		virtual void reallocObjectMembers(JsonData* objectData, size_t newSize) override;

		virtual void freeJsonData(JsonData* data) override;
		virtual void freeJsonObjectData(JsonObject* obj) override;

		virtual void clearMemory() override;

	private:
		std::allocator<JsonData> m_jsonDataAllocator;
		std::allocator<JsonObject> m_jsonObjectAllocator;
		std::unordered_map<JsonStringData, JsonStringData> m_keys;
	};
}
