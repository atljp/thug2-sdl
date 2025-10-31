#include "modloader.h"

struct extmodsettings mExtModsettings;
struct stat info;
char modname[MAX_PATH];
char modfolder[MAX_PATH];
char modfolder_fullpath[MAX_PATH];
char modini_fullpath[MAX_PATH];
char to_be_injected[MAX_PATH];

std::map<std::string, std::string> preFilesMap;
std::map<std::string, std::string> qbFilesMap;


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- Init -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

void patchPreWrappers() {
	patchCall((void*)0x005A59FB, PIPLoadPre_Wrapper);
	patchCall((void*)0x005B7ADE, PIPLoadPre_Wrapper);
	patchCall((void*)0x005B94F7, PreMgrLoadPre_Wrapper);
}

void patchQbWrappers() {
	patchCall((void*)0x0046EEA3, parseQB_Patched);
}

void InitModloader() {

	// Get info to determine if the mod loader is active
	// Get handles to thug2sdl.ini, game dir and window title
	loadModSettings(&mExtModsettings);

	// Only load mods if it's activated in the ini
	if (mExtModsettings.usemod) {

		if (!(mExtModsettings.noadditionalscriptmods))
			Log::TypedLog(CHN_MOD, "WARNING! SCRIPT MODS FROM THUG2-SDL ARE ENABLED. COMPATIBILITY IS NOT GUARANTEED!\n");

		// Check if modfolder and mod.ini are valid
		// This will return a handle to the specified mod.ini and the mod folder.
		// The mod name will also be passed to the window title bar
		if (getModIni()) {

			// Get all defined pre files and store them in a map of form qb_scripts.prx=mycustomfile.prx
			if (getAllPreFiles()) {
				Log::TypedLog(CHN_MOD, "Patching PIP::LoadPre and PreMgr::LoadPre\n");
				patchPreWrappers();
			}
			else {
				Log::TypedLog(CHN_MOD, "Failed to load Pre files\n");
			}
			if (getAllQbFiles()) {
				Log::TypedLog(CHN_MOD, "Patching PIP::Load\n");
				patchQbWrappers();
			}
			else {
				Log::TypedLog(CHN_MOD, "Failed to load Qb files\n");
			}

		}
	}
	else if (!(mExtModsettings.noadditionalscriptmods)) {

		// Load custom qb_scripts.prx by default. 
		// This contains most of the data that was previously loaded in script.cpp as well as the observe menu, in game menu, physics etc.
		if (getModDefaultPreFile()) {
			Log::TypedLog(CHN_DLL, "Patching PIP::LoadPre and PreMgr::LoadPre for default mod scripts\n");
			patchPreWrappers();
		}
		else {
			Log::TypedLog(CHN_MOD, "Failed to load default mod Pre file\n");
		}
	}
}


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=- Function definitions -=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

typedef void __cdecl PIPLoadPre_NativeCall(uint8_t* p_data);
PIPLoadPre_NativeCall* PIPLoadPre_Native = (PIPLoadPre_NativeCall*)(0x005B75A0);

typedef void(__thiscall* PreMgrLoadPre_NativeCall)(void* arg1, uint8_t* arg2, char* arg3, char* arg4, char arg5);
PreMgrLoadPre_NativeCall PreMgrLoadPre = (PreMgrLoadPre_NativeCall)(0x005B8EE0);

typedef uint8_t* pipLoad_NativeCall(const char* p_fileName);
pipLoad_NativeCall* pipLoad_Native = (pipLoad_NativeCall*)(0x005B7C00);

typedef void ParseQB_NativeCall(const char* p_fileName, uint8_t* p_qb, int ecx, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);
ParseQB_NativeCall* ParseQB_Native = (ParseQB_NativeCall*)(0x00472420);


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=- Mod loader functions -=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

