#include "modloader.h"
#include <map>


struct modsettings mModsettings;
struct stat info;
char modname[MAX_PATH];
char modfolder[MAX_PATH];
char full_modfolder[MAX_PATH];
char ini_file[MAX_PATH];
bool filemissing = FALSE;
char to_be_injected[60];

typedef void __cdecl PIPLoadPre_NativeCall(uint8_t* p_data);
PIPLoadPre_NativeCall* PIPLoadPre_Native = (PIPLoadPre_NativeCall*)(0x005B75A0);

typedef void(__thiscall* PreMgrLoadPre_NativeCall)(void* arg1, uint8_t* arg2, char* arg3, char* arg4, char arg5);
PreMgrLoadPre_NativeCall PreMgrLoadPre = (PreMgrLoadPre_NativeCall)(0x005B8EE0);

typedef void ParseQB_NativeCall(const char* p_fileName, uint8_t* p_qb, int ecx, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable);
ParseQB_NativeCall* ParseQB_Native = (ParseQB_NativeCall*)(0x00472420);

//##############################
std::unordered_set<std::string> all_pre_files() {
	return {
		"anims",
		"apcol",
		"apped",
		"apscn",
		"ap_scripts",
		"aucol",
		"auped",
		"auscn",
		"autempprofile",
		"au_scripts",
		"bacol",
		"baped",
		"bascn",
		"ba_scripts",
		"becol",
		"beped",
		"bescn",
		"be_scripts",
		"bits",
		"bocol",
		"boped",
		"boscn",
		"bo_scripts",
		"cacol",
		"cagpieces",
		"cagpremade",
		"caped",
		"cascn",
		"casfiles",
		"ca_scripts",
		"djcol",
		"djped",
		"djscn",
		"dj_scripts",
		"fonts",
		"lacol",
		"laped",
		"lascn",
		"la_scripts",
		"levelselect",
		"mainmenucol",
		"mainmenuped",
		"mainmenuscn",
		"mainmenusprites",
		"mainmenu_scripts",
		"multiplayersprites",
		"netanims",
		"nocol",
		"noped",
		"noscn",
		"no_scripts",
		"panelsprites",
		"parkedanims",
		"parked_sounds",
		"permtex",
		"phcol",
		"phped",
		"phscn",
		"ph_scripts",
		"qb_scripts",
		"sccol",
		"scped",
		"scscn",
		"sc_scripts",
		"se2col",
		"se2ped",
		"se2scn",
		"se2_scripts",
		"secol",
		"seped",
		"sescn",
		"se_scripts",
		"sk5ed2_shellcol",
		"sk5ed2_shellped",
		"sk5ed2_shellscn",
		"sk5ed2_shell_scripts",
		"sk5ed3_shellcol",
		"sk5ed3_shellped",
		"sk5ed3_shellscn",
		"sk5ed3_shell_scripts",
		"sk5ed4_shellcol",
		"sk5ed4_shellped",
		"sk5ed4_shellscn",
		"sk5ed4_shell_scripts",
		"sk5ed5_shellcol",
		"sk5ed5_shellped",
		"sk5ed5_shellscn",
		"sk5ed5_shell_scripts",
		"sk5edcol",
		"sk5edped",
		"sk5edscn",
		"sk5ed_scripts",
		"sk5ed_shellcol",
		"sk5ed_shellped",
		"sk5ed_shellscn",
		"sk5ed_shell_scripts",
		"skaterparts",
		"skaterparts_secret",
		"skaterparts_temp",
		"skater_sounds",
		"skeletons",
		"stcol",
		"stped",
		"stscn",
		"st_scripts",
		"testlevelcol",
		"testlevelped",
		"testlevelscn",
		"testlevel_scripts",
		"themes",
		"trcol",
		"trped",
		"trscn",
		"tr_scripts",
		"unloadableanims",
	};
}

std::vector<std::string> mmoddedFileNames_original;
std::vector<std::string> mmoddedFileNames_modded;
std::map<std::string, std::string> fileNameMapping;

