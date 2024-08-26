#include "config.h"
#include <d3d9types.h>
#include <modloader.h>

struct modsettings windowtitle_maybe;
char* executableDirectory[MAX_PATH];
char configFile[MAX_PATH];
char window_title[MAX_PATH] = "THUG2 PARTYMOD";

float* screenAspectRatio = (float*)0x00701340;
uint8_t* antialiasing = (uint8_t*)(0x007D6434);
uint8_t* hq_shadows = (uint8_t*)0x007D6435;
uint8_t* distance_clipping = (uint8_t*)(0x007D643A);
uint8_t* fog = (uint8_t*)(0x007D6436);
uint8_t* clipping_distance = (uint8_t*)(0x007D6440);	//0x01 - 0x64 val
uint16_t* clipping_distance2 = (uint16_t*)(0x007D6444);	//0x64 - 0x253 val*5 +95
HWND* hwnd = (HWND*)0x007D6A28;
SDL_Window* window;

uint32_t WCC_Update_Native_Var = 0x005251D0;
uint32_t Cam_GetComponent_Native_Var = 0x0045DB90;
uint32_t AddShake_Native_Var = 0x004F9F00;
uint32_t WallRideAnywhere_RetAddr = 0x00500468;

uint8_t isWindowed;
int8_t isBorderless;
uint8_t console;
uint8_t language;
uint8_t buttonfont;
uint8_t intromovies;
uint8_t spindelay;
uint8_t airdrift;
uint8_t dropdowncontrol;
uint8_t suninnetgame;
uint8_t boardscuffs;
uint8_t Ps2Controls;
uint8_t quickgetup;
uint8_t wallrideanywhere;
uint8_t jankdrops;
uint8_t invertRXplayer1;
uint8_t invertRYplayer1;
bool usemod = false;

int resX;
int resY;
int defWidth;
int defHeight;
graphicsSettings graphics_settings;

typedef uint32_t ButtonLookup_NativeCall(char* button);
ButtonLookup_NativeCall* ButtonLookup_Native = (ButtonLookup_NativeCall*)(0x004020E0);

typedef uint32_t __cdecl unkButtonMap_NativeCall(uint32_t buttonmap);
unkButtonMap_NativeCall* unkButtonMap_Native = (unkButtonMap_NativeCall*)(0x00479070);

