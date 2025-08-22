#include "script.h"

struct modsettings mSettings;
LPVOID pResource_oslogo;
LPVOID pResource_keyboard_restored;

//Static asm addresses
uint32_t sCreateScriptSymbol = 0x0046FE40;

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

uint8_t console_wait_time[] = { //0x074DBDBB
	/*console_wait_time = 30*/
	0x17, 0x1E, 0x00, 0x00, 0x00, 0x01
};


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Init -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

void patchScripts() {
	loadSettings(&mSettings); /*get config from INI. struct defined in config.h*/
	patchCall((void*)0x005A5F42, &patchCFuncs);
	patchJump((void*)0x005A5B32, initScriptPatch);
}

void patchCFuncs() {
	CFuncs::RedirectFunction("CreateScreenElement", CreateScreenElement_Patched);
	CFuncs::RedirectFunction("IsPS2", IsPS2_Patched);
	CFuncs::RedirectFunction("GetMemCardSpaceAvailable", GetMemCardSpaceAvailable_Patched);
	
	if (!mSettings.noadditionalscriptmods) {
		CFuncs::RedirectFunction("SetScreenElementProps", SetScreenElementProps_Patched);
		if (pResource_oslogo = getResource(IDR_OSLOGO))  CFuncs::RedirectFunction("IsXBOX", IsXBOX_Patched);
	}
	if (mSettings.menubuttons == 2)
		/*Actually change the button actions when ps2 style menu navigation is selected*/
		CFuncs::RedirectFunction("SetButtonEventMappings", SetButtonEventMappings_Patched);

	Log::TypedLog(CHN_DLL, "Initializing CFuncs\n");
}

void initScriptPatch(){
	if (!mSettings.noadditionalscriptmods) {
		//TODO setButtonPrompts();
		Log::TypedLog(CHN_DLL, "Adjusting button prompts\n");
		editScriptsInMemory(); /*loads single functions of scripts and overwrites existing ones*/
		Log::TypedLog(CHN_DLL, "Patching scripts in memory\n");
		
	}
	setDropDownKeys();
	setCavemanKeys();
	setLadderGrabKeys();
}


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

typedef uint32_t __cdecl ScriptGetArray_NativeCall(uint32_t partChecksum);
ScriptGetArray_NativeCall* ScriptGetArray_Native = (ScriptGetArray_NativeCall*)(0x00478CC0);

typedef bool __cdecl SetButtonEventMappings_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
SetButtonEventMappings_NativeCall* SetButtonEventMappings_Native = (SetButtonEventMappings_NativeCall*)(0x0054F1B0);

typedef uint32_t __cdecl ScriptCleanUpAndRemoveSymbol_NativeCall(uint32_t p_symbolName);
ScriptCleanUpAndRemoveSymbol_NativeCall* ScriptCleanUpAndRemoveSymbol_Native = (ScriptCleanUpAndRemoveSymbol_NativeCall*)(0x004711D0);

typedef bool ExecuteCFuncPointer_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
ExecuteCFuncPointer_NativeCall* ExecuteCFuncPointer_Native = (ExecuteCFuncPointer_NativeCall*)(0x0044BD30);

typedef Script::LazyArray* __cdecl GlobalGetArray_NativeCall(uint32_t nameChecksum);
GlobalGetArray_NativeCall* GlobalGetArray_Native = (GlobalGetArray_NativeCall*)(0x00479070);

typedef void(__thiscall* LoadTextureFromBuffer_NativeCall)(int sp_sprite_tex_dict, uint8_t* p_buffer, uint32_t buffer_size, uint32_t texture_checksum, bool sprite, bool alloc_vram, bool perm, bool unk4);
LoadTextureFromBuffer_NativeCall LoadTextureFromBuffer_Native = (LoadTextureFromBuffer_NativeCall)(0x0049FA20);

typedef bool __cdecl ProfileLoggedIn_NativeCall(Script::LazyStruct* pParams, DummyScript* pScript);
ProfileLoggedIn_NativeCall* ProfileLoggedIn_Native = (ProfileLoggedIn_NativeCall*)(0x00548360);

typedef Script::LazyStruct* __cdecl GlobalGetStructure_NativeCall(uint32_t nameChecksum);
GlobalGetStructure_NativeCall* GlobalGetStructure_Native = (GlobalGetStructure_NativeCall*)(0x00479020);


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-= Patched CFuncs =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

bool IsPS2_Patched(void* pParams, DummyScript* pScript) {
	if (pScript->mScriptNameChecksum == 0x6AEC78DA /*check_for_neversoft_skaters*/)
		return true;
	return false;
}

