#pragma once
#include "config.h"
#include <unordered_set>
#include <string>
#include <map>

void InitModloader();
bool getModIni();
bool getModDefaultPreFile();
std::map<std::string, std::string> extractSection(const std::string& section, bool isPreFile);
bool getAllPreFiles();
bool getAllQbFiles();
uint8_t* getQbData(const std::string& fileName);
bool checkFolderExists(char* folder);
bool checkFileExists(char* file);
char* getWindowTitle();
bool isKeyInMap(const std::map<std::string, std::string>& keyValues, const std::string& key);
void PIPLoadPre_Wrapper(uint8_t* p_data);
void __fastcall PreMgrLoadPre_Wrapper(void* arg1, void* unused, uint8_t* p_data, char* arg3, char* arg4, char arg5);
void __cdecl parseQB_Patched(char* p_fileName, uint8_t* p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);