bool getModIni() {

	// Check if modfolder was specified in partymod.ini. Folders have to be relative to the game directory (data\pre\mymod)
	GetPrivateProfileString(MOD_SECTION, "Folder", "", modfolder, sizeof(modfolder), mExtModsettings.configfile);

	if (strlen(modfolder)) {

		// Maybe replace forward slashes with backslashes
		for (int i = 0; i < strlen(modfolder); ++i) {
			if (modfolder[i] == '/') {
				modfolder[i] = '\\';
			}
		}

		// Get full path to modfolder
		sprintf_s(modfolder_fullpath, "%s%s", mExtModsettings.workingdir, modfolder);
		Log::TypedLog(CHN_MOD, "Trying to load files from specified mod folder: %s\n", modfolder);

		if (checkFolderExists(modfolder_fullpath)) {
			Log::TypedLog(CHN_MOD, "Found mod folder: %s\n", modfolder_fullpath);
		}
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t exist!\n", modfolder_fullpath); return false;
		}

		// Get full path of mod.ini file
		sprintf(modini_fullpath, "%s%s", modfolder_fullpath, "\\mod.ini");

		// Check if mod.ini file exists on hard drive
		if (checkFileExists(modini_fullpath)) {
			Log::TypedLog(CHN_MOD, "Found mod.ini\n");
		}
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t contain a mod.ini\n");
			return false;
		}

		// If ini was found: Get mod name
		GetPrivateProfileString("MODINFO", "Name", "UNDEFINED", modname, sizeof(modname), modini_fullpath);
		Log::TypedLog(CHN_MOD, "Attempting to load mod: %s\n", modname);
		return true;
	}
	else {
		Log::TypedLog(CHN_MOD, "ERROR: No mod folder specified!\n"); return false;
	}
}

bool getModDefaultPreFile() {
	char preFile_fullpath[MAX_PATH];
	sprintf_s(preFile_fullpath, "%s%s", mExtModsettings.workingdir, "thug2sdl.prx");

	if (checkFileExists(preFile_fullpath)) {
		preFilesMap["qb_scripts.prx"] = "..\\..\\thug2sdl.prx";
		Log::TypedLog(CHN_MOD, "Found file %s\n", preFile_fullpath);
		return true;
	}
	// Fallback
	sprintf_s(preFile_fullpath, "%s%s", mExtModsettings.workingdir, "data\\pre\\thug2sdl.prx");
	if (checkFileExists(preFile_fullpath)) {
		preFilesMap["qp_scripts.prx"] = "thugsdl.prx";
		Log::TypedLog(CHN_MOD, "Found file %s\n", preFile_fullpath);
		return true;
	}
	printf("Could not find thug2sdl.prx. Falling back to data\\pre\\qb_scripts.prx\n");
	return false;
}

std::map<std::string, std::string> extractSection(const std::string& section, bool isPreFile) {

	std::ifstream iniFile(modini_fullpath);
	std::string line;
	bool inSection = false;
	char preFile_fullpath[MAX_PATH];
	std::map<std::string, std::string> keyValues;

	if (!iniFile.is_open()) {
		Log::TypedLog(CHN_MOD, "Error: Could not open the .ini file!\n");
		return keyValues;
	}
	else {
		Log::TypedLog(CHN_MOD, "Successfully loaded %s\n", modini_fullpath);
	}

	// Go through mod.ini and extract all "key, value" pairs for a given section. Trim whitespaces and append file endings if needed
	while (std::getline(iniFile, line)) {

		// Ignore lines that are comments or empty
		if (line.empty() || line[0] == ';' || line[0] == '#') {
			continue;
		}

		// Trim whitespace from the beginning and end of the line
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		// Check if the line is a section header
		if (line.front() == '[' && line.back() == ']') {
			std::string currentSection = line.substr(1, line.size() - 2);
			inSection = (currentSection == section);
		}
		else if (inSection) {
			// Check if the line is a key-value pair
			std::size_t pos = line.find('=');

			if (pos != std::string::npos) {
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);

				// Trim whitespace from the key and value
				key.erase(0, key.find_first_not_of(" \t"));
				key.erase(key.find_last_not_of(" \t") + 1);
				value.erase(0, value.find_first_not_of(" \t"));
				value.erase(value.find_last_not_of(" \t") + 1);

				// Check for file endings
				if (isPreFile) {
					if (key.find(".prx") == std::string::npos) { key += ".prx"; }
					if (value.find(".prx") == std::string::npos) { value += ".prx"; }
				}
				else {
					if (key.find(".qb") == std::string::npos) { key += ".qb"; }
					if (value.find(".qb") == std::string::npos) { value += ".qb"; }
				}

				// Fill the map which will be iterated in the hooked functions
				// Don't add double entries
				if (!isKeyInMap(keyValues, key)) {

					// Generate injection string, this will be passed to LoadPre
					sprintf_s(to_be_injected, "%s%s%s", strrchr(modfolder, '\\') + 1, "\\", value.c_str());

					// Check if file exists on hard disk
					sprintf_s(preFile_fullpath, "%s%s%s", mExtModsettings.workingdir, "data\\pre\\", to_be_injected);

					if (checkFileExists(preFile_fullpath)) {
						keyValues[key] = to_be_injected;
					}
					else {
						Log::TypedLog(CHN_MOD, "File not found: %s\n", to_be_injected);
					}
				}
			}
		}
	}
	return keyValues;
}

