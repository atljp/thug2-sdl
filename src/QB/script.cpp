#include "script.h"
#include "config.h"
#include "scriptcontent.h"
#include "LazyArray.h"
#include "malloc.h"

struct scriptsettings mScriptsettings;
struct inputsettings mInputsettings;
uint32_t sCreateScriptSymbol = 0x0046FE40; /* called in sCreateScriptSymbol wrapper */
bool walkspinpatched = false;
bool boardscuffpatched = false;
bool a = false;

struct DummyScript
{
	char unk1[20];
	Script::LazyStruct* GetParams;
	char unk2[176];
	uint32_t mScriptNameChecksum;
};

struct SkateInstance /* singleton of Skate::Instance() */
{
	char unk[888];
	uint32_t level;
};

/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=- Function definitions -=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

typedef bool __cdecl GetMemCardSpaceAvailable_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
GetMemCardSpaceAvailable_NativeCall* GetMemCardSpaceAvailable_Native = (GetMemCardSpaceAvailable_NativeCall*)(0x005A6E40);

typedef void* __cdecl sCreateScriptSymbol_NativeCall(uint32_t nameChecksum, uint32_t contentsChecksum, const uint8_t* p_data, uint32_t size, const char* p_fileName);
sCreateScriptSymbol_NativeCall* sCreateScriptSymbol_Native = (sCreateScriptSymbol_NativeCall*)(0x0046FE40);

typedef uint32_t CalculateScriptContentsChecksum_NativeCall(uint8_t* p_token);
CalculateScriptContentsChecksum_NativeCall* CalculateScriptContentsChecksum_Native = (CalculateScriptContentsChecksum_NativeCall*)(0x0046F960);

typedef bool __cdecl CreateScreenElement_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
CreateScreenElement_NativeCall* CreateScreenElement_Native = (CreateScreenElement_NativeCall*)(0x004AD240);

typedef bool __cdecl SetScreenElementProps_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
SetScreenElementProps_NativeCall* SetScreenElementProps_Native = (SetScreenElementProps_NativeCall*)(0x004AD4C0);

typedef uint8_t* (__thiscall* sCreateSymbolOfTheFormNameEqualsValue_NativeCall)(uint8_t* p_token, uint32_t nameChecksum, const char* p_fileName);
sCreateSymbolOfTheFormNameEqualsValue_NativeCall sCreateSymbolOfTheFormNameEqualsValue_Native = (sCreateSymbolOfTheFormNameEqualsValue_NativeCall)(0x00472240);

typedef uint32_t __cdecl GenerateCRCFromString_NativeCall(char* pName);
GenerateCRCFromString_NativeCall* GenerateCRCFromString_Native = (GenerateCRCFromString_NativeCall*)(0x00401B90);

typedef uint32_t __cdecl AddChecksumName_NativeCall(uint32_t checksum, char* p_name);
AddChecksumName_NativeCall* AddChecksumName_Native = (AddChecksumName_NativeCall*)(0x0046CF60);

typedef uint32_t* __cdecl CSymbolTableEntryResolve_NativeCall(uint32_t checksum);
CSymbolTableEntryResolve_NativeCall* CSymbolTableEntryResolve_Native = (CSymbolTableEntryResolve_NativeCall*)(0x00478CF0);

typedef void ParseQB_NativeCall(const char* p_fileName, uint8_t* p_qb, int ecx, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);
ParseQB_NativeCall* ParseQB_Native = (ParseQB_NativeCall*)(0x00472420);

typedef uint32_t __cdecl ScriptGetArray_NativeCall(uint32_t partChecksum);
ScriptGetArray_NativeCall* ScriptGetArray_Native = (ScriptGetArray_NativeCall*)(0x00478CC0);

typedef uint32_t __cdecl ScriptCleanUpAndRemoveSymbol_NativeCall(uint32_t p_symbolName);
ScriptCleanUpAndRemoveSymbol_NativeCall* ScriptCleanUpAndRemoveSymbol_Native = (ScriptCleanUpAndRemoveSymbol_NativeCall*)(0x004711D0);

