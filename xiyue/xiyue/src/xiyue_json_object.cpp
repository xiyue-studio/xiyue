#include "stdafx.h"
#include "xiyue_json_object.h"
#include "xiyue_encoding.h"
#include "xiyue_default_json_data_allocator.h"
#include "xiyue_logger_manager.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

using namespace std;
using namespace xiyue;

static DefaultJsonDataAllocator g_defaultAllocator;
JsonDataAllocator* JsonObject::m_allocator = &g_defaultAllocator;

JsonObject::JsonObject(json_int_t val)
{
	m_data = m_allocator->allocIntData(val);
}

JsonObject::JsonObject(uint32_t val)
{
	m_data = m_allocator->allocIntData((json_int_t)val);
}

JsonObject::JsonObject(bool val)
{
	m_data = m_allocator->allocBoolData(val);
}

JsonObject::JsonObject(const wchar_t* val, bool shouldFree /*= true*/)
{
	m_data = m_allocator->allocStringData(val, (json_int_t)wcslen(val), shouldFree);
}

JsonObject::JsonObject(ConstString val, bool shouldFree /*= true*/)
{
	if (val.isUnmanagedString())
		shouldFree = false;

	m_data = m_allocator->allocStringData(val.data(), (json_int_t)val.length(), shouldFree);
}

JsonObject::JsonObject(double val)
{
	m_data = m_allocator->allocRealData(val);
}

JsonObject::JsonObject()
{
	m_data = m_allocator->allocNullData();
}

JsonObject::JsonObject(const JsonObject& o)
{
	m_data = addJsonDataReference(o.m_data);
}

JsonObject::JsonObject(JsonObject&& o)
{
	m_data = o.m_data;
	o.m_data = nullptr;
}

JsonObject::~JsonObject()
{
	releaseJsonDataReference(m_data, m_allocator);
}

JsonObject JsonObject::list(size_t reservedSize /*= 0*/)
{
	return JsonObject(m_allocator->allocListData(reservedSize));
}

JsonObject JsonObject::list(const initializer_list<JsonObject>& ilist)
{
	JsonObject obj(m_allocator->allocListData(ilist.size()));
	for (auto& o : ilist)
	{
		obj.appendNoCheck(o);
	}

	return obj;
}

JsonObject JsonObject::object(size_t reservedSize /*= 0*/)
{
	return JsonObject(m_allocator->allocObjectData(reservedSize));
}

JsonObject JsonObject::object(const initializer_list<std::pair<ConstString, JsonObject>>& ilist)
{
	JsonObject obj(m_allocator->allocObjectData(ilist.size()));
	for (auto& p : ilist)
	{
		obj.setMember(p.first, p.second);
	}

	return obj;
}

JsonObject& JsonObject::operator=(const JsonObject& o)
{
	if (m_data == o.m_data)
		return *this;

	JsonData* data = addJsonDataReference(o.m_data);
	releaseJsonDataReference(m_data, m_allocator);
	m_data = data;

	return *this;
}

JsonObject& JsonObject::operator[](int index)
{
	assert(isList());
	assert((uint32_t)index < m_data->listValue.valueSize);
	return *m_data->listValue.values[index];
}

JsonObject& JsonObject::operator[](ConstString key)
{
	assert(isObject());
	auto& memberValue = m_data->memberValue;
	uint32_t index = findMember(key);
	if (index == UINT32_MAX)
	{
		appendMember(key, JsonObject());
		return *memberValue.members[memberValue.valueSize - 1].value;
	}

	return *memberValue.members[index].value;
}

JsonObject& JsonObject::operator[](const wchar_t* key)
{
	assert(isObject());
	auto& memberValue = m_data->memberValue;
	uint32_t index = findMember(key);
	if (index == UINT32_MAX)
	{
		appendMember(key, JsonObject());
		return *memberValue.members[memberValue.valueSize - 1].value;
	}

	return *memberValue.members[index].value;
}

