//------------------------------------------------
// Contains generic QB functions.
//------------------------------------------------

#include "QB.h"
#include <QB/LazyStruct.h>
#include <config.h>
#include <windows.h>

//---------------------------------------
// Run a script.
//---------------------------------------

typedef bool __cdecl RunScript_NativeCall(uint32_t name, void* params, void* object, void* object2);
RunScript_NativeCall* RunScript_Native = (RunScript_NativeCall*)(0x00475790);

bool RunScript(uint32_t name, Script::LazyStruct* params, void* object, void* object2)
{
	return RunScript_Native(name, params, object, 0);
}

//---------------------------------------
// Run a script. Arguments go into return struct.
//---------------------------------------

typedef void __cdecl RunScriptR_NativeCall(uint32_t name, Script::LazyStruct* params, void* object, Script::LazyStruct* params_out);
RunScriptR_NativeCall* RunScriptR_Native = (RunScriptR_NativeCall*)(0x00413200);

void RunScriptWithReturn(uint32_t name, Script::LazyStruct* params, void* object, Script::LazyStruct* params_out)
{
	RunScriptR_Native(name, params, object, params_out);
}