bool IsXBOX_Patched(void* pParams, DummyScript* pScript) {
	if (pScript->mScriptNameChecksum == 0x98B57854) /*load_ps2_textures_to_main_memory*/
	{
		Script::LazyStruct* sprite_struct = Script::LazyStruct::s_create();
		if (sprite_struct)
		{
			sprite_struct->AddChecksum(0, 0x0B1BA9DE); /*permanenttextureheap*/
			RunScript(0xEB117FD0 /*MemPushContext*/, sprite_struct, nullptr, nullptr);
			LoadTextureFromBuffer_Native(*(int*)0x00700F1C, (uint8_t*)pResource_oslogo, 0x4000, 0x2074BEAE /*gslogo*/, true, true, true, true);
			RunScript(0x3417C307 /*MemPopContext*/, nullptr, nullptr, nullptr);
			sprite_struct->Clear();
			FreeQBStruct(sprite_struct);
			return false;
		}
	}
	return true;
}

bool GetMemCardSpaceAvailable_Patched(Script::LazyStruct* pParams, /*ebp + 0x8*/
									DummyScript* pScript, /*ebp+0xC*/
									uint32_t a, /*ebp+0x10*/
									ULARGE_INTEGER b, /*ebp+0x14*/
									ULARGE_INTEGER c, /*ebp+0x1C*/
									ULARGE_INTEGER d, /*ebp+0x24*/
									ULARGE_INTEGER e, /*ebp+0x2C*/
									ULARGE_INTEGER f, /*epc+0x34*/
									uint8_t p_card) { /*ebp+0x3C*/

	ULARGE_INTEGER mlpFreeBytesAvailableToCaller = {};
	ULARGE_INTEGER mlpTotalNumberOfBytes = {};
	uint32_t space_available = (INT_MAX - (UINT16_MAX * 2)); /*stdint.h*/
	uint32_t space_available_result = 0;
	uint32_t GetNumFreeClusters_Result = 0;

	pScript->GetParams->AddInteger(0x855b2FC, 1000000); /*FilesLeft*/

	if (p_card && GetDiskFreeSpaceExA(NULL, &mlpFreeBytesAvailableToCaller, &mlpTotalNumberOfBytes, NULL)) {
		space_available_result = ((mlpFreeBytesAvailableToCaller.HighPart << 0x16) + (mlpFreeBytesAvailableToCaller.LowPart >> 0xA));
		if (space_available_result <= space_available)
			space_available = space_available_result;
	}
	pScript->GetParams->AddInteger(0xC37C363, space_available); /*SpaceAvailable*/
	return true;

	/*GetMemCardSpaceAvailable_Native(pParams, pScript);*/
}

