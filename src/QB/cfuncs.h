#pragma once
#include "global.h"

#include <string.h>
#include <cstdint>

namespace CFuncs
{
	struct CFunc
	{
		public:
			char* id;
			void* offset;
	};

	uint32_t NumberOfFuncs();

	CFunc* FindFunction(const char* func_name);
	void RedirectFunction(const char* func_name, void* new_offset);

	uint32_t Pointer_Functions();
	uint32_t Pointer_FunctionCount();

	void CopyStockFunctions();
	void AddFunction(const char* func_id, void* func_offset);
}