uint32_t JsonObject::getMemberCount() const
{
	switch (m_data->jsonType)
	{
	case Json_object:
		return m_data->memberValue.valueSize;
	case Json_list:
		return m_data->listValue.valueSize;
	default:
		return 0;
	}
}

void JsonObject::append(const JsonObject& o)
{
	if (!isList())
	{
		XIYUE_LOG_WARNING("Try to append a json object to a non-list object.");
		return;
	}

	auto& listData = m_data->listValue;
	if (listData.valueSize == listData.reservedSize)
	{
		m_allocator->reallocList(m_data, listData.reservedSize * 2);
		listData = m_data->listValue;
	}
	
	listData.values[listData.valueSize++] = m_allocator->allocJsonObject(o);
}

void JsonObject::appendNoCheck(const JsonObject& o)
{
	auto& listData = m_data->listValue;
	listData.values[listData.valueSize++] = m_allocator->allocJsonObject(o);
}

void JsonObject::setMember(ConstString key, const JsonObject& val)
{
	if (!isObject())
	{
		XIYUE_LOG_WARNING("Try to set a member to a non-object json object.");
		return;
	}

	uint32_t index = findMember(key);
	if (index == UINT32_MAX)
		appendMember(key, val);
	else
		*m_data->memberValue.members[index].value = val;
}

void JsonObject::setMember(const wchar_t* key, const JsonObject& val)
{
	if (!isObject())
	{
		XIYUE_LOG_WARNING("Try to set a member to a non-object json object.");
		return;
	}

	uint32_t index = findMember(key);
	if (index == UINT32_MAX)
		appendMember(key, val);
	else
		*m_data->memberValue.members[index].value = val;
}

ConstString JsonObject::getKeyAtIndex(uint32_t index)
{
	assert(isObject());
	assert(index < m_data->memberValue.valueSize);

	auto& key = m_data->memberValue.members[index].key;
	return ConstString(key.stringValue, key.strLen);
}

ConstString JsonObject::getMemberAtIndex(uint32_t index, JsonObject& member)
{
	assert(isObject());
	assert(index < m_data->memberValue.valueSize);

	auto& key = m_data->memberValue.members[index].key;
	member = *m_data->memberValue.members[index].value;
	return ConstString(key.stringValue, key.strLen);
}

uint32_t JsonObject::findMember(ConstString key)
{
	assert(isObject());

	auto& memberValue = m_data->memberValue;
	auto it = find_if(memberValue.members, memberValue.members + memberValue.valueSize, [key](JsonMember& m) {
		return m.key.strLen == (json_int_t)key.length() &&
			wcsncmp(m.key.stringValue, key.data(), m.key.strLen) == 0;
	});

	if (it == memberValue.members + memberValue.valueSize)
		return UINT32_MAX;
	return it - memberValue.members;
}

uint32_t JsonObject::findMember(const wchar_t* key)
{
	assert(isObject());

	auto& memberValue = m_data->memberValue;
	auto it = find_if(memberValue.members, memberValue.members + memberValue.valueSize, [key](JsonMember& m) {
		return m.key.strLen == (json_int_t)wcslen(key) &&
			wcsncmp(m.key.stringValue, key, m.key.strLen) == 0;
	});

	if (it == memberValue.members + memberValue.valueSize)
		return UINT32_MAX;
	return it - memberValue.members;
}

bool JsonObject::is(JsonObjectType type) const
{
	return type == m_data->jsonType;
}

json_int_t JsonObject::intValue() const
{
	if (isInt())
		return m_data->intValue;

	if (isReal())
		return static_cast<json_int_t>(m_data->realValue);

	XIYUE_LOG_WARNING("Try call intValue() for a non-number json object.");
	return 0;
}

double JsonObject::realValue() const
{
	if (isInt())
		return static_cast<double>(m_data->intValue);

	if (isReal())
		return m_data->realValue;

	XIYUE_LOG_WARNING("Try call intValue() for a non-number json object.");
	return 0;
}