bool CreateScreenElement_Patched(Script::LazyStruct* pParams, DummyScript* pScript) {
	SkateInstance* Skate = (SkateInstance*)*(uint32_t*)(0x007CE478);
	uint32_t p_checksum = 0;
	uint32_t p_checksum2 = 0;

	if ((Skate->level == 0xE92ECAFE && getaspectratio() > 1.34f && getaspectratio() < 1.8f)) { /*level: load_mainmenu*/

		if (pScript->mScriptNameChecksum == 0x7C92D11A) {  /*script: make_mainmenu_3d_plane*/

			pParams->GetChecksum(0x40C698AF, &p_checksum, false);  /*id*/

			if (p_checksum == 0xBC4B9584) /*bg_plane*/
				pParams->AddInteger(0xED7C6031, -281); /*cameraz*/
		}
		else if (pScript->mScriptNameChecksum == 0xAD62B0B3) { /*script: build_roundbar*/

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /*type*/
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /*id*/

			if (p_checksum == 0x5B9DA842 /*containerelement*/ && p_checksum2 == 0x1954867E /*roundbar_bar*/) {
				if (getaspectratio() > 1.6f) {
					pParams->AddFloat(0x13B9DA7B, 0.80f); /*scale*/
					pParams->AddPair(0x7F261953, 157.0f, 213.0f); /*pos*/
				}
				else {
					pParams->AddFloat(0x13B9DA7B, 0.88f); /*scale*/
					pParams->AddPair(0x7F261953, 142.0f, 212.0f); /*pos*/
				}
			}
		}
		else if (pScript->mScriptNameChecksum == 0x59F6E121) { /*script: make_spin_menu*/

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /*type*/
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /*id*/

			if (p_checksum == 0x130EF802 /* vmenu */ && p_checksum2 == 0xB0524B44 /*main_vmenu*/) {
				if (getaspectratio() > 1.6f) {
					pParams->AddPair(0x7F261953, 116.0f, 214.0f); /*pos*/
					pParams->AddFloat(0x13B9DA7B, 0.72f); /*scale*/
				}
				else {
					pParams->AddPair(0x7F261953, 95.0f, 213.0f); /*pos*/
					pParams->AddFloat(0x13B9DA7B, 0.82f); /*scale*/
				}
			}
		}
	}
	else if (Skate->level == 0xE92ECAFE && getaspectratio() > 1.8f) {

		if (pScript->mScriptNameChecksum == 0x7C92D11A) {  /*script: make_mainmenu_3d_plane*/

			pParams->GetChecksum(0x40C698AF, &p_checksum, false);  /*id*/

			if (p_checksum == 0xBC4B9584) /*bg_plane*/
				pParams->AddInteger(0xED7C6031, -281); /*cameraz*/
		}
		else if (pScript->mScriptNameChecksum == 0xAD62B0B3) { /*script: build_roundbar*/

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /*type*/
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /*id*/

			if (p_checksum == 0x5B9DA842 /*containerelement*/ && p_checksum2 == 0x1954867E /*roundbar_bar*/) {
				
					pParams->AddFloat(0x13B9DA7B, 0.60f); /*scale*/
					pParams->AddPair(0x7F261953, 200.0f, 216.0f); /*pos*/
			}
		}
		else if (pScript->mScriptNameChecksum == 0x59F6E121) { /*script: make_spin_menu*/

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false); /*type*/
			pParams->GetChecksum(0x40C698AF, &p_checksum2, false); /*id*/

			if (p_checksum == 0x130EF802 /* vmenu */ && p_checksum2 == 0xB0524B44 /*main_vmenu*/) {
					pParams->AddPair(0x7F261953, 164.0f, 227.0f); /*pos*/
					pParams->AddFloat(0x13B9DA7B, 0.60f); /*scale*/
			}
		}
	}
	if (!mSettings.noadditionalscriptmods) {
		if (pScript->mScriptNameChecksum == 0x85E146D5) { /*create_snazzy_dialog_box*/

			pParams->GetChecksum(0x7321A8D6, &p_checksum, false);  /*type*/

			if (p_checksum == 0x5200DFB6 || p_checksum == 0x40D92263) { /*textelement, textBlockElement*/

				const char* text_content_gs;

				pParams->GetText(0xC4745838/*text*/, &text_content_gs, 0);
				if (strstr(text_content_gs, "GameSpy")) {
					char* text_content_os = setText(text_content_gs, "GameSpy", "OpenSpy");
					pParams->AddString(0xC4745838/*text*/, text_content_os);
				}
				else if (strstr(text_content_gs, "GAMESPY")) {
					char* text_content_os = setText(text_content_gs, "GAMESPY", "OPENSPY");
					pParams->AddString(0xC4745838/*text*/, text_content_os);
				}
			}
		}
		else {
			pParams->GetChecksum(0x7321A8D6, &p_checksum, false);  /*type*/

			if (p_checksum == 0xB12B510A /*SpriteElement*/) {
				pParams->GetChecksum(0x7D99F28D, &p_checksum, false);  /*texture*/

				if (p_checksum == 0x2074BEAE) { /*gslogo*/
					if (pScript->mScriptNameChecksum == 0x67DA84A7) { /*make_server_list_menu*/
						pParams->AddPair(0x7F261953 /*pos*/, -320.0f, -35.0f);
						pParams->AddFloat(0x13B9DA7B /*scale*/, 1.3f);
					}
				}
			}
		}
	}
	return CreateScreenElement_Native(pParams, pScript);
}