void generateModdedFileVector(std::vector<std::string>& mmoddedFileNames_original)
{
	std::unordered_set<std::string> validFileNames = all_pre_files();
	std::ifstream iniFile(ini_file);

	if (!iniFile.is_open()) {
		Log::TypedLog(CHN_MOD, "Error: Could not open the .ini file!\n"); return;
	}
	else { Log::TypedLog(CHN_MOD, "Successfully loaded mod.ini\n"); }

	std::string line;
	while (std::getline(iniFile, line)) {

		// Ignore lines that are comments or empty
		if (line.empty() || line[0] == ';' || line[0] == '#') {
			continue;
		}

		// Find the position of the '=' which separates key and value
		size_t pos = line.find('=');
		if (pos == std::string::npos) {
			continue; // Malformed line, skip it
		}

		// Extract the key
		std::string key = line.substr(0, pos);
		key.erase(0, key.find_first_not_of(" \t")); // Trim leading spaces
		key.erase(key.find_last_not_of(" \t") + 1); // Trim trailing spaces

		// Maybe remove file ending
		size_t dotPosition = key.find('.');
		if (dotPosition != std::string::npos) {
			// Remove everything from the dot to the end of the string
			key.erase(dotPosition);
		}

		// Check if the key is in the valid file names set
		if (validFileNames.find(key) != validFileNames.end()) {
			Log::TypedLog(CHN_MOD, "Valid key found: %s\n", key.c_str());
			mmoddedFileNames_original.push_back(key);
		}
		else {
			if (key != "Name") {
				Log::TypedLog(CHN_MOD, "Invalid key found: %s\n", key.c_str());
				filemissing = TRUE;
			}
		}
	}
	iniFile.close();
}

bool getModIni()
{
	// Check if modfolder was specified in partymod.ini. Folders have to be relative to the game directory (data\pre\mymod)
	GetPrivateProfileString(MOD_SECTION, "Folder", "", modfolder, sizeof(modfolder), mModsettings.configfile);
	if (strlen(modfolder))
	{
		// Maybe replace forward slashes with backslashes
		for (int i = 0; i < strlen(modfolder); ++i) {
			if (modfolder[i] == '/') {
				modfolder[i] = '\\';
			}
		}

		// Get full path to modfolder
		sprintf_s(full_modfolder, "%s%s", mModsettings.workingdir, modfolder);
		Log::TypedLog(CHN_MOD, "Trying to load files from specified mod folder: %s\n", modfolder);

		// Check if specified mod folder exists on hard drive
		stat(full_modfolder, &info);
		if (info.st_mode & S_IFDIR)
			Log::TypedLog(CHN_MOD, "Found mod folder: %s\n", full_modfolder);
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t exist!\n", full_modfolder); return FALSE;
		}

		// Get full path of mod.ini file
		sprintf(ini_file, "%s%s", full_modfolder, "\\mod.ini");

		// Check if mod.ini file exists on hard drive
		std::ifstream infile_ini(ini_file);
		if (infile_ini.good())
			Log::TypedLog(CHN_MOD, "Found mod.ini!\n");
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t contain a mod.ini\n"); return FALSE;
		}

		GetPrivateProfileString("MODINFO", "Name", "UNDEFINED", modname, sizeof(modname), ini_file);
		Log::TypedLog(CHN_MOD, "Attempting to load mod: %s\n", modname);
		return TRUE;
	}
	else {
		Log::TypedLog(CHN_MOD, "ERROR: No mod folder specified!\n"); return FALSE;
	}
}

void getModdedFileName(char* filename)
{
	char a[30];
	char b[MAX_PATH];
	char filename_prx[30];

	sprintf_s(filename_prx, "%s%s", filename, ".prx");

	// get info from mod.ini: get the new qb_scripts.prx filename
	GetPrivateProfileString("MODINFO", filename_prx, "UNDEFINED", a, sizeof(a), ini_file); 


	if (strcmp((const char*)a, "UNDEFINED") && strlen(a)) {
		//printf("FILENAME: %s -- AAA: %s\n", filename_prx, a);
		sprintf_s(b, "%s%s%s", full_modfolder, "\\", a); // check if file exists on hard drive
		//printf("CHECK: %s\n", b);
		std::ifstream infile(b);
		if (infile.good()) {
			Log::TypedLog(CHN_MOD, "Found modded file: %s\n", a);
			sprintf_s(to_be_injected, "%s%s%s", strrchr(modfolder, '\\') + 1, "/", a); // generate injection string, this will be passed to LoadPre
			//printf("TO BE INJECTED: %s\n", to_be_injected);
			mmoddedFileNames_modded.push_back(to_be_injected);
		}
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded file on hard drive: %s\n", a);
			filemissing = TRUE;
		}

	}
	else {
		// If files can't be found or were specified incorrectly, no PRE loading functions will be patched
		Log::TypedLog(CHN_MOD, "ERROR: Invalid file specified for: %s\n", filename_prx);
		filemissing = TRUE;
	}
}

