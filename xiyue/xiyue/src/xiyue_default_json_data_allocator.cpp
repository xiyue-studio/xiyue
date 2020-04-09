#include "stdafx.h"
#include "xiyue_default_json_data_allocator.h"
#include "xiyue_json_object.h"

using namespace std;
using namespace xiyue;

JsonData* DefaultJsonDataAllocator::allocIntData(json_int_t val)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_int;
	data->refCount = 1;
	data->intValue = val;

	return data;
}

JsonData* DefaultJsonDataAllocator::allocRealData(double val)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_real;
	data->refCount = 1;
	data->realValue = val;

	return data;
}

JsonData* DefaultJsonDataAllocator::allocBoolData(bool val)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_boolean;
	data->refCount = 1;
	data->boolValue = val;

	return data;
}

JsonData* DefaultJsonDataAllocator::allocStringData(const wchar_t* str, json_int_t len, bool managed)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_string;
	data->refCount = 1;

	data->stringValue.strLen = len;
	if (managed)
	{
		len = (len + 1) * sizeof(wchar_t);
		wchar_t* buffer = (wchar_t*)malloc(len);
		memcpy(buffer, str, len);
		data->stringValue.stringValue = buffer;
		data->jsonStringType = JsonString_wcharString;
	}
	else
	{
		data->jsonStringType = JsonString_refWcharString;
		data->stringValue.stringValue = str;
	}

	return data;
}

JsonData* DefaultJsonDataAllocator::allocListData(size_t reservedSize)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_list;
	data->refCount = 1;

	data->listValue.valueSize = 0;
	data->listValue.reservedSize = reservedSize;
	if (reservedSize == 0)
		data->listValue.values = nullptr;
	else
		data->listValue.values = (JsonObject**)malloc(getMallocSize<JsonObject*>(reservedSize));

	return data;
}

JsonData* DefaultJsonDataAllocator::allocObjectData(size_t reservedSize)
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_object;
	data->refCount = 1;

	data->memberValue.valueSize = 0;
	data->memberValue.reservedSize = reservedSize;
	if (reservedSize == 0)
		data->memberValue.members = nullptr;
	else
		data->memberValue.members = (JsonMember*)malloc(sizeof(JsonMember) * reservedSize);

	return data;
}

JsonStringData DefaultJsonDataAllocator::allocKeyString(const wchar_t* str, json_int_t len, bool managed)
{
	JsonStringData data = { str, len };

	if (!managed)
		return data;

	auto it = m_keys.find(data);
	if (it != m_keys.end())
		return it->second;

	len = getMallocSize<wchar_t>(len + 1);
	wchar_t* buffer = (wchar_t*)malloc(len);
	memcpy(buffer, str, len);
	data.stringValue = buffer;
	m_keys.insert(make_pair(data, data));

	return data;
}

void DefaultJsonDataAllocator::reallocList(JsonData* listData, size_t newSize)
{
	assert(newSize > listData->listValue.valueSize);

	auto& valueAddr = listData->listValue.values;
	valueAddr = (JsonObject**)realloc(valueAddr, getMallocSize<JsonObject*>(newSize));
	listData->listValue.reservedSize = newSize;
}

void DefaultJsonDataAllocator::reallocObjectMembers(JsonData* objectData, size_t newSize)
{
	assert(newSize > objectData->memberValue.valueSize);

	auto& valueAddr = objectData->memberValue.members;
	valueAddr = (JsonMember*)realloc(valueAddr, getMallocSize<JsonMember>(newSize));
	objectData->memberValue.reservedSize = newSize;
}

void DefaultJsonDataAllocator::freeJsonData(JsonData* data)
{
	assert(data->refCount == 0);

	switch (data->jsonType)
	{
	case Json_list:
		for (uint32_t i = 0; i < data->listValue.valueSize; ++i)
			freeJsonObjectData(data->listValue.values[i]);
		free(data->listValue.values);
		break;
	case Json_object:
		for (uint32_t i = 0; i < data->memberValue.valueSize; ++i)
			freeJsonObjectData(data->memberValue.members[i].value);
		free(data->memberValue.members);
		break;
	case Json_string:
		if (data->jsonStringType == JsonString_wcharString)
			free((void*)data->stringValue.stringValue);
		break;
	}

	m_jsonDataAllocator.destroy(data);
	m_jsonDataAllocator.deallocate(data, 1);
}

void DefaultJsonDataAllocator::clearMemory()
{
	for (auto& key : m_keys)
	{
		free((void*)key.second.stringValue);
	}

	m_keys.clear();
}

JsonData* DefaultJsonDataAllocator::allocNullData()
{
	JsonData* data = m_jsonDataAllocator.allocate(1);
	m_jsonDataAllocator.construct(data);
	data->jsonType = Json_null;
	data->refCount = 1;

	return data;
}

JsonObject* DefaultJsonDataAllocator::allocJsonObject(const JsonObject& obj)
{
	JsonObject* o = m_jsonObjectAllocator.allocate(1);
	m_jsonObjectAllocator.construct(o, obj);

	return o;
}

void DefaultJsonDataAllocator::freeJsonObjectData(JsonObject* obj)
{
	m_jsonObjectAllocator.destroy(obj);
	m_jsonObjectAllocator.deallocate(obj, 1);
}
