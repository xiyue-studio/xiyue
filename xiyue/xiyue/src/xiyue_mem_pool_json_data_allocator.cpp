#include "stdafx.h"
#include "xiyue_mem_pool_json_data_allocator.h"
#include "xiyue_json_object.h"

using namespace std;
using namespace xiyue;

JsonData* MemPoolJsonDataAllocator::allocIntData(json_int_t val)
{
	JsonData* result = m_jsonDataAllocator.alloc();
	new(result)JsonData();
	result->jsonType = Json_int;
	result->intValue = val;
	result->refCount = 1;

	return result;
}

JsonData* MemPoolJsonDataAllocator::allocRealData(double val)
{
	JsonData* result = m_jsonDataAllocator.alloc();
	new(result)JsonData();
	result->jsonType = Json_real;
	result->realValue = val;
	result->refCount = 1;

	return result;
}

JsonData* MemPoolJsonDataAllocator::allocBoolData(bool val)
{
	JsonData* result = m_jsonDataAllocator.alloc();
	new(result)JsonData();
	result->jsonType = Json_boolean;
	result->boolValue = val;
	result->refCount = 1;

	return result;
}

JsonData* MemPoolJsonDataAllocator::allocStringData(const wchar_t* str, json_int_t len, bool managed)
{
	JsonData* result = m_jsonDataAllocator.alloc();
	new(result)JsonData();
	result->jsonType = Json_string;
	result->refCount = 1;

	result->stringValue.strLen = len;
	if (managed)
	{
		len = (len + 1) * sizeof(wchar_t);
		wchar_t* buffer = (wchar_t*)malloc(len);
		memcpy(buffer, str, len);
		result->stringValue.stringValue = buffer;
		result->jsonStringType = JsonString_wcharString;
	}
	else
	{
		result->jsonStringType = JsonString_refWcharString;
		result->stringValue.stringValue = str;
	}

	return result;
}

JsonData* MemPoolJsonDataAllocator::allocListData(size_t reservedSize)
{
	JsonData* data = m_jsonDataAllocator.alloc();
	new(data)JsonData();
	data->jsonType = Json_list;
	data->refCount = 1;

	data->listValue.valueSize = 0;
	data->listValue.reservedSize = reservedSize;
	if (reservedSize == 0)
		data->listValue.values = nullptr;
	else
		data->listValue.values = m_listDataAllocator.allocArray(reservedSize);

	return data;
}

JsonData* MemPoolJsonDataAllocator::allocObjectData(size_t reservedSize)
{
	JsonData* data = m_jsonDataAllocator.alloc();
	new(data)JsonData();
	data->jsonType = Json_object;
	data->refCount = 1;

	data->memberValue.valueSize = 0;
	data->memberValue.reservedSize = reservedSize;
	if (reservedSize == 0)
		data->memberValue.members = nullptr;
	else
		data->memberValue.members = m_memberAllocator.allocArray(reservedSize);

	return data;
}

JsonData* MemPoolJsonDataAllocator::allocNullData()
{
	JsonData* data = m_jsonDataAllocator.alloc();
	new(data)JsonData();
	data->jsonType = Json_null;
	data->refCount = 1;

	return data;
}

JsonObject* MemPoolJsonDataAllocator::allocJsonObject(const JsonObject& obj)
{
	JsonObject* o = m_jsonObjectAllocator.alloc();
	new(o)JsonObject(obj);

	return o;
}

JsonStringData MemPoolJsonDataAllocator::allocKeyString(const wchar_t* str, json_int_t len, bool managed)
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

void MemPoolJsonDataAllocator::reallocList(JsonData* listData, size_t newSize)
{
	assert(newSize > listData->listValue.valueSize);

	auto& valueAddr = listData->listValue.values;
	valueAddr = m_listDataAllocator.reallocArray(valueAddr, listData->listValue.valueSize, getMallocSize<JsonObject*>(newSize));
	listData->listValue.reservedSize = newSize;
}

void MemPoolJsonDataAllocator::reallocObjectMembers(JsonData* objectData, size_t newSize)
{
	assert(newSize > objectData->memberValue.valueSize);

	auto& valueAddr = objectData->memberValue.members;
	valueAddr = m_memberAllocator.reallocArray(valueAddr, objectData->memberValue.valueSize, getMallocSize<JsonMember>(newSize));
	objectData->memberValue.reservedSize = newSize;
}

void MemPoolJsonDataAllocator::freeJsonData(JsonData* data)
{
	m_jsonDataAllocator.free(data);
}

void MemPoolJsonDataAllocator::freeJsonObjectData(JsonObject* obj)
{
	m_jsonObjectAllocator.free(obj);
}

void MemPoolJsonDataAllocator::clearMemory()
{
	m_jsonDataAllocator.clear();
	m_jsonObjectAllocator.clear();
	m_listDataAllocator.clear();
	m_memberAllocator.clear();

	for (auto& key : m_keys)
	{
		free((void*)key.second.stringValue);
	}

	m_keys.clear();
}
