//------------------------------------------------
// Contains generic QB functions.
//------------------------------------------------

#pragma once

#include <cstdint>
#include <QB/LazyStruct.h>

bool RunScript(uint32_t name, Script::LazyStruct* params, void* object);
void RunScriptWithReturn(uint32_t name, Script::LazyStruct* params, void* object, Script::LazyStruct* params_out);