bool SetScreenElementProps_Patched(Script::LazyStruct* pParams, DummyScript* pScript) {
	SkateInstance* Skate = (SkateInstance*)*(uint32_t*)(0x007CE478);
	uint32_t p_checksum = 0;
	bool return_val = 0;
	BOOL done = FALSE;
	BOOL patched_color_sliders = FALSE;

	if (Skate->level == 0xE92ECAFE) { /*load_mainmenu*/

		if (pScript->mScriptNameChecksum == 0xE2873769) { /*script: create_cas_modifier_menu*/

			Script::LazyStruct* cas_menu = Script::LazyStruct::s_create();
			cas_menu->AddChecksum(0, 0xB94B715A); /*add_scaling*/
			if (ExecuteCFuncPointer_Native(cas_menu, pScript))
			{
				if (!done)
				{
					/*
					exit_board_scaling = [ 
						event_handlers = { 
							id = mod_vmenu
							pad_back
							showboardmyan params = { turn_off } 
						} 
					]
					*/

					done = TRUE;
					BOOL native_val = SetScreenElementProps_Native(pParams, pScript);
					RunScript(0x36150445, pScript->GetParams, nullptr, nullptr); /*showboardmyan*/
					cas_menu->Clear();

					Script::LazyStruct* event_handlers = Script::LazyStruct::s_create();
					Script::LazyStruct* params = Script::LazyStruct::s_create();

					cas_menu->AddChecksum(0x40C698AF, 0x15E31D81); /*id, mod_vmenu*/
					event_handlers->AddChecksum(0, 0x7EE0FD2A); /*pad_back*/
					event_handlers->AddChecksum(0, 0x36150445); /*showboardmyan*/
					params->AddChecksum(0, 0x2BECBE33); /*turn_off*/
					event_handlers->AddStructure(0x7031F10C, params); /*params*/

					Script::LazyArray* exit_board_scaling = Script::LazyArray::s_create();
					exit_board_scaling->SetSizeAndType(1, ESYMBOLTYPE_INTEGER);
					exit_board_scaling->SetStructure(0, event_handlers);
					cas_menu->AddArray(0x475BF03C, exit_board_scaling); /*event_handlers*/
					SetScreenElementProps_Native(cas_menu, pScript);

					/*clean up allocated space*/
					if (params) {
						params->Clear();
						FreeQBStruct(params);
					}
					if (event_handlers) {
						event_handlers->Clear();
						FreeQBStruct(event_handlers);
					}
					if (exit_board_scaling) {
						exit_board_scaling->Clear();
						FreeQBArray(exit_board_scaling);
					}
					if (cas_menu) {
						cas_menu->Clear();
						FreeQBStruct(cas_menu);
						return native_val;
					}
				}
			}
			else if (!patched_color_sliders)
			{
				/*patch color sliders while we're in the CAS menu. earlier patches get overwritten*/
				removeScript(0xB26B0D6F);
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)float_val_zero, 0xB26B0D6F /*colormenu_min_value*/, "scripts\\mainmenu\\levels\\mainmenu\\colormenu.qb");
				removeScript(0x6580BF2E);
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)float_val_hundred, 0x6580BF2E /*colormenu_max_value*/, "scripts\\mainmenu\\levels\\mainmenu\\colormenu.qb");
				removeScript(0x63BBA1ED);
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)float_val_zero, 0x63BBA1ED /*colormenu_min_saturation*/, "scripts\\mainmenu\\levels\\mainmenu\\colormenu.qb");
				removeScript(0xDA9D3A9C);
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)float_val_hundred, 0xDA9D3A9C /*colormenu_max_saturation*/, "scripts\\mainmenu\\levels\\mainmenu\\colormenu.qb");
				patched_color_sliders = TRUE;
			}
		}
		else if (pScript->mScriptNameChecksum == 0x1B95F333) /*script: create_scale_options_menu*/
		{
			pParams->GetChecksum(0x40C698AF, &p_checksum, false); /*id*/

			if (p_checksum == 0x5E430716) /*scaling_vmenu*/
			{
				removeScript(0xD2BE4CAF); /*skateshop_scaling_options*/
				sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)skateshop_scaling_options_new, 0xD2BE4CAF, "scripts\\myan.qb"); /*data without newlines, ends with newline (token 0x01). returns pointer to last newline token*/
			}
		}
	}

	if (pScript->mScriptNameChecksum == 0xB9ED9B74) { /*script: create_internet_options*/

		pParams->GetChecksum(0x40C698AF /*id*/, &p_checksum, 0);

		if (p_checksum == 0x455A37D3 /*menu_create_profile*/) {
			if (ProfileLoggedIn_Native(pParams, pScript)) {
				return_val = SetScreenElementProps_Native(pParams, pScript);
				Script::LazyStruct* new_item = Script::LazyStruct::s_create();
				Script::LazyStruct* menu_create_profile_struct = Script::LazyStruct::s_create();

				//new_item = { id = { menu_create_profile child = 0} text = "Change OpenSpy Login" }
				if (new_item) {
					menu_create_profile_struct->AddChecksum(0, 0x455A37D3 /*menu_create_profile*/);
					menu_create_profile_struct->AddInteger(0xDD4CABD6 /*child*/, 0);
					new_item->AddStructure(0x40C698AF /*id*/, menu_create_profile_struct);
					new_item->AddString(0xC4745838 /*text*/, "Change OpenSpy Login");
					SetScreenElementProps_Native(new_item, pScript);

					if (menu_create_profile_struct) {
						menu_create_profile_struct->Clear();
						FreeQBStruct(menu_create_profile_struct);
						new_item->Clear();
						FreeQBStruct(new_item);
					}
				}
				return return_val;
			}
		}
		else if (p_checksum == 0x62D6356F /*menu_save*/) {
			/*
			 theme_menu_add_item {
				text = 'Save Settings'
				id = menu_save
				pad_choose_script = launch_save_internet_settings
				centered
				focus_script = internet_options_focus
				focus_params = { texture = io_settings }
				unfocus_script = internet_options_unfocus
			}
			SetScreenElementProps {
				id = menu_save
				event_handlers = [
					{focus PlayHelperDesc params = { save_settings_helper_desc }}
					{ unfocus StopHelperDesc }
				]
			}
			*/
			Script::LazyStruct* new_item = Script::LazyStruct::s_create();

			if (new_item) {
				new_item->AddString(0xC4745838 /*text*/, "Load Settings");
				new_item->AddChecksum(0x40C698AF /*id*/, 0x12B6EBAC /*menu_load*/);
				new_item->AddChecksum(0x2B0CFCA1 /*pad_choose_script*/, 0x07440DFA /*LoadInternetOptions_PadChooseScript_myan*/); //0x9a16e1e1
				new_item->AddChecksum(0, 0x2A434D05 /*centered*/);
				new_item->AddChecksum(0x5A8C82DB /*focus_script*/, 0x33B80F0B /*internet_options_focus*/);
				Script::LazyStruct* focus_params = Script::LazyStruct::s_create();

				if (focus_params) {
					focus_params->AddChecksum(0x7D99F28D /*texture*/, 0xB2814858 /*io_settings*/);
					new_item->AddStructure(0xC9C30B12 /*focus_params*/, focus_params);
					new_item->AddChecksum(0x6A3B36B5 /*unfocus_script*/, 0x9E0794DA /*internet_options_unfocus*/);
					RunScript(0xC16BF7BD /*theme_menu_add_item*/, new_item, nullptr, nullptr);

					if (focus_params) {
						new_item->Free();
						new_item->AddChecksum(0x40C698AF /*id*/, 0x12B6EBAC /*menu_load*/);
						Script::LazyStruct* event_handlers_struct = Script::LazyStruct::s_create();
						Script::LazyStruct* save_settings_helper_desc = Script::LazyStruct::s_create();
						save_settings_helper_desc->AddChecksum(0, 0x926D3E69 /*LoadInternetOptions_HelperDesc_myan*/); //0x4f09c570
						event_handlers_struct->AddChecksum(0, 0x9D3FB516 /*focus*/);
						event_handlers_struct->AddChecksum(0, 0x28C8A177 /*PlayHelperDesc*/);
						event_handlers_struct->AddStructure(0x7031F10C /*params*/, save_settings_helper_desc);
						focus_params->AddChecksum(0, 0x4ADF0CD3 /*unfocus*/);
						focus_params->AddChecksum(0, 0x28D11C80  /*StopHelperDesc*/);

						Script::LazyArray* event_handlers = Script::LazyArray::s_create();
						if (event_handlers_struct) {
							event_handlers->SetSizeAndType(2, ESYMBOLTYPE_STRUCTURE);
							event_handlers->SetStructure(0, event_handlers_struct);
							event_handlers->SetStructure(1, focus_params);
							new_item->AddArray(0x475BF03C /*event_handlers*/, event_handlers);
							SetScreenElementProps_Native(new_item, pScript);

							save_settings_helper_desc->Clear();
							FreeQBStruct(save_settings_helper_desc);
							event_handlers_struct->Clear();
							FreeQBStruct(event_handlers_struct);
							if (focus_params) {
								focus_params->Clear();
								FreeQBStruct(focus_params);
							}
							event_handlers->Clear();
							FreeQBArray(event_handlers);
							new_item->Clear();
							FreeQBStruct(new_item);
						}
					}
				}
			}

		}
	}
	/*scripts/mainmenu/levels/mainmenu/scalingmenu.txt*/
	return SetScreenElementProps_Native(pParams, pScript);
}

