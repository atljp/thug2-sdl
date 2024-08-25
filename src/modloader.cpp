#include "modloader.h"
#include <map>


struct modsettings mModsettings;
struct stat info;
char modname[30];
char modfolder[MAX_PATH];
char full_modfolder[MAX_PATH];
char ini_file[MAX_PATH];
char qbscripts_from_mod[30];
char anims_from_mod[30];
char netanims_from_mod[30];
char full_qbscripts_from_mod[MAX_PATH];
char full_anims_from_mod[MAX_PATH];
char full_netanims_from_mod[MAX_PATH];
char qb_scripts_for_injection[60];
char anims_for_injection[60];
char netanims_for_injection[60];
bool filemissing = FALSE;
bool using_qbscripts = TRUE;
bool using_anims = TRUE;
bool using_netanims = TRUE;
bool using_mainmenu_scripts = TRUE;
char mainmenu_scripts_from_mod[30];
char full_mainmenu_scripts_from_mod[MAX_PATH];
char mainmenu_scripts_for_injection[60];

char to_be_injected[60];

typedef void __cdecl PIPLoadPre_NativeCall(uint8_t* p_data);
PIPLoadPre_NativeCall* PIPLoadPre_Native = (PIPLoadPre_NativeCall*)(0x005B75A0);

typedef void(__thiscall* PreMgrLoadPre_NativeCall)(void* arg1, uint8_t* arg2, char* arg3, char* arg4, char arg5);
PreMgrLoadPre_NativeCall PreMgrLoadPre = (PreMgrLoadPre_NativeCall)(0x005B8EE0);

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
	else { Log::TypedLog(CHN_MOD, "FOUND MOD.INI\n"); }

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
			Log::TypedLog(CHN_MOD, "Invalid key found: %s\n", key.c_str());
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

	GetPrivateProfileString("MODINFO", filename_prx, "UNDEFINED", a, sizeof(a), ini_file); // get info from mod.ini: get the new qb_scripts.prx filename


	if (strcmp((const char*)a, "UNDEFINED") && strlen(a)) {
		printf("FILENAME: %s -- AAA: %s\n", filename_prx, a);
		sprintf_s(b, "%s%s%s", full_modfolder, "\\", a); // check if file exists on hard drive
		printf("CHECK: %s\n", b);
		std::ifstream infile(b);
		if (infile.good()) {
			Log::TypedLog(CHN_MOD, "Found modded file: %s\n", a);
			sprintf_s(to_be_injected, "%s%s%s", strrchr(modfolder, '\\') + 1, "/", a); // generate injection string, this will be passed to LoadPre
			//printf("TO BE INJECTED: %s\n", to_be_injected);
			mmoddedFileNames_modded.push_back(to_be_injected);
		}
		else {
			Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded qb_scripts!\n");
			filemissing = TRUE;
		}

	}
	else { // if the file was not defined, it won't be loaded in the LoadPre wrapper
		using_qbscripts = FALSE;
	}
}