ConstString JsonObject::stringValue() const
{
	if (isString())
		return ConstString(m_data->stringValue.stringValue, m_data->stringValue.strLen);

	XIYUE_LOG_WARNING("Try call stringValue() for a non-string json object.");
	return operator ConstString();
}

bool JsonObject::booleanValue() const
{
	if (!isBoolean())
	{
		XIYUE_LOG_WARNING("Try call booleanValue() for non-boolean json object.");
	}

	return operator bool();
}

const wchar_t* JsonObject::stringData() const
{
	assert(isString());
	return m_data->stringValue.stringValue;
}

uint32_t JsonObject::stringLength() const
{
	assert(isString());
	return m_data->stringValue.strLen;
}

void JsonObject::appendMemberNoCheck(ConstString key, const JsonObject& val)
{
	assert(isObject());
	auto& member = m_data->memberValue.members[m_data->memberValue.valueSize++];
	member.value = m_allocator->allocJsonObject(val);
	member.key = m_allocator->allocKeyString(key.data(), key.length(), !key.isUnmanagedString());
}

void JsonObject::appendMemberNoCheck(const wchar_t* key, const JsonObject& val)
{
	assert(isObject());
	auto& member = m_data->memberValue.members[m_data->memberValue.valueSize++];
	member.value = m_allocator->allocJsonObject(val);
	member.key = m_allocator->allocKeyString(key, wcslen(key), true);
}

void JsonObject::appendMember(ConstString key, const JsonObject& val)
{
	assert(isObject());
	auto& memberValue = m_data->memberValue;
	if (memberValue.valueSize == memberValue.reservedSize)
		m_allocator->reallocObjectMembers(m_data, std::max(1u, memberValue.reservedSize * 2));
	appendMemberNoCheck(key, val);
}

void JsonObject::appendMember(const wchar_t* key, const JsonObject& val)
{
	assert(isObject());
	auto& memberValue = m_data->memberValue;
	if (memberValue.valueSize == memberValue.reservedSize)
		m_allocator->reallocObjectMembers(m_data, memberValue.reservedSize * 2);
	appendMemberNoCheck(key, val);
}

JsonDataAllocator* JsonObject::selectAllocator(JsonDataAllocator* allocator)
{
	assert(allocator != nullptr);
	JsonDataAllocator* old = m_allocator;
	m_allocator = allocator;
	return old;
}

JsonObject::operator ConstString() const
{
	switch (m_data->jsonType)
	{
	case Json_null:
		return L"null"_cs;
	case Json_boolean:
		return m_data->boolValue ? L"true"_cs : L"false"_cs;
	case Json_string:
		return ConstString(m_data->stringValue.stringValue, m_data->stringValue.strLen);
	case Json_list:
		return ConstString::makeByFormat(L"[ size = %u ]", m_data->listValue.valueSize);
	case Json_object:
		return ConstString::makeByFormat(L"{ size = %u }", m_data->memberValue.valueSize);
	case Json_int:
		return ConstString::makeByFormat(L"%d", m_data->intValue);
	case Json_real:
		return ConstString::makeByFormat(L"%f", m_data->realValue);
	default:
		return L""_cs;
	}
}

JsonObject::operator bool() const
{
	switch (m_data->jsonType)
	{
	case Json_null:
		return false;
	case Json_boolean:
		return m_data->boolValue;
	case Json_string:
		return m_data->stringValue.strLen != 0;
	case Json_list:
		return m_data->listValue.valueSize != 0;
	case Json_object:
		return m_data->memberValue.valueSize != 0;
	case Json_int:
		return m_data->intValue != 0;
	case Json_real:
		return m_data->realValue != 0;
	default:
		return false;
	}
}

bool JsonObject::operator==(const JsonObject& r) const
{
	if (this == &r)
		return true;

	if (m_data == r.m_data)
		return true;

	if (m_data->jsonType != r.m_data->jsonType)
		return false;

	switch (m_data->jsonType)
	{
	case Json_null:
		return true;
	case Json_int:
		return m_data->intValue == r.m_data->intValue;
	case Json_real:
		return m_data->realValue == r.m_data->realValue;
	case Json_boolean:
		return m_data->boolValue == r.m_data->boolValue;
	case Json_string:
		return m_data->stringValue == r.m_data->stringValue;
	}

	XIYUE_LOG_WARNING("List or object element is not supported == operation.");
	return false;
}

