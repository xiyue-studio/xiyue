#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <algorithm>
#include <stack>
#include <deque>
#include <functional>
#include <initializer_list>
#include <atomic>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif // _WIN32


#include "xiyue_const_string.h"

typedef unsigned char byte;
typedef unsigned int uint32;

#define XIYUE_STRING(str) __XIYUE_STRING(str)
#define __XIYUE_STRING(str) L ## str

#define XIYUE_ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#ifdef assert
#undef assert
#ifdef _DEBUG
#define assert(condition) if (!(condition)) {		\
	printf("%s(%d): Assertion `%s` failed!\n", __FILE__, __LINE__, #condition);		\
	__debugbreak();		\
}
#else
#define assert(condition)
#endif
#endif

#ifndef force_inline
#ifdef _DEBUG
#define force_inline inline
#elif (_MSC_VER >= 1200)
#define force_inline __forceinline
#elif defined(_WIN32)
#define force_inline __inline
#else
#define force_inline inline
#endif
#endif

namespace xiyue
{
	struct SmallRect
	{
		uint8_t left;
		uint8_t top;
		uint8_t right;
		uint8_t bottom;

		inline uint8_t width() const { return right - left; }
		inline uint8_t height() const { return bottom - top; }
	};

	template<typename T>
	struct Range
	{
		T start;
		T length;

		inline T end() const { return start + length; }
	};

	template<typename T>
	class Set : public std::set<T>
	{
	public:
		inline bool contains(const T& val) const { return std::set<T>::find(val) != std::set<T>::end(); }
	};

	template<typename K, typename V>
	class Map : public std::map<K, V>
	{
	public:
		inline bool insert(const K& key, const V& value) { return std::map<K, V>::insert(std::make_pair(key, value)).second; }
		inline bool contains(const K& key) { return std::map<K, V>::find(key) != std::map<K, V>::end(); }
	};

	template<typename T>
	inline bool isInRange(const T& val, const T& start, const T& end) {
		return val >= start && val < end;
	}

	wchar_t* itow(wchar_t* buffer, size_t length, int64_t number, int radix = 10);

	template<size_t size>
	inline wchar_t* itow(wchar_t(&buffer)[size], int64_t number, int radix = 10) {
		return itow(buffer, size, number, radix);
	}

	int64_t wtoi(const wchar_t* buffer);

	int64_t wcstoi(const wchar_t* start, const wchar_t* end, int radius);

	wchar_t* ftow(wchar_t* buffer, size_t length, double number);

	template<size_t size>
	inline wchar_t* ftow(wchar_t(&buffer)[size], double number) {
		return ftow(buffer, size, number);
	}

	double wtof(const wchar_t* buffer);

	template<typename T>
	inline size_t getMallocSize(size_t count) { return sizeof(T) * count; }
}