typedef bool ExecuteCFuncPointer_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
ExecuteCFuncPointer_NativeCall* ExecuteCFuncPointer_Native = (ExecuteCFuncPointer_NativeCall*)(0x0044BD30);

typedef Script::LazyArray* __cdecl GlobalGetArray_NativeCall(uint32_t nameChecksum, uint32_t checksum, const Script::LazyStruct* p_struct);
GlobalGetArray_NativeCall* GlobalGetArray_Native = (GlobalGetArray_NativeCall*)(0x00479070);

/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-= Patched CFuncs =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

bool IsPS2_Patched(void* pParams, DummyScript* pScript)
{
	if (pScript->mScriptNameChecksum == 0x6AEC78DA)
		return true;
	return false;
}

bool GetMemCardSpaceAvailable_Patched(Script::LazyStruct* pParams, /* ebp + 0x8 */
									DummyScript* pScript, /* ebp+0xC */
									uint32_t a, /* ebp+0x10 */
									ULARGE_INTEGER b, /* ebp+0x14 */
									ULARGE_INTEGER c, /* ebp+0x1C */
									ULARGE_INTEGER d, /* ebp+0x24 */
									ULARGE_INTEGER e, /* ebp+0x2C */
									ULARGE_INTEGER f, /* epc+0x34 */
									uint8_t p_card) { /* ebp+0x3C */

	ULARGE_INTEGER mlpFreeBytesAvailableToCaller = {};
	ULARGE_INTEGER mlpTotalNumberOfBytes = {};
	uint32_t space_available = (INT_MAX - (UINT16_MAX * 2)); /* stdint.h */
	uint32_t space_available_result = 0;
	uint32_t GetNumFreeClusters_Result = 0;

	pScript->GetParams->AddInteger(0x855b2FC, 1000000); /* FilesLeft */

	if (p_card && GetDiskFreeSpaceExA(NULL, &mlpFreeBytesAvailableToCaller, &mlpTotalNumberOfBytes, NULL)) {
		space_available_result = ((mlpFreeBytesAvailableToCaller.HighPart << 0x16) + (mlpFreeBytesAvailableToCaller.LowPart >> 0xA));
		if (space_available_result <= space_available)
			space_available = space_available_result;
	}
	pScript->GetParams->AddInteger(0xC37C363, space_available); /* SpaceAvailable */
	return true;

	/* GetMemCardSpaceAvailable_Native(pParams, pScript); */
}

bool CreateScreenElement_Wrapper(Script::LazyStruct* pParams, DummyScript* pScript)
{
	SkateInstance* Skate = (SkateInstance*)*(uint32_t*)(0x007CE478);
	uint32_t p_checksum = 0;
	uint32_t p_checksum2 = 0;
	/* float values are processed according to the IEEE - 754 specification */

	if (Skate->level == 0xE92ECAFE && getaspectratio() > 1.34f) { /* level: load_mainmenu */

		if (pScript->mScriptNameChecksum == 0x7C92D11A) {  /* script: make_mainmenu_3d_plane */

			pParams->GetChecksum(0x40C698AF, &p_checksum, false);  /* id */

			if (p_checksum == 0xBC4B9584) /* bg_plane */
				pParams->AddInteger(0xED7C6031, 0xFFFFFEE7); /* cameraz */
		}
		else if (pScript->mScriptNameChecksum == 0xAD62B0B3) { /* script: build_roundbar */

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /* type */
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /* id */

			if (p_checksum == 0x5B9DA842 /* containerelement */ && p_checksum2 == 0x1954867E /* roundbar_bar */) {
				if (getaspectratio() > 1.6f) {
					pParams->AddFloat(0x13B9DA7B, 0.80f); /* scale */
					pParams->AddPair(0x7F261953, 157.0f, 213.0f); /* pos */
				}
				else {
					pParams->AddFloat(0x13B9DA7B, 0.88f); /* scale */
					pParams->AddPair(0x7F261953, 142.0f, 212.0f); /* pos */
				}
			}
		}
		else if (pScript->mScriptNameChecksum == 0x59F6E121) { /* script: make_spin_menu */

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /* type */
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /* id */

			if (p_checksum == 0x130EF802 /* vmenu */ && p_checksum2 == 0xB0524B44 /* main_vmenu */) {
				if (getaspectratio() > 1.6f) {
					pParams->AddPair(0x7F261953, 116.0f, 214.0f); /* pos */
					pParams->AddFloat(0x13B9DA7B, 0.72f); /* scale */
				}
				else {
					pParams->AddPair(0x7F261953, 95.0f, 213.0f); /* pos */
					pParams->AddFloat(0x13B9DA7B, 0.82f); /* scale */
				}
			}
		}
	}
	/* call CreateScreenElement with the received parameters */
	return CreateScreenElement_Native(pParams, pScript);
}