bool SetButtonEventMappings_Patched(Script::LazyStruct* pParams, DummyScript* pScript) {

	if (pScript->mScriptNameChecksum == 0xE2602BAC /*setup_main_button_event_mappings*/) {
		Script::LazyArray* ButtonEventMap_xbox = nullptr;
		Script::LazyArray* ButtonMap = nullptr;

		pParams->GetArray(0x87D839B8 /*xbox*/, &ButtonEventMap_xbox);		
		ButtonMap = ButtonEventMap_xbox->GetArray(6); //edit first entry inside array: [ b pad_back ] => [ y pad_back ]
		ButtonMap->SetChecksum(0, 0x0424D9EA /*y*/);
		//This makes speech bubbles react to triangle
		//ButtonMap = ButtonEventMap_xbox->GetArray(7); //[ b pad_circle ] => [ y pad_circle ]
		//ButtonMap->SetChecksum(0, 0x0424D9EA /*y*/);
		ButtonMap = ButtonEventMap_xbox->GetArray(12); //[ y pad_triangle2 ] => [ b pad_triangle2 ]
		ButtonMap->SetChecksum(0, 0x8E411006 /*b*/);
		ButtonMap = ButtonEventMap_xbox->GetArray(14); //[ y pad_space ] => [ b pad_space ]
		ButtonMap->SetChecksum(0, 0x8E411006 /*b*/);
		ButtonMap = ButtonEventMap_xbox->GetArray(17); //[ y pad_expand ] => [ b pad_expand ]
		ButtonMap->SetChecksum(0, 0x8E411006 /*b*/);
	}
	return SetButtonEventMappings_Native(pParams, pScript);
}