void PIPLoadPre_Wrapper(uint8_t* p_data)
{
	//printf("uebergabeparameter: 0x%08x\n", p_data);
	printf("text: %s\n", (const char*)p_data);

	//int32_t edi = *reinterpret_cast<int32_t*>(reinterpret_cast<uint32_t>(_AddressOfReturnAddress()) + 4);
	//printf("esp+4 address: 0x%08x\n", (uint32_t*)edi);
	//printf("AAA:%s\n", (const char*)edi);

	//ignore first letter in case it is capitalized
	printf("ZZINSIDE PIP LOAD PRE WITH %s\n", mmoddedFileNames_original[0].c_str());

	// Create mapping, this could be done once prior to all wrapper-calls. Maybe working with a map directly? TODO!

	// Ensure the vectors have the same length
	if (mmoddedFileNames_original.size() != mmoddedFileNames_modded.size()) {
		std::cerr << "Vectors are not of the same length!" << std::endl;
	}

	// Create the mapping using std::map
	std::map<std::string, std::string> fileNameMapping;
	for (size_t i = 0; i < mmoddedFileNames_original.size(); ++i) {
		fileNameMapping[mmoddedFileNames_original[i]] = mmoddedFileNames_modded[i];
	}

	//for (const auto& pair : fileNameMapping) {
	//	printf("%s -> %s\n", pair.first.c_str(), pair.second.c_str());
	//}

	for (const auto& pair : fileNameMapping)
	{ 
		printf("COMPARING: %s with %s\m", (const char*)p_data + 1, pair.first.c_str() + 1);
		if (strncmp((const char*)p_data+1, pair.first.c_str() + 1, strlen((const char*)p_data) - 5) == 0) // Compare the original data pointer with our files (without the ending and without the first letter to account for capital first letters)
		{
			printf("OHNO\n");
			p_data = (uint8_t*)pair.second.c_str(); // Mapping orignal to modded file here TODO!1
		}
	}

	


	//if ((!strcmp((const char*)p_data + 1, "b_scripts.prx") || !strcmp((const char*)p_data + 1, "b_scripts.pre")) && using_qbscripts) {
	//	Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, qb_scripts_for_injection);
	//	p_data = (uint8_t*)qb_scripts_for_injection;
	//}
	
	PIPLoadPre_Native(p_data);
}

