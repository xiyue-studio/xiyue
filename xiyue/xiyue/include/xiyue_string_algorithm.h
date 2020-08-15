#pragma once

namespace xiyue
{
	class BoyerMooreStringMatcher
	{
	public:
		explicit BoyerMooreStringMatcher(const wchar_t* pattern);
		BoyerMooreStringMatcher(const wchar_t* pattern, size_t patternLength);
		~BoyerMooreStringMatcher();

	public:
		const wchar_t* searchIn(const wchar_t* str, size_t strLen, size_t startPos = 0);
		const wchar_t* searchIn(const wchar_t* str, size_t startPos = 0) {
			return searchIn(str, wcslen(str), startPos);
		}

	protected:
		void prepareBmBc();
		void prepareBmGs();
		void prepareSuff();
		size_t getBcStep(wchar_t bc, size_t index);

	private:
		const wchar_t* m_pattern;
		size_t m_patternLength;
		std::unordered_map<wchar_t, size_t> m_bmBc;
		size_t* m_suff;
		size_t* m_bmGs;
	};
}
