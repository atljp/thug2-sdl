#pragma once
#include "config.h"
#include <unordered_set>
#include <string>


bool getModIni();
void initMod_temp();
void initMod();
void getWindowTitle(struct modsettings* modsettingsOut);
void PIPLoadPre_Wrapper(uint8_t* p_data);
void __fastcall PreMgrLoadPre_Wrapper(void* arg1, void* unused, uint8_t* p_data, char* arg3, char* arg4, char arg5);