void initPatch() {
	/* First, patch static values into the exe */
	patchStaticValues();

	/* Copy CFuncs to our own area which can be searched and expanded */
	CFuncs::CopyStockFunctions();

	/* Make game refer to our function list */
	patchDWord((void*)ADDR_CFuncListRef, CFuncs::Pointer_Functions());

	/* Refer to new function for grabbing amount of CFuncs */
	patchCall((void*)ADDR_CFuncCountRef, (void*)CFuncs::Pointer_FunctionCount());

	/* Read INI config values */
	getConfigFilePath(configFile);

	console = GetPrivateProfileInt(MISC_SECTION, "Console", 0, configFile);
	language = GetPrivateProfileInt(MISC_SECTION, "Language", 1, configFile);
	buttonfont = GetPrivateProfileInt(MISC_SECTION, "ButtonFont", 1, configFile);
	intromovies = getIniBool(MISC_SECTION, "IntroMovies", 1, configFile);
	spindelay = getIniBool(GAMEPLAY_SECTION, "SpinDelay", 1, configFile);
	airdrift = getIniBool(GAMEPLAY_SECTION, "THUGAirDrift", 0, configFile);
	suninnetgame = getIniBool(MISC_SECTION, "SunInNetGame", 0, configFile);
	boardscuffs = getIniBool(MISC_SECTION, "Boardscuffs", 1, configFile);
	graphics_settings.bettergraphics = getIniBool(GRAPHICS_SECTION, "BetterGraphics", 0, configFile);
	graphics_settings.antialiasing = getIniBool(GRAPHICS_SECTION, "AntiAliasing", 0, configFile);
	graphics_settings.hqshadows = getIniBool(GRAPHICS_SECTION, "HQShadows", 0, configFile);
	graphics_settings.distanceclipping = getIniBool(GRAPHICS_SECTION, "DistanceClipping", 0, configFile);
	graphics_settings.clippingdistance = GetPrivateProfileInt("Graphics", "ClippingDistance", 100, configFile);
	graphics_settings.fog = getIniBool(GRAPHICS_SECTION, "Fog", 0, configFile);
	resX = GetPrivateProfileInt(GRAPHICS_SECTION, "ResolutionX", 640, configFile);
	resY = GetPrivateProfileInt(GRAPHICS_SECTION, "ResolutionY", 480, configFile);
	isWindowed = getIniBool(GRAPHICS_SECTION, "Windowed", 0, configFile);
	isBorderless = getIniBool(GRAPHICS_SECTION, "Borderless", 0, configFile);
	Ps2Controls = getIniBool(CONTROLS_SECTION, "Ps2Controls", 1, configFile);
	dropdowncontrol = GetPrivateProfileInt(CONTROLS_SECTION, "DropDownControl", 1, configFile);
	quickgetup = GetPrivateProfileInt(GAMEPLAY_SECTION, "QuickGetUp", 0, configFile);
	wallrideanywhere = GetPrivateProfileInt(EXTRA_SECTION, "WallrideAnywhere", 0, configFile);
	jankdrops = GetPrivateProfileInt(EXTRA_SECTION, "JankDrops", 0, configFile);
	invertRXplayer1 = getIniBool(CONTROLS_SECTION, "InvertRXPlayer1", 0, configFile);
	invertRYplayer1 = getIniBool(CONTROLS_SECTION, "InvertRYPlayer1", 0, configFile);
	usemod = getIniBool(MOD_SECTION, "UseMod", 0, configFile);

	/* Allocate console */
	if (console) {
		Log::Initialize();
		patchDWord((void*)0x0067F3D4, (uint32_t)&Log::CFunc_PrintF);
		if (console == 2) { patchJump((void*)0x00401C30, &Log::PrintLog); }
	}
	Log::TypedLog(CHN_DLL, "PARTYMOD for THUG2 %d.%d\n", VERSION_NUMBER_MAJOR, VERSION_NUMBER_MINOR);
	Log::TypedLog(CHN_DLL, "DIRECTORY: %s\n", (LPSTR)executableDirectory);
	Log::TypedLog(CHN_DLL, "Patch initialized\n");
	Log::TypedLog(CHN_DLL, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Initializing INI settings\n");

	/* Set language */
	patchNop((void*)ADDR_FUNC_LangFromReg, 5);		//Don't get the value from registry

	if (language == 1)
		patchByte((void*)ADDR_LanguageFlag, 1);		//English
	else if (language == 2)
		patchByte((void*)ADDR_LanguageFlag, 3);		//French
	else if (language == 3)
		patchByte((void*)ADDR_LanguageFlag, 5);		//German
	else
		patchByte((void*)ADDR_LanguageFlag, 1);

	patchByte((void*)(ADDR_LanguageFlag + 0x8), 0x07);	//Load and save savegames across different language settings
	patchByte((void*)(ADDR_LanguageFlag + 0xC), 0x01);
	Log::TypedLog(CHN_DLL, "Loading language setting: %s\n", (language == 1) ? "English" : ((language == 2) ? "French" : ((language == 3) ? "German" : "English")));

	/* Set button font */
	patch_button_font(buttonfont);
	if (buttonfont > 1)
		Log::TypedLog(CHN_DLL, "Loading button font: %s\n", (buttonfont == 2) ? "Ps2" : ((buttonfont == 3) ? "Xbox" : ((buttonfont == 4) ? "NGC" : "PC")));
	else
		Log::TypedLog(CHN_DLL, "Loading button font: PC\n");

	/* Set intro movies */
	if (!intromovies)
		patchBytesM((void*)ADDR_IntroMovies, (BYTE*)"\x83\xf8\x01\x90\x90\x75\x01\xc3\xe9\x83\x05\x00\x00", 13);
	Log::TypedLog(CHN_DLL, "Intro movies: %s\n", intromovies ? "Enabled" : "Disabled");

	/* Set THUG airdrift */
	if (airdrift) {
		patchNop((void*)0x00526A36, 8); //Lock camera fix
		patchNop((void*)ADDR_AirDrift, 8);
		/* Walkspin is disabled in script.cpp */
	}
	Log::TypedLog(CHN_DLL, "Airdrift: %s\n", airdrift ? "Enabled" : "Disabled");

	/* Ps2Controls */
	Log::TypedLog(CHN_DLL, "Ps2Controls: %s\n", Ps2Controls ? "Enabled" : "Disabled");

	/* Drop Down Control */
	switch (dropdowncontrol) {
		case 1: Log::TypedLog(CHN_DLL, "DropDownControl: L2+R2 (PC default)\n"); break;
		case 2: Log::TypedLog(CHN_DLL, "DropDownControl: L1\n"); break;
		case 3: Log::TypedLog(CHN_DLL, "DropDownControl: R1\n"); break;
		case 4: Log::TypedLog(CHN_DLL, "DropDownControl: L2\n"); break;
		case 5: Log::TypedLog(CHN_DLL, "DropDownControl: R2\n"); break;
	}

	/*Quick get up from bails*/
	Log::TypedLog(CHN_DLL, "Quick bail get up: %s\n", quickgetup ? "Enabled" : "Disabled");

	/* Set spindelay. Off is Ps2 default, on is PC default (value = 100) */
	if (!spindelay) {
		patchNop((void*)ADDR_SpinLagL, 2);
		patchNop((void*)ADDR_SpinLagR, 2);
	}
	Log::TypedLog(CHN_DLL, "Spindelay: %s\n", spindelay ? "Enabled (PC default)" : "Disabled (Ps2 default)");

	/*Extra tech: Wallride anywhere*/
	if (wallrideanywhere)
	{
		patchJump((void*)0x00500441, wallrideanywhere_patch);
		if (wallrideanywhere == 2) patchNop((void*)0x00500424, 6);
	}
	Log::TypedLog(CHN_DLL, "Wallride Anywhere: %s\n", (wallrideanywhere == 2) ? "Extended" : (wallrideanywhere ? "Enabled" : "Disabled"));

	if (jankdrops)
	{
		patchNop((void*)0x00502629, 6);
		patchDWord((void*)0x006467BC, 0x3F733333);
	}
	Log::TypedLog(CHN_DLL, "Jank Drops: %s\n", jankdrops ? "Enabled" : "Disabled");

	Log::TypedLog(CHN_DLL, "Custom mods: %s\n", usemod ? "Enabled" : "Disabled");

	/* Graphic settings */
	if (graphics_settings.bettergraphics) {
		/* Slight graphical improvements. this may break flash effects */
		patchNop((void*)0x0044F045, 8);
		patchNop((void*)0x0048C330, 5);
		patchNop((void*)0x004B2DC4, 5);
		patchNop((void*)0x004B3405, 5);
		/* very high shadow quality */
		patchByte((void*)(0x004A19E5 + 2), 0x04); 
		patchByte((void*)(0x004A19EA + 2), 0x04);
	}

	Log::TypedLog(CHN_DLL, "Better graphics for shadows and edges: %s\n", graphics_settings.bettergraphics ? "Enabled" : "Disabled");
	Log::TypedLog(CHN_DLL, "Graphic settings - Fullscreen Anti-Aliasing: %s\n", graphics_settings.antialiasing ? "Enabled" : "Disabled");
	Log::TypedLog(CHN_DLL, "Graphic settings - HQ Shadows: %s\n", graphics_settings.hqshadows ? "Enabled" : "Disabled");
	Log::TypedLog(CHN_DLL, "Graphic settings - Distance Clipping: %s\n", graphics_settings.distanceclipping ? "Enabled" : "Disabled");
	if (graphics_settings.distanceclipping) {
		Log::TypedLog(CHN_DLL, "Graphic settings - Clipping Distance: %d\n", graphics_settings.clippingdistance);
		Log::TypedLog(CHN_DLL, "Graphic settings - Fog: %s\n", graphics_settings.fog ? "Enabled" : "Disabled");
	}
	Log::TypedLog(CHN_DLL, "Graphic settings - Resolution from INI: %d x %d\n", resX, resY);
	Log::TypedLog(CHN_DLL, "Graphic settings - Window mode: %s \n", (isWindowed && !isBorderless) ? "Enabled (default)" : ((isWindowed && isBorderless) ? "Enabled (borderless)" : "Disabled"));

	Log::TypedLog(CHN_DLL, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Finished initializing INI settings\n");
}

void patchStaticValues() {
	/* Increase startup speed */
	patchByte((void*)0x0045002C, 0x74);
	patchByte((void*)0x0052F70F, 0xEB);
	patchByte((void*)(0x0052F7DF + 1), 0x00);
	patchByte((void*)0x0053654F, 0xEB);
	patchDWord((void*)(0x00564E64 + 1), 0x000001FF);
	patchByte((void*)0x005F3391, 0x13);
	patchByte((void*)(0x005F5EC5 + 1), 0xC7);
	patchNop((void*)(0x005F5EC5 + 2), 4);
	patchNop((void*)0x005F5EDC, 6);
	patchByte((void*)(0x005F5F09 + 1), 0xD7);
	patchNop((void*)(0x005F5F09 + 2), 4);
	patchNop((void*)(0x005F5F1D), 6);
	patchByte((void*)(0x005F5F4D + 1), 0xCF);
	patchNop((void*)(0x005F5F4D + 2), 4);
	patchNop((void*)0x005F5F61, 6);
	patchByte((void*)(0x005F5F91 + 1), 0xC7);
	patchNop((void*)(0x005F5F91 + 2), 4);
	patchNop((void*)0x005F5FA5, 6);
	patchDWord((void*)(0x005F88F7 + 2), 0x000001F4);
	patchByte((void*)(0x005F8AD6 + 6), 0x6F);
	patchByte((void*)(0x005F8ADD + 6), 0x70);
	patchByte((void*)(0x005F8AE4 + 6), 0x65);
	patchByte((void*)(0x005F8AEB + 6), 0x6E);
	patchDWord((void*)(0x005FBAF4 + 1), 0x00001388);

	/* Stability fix */
	patchBytesM((void*)0x004DB357, (BYTE*)"\xC7\x05\xAC\x6C\x78\x00\x00\x00\x7A\x47", 10);

	/* Increase script memory region */
	patchByte((void*)(0x005BBCBE + 4), 0x10);
	patchByte((void*)(0x005BBCD9 + 4), 0x10);

	/* Blur fix (since Windows Vista) */
	patchBytesM((void*)ADDR_FUNC_BlurEffect, (BYTE*)"\xB0\x01\xC3\x90\x90", 5);

	/* No CAS integrity check */
	patchNop((void*)0x005A8A01, 2);

	/* Patch static values for online play */
	patchBytesM((void*)0x0064CD97, (BYTE*)"\x74\x68\x6D\x6F\x64\x73\x2E\x63\x6F\x6D\x2F\x6D\x6F\x74\x64\x2E\x64\x61\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 65);
	patchBytesM((void*)0x0064D67A, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0066729D, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0066731A, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x006673BC, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x006673D0, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0066776C, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x00667D61, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x00668014, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0068E7E2, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0068EA7D, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x0068EABD, (BYTE*)"\x6F\x70\x65\x6E\x73\x70\x79\x2E\x6E\x65\x74", 11);
	patchBytesM((void*)0x00668074, (BYTE*)"\x54\x32\x43\x72\x61\x63\x6B\x45\x00\x00\x00\x00\x00\x00\x00\x00", 16);

	/* Connection refused fix */
	patchByte((void*)0x005F95BB, 0xEB);

	/* MOTD patch: http://www.thugonline.com/motd.txt */
	patchBytesM((void*)0x0064CD97, (BYTE*)"\x74\x68\x75\x67\x6F\x6E\x6C\x69\x6E\x65\x2E\x63\x6F\x6D\x2F\x6D\x6F\x74\x64\x2E\x74\x78\x74", 23);

	/* Patch fixed RNG */
	srand(time(0));
	patchCall((void*)0x004523A7, &Rnd_fixed);
	patchCall((void*)0x004523B4, &Rnd_fixed);
	patchCall((void*)0x004523F6, &Rnd_fixed);

	// Allows shake for walk camera! This will not work otherwise.
	patchDWord((void*)0x0064C5E4, (int32_t)&WalkCamComponent_Update_Hook);
}

void __fastcall reorderFlashVertices(void* unused, uint32_t* d3dDevice, void* alsodevice, uint32_t prim, uint32_t count, struct flashVertex* vertices, uint32_t stride) {

	/* vertices are passed into the render function in the wrong order when drawing screen flashes; reorder them before passing to draw */

	typedef void(__fastcall* drawPrimitiveUP_NativeCall)(void*, void*, void*, uint32_t, uint32_t, struct flashVertex*, uint32_t);
	drawPrimitiveUP_NativeCall drawPrimitiveUP_Native = (drawPrimitiveUP_NativeCall)(d3dDevice[83]);

	struct flashVertex tmp;
	tmp = vertices[0];
	vertices[0] = vertices[1];
	vertices[1] = vertices[2];
	vertices[2] = tmp;

	drawPrimitiveUP_Native(unused, d3dDevice, alsodevice, prim, count, vertices, stride);
}

void patchWindow() {
	/* replace the window with an SDL2 window. this kind of straddles the line between input and config */
	patchCall((void*)ADDR_FUNC_CreateWindow, &createSDLWindow);
	patchByte((void*)(ADDR_FUNC_CreateWindow + 5), 0xC3);

	/* don't move window to corner */
	patchNop((void*)ADDR_FixWindowPos, 14);

	/* don't load launcher settings from registry, use our own INI values instead */
	patchCall((void*)0x005F4591, writeConfigValues);

	/* patch screen flash (e.g., happens during Franklin Grind) */
	patchNop((void*)0x004B91E7, 6);
	patchCall((void*)0x004B91E7, reorderFlashVertices);
}

void enforceMaxResolution() {
	defWidth = GetSystemMetrics(SM_CXSCREEN);	/* The width of the screen of the primary display monitor, in pixels.  */
	defHeight = GetSystemMetrics(SM_CYSCREEN);	/* The height of the screen of the primary display monitor, in pixels.  */

	uint8_t isValidX = 0;
	uint8_t isValidY = 0;
	DEVMODE deviceMode = {};
	int i = 0;

	if (!isWindowed) {
		while (EnumDisplaySettings(NULL, i, &deviceMode)) {
			if (deviceMode.dmPelsWidth == resX) {
				isValidX = 1;
			}
			if (deviceMode.dmPelsHeight == resY) {
				isValidY = 1;
			}
			i++;
		}
		if (!isValidX || !isValidY) {
			resX = 0;
			resY = 0;
		}
	}
	else if (resX > defWidth || resY > defHeight) {
		resX = 0;
		resY = 0;
	}
}

void createSDLWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	uint32_t flags = isWindowed ? (SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE) : SDL_WINDOW_FULLSCREEN;

	if (isWindowed && isBorderless) {
		flags |= SDL_WINDOW_BORDERLESS;
	}

	/* Fullscreen mode: Sets resX and resY to 0 if the resolution from INI is not supported on the device. */
	/* Window mode : Sets resX and resY to 0 if the resoltion from INI is bigger than the max supported one */
	enforceMaxResolution();

	if (resX == 0 || resY == 0) {
		resX = defWidth;
		resY = defHeight;
		Log::TypedLog(CHN_DLL, "Found invalid resolution. Falling back to: %d x %d\n", defWidth, defHeight);
	}
	else if (resX < 640 || resY < 480) {
		resX = 640;
		resY = 480;
		Log::TypedLog(CHN_DLL, "Found invalid resolution. Falling back to: %d x %d\n", resX, resY);
	}
	else {
		Log::TypedLog(CHN_DLL, "Setting resolution: %d x %d\n", resX, resY);
	}

	Log::TypedLog(CHN_DLL, "Aspect ratio: %f\n", getaspectratio());

	if (usemod) {
		getWindowTitle(&windowtitle_maybe);
		if (strlen(windowtitle_maybe.windowtitle))
			sprintf_s(window_title, "%s%s%s%s", window_title, " (", windowtitle_maybe.windowtitle, ")");
	}
	window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, resX, resY, flags);   // TODO: move / resize borderless window
	SDL_SetWindowResizable(window, SDL_TRUE);

	if (!window)
		Log::TypedLog(CHN_SDL, "Failed to create window! Error: %s\n", SDL_GetError());
	else
		Log::TypedLog(CHN_SDL, "Window successfully created!\n");

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	*hwnd = wmInfo.info.win.window;
	*(int*)ADDR_IsFocused = 1;

	//DirectX9: D3DPRESENTPARAMS
	if (isWindowed) {
		patchBytesM((void*)0x004D871F, (BYTE*)"\xA3\x9C\x6A\x78\x00\x90", 6); /* Windowed = 1 */
		SDL_ShowCursor(1);
	}
	else {
		SDL_ShowCursor(0);
	}

	/* patch resolution of the window */
	patchDWord((void*)ADDR_WindowResoltionX, resX);
	patchDWord((void*)ADDR_WindowResoltionY, resY);

	/* set aspect ratio and FOV */
	patchJump((void*)ADDR_FUNC_AspectRatio, setAspectRatio);
	patchJump((void*)ADDR_FUNC_ScreenAngleFactor, getScreenAngleFactor);
}

SDL_Window* getWindowHandle() {
	return window;
}

void writeConfigValues() {
	*antialiasing = graphics_settings.antialiasing;
	if (graphics_settings.hqshadows) {
		*hq_shadows = graphics_settings.hqshadows;
	}
	*distance_clipping = graphics_settings.distanceclipping;
	*fog = graphics_settings.fog;

	uint32_t distance = graphics_settings.clippingdistance;
	uint32_t distance2 = 1;

	if (distance > 100)
		distance = 100;
	else if (distance < 2) {
		distance = 1;
		distance2 = 0;
	}
	*clipping_distance = distance;
	*clipping_distance2 = distance * 5.0 + 95.0;

	Log::TypedLog(CHN_DLL, "Setting launcher settings\n");
}

void __cdecl setAspectRatio(float aspect) {
	*screenAspectRatio = (float)resX / (float)resY;
}

float __cdecl getScreenAngleFactor() {
	return ((float)resX / (float)resY) / (4.0f / 3.0f);
}

/* called from patchScripts */
float getaspectratio() {
	return ((float)resX / (float)resY);
}

/* called from patchScripts */
void loadScriptSettings(struct scriptsettings* scriptsettingsOut) {
	if (scriptsettingsOut) {
		scriptsettingsOut->airdrift = airdrift;
		scriptsettingsOut->suninnetgame = suninnetgame;
		scriptsettingsOut->boardscuffs = boardscuffs;
		scriptsettingsOut->dropdowncontrol = dropdowncontrol;
		scriptsettingsOut->quickgetup = quickgetup;
	}
}

/* called from initMod */
void loadModSettings(struct modsettings* modsettingsOut) {
	modsettingsOut->usemod = usemod;
	modsettingsOut->configfile = configFile;
	modsettingsOut->workingdir = (char*)executableDirectory;

}

int getIniBool(const char* section, const char* key, int def, char* file) {
	int result = GetPrivateProfileInt(section, key, def, file);
	if (result) {
		return 1;
	}
	else {
		return 0;
	}
}

int Rnd_fixed(int n) {
	return (rand() % n);
}

uint32_t patchButtonLookup(char* p_button) {
	uint8_t value = *p_button;
	uint8_t original_value = *p_button;
	uint8_t base = *(p_button - 0x30);
	uint8_t* map = (uint8_t*)0x005E2155;

	if ((value > 0x4C) && (value < 0x75)) {
		value = *(uint8_t*)(map + value - 0x4D); /* step through map */

		switch (value) {
		case 0:
			value = 0x33;
			break;
		case 1:
			value = 0x32;
			break;
		case 2:
			value = 0x31;
			break;
		case 3:
			value = 0x30;
			break;
		case 4:
			value = 0x65;
			break;
		case 5:
			value = 0x66;
			break;
		case 6:
			value = 0x67;
			break;
		case 7:
			value = 0x68;
			break;
		default:
			value = original_value;
			break;
		}
	}
	if (value < 0x3A) {
		return (value - 0x30);
	}
	else if (value < 0x57) {
		return (value - 0x37);
	}
	else if (value < 0x77) {
		return (value - 0x57);
	}
	else { return 0; }

	/* return ButtonLookup_Native(button); */
	/* Original implementation */
	/*
	uint8_t value = *p_button;

	if ((value > 0x30) && (value < 0x39)) {
		return (value - 0x30);
	}
	else if ((value > 0x41) && (value < 0x56)) {
		return (value - 0x37);
	}
	else if ((value > 0x61) && (value < 0x76)) {
		return (value - 0x57);
	}
	else {
		return 0;
	}
	*/

	/*
	0x00 = Triangle
	0x01 = Square
	0x02 = Circle
	0x03 = X
	0x04 = Arrow Down
	0x05 = Arrow Right
	0x06 = Arrow Left
	0x07 = Arrow Up
	0x08 = Select
	0x09 = Start
	0x0A = Arrow Down Right
	0x0B = Arrow Up Left
	0x0C = Arrow Down Left
	0x0D = Arrow Up Right
	0x0E = [L1]
	0x0F = [R1]
	0x10 = [L2]
	0x11 = [R2]
	0x12 = +
	0x13 = Y/C
	0x14 = S/X
	0x15 = <empty>
	0x16 = ENT
	0x17 = ESC
	0x18 = E
	0x19 = R
	0x1A = ß
	0x1B = `
	0x1C = 1
	0x1D = 2
	Rest is empty
	*/
}

uint32_t patchMetaButtonMap() {
	if (buttonfont == 2)
		return unkButtonMap_Native(0x6030A16D); /* meta_button_map_ps2 */
	else if (buttonfont == 3)
		return unkButtonMap_Native(0xBAF816FB); /* meta_button_map_xbox */
	else if (buttonfont == 4)
		return unkButtonMap_Native(0xEE6CDAC5); /* meta_button_map_gamecube */
}

void patch_button_font(uint8_t sel) {
	/* 1 = PC (default), 3 = Xbox. Both have the same text: Xbox.buttons_font */

	if (sel == 2)
		patchDWord((void*)0x00648B03, 0x00327350); // Ps2..buttons_font
	else if (sel == 4)
		patchDWord((void*)0x00648B03, 0x0063674E); // Ngc..buttons_font

	if (1 < sel && sel < 5)
	{
		patchCall((void*)0x0048D8FC, patchMetaButtonMap);
		patchByte((void*)(0x004CFF36 + 1), 0x11);
		patchByte((void*)0x004CFF38, 0x77); /* jump if above 0x11 */

		patchByte((void*)0x004CFF3C, 0xEB);
		patchByte((void*)(0x004CFF3C + 1), 0x68); /* stop and jump to end */

		/* first code cave that holds at least 40 bytes */
		patchBytesM((void*)0x005E2155, (BYTE*)"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x00\x01\x02\x03\x04\x05\x06\x07", 40);
		patchCall((void*)0x004CFF0C, &patchButtonLookup);
	}
}

/* provide info about input settings for input.cpp */
void loadInputSettings(struct inputsettings* settingsOut) {
	if (settingsOut) {
		settingsOut->isPs2Controls = Ps2Controls;
		settingsOut->invertRXplayer1 = invertRXplayer1;
		settingsOut->invertRYplayer1 = invertRYplayer1;
	}
}

void getConfigFilePath(char mConfigFile[MAX_PATH]) {
	GetModuleFileName(NULL, (LPSTR)&executableDirectory, MAX_PATH);

	// find last slash
	char* exe = strrchr((LPSTR)executableDirectory, '\\');
	if (exe) {
		*(exe + 1) = '\0';
	}
	sprintf(mConfigFile, "%s%s", executableDirectory, CONFIG_FILE_NAME);
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

/* Hook for Obj::CWalkCameraComponent::Update. Used for shakes! */
void __declspec(naked) WalkCamComponent_Update_Hook()
{
	/*
	WCC_Update_Native(cam);

	// Add shake from the skater cam onto this cam.
	// This allows shaking even when we're off the board.

	Obj::CSkaterCameraComponent* skaterCam = (Obj::CSkaterCameraComponent*)GetComponent_Native(cam->mp_object, 0x5E43A604); // skatercamera
	if (skaterCam && skaterCam->mShakeDuration > 0.0) {
		Mth::Vector* pos = (Mth::Vector*)&(cam->mp_object->m_pos);
		Mth::Matrix* mtr = (Mth::Matrix*)&(cam->mp_object->orientation);
		AddShake_Native(skaterCam, pos, mtr);
	}
	*/
	__asm {
		push ebx
		mov ebx, ecx
		sub esp, 0x18
		call dword ptr ds : WCC_Update_Native_Var
		mov ecx, dword ptr ds : [ebx + 0xC]
		mov dword ptr ss : [esp] , 0x5E43A604
		call dword ptr ds : Cam_GetComponent_Native_Var
		sub esp, 4
		test eax, eax
		je label_end
		fldz
		fld dword ptr[eax + 0x154]
		mov ecx, eax
		fcomip st(0), st(1)
		fstp st(0)
		jbe label_end
		mov eax, dword ptr ds : [ebx + 0xC]
		lea edx, ds : [eax + 0x6C]
		add eax, 0x4C
		mov dword ptr ss : [esp + 0x4] , edx
		mov dword ptr ss : [esp] , eax
		call dword ptr ds : AddShake_Native_Var
		sub esp, 0x8
		label_end :
		add esp, 0x18
		pop ebx
		ret
	}
}

/* Keyboard binds */
void loadKeyBinds(struct keybinds* bindsOut) {
	char configFile[MAX_PATH];
	getConfigFilePath(configFile);

	if (bindsOut) {
		//bindsOut->menu = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Pause", SDL_SCANCODE_RETURN, configFile);
		bindsOut->cameraToggle = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "ViewToggle", SDL_SCANCODE_TAB, configFile);
		bindsOut->cameraSwivelLock = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "SwivelLock", SDL_SCANCODE_GRAVE, configFile);
		bindsOut->focus = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Focus", SDL_SCANCODE_KP_0, configFile);
		bindsOut->caveman = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Caveman", SDL_SCANCODE_KP_1, configFile);
		bindsOut->caveman2 = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Caveman2", SDL_SCANCODE_KP_3, configFile);

		bindsOut->grind = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Grind", SDL_SCANCODE_KP_8, configFile);
		bindsOut->grab = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Grab", SDL_SCANCODE_KP_6, configFile);
		bindsOut->ollie = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Ollie", SDL_SCANCODE_KP_2, configFile);
		bindsOut->kick = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Flip", SDL_SCANCODE_KP_4, configFile);

		bindsOut->leftSpin = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "SpinLeft", SDL_SCANCODE_KP_7, configFile);
		bindsOut->rightSpin = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "SpinRight", SDL_SCANCODE_KP_9, configFile);

		bindsOut->right = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Right", SDL_SCANCODE_D, configFile);
		bindsOut->left = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Left", SDL_SCANCODE_A, configFile);
		bindsOut->up = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Forward", SDL_SCANCODE_W, configFile);
		bindsOut->down = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "Backward", SDL_SCANCODE_S, configFile);

		bindsOut->cameraRight = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "CameraRight", SDL_SCANCODE_V, configFile);
		bindsOut->cameraLeft = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "CameraLeft", SDL_SCANCODE_C, configFile);
		bindsOut->cameraUp = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "CameraUp", SDL_SCANCODE_Z, configFile);
		bindsOut->cameraDown = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "CameraDown", SDL_SCANCODE_X, configFile);

		bindsOut->item_up = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "ItemUp", SDL_SCANCODE_HOME, configFile);
		bindsOut->item_down = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "ItemDown", SDL_SCANCODE_END, configFile);
		bindsOut->item_left = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "ItemLeft", SDL_SCANCODE_DELETE, configFile);
		bindsOut->item_right = (SDL_Scancode)GetPrivateProfileInt(KEYBIND_SECTION, "ItemRight", SDL_SCANCODE_PAGEDOWN, configFile);
	}
}