bool SetScreenElementProps_Wrapper(Script::LazyStruct* pParams, DummyScript* pScript)
{
	SkateInstance* Skate = (SkateInstance*)*(uint32_t*)(0x007CE478);
	uint32_t p_checksum = 0;
	bool done = FALSE;

	if (Skate->level == 0xE92ECAFE) { /* level: load_mainmenu */

		if (pScript->mScriptNameChecksum == 0xE2873769) { /* script: create_cas_modifier_menu */

			Script::LazyStruct* cas_menu_struct = Script::LazyStruct::s_create();
			cas_menu_struct->AddChecksum(0, 0xB94B715A); /* add_scaling */
			if (ExecuteCFuncPointer_Native(cas_menu_struct, pScript))
			{
				if (!done)
				{
					done = TRUE;
					bool x = SetScreenElementProps_Native(pParams, pScript);
					RunScript(0x36150445, pScript->GetParams, nullptr, nullptr); /* showboardmyan */
					cas_menu_struct->Clear();

					Script::LazyStruct* a = Script::LazyStruct::s_create();
					Script::LazyStruct* b = Script::LazyStruct::s_create();

					cas_menu_struct->AddChecksum(0x40C698AF, 0x15E31D81); /* id, mod_vmenu */
					a->AddChecksum(0, 0x7EE0FD2A); /* pad_back */
					a->AddChecksum(0, 0x36150445); /* showboardmyan */
					b->AddChecksum(0, 0x2BECBE33); /* turn_off */
					a->AddStructure(0x7031F10C, b); /* params */
					//new lazyarray
					Script::LazyArray* testarray = Script::LazyArray::s_create();
					testarray->SetSizeAndType(1, ESYMBOLTYPE_INTEGER);
					testarray->SetInteger(0, (int)a);
					cas_menu_struct->AddArray(0x475BF03C, testarray); /* event_handlers */
					SetScreenElementProps_Native(cas_menu_struct, pScript);

					/* clean up allocated space */
					if (b) {
						b->Clear();
						FreeQBStruct(b);
					}
					if (a) {
						a->Clear();
						FreeQBStruct(a);
					}
					if (testarray) {
						testarray->Clear();
						FreeQBArray(testarray);
					}
					if (cas_menu_struct) {
						cas_menu_struct->Clear();
						FreeQBStruct(cas_menu_struct);
						return x;
					}
				}
			}
		}
		else if (pScript->mScriptNameChecksum == 0x1B95F333) /* script: create_scale_options_menu */
		{
			pParams->GetChecksum(0x40C698AF, &p_checksum, false); /* id */

			if (p_checksum == 0x5E430716) /* scaling_vmenu */
			{
				removeScript(0xD2BE4CAF); /* skateshop_scaling_options */
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)skateshop_scaling_options_new, 0xD2BE4CAF, "scripts\\myan.qb"); /* data must not contain newlines but must end with one (token 0x01). returns pointer to last newline token */
			}
		}
	}
	/* scripts/mainmenu/levels/mainmenu/scalingmenu.txt*/
	return SetScreenElementProps_Native(pParams, pScript);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Scripty Stuff =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

