#include "stdafx.h"
#include "xiyue_json_data_allocator.h"

using namespace std;
using namespace xiyue;

void xiyue::releaseJsonDataReference(JsonData* data, JsonDataAllocator* allocator)
{
	if (data == nullptr)
		return;

	if (--data->refCount <= 0)
	{
		allocator->freeJsonData(data);
	}
}
