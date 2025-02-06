#pragma once
#include "QB/LazyStruct.h"
#include "QB/Qb.h"
#include "stdio.h"
#include "patch.h"
#include "Logger/Logger.h"
#include "QB/CFuncs.h"
#include "config.h"
#include "QB/scriptcontent.h"
#include "QB/LazyArray.h"
#include "QB/malloc.h"

enum ESymbolType {
	ESYMBOLTYPE_NONE = 0,
	ESYMBOLTYPE_INTEGER,
	ESYMBOLTYPE_FLOAT,
	ESYMBOLTYPE_STRING,
	ESYMBOLTYPE_LOCALSTRING,
	ESYMBOLTYPE_PAIR,
	ESYMBOLTYPE_VECTOR,
	ESYMBOLTYPE_QSCRIPT,
	ESYMBOLTYPE_CFUNCTION,
	ESYMBOLTYPE_MEMBERFUNCTION,
	ESYMBOLTYPE_STRUCTURE,
	ESYMBOLTYPE_STRUCTUREPOINTER,
	ESYMBOLTYPE_ARRAY,
	ESYMBOLTYPE_NAME,
	ESYMBOLTYPE_INTEGER_ONE_BYTE,
	ESYMBOLTYPE_INTEGER_TWO_BYTES,
	ESYMBOLTYPE_UNSIGNED_INTEGER_ONE_BYTE,
	ESYMBOLTYPE_UNSIGNED_INTEGER_TWO_BYTES,
	ESYMBOLTYPE_ZERO_INTEGER,
	ESYMBOLTYPE_ZERO_FLOAT,
};

struct DummyScript;

void patchScripts();
void initScriptPatch();
void LookUpSymbol_Patched(uint32_t checksum);
void ParseQB_Patched(const char* p_fileName, uint8_t* p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);
uint32_t __fastcall removeScript(uint32_t partChecksum);
void __fastcall sCreateScriptSymbolWrapper(uint32_t size, const uint8_t* p_data, uint32_t nameChecksum, uint32_t contentsChecksum, const char* p_fileName);
void initMod();
void editScriptsInMemory();
void setDropDownKeys();
void patchCFuncs();
bool IsPS2_Patched(void* pParams, DummyScript* pScript);
bool IsXBOX_Patched(void* pParams, DummyScript* pScript);
bool GetMemCardSpaceAvailable_Patched(Script::LazyStruct* pParams, /*ebp + 0x8*/
	DummyScript* pScript, /*ebp+0xC*/
	uint32_t a, /*ebp+0x10*/
	ULARGE_INTEGER b, /*ebp+0x14*/
	ULARGE_INTEGER c, /*ebp+0x1C*/
	ULARGE_INTEGER d, /*ebp+0x24*/
	ULARGE_INTEGER e, /*ebp+0x2C*/
	ULARGE_INTEGER f, /*epc+0x34*/
	uint8_t p_card);
bool CreateScreenElement_Patched(Script::LazyStruct* pParams, DummyScript* pScript);
bool SetScreenElementProps_Patched(Script::LazyStruct* pParams, DummyScript* pScript);
bool SetButtonEventMappings_Patched(Script::LazyStruct* pParams, DummyScript* pScript);
void setCavemanKeys();
void setLadderGrabKeys();
char* setText(const char* text_content, const char* old_word, const char* new_word);