void initScriptPatch()
{
	setDropDownKeys();
	loadScripts();
}

void LookUpSymbol_Patched(uint32_t checksum)
{
	if (mScriptsettings.airdrift && checksum == 0x1CA80417 && !walkspinpatched) {
		patchDWord((void*)(uint32_t)CSymbolTableEntryResolve_Native(checksum), 0);
		walkspinpatched = true;
	}
	else if (!mScriptsettings.boardscuffs && checksum == 0x9CE4DA4F && !boardscuffpatched) {
		patchDWord((void*)(uint32_t)CSymbolTableEntryResolve_Native(checksum), 0);
		boardscuffpatched = true;
	}
	if (walkspinpatched && boardscuffpatched)
		patchBytesM((void*)0x00474F25, (BYTE*)"\xE8\xC6\x3D\x00\x00", 5); /* unhook */

	CSymbolTableEntryResolve_Native(checksum);
}

void ParseQB_Patched(const char *p_fileName, uint8_t *p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable)
{
	if (!strcmp(p_fileName, "scripts\\game\\game.qb")) {
		ParseQB_Native(p_fileName, (uint8_t*)&game_new, 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);
	}
	else {
		ParseQB_Native(p_fileName, p_qb, 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);
	}
}

uint32_t __fastcall removeScript(uint32_t partChecksum)
{	
	uint32_t p_script = 0;
	p_script = ScriptGetArray_Native(partChecksum);
	if (p_script)	
		p_script = ScriptCleanUpAndRemoveSymbol_Native(p_script);
	return p_script;
}

void __fastcall sCreateScriptSymbolWrapper(uint32_t size, const uint8_t* p_data, uint32_t nameChecksum, uint32_t contentsChecksum, const char* p_fileName)
{
	__asm {
		push dword ptr ss : [ebp + 0x10] /* *p_fileName */
		push dword ptr ss : [ebp + 0xC] /* contentsChecksum */
		push dword ptr ss : [ebp + 0x8] /* nameChecksum */
		mov ebx, edx /* *p_data */
		mov eax, ecx  /* size */
		call dword ptr ds : sCreateScriptSymbol
		mov esp, ebp /* epilogue */
		pop ebp
		ret 0x0C
	}
}

void loadScripts()
{
	/* qb data in scriptcontent.h */

	removeScript(0x3B4548B8); /* POC */
	uint32_t contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)enter_kb_chat_new);
	sCreateScriptSymbolWrapper(0x9E, (uint8_t*)enter_kb_chat_new, 0x3B4548B8, contentsChecksum, "scripts\\game\\game.qb");

	//removeScript(0x5C51FEAB); /* test */
	//uint32_t contentsChecksum2 = CalculateScriptContentsChecksum_Native((uint8_t*)enablesun_new);
	//sCreateScriptSymbolWrapper(0x2B, (uint8_t*)enablesun_new, 0x5C51FEAB, contentsChecksum2, "scripts\\game\\env_fx.qb");

	removeScript(0x9F95228A); /* scalingmenu_get_limits */
	uint32_t temp = CalculateScriptContentsChecksum_Native((uint8_t*)scalingmenu_get_limits_original);
	//printf("AAAAAAAA: 0x%08x\n", temp);
	sCreateScriptSymbolWrapper(0x37, (uint8_t*)scalingmenu_get_limits_addition, 0x9F95228A, temp, "scripts\\myan.qb");

	//removeScript(0x1B95F333); /* create_scale_options_menu */
	//sCreateScriptSymbolWrapper(0x4EC, (uint8_t*)create_scale_options_menu_addition1, 0x1B95F333, 0xFC4A3248, "scripts\\myan.qb"); /* 0xFC4A3248 = contentsChecksum of original create_scale_options_menu script */

	uint32_t contentsChecksum3 = CalculateScriptContentsChecksum_Native((uint8_t*)showboardmyan);
	sCreateScriptSymbolWrapper(0x9C, (uint8_t*)showboardmyan, 0x36150445, contentsChecksum3, "scripts\\myan.qb"); /* new script: showboardmyan 0x36150445 */

	if (mScriptsettings.suninnetgame)
		removeScript(0x8054f197); /* disablesun */
}