void PIPLoadPre_Wrapper(uint8_t* p_data)
{
	for (const auto& pair : fileNameMapping)
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
	
	//printf("ALL: %s\n", (const char*)p_data);
	for (const auto& pair : fileNameMapping)
	{
		if (strncmp((const char*)p_data + 1, pair.first.c_str() + 1, strlen((const char*)p_data) - 5) == 0) // Compare the original data pointer with our files (without the ending and without the first letter to account for capital first letters)
		{
			Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, pair.second.c_str());
			p_data = (uint8_t*)pair.second.c_str();
		}
	}
	PreMgrLoadPre(arg1, p_data, arg3, arg4, arg5);
}

void getWindowTitle(struct modsettings* modsettingsOut)
{
	modsettingsOut->windowtitle = modname;
}

void initMod()
{
	// Get info to determine if the mod loader is active. Also get handles to partymod.ini, the game directory and the window title
	// The info is stored in the mModsettings struct
	loadModSettings(&mModsettings);

	// Only load mods if it's activated in the ini
	if (mModsettings.usemod) {

		// Check if modfolder and mod.ini are valid
		// When successful, we have a handle to the specified mod.ini and the mod folder. The mod name will also be passed to the window title bar
		if (getModIni()) {
			
			// Check the contents of mod.ini and verify valid file names (checked against all_pre_files vector)
			generateModdedFileVector(mmoddedFileNames_original);

			// Go through mod.ini and get all modded file names and put it into mmoddedFileNames_modded
			for (const auto& entry : mmoddedFileNames_original)
				getModdedFileName((char*)(entry.c_str()));
			
			if (!filemissing)
			{
				// Ensure the vectors have the same length
				if (mmoddedFileNames_original.size() != mmoddedFileNames_modded.size()) {
					Log::TypedLog(CHN_MOD, "Vectors are not of the same length!\n"); return;
				}

				// Create the mapping using std::map
				for (size_t i = 0; i < mmoddedFileNames_original.size(); ++i) {
					fileNameMapping[mmoddedFileNames_original[i]] = mmoddedFileNames_modded[i];
				}

				// Patch PRE loading functions when everything was initialized correctly
				patchCall((void*)0x005A59FB, PIPLoadPre_Wrapper);
				patchCall((void*)0x005B7ADE, PIPLoadPre_Wrapper);
				patchCall((void*)0x005B94F7, PreMgrLoadPre_Wrapper);
				Log::TypedLog(CHN_MOD, "Patching PIP::LoadPre and PreMgr::LoadPre\n");
			}
		}
	}

	/*
	for (auto& pair : fileNameMapping) {
		printf("std::map contents: %s -> %s\n", pair.first.c_str(), pair.second.c_str());
	}
	printf("LENN: %d\n", fileNameMapping.size());

	for (const auto& entry : mmoddedFileNames_original) {
		std::cout << "ORIGINAL: " << entry << std::endl;
	}
	for (const auto& entry : mmoddedFileNames_modded) {
		std::cout << "HERE: " << entry << std::endl;
	}
	for (const auto& entry : mmoddedFileNames_original) {
		std::cout << "ORIGINAL: " << entry << std::endl;
	}
	for (const auto& entry : mmoddedFileNames_modded) {
		std::cout << "MODDED: " << entry << std::endl;
	}
	*/
}

/*
* TODO: Add the option to load loose qb files
* patchCall((void*)0x0046EEA3, ParseQB_Patched); // loads script files
void ParseQB_Patched(const char* p_fileName, uint8_t* p_qb, int unused, int assertIfDuplicateSymbols, bool allocateChecksumNameLookupTable)
{
	if (!strcmp(p_fileName, "scripts\\game\\skater\\physics.qb")) {
		//ParseQB_Native(p_fileName, (uint8_t*)"pre\\imaginemod\\gamemenu_levelselect.qb", 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);

		const char* filePath = "D:\\Tony Hawk's 6 - Underground 2\\Game\\Data\\pre\\imaginemod\\physics.qb";

		// Open the file in binary mode
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);
		uint8_t* buffer = new uint8_t[size];
		if (!file.read(reinterpret_cast<char*>(buffer), size)) {
			std::cerr << "Failed to read the file." << std::endl;
			delete[] buffer;
		}
		file.close();
		uint8_t* ptr = buffer;
		printf("replacing physics.qb");
		ParseQB_Native("scripts\\game\\skater\\physics.qb", ptr, 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);
		delete[] buffer;
	}
	else {
		ParseQB_Native(p_fileName, p_qb, 1, assertIfDuplicateSymbols, allocateChecksumNameLookupTable);
	}
}
*/