/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Scripty Stuff =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

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

void editScriptsInMemory()
{
	/* 
	qb data in scriptcontent.h
	calling sCreateSymbolOfTheFormNameEqualsValue_Native here requires manual stack cleanup: __asm {add esp, 0x8}
	*/

	removeScript(0x3B4548B8); /* longer text input */
	uint32_t contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)enter_kb_chat_new);
	sCreateScriptSymbolWrapper(0x9E, (uint8_t*)enter_kb_chat_new, 0x3B4548B8, contentsChecksum, "scripts\\game\\game.qb");

	//removeScript(0x5C51FEAB);
	//uint32_t contentsChecksum2 = CalculateScriptContentsChecksum_Native((uint8_t*)enablesun_new);
	//sCreateScriptSymbolWrapper(0x2B, (uint8_t*)enablesun_new, 0x5C51FEAB, contentsChecksum2, "scripts\\game\\env_fx.qb");

	removeScript(0x9F95228A); /* scalingmenu_get_limits */
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)scalingmenu_get_limits_original);
	sCreateScriptSymbolWrapper(0x37, (uint8_t*)scalingmenu_get_limits_addition, 0x9F95228A, contentsChecksum, "scripts\\myan.qb");

	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)showboardmyan);
	sCreateScriptSymbolWrapper(0x9C, (uint8_t*)showboardmyan, 0x36150445, contentsChecksum, "scripts\\myan.qb"); /* new script: showboardmyan 0x36150445 */

	//Helper: LoadInternetOptions_AbortAndDoneScript_myan
	removeScript(0x33317668);
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)LoadInternetOptions_AbortAndDoneScript_myan);
	sCreateScriptSymbolWrapper(sizeof(LoadInternetOptions_AbortAndDoneScript_myan), (uint8_t*)LoadInternetOptions_AbortAndDoneScript_myan, 0x33317668, contentsChecksum, "scripts\\myan.qb");

	//Helper: LoadInternetOptions_RetryScript_myan
	removeScript(0x1C253B2E);
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)LoadInternetOptions_RetryScript_myan);
	sCreateScriptSymbolWrapper(sizeof(LoadInternetOptions_RetryScript_myan), (uint8_t*)LoadInternetOptions_RetryScript_myan, 0x1C253B2E, contentsChecksum, "scripts\\myan.qb");

	//Helper: LoadInternetOptions_PadChooseScript_myan
	removeScript(0x07440DFA);
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)LoadInternetOptions_PadChooseScript_myan);
	sCreateScriptSymbolWrapper(sizeof(LoadInternetOptions_PadChooseScript_myan), (uint8_t*)LoadInternetOptions_PadChooseScript_myan, 0x07440DFA, contentsChecksum, "scripts\\myan.qb");

	//Helper: LoadInternetOptions_HelperDesc_myan
	removeScript(0x926D3E69);
	sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)LoadInternetOptions_HelperDesc_myan, 0x926D3E69, "scripts\\myan.qb");
	__asm {add esp, 0x8}

	if (!mSettings.boardscuffs)
		removeScript(GenerateCRCFromString_Native("DoBoardScuff"));

	if (mSettings.quickgetup)
	{
		removeScript(0x8F488DCA); /*bail_quick_getup2*/
		contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)bail_quick_getup2_new);
		sCreateScriptSymbolWrapper(0x5A, (uint8_t*)bail_quick_getup2_new, 0x8F488DCA, contentsChecksum, "scripts\\game\\skater\\bails.qb");

		removeScript(0x67823B68); /*baildone*/
		contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)baildone_new);
		sCreateScriptSymbolWrapper(0xC6, (uint8_t*)baildone_new, 0x67823B68, contentsChecksum, "scripts\\game\\skater\\bails.qb");

		removeScript(GenerateCRCFromString_Native("NoQuickGetup"));
	}

	if (!mSettings.walkspin)
		removeScript(0x1CA80417); /*flip_skater_if_180_off*/

	//No freeze when changing levels in net games
	removeScript(0x39C58EA1); /*change_level*/
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)change_level_new);
	sCreateScriptSymbolWrapper(sizeof(change_level_new), (uint8_t*)change_level_new, 0x39C58EA1, contentsChecksum, "game\\skutils.qb");

	//No chat reset after games start
	if (mSettings.consolewaittime != 30) {
		if (mSettings.consolewaittime && mSettings.consolewaittime <= 120) console_wait_time[1] = mSettings.consolewaittime;
	}

	removeScript(0x074DBDBB); /*console_wait_time*/
	sCreateSymbolOfTheFormNameEqualsValue_Native((uint8_t*)console_wait_time, 0x074DBDBB, "engine\\menu\\consolemessage.qb");
	__asm {add esp, 0x8}

	removeScript(0x0C42890C); /*kill_net_panel_messages*/
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)kill_net_panel_messages_new);
	sCreateScriptSymbolWrapper(sizeof(kill_net_panel_messages_new), (uint8_t*)kill_net_panel_messages_new, 0x0C42890C, contentsChecksum, "scripts\\game\\net\\net.qb");

	removeScript(0xCCB19938); /*create_console_message*/
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)create_console_message_new);
	sCreateScriptSymbolWrapper(sizeof(create_console_message_new), (uint8_t*)create_console_message_new, 0xCCB19938, contentsChecksum, "engine\\menu\\consolemessage.qb");

	removeScript(0xF2F8DF40); /*console_message_wait_and_die*/
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)console_message_wait_and_die_new);
	sCreateScriptSymbolWrapper(sizeof(console_message_wait_and_die_new), (uint8_t*)console_message_wait_and_die_new, 0xF2F8DF40, contentsChecksum, "engine\\menu\\consolemessage.qb");

	removeScript(0x8C34FE0A); /*gameflow_startrun*/
	contentsChecksum = CalculateScriptContentsChecksum_Native((uint8_t*)gameflow_startrun_new);
	sCreateScriptSymbolWrapper(sizeof(gameflow_startrun_new), (uint8_t*)gameflow_startrun_new, 0x8C34FE0A, contentsChecksum, "game\\gameflow.qb");
}