void initMod_temp()
{
	loadModSettings(&mModsettings);

	// Only load mods if it's activated in the ini
	if (mModsettings.usemod) {

		// Check if modfolder and mod.ini are valid
		if (getModIni()) {
			printf("GOOD\n");
			generateModdedFileVector(mmoddedFileNames_original);

			// Go through mod.ini and get all modded file names and put it into mmoddedFileNames_modded
			for (const auto& entry : mmoddedFileNames_original)
				getModdedFileName((char*)(entry.c_str()));
			


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
	*/
	patchCall((void*)0x005A59FB, PIPLoadPre_Wrapper);
	patchCall((void*)0x005B7ADE, PIPLoadPre_Wrapper);

}















void initMod()
{
	loadModSettings(&mModsettings);
	if (mModsettings.usemod)
	{
		/*check if modfolder was specified in partymod.ini. Folders are specified relative to the game directory (data\pre\mymod)*/
		
		GetPrivateProfileString(MOD_SECTION, "Folder", "", modfolder, sizeof(modfolder), mModsettings.configfile);
		if (strlen(modfolder))
		{
			/*maybe replace forward slashes with backslashes*/
			for (int i = 0; i < strlen(modfolder); ++i) {
				if (modfolder[i] == '/') {
					modfolder[i] = '\\';
				}
			}

			/*get full path to modfolder*/
			sprintf_s(full_modfolder, "%s%s", mModsettings.workingdir, modfolder);

			Log::TypedLog(CHN_MOD, "Trying to load files from specified mod folder: %s\n", modfolder);
			/*check if specified mod folder exists on hard drive*/
			stat(full_modfolder, &info);
			if (info.st_mode & S_IFDIR)
				Log::TypedLog(CHN_MOD, "Found mod folder: %s\n", full_modfolder);
			else {
				Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t exist!\n", full_modfolder); return;
			}

			/*get full path of mod.ini file*/
			sprintf(ini_file, "%s%s", full_modfolder, "\\mod.ini");

			/*check if mod.ini file exists on hard drive*/
			std::ifstream infile_ini(ini_file);
			if (infile_ini.good())
				Log::TypedLog(CHN_MOD, "Found mod.ini!\n");
			else {
				Log::TypedLog(CHN_MOD, "ERROR: Mod folder doesn\'t contain a mod.ini\n"); return;
			}

			GetPrivateProfileString("MODINFO", "Name", "UNDEFINED", modname, sizeof(modname), ini_file);
			Log::TypedLog(CHN_MOD, "Attempting to load mod: %s\n", modname);

			/*check if modded files exist as defined in mod.ini*/
			char* lastSlash = strrchr(modfolder, '\\'); /*lastSlash+1 has the last word of a path (e.g. mymod in data\mod\pre\mymod)*/

			GetPrivateProfileString("MODINFO", "qb_scripts.prx", "UNDEFINED", qbscripts_from_mod, sizeof(qbscripts_from_mod), ini_file); /*get info from mod.ini: get the new qb_scripts.prx filename*/
			if (strcmp((const char*)qbscripts_from_mod, "UNDEFINED")) { /*only check for the file if it was specified in mod.ini*/
				sprintf_s(full_qbscripts_from_mod, "%s%s%s", full_modfolder, "\\", qbscripts_from_mod); /*check if file exists on hard drive*/
				std::ifstream infile_qb_scripts(full_qbscripts_from_mod);
				if (infile_qb_scripts.good()) {
					Log::TypedLog(CHN_MOD, "Found modded qb_scripts! File: %s\n", qbscripts_from_mod);
					sprintf_s(qb_scripts_for_injection, "%s%s%s", lastSlash + 1, "/", qbscripts_from_mod); /*generate injection string, this will be passed to LoadPre*/
				}
				else {
					Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded qb_scripts!\n");
					filemissing = TRUE;
				}
			}
			else { /*if the file was not defined, it won't be loaded in the LoadPre wrapper*/
				using_qbscripts = FALSE;
			}

			GetPrivateProfileString("MODINFO", "anims.prx", "UNDEFINED", anims_from_mod, sizeof(anims_from_mod), ini_file);
			if (strcmp((const char*)anims_from_mod, "UNDEFINED")) {
				sprintf_s(full_anims_from_mod, "%s%s%s", full_modfolder, "\\", anims_from_mod);
				std::ifstream infile_anims(full_anims_from_mod);
				if (infile_anims.good()) {
					Log::TypedLog(CHN_MOD, "Found modded anims! File: %s\n", anims_from_mod);
					sprintf_s(anims_for_injection, "%s%s%s", lastSlash + 1, "/", anims_from_mod);
				}
				else {
					Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded anims!\n");
					filemissing = TRUE;
				}
			}
			else {
				using_anims = FALSE;
			}

			GetPrivateProfileString("MODINFO", "netanims.prx", "UNDEFINED", netanims_from_mod, sizeof(netanims_from_mod), ini_file);
			if (strcmp((const char*)netanims_from_mod, "UNDEFINED")) {
				sprintf_s(full_netanims_from_mod, "%s%s%s", full_modfolder, "\\", netanims_from_mod);
				std::ifstream infile_netanims(full_netanims_from_mod);
				if (infile_netanims.good()) {
					Log::TypedLog(CHN_MOD, "Found modded netanims! File: %s\n", netanims_from_mod);
					sprintf_s(netanims_for_injection, "%s%s%s", lastSlash + 1, "/", netanims_from_mod);
				}
				else {
					Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded netanims!\n");
					filemissing = TRUE;
				}
			}
			else {
				using_netanims = FALSE;
			}

			GetPrivateProfileString("MODINFO", "mainmenu_scripts.prx", "UNDEFINED", mainmenu_scripts_from_mod, sizeof(mainmenu_scripts_from_mod), ini_file);
			if (strcmp((const char*)mainmenu_scripts_from_mod, "UNDEFINED")) {
				sprintf_s(full_mainmenu_scripts_from_mod, "%s%s%s", full_modfolder, "\\", mainmenu_scripts_from_mod);
				std::ifstream infile_mainemenu_scripts(full_mainmenu_scripts_from_mod);
				if (infile_mainemenu_scripts.good()) {
					Log::TypedLog(CHN_MOD, "Found modded mainmenu_scripts! File: %s\n", mainmenu_scripts_from_mod);
					sprintf_s(mainmenu_scripts_for_injection, "%s%s%s", lastSlash + 1, "/", mainmenu_scripts_from_mod);
				}
				else {
					Log::TypedLog(CHN_MOD, "ERROR: Couldn\'t find modded mainmenu_scripts!\n");
					filemissing = TRUE;
				}
			}
			else {
				using_mainmenu_scripts = FALSE;
			}

			if (filemissing) {
				Log::TypedLog(CHN_MOD, "ERROR: Missing files defined in mod.ini\n"); return;
			}
		}
		else {
			Log::TypedLog(CHN_MOD, "ERROR: No mod folder specified!\n"); return;
		}
		patchCall((void*)0x005A59FB, PIPLoadPre_Wrapper);
		patchCall((void*)0x005B7ADE, PIPLoadPre_Wrapper);
		patchCall((void*)0x005B94F7, PreMgrLoadPre_Wrapper);
		
	}
}

void getWindowTitle(struct modsettings* modsettingsOut)
{
	modsettingsOut->windowtitle = modname;
}
/*
void PIPLoadPre_Wrapper(uint8_t* p_data)
{
	//printf("uebergabeparameter: 0x%08x\n", p_data);
	//printf("text: %s\n", (const char*)p_data);

	//int32_t edi = *reinterpret_cast<int32_t*>(reinterpret_cast<uint32_t>(_AddressOfReturnAddress()) + 4);
	//printf("esp+4 address: 0x%08x\n", (uint32_t*)edi);
	//printf("AAA:%s\n", (const char*)edi);

	//ignore first letter in case it is capitalized
	if ((!strcmp((const char*)p_data + 1, "b_scripts.prx") || !strcmp((const char*)p_data + 1, "b_scripts.pre")) && using_qbscripts) {
		Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, qb_scripts_for_injection);
		p_data = (uint8_t*)qb_scripts_for_injection;
	}
	else if ((!strcmp((const char*)p_data + 1, "nims.prx") || !strcmp((const char*)p_data + 1, "nims.pre")) && using_anims) {
		Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, anims_for_injection);
		p_data = (uint8_t*)anims_for_injection;
	}
	else if ((!strcmp((const char*)p_data + 1, "etanims.prx") || !strcmp((const char*)p_data + 1, "etanims.pre")) && using_netanims) {
		Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, netanims_for_injection);
		p_data = (uint8_t*)netanims_for_injection;
	}
	PIPLoadPre_Native(p_data);
}
*/
void __fastcall PreMgrLoadPre_Wrapper(void* arg1, void* unused, uint8_t* p_data, char* arg3, char* arg4, char arg5) {
		//printf("uebergabeparameter: 0x%08x\n", arg1);
		printf("ALL: %s\n", (const char*)p_data);

		//ignore first letter in case it is capitalized
		if ((!strcmp((const char*)p_data + 1, "ainmenu_scripts.prx") || !strcmp((const char*)p_data + 1, "ainmenu_scripts.pre")) && using_mainmenu_scripts) {
			Log::TypedLog(CHN_MOD, "Successfully replaced %s with %s\n", (const char*)p_data, mainmenu_scripts_for_injection);
			p_data = (uint8_t*)mainmenu_scripts_for_injection;
		}

	//else if (!strcmp((const char*)p_data + 1, "anelsprites.prx") || !strcmp((const char*)p_data + 1, "anelsprites.pre")) {
	//	printf("PANELSPRITES: %s\n", (const char*)p_data);
	//	p_data = (uint8_t*)"BelaMod/bela_panelsprites.prx";
	//}
	//else if (!strcmp((const char*)p_data + 1, "ainmenu_scripts.prx") || !strcmp((const char*)p_data + 1, "ainmenu_scripts.pre")) {
	//	printf("MAINMENU: %s\n", (const char*)p_data);
	//	p_data = (uint8_t*)"BelaMod/e9_mainmenu_scripts.prx";
	//}

/*
		
*/

	PreMgrLoadPre(arg1, p_data, arg3, arg4, arg5);

}