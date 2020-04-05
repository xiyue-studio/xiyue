#pragma once

namespace xiyue
{
	class ConstString;
	class ConstStringPointer;
	ConstString operator"" _cs(const wchar_t* str, size_t size);

	struct ConstStringData
	{
		long nRefCount;
		long nBufferSize;		// 以 wchar_t 为单位
		wchar_t* stringData() { return (wchar_t*)(this + 1); }
	};

	/**
		常量字符串
	*/
	class ConstString
	{
	public:
		ConstString(const wchar_t* str);
		ConstString(const wchar_t* str, int len);
		ConstString(const ConstString& str);
		ConstString(ConstString&& str);
		ConstString(std::nullptr_t) = delete;
		ConstString(std::nullptr_t, int) = delete;
		ConstString(const std::wstring& str);
		ConstString();

		~ConstString();

		/*
			makeUnmanagedString

			创建出来的 ConstString 只保留 str 的指针，不会对其进行释放。
			请自行在外部保证指针的有效性。
			建议当 ConstString 指向字面量的时候使用这个函数，可以提高性能。
		*/
		static ConstString makeUnmanagedString(const wchar_t* str, int len);
		static ConstString makeUnmanagedString(const wchar_t* str);
		/*
			makeByReservedSize

			创建一个预留空间大小的内存，以便以后使用 operator=(const wchar_t*)
			的时候避免重新分配空间。

			size 的单位是字符数。
		*/
		static ConstString makeByReservedSize(size_t size);
		static ConstString makeByFormat(const wchar_t* format, ...);
		static ConstString fromInteger(int val);
		static ConstString fromDouble(double val, const wchar_t* format = nullptr);
		static ConstString makeByRepeat(const ConstString& s, int repeatNum);
		static ConstString makeByRepeat(const wchar_t* s, int repeatNum);


	public:
		wchar_t operator[](int index) const;
		ConstString& operator=(const ConstString& str);
		ConstString& operator=(std::nullptr_t) = delete;
		ConstString& operator=(const wchar_t* str);
		bool operator==(const ConstString& str) const;
		bool operator==(const wchar_t* str) const;
		friend bool operator==(const wchar_t* l, const ConstString& r);
		bool operator<(const ConstString& str) const;
		bool operator<(const wchar_t* str) const;
		friend bool operator<(const wchar_t* l, const ConstString& r);
		operator const wchar_t*() const;
		ConstStringPointer operator&();
		ConstString operator+(ConstString r) const;

	public:
		int length() const { return m_length; }
		bool isEmpty() const { return length() == 0; }
		const wchar_t* data() const;
		ConstStringPointer getPointer() const;
		bool isUnmanagedString() const { return m_unmanagedStringData != nullptr; }
		const wchar_t* cstr() const { return (const wchar_t*)*this; }
		bool equalsIgnoreCase(const ConstString& r) const;
		bool equalsIgnoreCase(const wchar_t* r) const;

	public:
		ConstString duplicate() const;
		ConstString substr(int start, int size) const;
		ConstString substr(int start) const;
		ConstString left(int size) const;
		ConstString right(int size) const;
		ConstString lTrim(const ConstString& trimChars = L""_cs) const;
		ConstString rTrim(const ConstString& trimChars = L""_cs) const;
		ConstString trim(const ConstString& trimChars = L""_cs) const;
		int find(const ConstString& str, int start = 0) const;
		int find(wchar_t ch, int start = 0) const;
		int reverseFind(const ConstString& str, int start = -1) const;
		int reverseFind(wchar_t ch, int start = -1) const;
		bool containsChar(wchar_t ch) const;
		bool canTransformToInt() const;
		bool canTransformToDouble() const;
		ConstString replaceEscapedChars() const;
		int toInt(int radius = 10) const;
		double toDouble() const;
		const wchar_t* begin() const;
		const wchar_t* end() const;
		void splitLines(std::vector<ConstString>& linesOut) const;

	public:
		static const int npos = -1;

	public:
		// 谨慎使用以下函数！
		ConstStringData* _getStringData() { return m_data; }
		void _resetLength(int len) { m_length = len; }

	private:
		int normalizeIndex(int index) const;

	private:
		mutable int m_start;
		int m_length;
		mutable ConstStringData* m_data;
		mutable const wchar_t* m_unmanagedStringData;
	};

	/**
		常量字符串指针
	*/
	class ConstStringPointer
	{
	public:
		explicit ConstStringPointer(ConstString str);

	public:
		// *p
		wchar_t operator*();
		// p++
		ConstStringPointer operator++(int);
		// ++p
		ConstStringPointer& operator++();
		// p--
		ConstStringPointer operator--(int);
		// --p
		ConstStringPointer& operator--();
		// p + n
		ConstStringPointer operator+(int offset);
		// p - n
		ConstStringPointer operator-(int offset);
		// p += n
		ConstStringPointer& operator+=(int offset);
		// p -= n
		ConstStringPointer& operator-=(int offset);
		// p = p2
		ConstStringPointer& operator=(const ConstStringPointer& r);

	public:
		int getOffset() const { return m_offset; }
		void reset(int offset) { m_offset = offset; }

	private:
		ConstString m_string;
		int m_offset;
	};

	inline ConstString operator"" _cs(const wchar_t* str, size_t size) {
		return ConstString::makeUnmanagedString(str, size);
	}

	std::wstring operator+(const std::wstring& l, ConstString r);
	std::wstring operator+(ConstString r, const std::wstring& l);
	std::wstring& operator+=(std::wstring& l, ConstString r);

#define XIYUE_CONST_STRING(fmt, ...) ::xiyue::ConstString::makeByFormat(fmt, __VA_ARGS__)
	// 便于 GTEST 使用
#define EXPECT_CONST_STRING_EQ(l, r) EXPECT_EQ((l), (r)) << "Left : " << (l).cstr() << "\nRight: " << (r).cstr() << "\n"
}

/*
	ConstString 的哈希函数，参考 STL 的 string 的哈希函数
*/
template <>
struct std::hash<xiyue::ConstString>
{
	std::size_t operator()(const xiyue::ConstString &key) const
	{
#if defined(_WIN64)
		constexpr size_t baseOffset = 14695981039346656037ULL;
		constexpr size_t prime = 1099511628211ULL;
#else
		constexpr size_t baseOffset = 2166136261U;
		constexpr size_t prime = 16777619U;
#endif
		std::size_t val = baseOffset;

		for (int index = 0; index < key.length(); ++index)
		{
			val ^= static_cast<size_t>(key[index]);
			val *= prime;
		}

		return val;
	}
};