bool JsonObject::operator==(const ConstString& str) const
{
	if (getType() != Json_string)
		return false;

	return m_data->stringValue == JsonStringData{ str.data(), str.length() };
}

bool JsonObject::operator==(json_int_t val) const
{
	if (getType() != Json_int)
		return false;

	return m_data->intValue == val;
}

bool JsonObject::operator==(const wchar_t* str) const
{
	if (getType() != Json_string)
		return false;

	return operator==(ConstString::makeUnmanagedString(str));
}

bool JsonObject::operator==(double val) const
{
	if (getType() != Json_real)
		return false;

	return m_data->realValue == val;
}

bool JsonObject::operator==(bool val) const
{
	if (getType() != Json_boolean)
		return false;

	return m_data->boolValue == val;
}

static inline bool canTransformToInt(JsonObjectType type)
{
	switch (type)
	{
	case Json_null:
	case Json_boolean:
	case Json_int:
		return true;
	default:
		return false;
	}
}

JsonObject JsonObject::operator+(const JsonObject& r) const
{
	if (isString() || r.isString())
	{
		// 字符串相加，返回合并的字符串
		return JsonObject(toString() + r.toString());
	}

	if (canTransformToInt(getType()) && canTransformToInt(r.getType()))
	{
		// 能够转换成 int 就按照 int 类型计算
		return JsonObject(intValue() + r.intValue());
	}

	// 否则，转换成 double 进行计算
	return JsonObject(toReal() + r.toReal());
}

JsonObject JsonObject::operator-(const JsonObject& r) const
{
	if (canTransformToInt(getType()) && canTransformToInt(r.getType()))
	{
		// 能够转换成 int 就按照 int 类型计算
		return JsonObject(intValue() - r.intValue());
	}

	// 否则，转换成 double 进行计算
	return JsonObject(toReal() - r.toReal());
}

static inline bool canMultiply(const JsonObject& s, const JsonObject& i)
{
	if (!s.isString())
		return false;
	if (!i.isInt())
		return false;

	if (i.intValue() < 0)
		return false;

	return true;
}

JsonObject JsonObject::operator*(const JsonObject& r) const
{
	if (canMultiply(*this, r))
		return JsonObject(ConstString::makeByRepeat(stringValue(), r.toInteger()));
	
	if (canMultiply(r, *this))
		return JsonObject(ConstString::makeByRepeat(r.stringValue(), toInteger()));

	if (isInt() && r.isInt())
		return JsonObject(intValue() * r.intValue());

	return JsonObject(toReal() * r.toReal());
}

JsonObject JsonObject::operator/(const JsonObject& r) const
{
	if (isInt() && r.isInt())
	{
		if (r.intValue() == 0)
			return JsonObject(nan(""));

		return JsonObject(intValue() / r.intValue());
	}

	return JsonObject(toReal() / r.toReal());
}

ConstString JsonObject::toString() const
{
	return operator ConstString();
}

json_int_t JsonObject::toInteger() const
{
	switch (getType())
	{
	case Json_null:
		return 0;
	case Json_int:
		return intValue();
	case Json_real:
		return (json_int_t)realValue();
	case Json_boolean:
		return booleanValue() ? 1 : 0;
	case Json_string:
		if (stringValue().canTransformToInt())
			return stringValue().toInt();
	default:
		throw exception("JsonObject can not transform to int.");
	}
}

double JsonObject::toReal() const
{
	switch (getType())
	{
	case Json_null:
		return 0;
	case Json_int:
		return (double)intValue();
	case Json_real:
		return realValue();
	case Json_boolean:
		return booleanValue() ? 1.0 : 0.0;
	case Json_string:
		if (stringValue().canTransformToDouble())
			return stringValue().toDouble();
	default:
		return nan("");
	}
}