void setDropDownKeys() {
	/*
	if (pref_dropdown = L1)
		change GrindRelease = [ {
			trigger = { Press l1 100 }
			scr = SkateInOrBail
			params = { GrindRelease grindbail = Airborne moveright = -5 movey = 5 }
		} ]
	endif

	PC controls: white = L2 / black = R2 / L1 = L1 / R1 = R1
	*/
	if (mSettings.dropdowncontrol && mSettings.dropdowncontrol < 6)
	{
		Script::LazyStruct* grindrelease = Script::LazyStruct::s_create();
		Script::LazyStruct* trigger = Script::LazyStruct::s_create();
		Script::LazyStruct* params = Script::LazyStruct::s_create();

		if (mSettings.isPs2Controls)
		{
			if (mSettings.dropdowncontrol == 1 || mSettings.dropdowncontrol == 5)
			{
				/*Set R2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x6BF8A7F4); /*R2*/
			}
			if (mSettings.dropdowncontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
			}
			else if (mSettings.dropdowncontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/
			}
			else if (mSettings.dropdowncontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
			}
		}
		else {
			if (mSettings.dropdowncontrol == 1) {
				// No dropdowns on THUG PC originally. Set default to THUG2 PC dropdown L2+R2

				/*Set L2+R2*/
				trigger->AddChecksum(0, 0x3F369070); /*PressTwoAnyOrder*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
				trigger->AddChecksum(0, 0x6bF8A7F4); /*R2*/
			}
			if (mSettings.dropdowncontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBD30325B); /*white*/
			}
			else if (mSettings.dropdowncontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x767A45D7); /*black*/
			}
			else if (mSettings.dropdowncontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xbFB9982B); /*L2*/
			}
			else if (mSettings.dropdowncontrol == 5)
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

		Script::LazyArray* pref_dropdown = GlobalGetArray_Native(0x9077508B); // , 0, grindrelease); /*GrindRelease*/
		pref_dropdown->SetStructure(0, grindrelease);
	}
}

