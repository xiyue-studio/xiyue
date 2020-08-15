#pragma once
#include "xiyue_xy_re_program.h"

namespace xiyue
{
	struct XyReGroupPosition
	{
		uint32_t startPos;
		uint32_t endPos;
	};

	struct XyReThread
	{
		uint32_t id;
		XyReProgramPC pc;
		uint32_t backReferenceIndex;
		uint32_t backReferenceGroupID;
		bool isFailed;
		bool isSucceeded;
		bool isSuspended;
		bool isIgnoreCase;
		bool isMultiLineMode;
		bool isDotMatchNewLine;
		bool isUnicodeMode;
		bool isMatchingBackReference;
		bool isDelayed;

		inline void markFailed() { isFailed = true; isSucceeded = false; }
		inline void markSucceeded() { isSucceeded = true; isFailed = false; }
		inline void markDelayedAndPause() { isDelayed = true; isSuspended = true; }
		inline void markWorking() { isSuspended = false; }
		inline void pcInc() { ++pc; }
		inline void pcIncAndPause() { ++pc; isSuspended = true; }
		inline void pause() { isSuspended = true; }
		inline bool isWorking() const { return !isSuspended && !isSucceeded && !isFailed; }

		wchar_t getBackReferencedChar(const wchar_t* srcStr);
		uint32_t getGroupIndexById(const XyReProgram* program, int groupID);

		void saveGroupStart(const XyReProgram* program, int groupID, uint32_t startPos);
		void saveGroupEnd(const XyReProgram* program, int groupID, uint32_t endPos);

		void copyGroupsFromThread(const XyReProgram* program, XyReThread* t);

		XyReGroupPosition* getGroups();
	};
}
