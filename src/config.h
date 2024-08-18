#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "patch.h"
#include "displayconfig.h"
#include "global.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "input.h"
#include "QB/LazyStruct.h"
#include "QB/Qb.h"
#include "QB/script.h"
#include "QB/CFuncs.h"

void InitInstance(HANDLE hModule);
void LoadOriginalDll(void);
void ExitInstance(void);

#define CONFIG_FILE_NAME "partymod.ini"
#define GRAPHICS_SECTION "Graphics"
#define KEYBIND_SECTION "Keybinds"
#define CONTROLLER_SECTION "Gamepad"
#define MISC_SECTION "Miscellaneous"
#define GAMEPLAY_SECTION "Gameplay"
#define EXTRA_SECTION "Extra"
#define CONTROLS_SECTION "Controls"
#define VERSION_NUMBER_MAJOR 0
#define VERSION_NUMBER_MINOR 4

struct keybinds {
	//SDL_Scancode menu;
	SDL_Scancode cameraToggle;
	SDL_Scancode cameraSwivelLock;
	SDL_Scancode focus;
	SDL_Scancode caveman;
	SDL_Scancode caveman2;
	SDL_Scancode grind;
	SDL_Scancode grab;
	SDL_Scancode ollie;
	SDL_Scancode kick;
	SDL_Scancode leftSpin;
	SDL_Scancode rightSpin;
	SDL_Scancode nollie;
	SDL_Scancode switchRevert;
	SDL_Scancode right;
	SDL_Scancode left;
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode cameraRight;
	SDL_Scancode cameraLeft;
	SDL_Scancode cameraUp;
	SDL_Scancode cameraDown;
	SDL_Scancode item_up;
	SDL_Scancode item_down;
	SDL_Scancode item_left;
	SDL_Scancode item_right;
};

/* a recreation of the SDL_GameControllerButton enum, but with the addition of right / left trigger */
typedef enum {
	CONTROLLER_UNBOUND = -1,
	CONTROLLER_BUTTON_A = SDL_CONTROLLER_BUTTON_A,
	CONTROLLER_BUTTON_B = SDL_CONTROLLER_BUTTON_B,
	CONTROLLER_BUTTON_X = SDL_CONTROLLER_BUTTON_X,
	CONTROLLER_BUTTON_Y = SDL_CONTROLLER_BUTTON_Y,
	CONTROLLER_BUTTON_BACK = SDL_CONTROLLER_BUTTON_BACK,
	CONTROLLER_BUTTON_GUIDE = SDL_CONTROLLER_BUTTON_GUIDE,
	CONTROLLER_BUTTON_START = SDL_CONTROLLER_BUTTON_START,
	CONTROLLER_BUTTON_LEFTSTICK = SDL_CONTROLLER_BUTTON_LEFTSTICK,
	CONTROLLER_BUTTON_RIGHTSTICK = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
	CONTROLLER_BUTTON_LEFTSHOULDER = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
	CONTROLLER_BUTTON_RIGHTSHOULDER = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
	CONTROLLER_BUTTON_DPAD_UP = SDL_CONTROLLER_BUTTON_DPAD_UP,
	CONTROLLER_BUTTON_DPAD_DOWN = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
	CONTROLLER_BUTTON_DPAD_LEFT = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
	CONTROLLER_BUTTON_DPAD_RIGHT = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
	CONTROLLER_BUTTON_MISC1 = SDL_CONTROLLER_BUTTON_MISC1,
	CONTROLLER_BUTTON_PADDLE1 = SDL_CONTROLLER_BUTTON_PADDLE1,
	CONTROLLER_BUTTON_PADDLE2 = SDL_CONTROLLER_BUTTON_PADDLE2,
	CONTROLLER_BUTTON_PADDLE3 = SDL_CONTROLLER_BUTTON_PADDLE3,
	CONTROLLER_BUTTON_PADDLE4 = SDL_CONTROLLER_BUTTON_PADDLE4,
	CONTROLLER_BUTTON_TOUCHPAD = SDL_CONTROLLER_BUTTON_TOUCHPAD,
	CONTROLLER_BUTTON_RIGHTTRIGGER = 21,
	CONTROLLER_BUTTON_LEFTTRIGGER = 22,
} controllerButton;

typedef enum {
	CONTROLLER_STICK_UNBOUND = -1,
	CONTROLLER_STICK_LEFT = 0,
	CONTROLLER_STICK_RIGHT = 1
} controllerStick;

struct controllerbinds {
	controllerButton menu;
	controllerButton cameraToggle;
	controllerButton cameraSwivelLock;
	controllerButton focus;
	controllerButton caveman;
	controllerButton caveman2;
	controllerButton grind;
	controllerButton grab;
	controllerButton ollie;
	controllerButton kick;
	controllerButton leftSpin;
	controllerButton rightSpin;
	controllerButton nollie;
	controllerButton switchRevert;
	controllerButton right;
	controllerButton left;
	controllerButton up;
	controllerButton down;
	controllerStick movement;
	controllerStick camera;
};

typedef struct {
	uint32_t bettergraphics;
	uint32_t antialiasing;
	uint32_t hqshadows;
	uint32_t distanceclipping;
	uint32_t clippingdistance;
	uint32_t fog;
} graphicsSettings;

struct inputsettings {
	uint8_t isPs2Controls;
	uint8_t invertRXplayer1;
	uint8_t invertRYplayer1;
};

struct scriptsettings {
	uint8_t airdrift;
	uint8_t suninnetgame;
	uint8_t boardscuffs;
	uint8_t dropdowncontrol;
};

struct flashVertex {
	float x, y, z, w;
	uint32_t color;
	float u, v;
};

void enforceMaxResolution();
void createSDLWindow();
void writeConfigValues();
void patchWindow();
SDL_Window* getWindowHandle();
void initPatch();
void patchStaticValues();
//void reorderFlashVertices(uint32_t* d3dDevice, void* unused, void* alsodevice, uint32_t prim, uint32_t count, struct flashVertex* vertices, uint32_t stride);
void loadInputSettings(struct inputsettings* settingsOut);
void loadControllerBinds(struct controllerbinds* bindsOut);
void loadKeyBinds(struct keybinds* bindsOut);
int getIniBool(const char* section, const char* key, int def, char* file);
int Rnd_fixed(int n);
void setAspectRatio(float aspect);
float getScreenAngleFactor();
void patchWindow();
void patch_button_font(uint8_t sel);
float getScreenAngleFactor();
float getaspectratio();
void loadScriptSettings(struct scriptsettings* scriptsettingsOut);
void getConfigFilePath(char mConfigFile[MAX_PATH]);
void loadKeyBinds(struct keybinds* bindsOut);
void loadControllerBinds(struct controllerbinds* bindsOut);