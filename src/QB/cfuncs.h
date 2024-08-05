#include "shared.h"


void prepareCFuncs();
uint32_t Pointer_Functions();
uint32_t Pointer_FunctionCount();
void CopyStockFunctions();
void RedirectFunction(const char* func_name, void* new_offset);
uint32_t NumberOfFuncs();
void AddFunction(const char* func_id, void* func_offset);
//bool CFunc_PrintF(THAWPlus::LazyStruct* pParams);