bool getAllPreFiles() {
	preFilesMap = extractSection("PRE", true);

	if (preFilesMap.empty())
		return false;

	for (const auto& kv : preFilesMap) {
		Log::TypedLog(CHN_MOD, "REGISTERING PRE FILE: %s\n", kv.second.c_str());
	}
	return true;
}

bool getAllQbFiles() {

	qbFilesMap = extractSection("QB", false);

	if (qbFilesMap.empty())
		return false;

	for (const auto& kv : qbFilesMap) {
		Log::TypedLog(CHN_MOD, "REGISTERING QB FILE: %s=%s\n", kv.first.c_str(), kv.second.c_str());
	}
	return true;
}

uint8_t* getQbData(const std::string& fileName) {

	char qbfile_fullpath[MAX_PATH];

	sprintf_s(qbfile_fullpath, "%s%s%s", mExtModsettings.workingdir, "data\\pre\\", fileName.c_str());
	std::ifstream infile(qbfile_fullpath, std::ios::binary | std::ios::ate);

	if (!infile) {
		Log::TypedLog(CHN_MOD, "Could not open file: %s\n", qbfile_fullpath);
		return nullptr;
	}

	size_t fileSize = infile.tellg();
	infile.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(fileSize);
	if (!infile.read(reinterpret_cast<char*>(buffer.data()), buffer.size())) {
		Log::TypedLog(CHN_MOD, "Error reading file: %s\n", qbfile_fullpath);
		return nullptr;
	}

	// Allocate memory for the data pointer and copy the contents
	uint8_t* dataPtr = new uint8_t[fileSize];
	std::memcpy(dataPtr, buffer.data(), fileSize);

	return (uint8_t*)dataPtr;
}

bool checkFolderExists(char* folder) {

	struct stat info;
	stat(folder, &info);

	if (info.st_mode & S_IFDIR)
		return true;
	else
		return false;
}

bool checkFileExists(char* file) {

	std::ifstream infile(file);

	if (infile.good())
		return true;
	else
		return false;
}

char* getWindowTitle() {
	return modname;
}

bool isKeyInMap(const std::map<std::string, std::string>& keyValues, const std::string& key) {
	for (const auto& kv : keyValues) {
		if (kv.first == key) {
			return true;
		}
	}
	return false;
}


/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=- Hooked functions =--=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/* -=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=- */

void PIPLoadPre_Wrapper(uint8_t* p_data)
{
	for (const auto& pair : preFilesMap)
	{
		//printf("COMPARING: %s with %s\n", (const char*)p_data + 1, pair.first.c_str() + 1);
		if (strncmp((const char*)p_data + 1, pair.first.c_str() + 1, strlen((const char*)p_data) - 5) == 0) // Compare the original data pointer with our files (without the ending and without the first letter to account for capital first letters)
		{
			Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, pair.second.c_str());
			p_data = (uint8_t*)pair.second.c_str();
		}
	}
	PIPLoadPre_Native(p_data);
}

void __fastcall PreMgrLoadPre_Wrapper(void* arg1, void* unused, uint8_t* p_data, char* arg3, char* arg4, char arg5) {
	
	for (const auto& pair : preFilesMap)
	{
		if (strncmp((const char*)p_data + 1, pair.first.c_str() + 1, strlen((const char*)p_data) - 5) == 0) // Compare the original data pointer with our files (without the ending and without the first letter to account for capital first letters)
		{
			Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, pair.second.c_str());
			p_data = (uint8_t*)pair.second.c_str();
		}
	}
	PreMgrLoadPre(arg1, p_data, arg3, arg4, arg5);
}

void __cdecl parseQB_Patched(char* p_fileName, uint8_t* p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable) {

	bool found_file = 0;

	for (const auto& kv : qbFilesMap) {

		if (!strcmp(p_fileName, kv.first.c_str())) {
			Log::TypedLog(CHN_MOD, "Loading %s -> %s\n", kv.first.c_str(), kv.second.c_str());
			ParseQB_Native(p_fileName, getQbData(kv.second), 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);
			found_file = 1;
		}
	}

	if (!found_file)
		ParseQB_Native(p_fileName, p_qb, 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);

}