/* Gamepad binds */
void loadControllerBinds(struct controllerbinds* bindsOut) {
	char configFile[MAX_PATH];
	getConfigFilePath(configFile);

	if (bindsOut) {
		bindsOut->menu = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Pause", CONTROLLER_BUTTON_START, configFile);
		bindsOut->cameraToggle = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "ViewToggle", CONTROLLER_BUTTON_BACK, configFile);
		bindsOut->cameraSwivelLock = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "SwivelLock", CONTROLLER_BUTTON_RIGHTSTICK, configFile);
		bindsOut->focus = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Focus", CONTROLLER_BUTTON_LEFTSTICK, configFile);
		bindsOut->caveman = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Caveman", CONTROLLER_BUTTON_RIGHTTRIGGER, configFile);
		bindsOut->caveman2 = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Caveman2", CONTROLLER_BUTTON_LEFTTRIGGER, configFile);

		bindsOut->grind = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Grind", CONTROLLER_BUTTON_Y, configFile);
		bindsOut->grab = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Grab", CONTROLLER_BUTTON_B, configFile);
		bindsOut->ollie = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Ollie", CONTROLLER_BUTTON_A, configFile);
		bindsOut->kick = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Flip", CONTROLLER_BUTTON_X, configFile);

		bindsOut->leftSpin = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "SpinLeft", CONTROLLER_BUTTON_LEFTSHOULDER, configFile);
		bindsOut->rightSpin = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "SpinRight", CONTROLLER_BUTTON_RIGHTSHOULDER, configFile);
		bindsOut->nollie = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Nollie", CONTROLLER_BUTTON_LEFTTRIGGER, configFile);
		bindsOut->switchRevert = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Switch", CONTROLLER_BUTTON_RIGHTTRIGGER, configFile);

		bindsOut->right = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Right", CONTROLLER_BUTTON_DPAD_RIGHT, configFile);
		bindsOut->left = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Left", CONTROLLER_BUTTON_DPAD_LEFT, configFile);
		bindsOut->up = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Forward", CONTROLLER_BUTTON_DPAD_UP, configFile);
		bindsOut->down = (controllerButton)GetPrivateProfileInt(CONTROLLER_SECTION, "Backward", CONTROLLER_BUTTON_DPAD_DOWN, configFile);

		bindsOut->movement = (controllerStick)GetPrivateProfileInt(CONTROLLER_SECTION, "MovementStick", CONTROLLER_STICK_LEFT, configFile);
		bindsOut->camera = (controllerStick)GetPrivateProfileInt(CONTROLLER_SECTION, "CameraStick", CONTROLLER_STICK_RIGHT, configFile);
	}
}
