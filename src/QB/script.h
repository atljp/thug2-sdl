#include <QB/LazyStruct.h>
#include <QB/Qb.h>
#include <stdio.h>
#include <patch.h>
#include <Logger/Logger.h>

void patchScripts();
void initScriptPatch();
void LookUpSymbol_Patched(uint32_t checksum);
void ParseQB_Patched(const char* p_fileName, uint8_t* p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);
uint32_t __fastcall removeScript(uint32_t partChecksum);
void __fastcall sCreateScriptSymbolWrapper(uint32_t size, const uint8_t* p_data, uint32_t nameChecksum, uint32_t contentsChecksum, const char* p_fileName);
void loadScripts();
void setDropDownKeys();