void setCavemanKeys() {
	//CavemanKey=1 ; 1 = default, 2 = L1, 3 = R1, 4 = L2, 5 = R2, 6 = L1+R1, 7 = L2+R2 ; PC default: black or white which is R2 or L2
	//In PC controls, white = L2 / black = R2
	if (mSettings.cavemancontrol > 1)
	{
		Script::LazyStruct* trigger = Script::LazyStruct::s_create();

		if (mSettings.isPs2Controls)
		{
			if (mSettings.cavemancontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 5)
			{
				/*Set R2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x6BF8A7F4); /*R2*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 6)
			{
				/*Set L1+R1*/
				trigger->AddChecksum(0, 0x3F369070); /*PressTwoAnyOrder*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/
				trigger->AddInteger(0, 400);
			}
			else if (mSettings.cavemancontrol == 7)
			{
				/*Set L2+R2*/
				trigger->AddChecksum(0, 0x3F369070); /*PressTwoAnyOrder*/
				trigger->AddChecksum(0, 0xBFB9982B); /*L2*/
				trigger->AddChecksum(0, 0x6BF8A7F4); /*R2*/
				trigger->AddInteger(0, 400);
			}
		}
		else {
			if (mSettings.cavemancontrol == 2)
			{
				/*Set L1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xBD30325B); /*white*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 3)
			{
				/*Set R1*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x767A45D7); /*black*/
				trigger->AddInteger(0, 10);

			}
			else if (mSettings.cavemancontrol == 4)
			{
				/*Set L2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
				trigger->AddInteger(0, 10);
			}
			else if (mSettings.cavemancontrol == 5)
			{
				/*Set R2*/
				trigger->AddChecksum(0, 0x823B8342); /*press*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/
				trigger->AddInteger(0, 10);

			}
			else if (mSettings.cavemancontrol == 6)
			{
				/*Set L1+R1*/
				trigger->AddChecksum(0, 0x3F369070); /*PressTwoAnyOrder*/
				trigger->AddChecksum(0, 0x767A45D7); /*black*/
				trigger->AddChecksum(0, 0xBD30325B); /*white*/
				trigger->AddInteger(0, 400);
			}
			else if (mSettings.cavemancontrol == 7)
			{
				/*Set L2+R2*/
				trigger->AddChecksum(0, 0x3F369070); /*PressTwoAnyOrder*/
				trigger->AddChecksum(0, 0x26B0C991); /*L1*/
				trigger->AddChecksum(0, 0xF2F1F64E); /*R1*/

				trigger->AddInteger(0, 400);
			}

		}
		Script::LazyStruct* pref_caveman = GlobalGetStructure_Native(0xB32BEA02); //SwitchControl_Trigger
		pref_caveman->Free();
		pref_caveman->AddStructurePointer(0xE594F0A2, trigger); //trigger
	}
}

void setLadderGrabKeys() {

	if (mSettings.laddergrabcontrol == 2) { // spinleft
		patchBytesM((BYTE*)(0x0052C7A0 + 2), (BYTE*)"\xC0\x00", 2);
		patchBytesM((BYTE*)(0x0052D4CB + 2), (BYTE*)"\xC0\x00", 2);
		patchBytesM((BYTE*)(0x0052D06C + 2), (BYTE*)"\xA0\x00", 2);
	}
	else if (mSettings.laddergrabcontrol == 3) { // L3 
		patchBytesM((BYTE*)(0x0052C7A0 + 2), (BYTE*)"\xE0\x00", 2);
		patchBytesM((BYTE*)(0x0052D4CB + 2), (BYTE*)"\xE0\x00", 2);
		patchBytesM((BYTE*)(0x0052D06C + 2), (BYTE*)"\xC0\x00", 2);
	}
}

char* setText(const char* text_content, const char* old_word, const char* new_word) {
	const char* pos = text_content;
	int count = 0;
	int old_word_len = strlen(old_word);
	int new_word_len = strlen(new_word);

	// Count occurrences of old_word in text_content 
	while ((pos = strstr(pos, old_word)) != NULL) {
		++count; pos += old_word_len;
	}

	// Allocate memory for the new string 
	char* result = new char[strlen(text_content) + count * (new_word_len - old_word_len) + 1];
	char* current_pos = result; // Replace old_word with new_word 

	while ((pos = strstr(text_content, old_word)) != NULL) {
		int len = pos - text_content;
		strncpy(current_pos, text_content, len);
		current_pos += len; strcpy(current_pos, new_word);
		current_pos += new_word_len; text_content = pos + old_word_len;
	}

	// Copy the remaining part of the text_content 
	strcpy(current_pos, text_content);
	return result;
}

void LookUpSymbol_Patched(uint32_t checksum)
{
	//patchCall((void*)0x00474F25, LookUpSymbol_Patched); /* accesses the global hash map */
	/*
	//unused for now
	if (mSettings.airdrift && checksum == 0x1CA80417 && !walkspinpatched) {
		patchDWord((void*)(uint32_t)CSymbolTableEntryResolve_Native(checksum), 0);
		walkspinpatched = true;
	}
	else if (!mSettings.boardscuffs && checksum == 0x9CE4DA4F && !boardscuffpatched) {
		patchDWord((void*)(uint32_t)CSymbolTableEntryResolve_Native(checksum), 0);
		boardscuffpatched = true;
	}
	if (walkspinpatched && boardscuffpatched)
		patchBytesM((void*)0x00474F25, (BYTE*)"\xE8\xC6\x3D\x00\x00", 5); //

	CSymbolTableEntryResolve_Native(checksum);
	*/
}