void setDropDownKeys()
{
	/*
	if (pref_dropdown = L1)
		change GrindRelease = [ {
			trigger = { Press l1 100 }
			scr = SkateInOrBail
			params = { GrindRelease grindbail = Airborne moveright = -5 movey = 5 }
		} ]
	endif
	*/
	
	if (mScriptsettings.dropdowncontrol > 1)
	{
		Script::LazyStruct* grindrelease = Script::LazyStruct::s_create();
		Script::LazyStruct* trigger = Script::LazyStruct::s_create();
		Script::LazyStruct* params = Script::LazyStruct::s_create();

		if (mInputsettings.isPs2Controls)
		{
			if (mScriptsettings.dropdowncontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
			}
			else if (mScriptsettings.dropdowncontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/
			}
			else if (mScriptsettings.dropdowncontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
			}
			else if (mScriptsettings.dropdowncontrol == 5)
			{
				/*Set R2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x6BF8A7F4); /*R2*/
			}

		}
		else {
			if (mScriptsettings.dropdowncontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x767A45D7); /*black*/
			}
			else if (mScriptsettings.dropdowncontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBD30325B); /*white*/
			}
			else if (mScriptsettings.dropdowncontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
			}
			else if (mScriptsettings.dropdowncontrol == 5)
			{
				/*Set R2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x6BF8A7F4); /*R2*/
			}
		}

		trigger->AddInteger(0, 100);

		params->AddChecksum(0, 0x9077508B); /*GrindRelease*/
		params->AddChecksum(0xA7E24442, 0xCF3C89F7); /*GrindBail, Airborne*/
		params->AddInteger(0x1878C644, -5); /*MoveRight*/
		params->AddInteger(0x5A151ED3, 5); /*MoveY*/

		grindrelease->AddStructure(0xE594F0A2, trigger); /*Trigger*/
		grindrelease->AddChecksum(0xA6D2D890, 0x90E528CA); /*Scr, SkateOrInBail*/
		grindrelease->AddStructure(0x7031F10C, params); /*params*/

		Script::LazyArray* pref_dropdown = GlobalGetArray_Native(0x9077508B, 0, grindrelease); /*GrindRelease*/
		pref_dropdown->SetInteger(0, (int)grindrelease);
	}
}

void patchScripts()
{
	/* First, get config from INI. struct defined in config.h */
	loadScriptSettings(&mScriptsettings);
	loadInputSettings(&mInputsettings);
	patchJump((void*)0x005A5B32, initScriptPatch);
	patchDWord((void*)0x0068146C, (uint32_t)&IsPS2_Patched); /* returns true for the neversoft test skater */
	patchDWord((void*)0x0067F7D4, (uint32_t)&GetMemCardSpaceAvailable_Patched);
	patchDWord((void*)0x00680C6C, (uint32_t)&CreateScreenElement_Wrapper); /* adjusts scale and position of main menu screen elements in widescreen */
	patchDWord((void*)0x00680C84, (uint32_t)&SetScreenElementProps_Wrapper);
	patchCall((void*)0x00474F25, LookUpSymbol_Patched); /* accesses the global hash map */

	printf("Initializing CFuncs\n");

	//TEST
	//patchCall((void*)0x0046EEA3, ParseQB_Patched); /* loads script files */
	//uint32_t bb = 0xDEADBEEF;
	uint32_t bb = GenerateCRCFromString_Native("white");
	printf("0x%08x\n", bb);

	//patchJump((void*)0x005480A0, loadScripts); /* loads single functions of scripts and overwrites existing ones */
}