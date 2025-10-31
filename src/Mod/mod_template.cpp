#include "mod_template.h"

/*
* Add custom CFuncs or patches for your mod here!
* After adding custom CFuncs they can be called from scripts. Example: MYMOD_ToggleWallrideAnywhere On

bool CFunc_ToggleWallrideAnywhere(Script::LazyStruct* pParams, void* pScript) {

	if (pParams->ContainsFlag(0xF649D637)) { //on
		patchJump((void*)0x00500441, wallrideanywhere_patch);
	}
	else if (pParams->ContainsFlag(0xDA427E80)) { //extended (wallrides don't stop when reaching the floor again)
		patchJump((void*)0x00500441, wallrideanywhere_patch);
		patchNop((void*)0x00500424, 6);
	}
	else if (pParams->ContainsFlag(0xD443A2BC)) { //off
		patchBytesM((void*)0x00500441, (BYTE*)"\x5E\x5B\x83\xC4\x10", 5);
	}
	return true;
}

bool CFunc_ToggleJankDrops(Script::LazyStruct* pParams, void* pScript) {

	if (pParams->ContainsFlag(0xF649D637)) { //on
		patchNop((void*)0x00502629, 6);
		patchDWord((void*)0x006467BC, 0x3F733333);
	}
	else if (pParams->ContainsFlag(0xD443A2BC)) { //off
		patchBytesM((void*)0x00502629, (BYTE*)"\x0F\x84\xF4\x00\x00\x00", 6);
		patchDWord((void*)0x006467BC, 0x3F34FDF4);
	}
	return true;
}

void __declspec(naked) wallrideanywhere_patch()
{
	__asm {
		cmp byte ptr ds : [esi + 0x228] , 0x0
		jne $ + 0x0E
		mov byte ptr ds : [esi + 0x22A] , 0x1
		pop esi
		pop ebx
		add esp, 0x10
		ret 8
	}
}

//Add a call to this function in DllMain
void addCustomCFuncs() {
	CFuncs::AddFunction("MYMOD_ToggleWallrideAnywhere", CFunc_ToggleWallrideAnywhere);
	CFuncs::AddFunction("MYMOD_ToggleJankDrops", CFunc_ToggleJankDrops);
	Log::TypedLog(CHN_DLL, "Adding custom CFuncs\n");
}
*/