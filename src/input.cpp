#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include <patch.h>
#include <config.h>
#include "keycodes.h"
#include <QB/LazyStruct.h>
#include <QB/Qb.h>

#define MAX_PLAYERS 2

void patchPs2Buttons();
void patchInput();
uint8_t isKeyboardTyping();
uint8_t menu_on_screen();
void CheckChatHotkey();
bool TextInputInNetGame();
SDL_Window* mWindow;

uint32_t checksum;
void __cdecl set_actuators(int port, uint16_t hight, uint16_t low);

SDL_Locale* locale = SDL_GetPreferredLocales();
HKL lang = ::GetKeyboardLayout(0);
LANGID language = PRIMARYLANGID(lang);


char* executableDirectory3[MAX_PATH];
typedef struct {
	uint32_t vtablePtr;
	uint32_t node;
	uint32_t type;
	uint32_t port;
	uint32_t slot;
	uint32_t isValid;
	uint32_t unk_24;
	uint8_t controlData[32];	// PS2 format control data
	uint8_t vibrationData_align[32];
	uint8_t vibrationData_direct[32];
	uint8_t vibrationData_max[32];
	uint8_t vibrationData_oldDirect[32];    // there may be something before this
	uint32_t unk5;
	//uint32_t unk6;
	uint32_t actuatorsDisabled;
	uint32_t capabilities;
	uint32_t unk7;
	uint32_t num_actuators;
	uint32_t unk8;
	uint32_t state;
	uint32_t test;
	uint32_t index;
	uint32_t isPluggedIn;
	uint32_t unplugged_counter;
	uint32_t unplugged_retry;
	uint32_t pressed;
	uint32_t start_or_a_pressed;

} device;

int controllerCount;
int controllerListSize;
SDL_GameController** controllerList;
struct inputsettings inputsettings;
struct keybinds keybinds;
struct controllerbinds padbinds;

uint8_t isUsingKeyboard = 1;

typedef bool __cdecl ScriptObjectExists_NativeCall(Script::LazyStruct* params);
ScriptObjectExists_NativeCall* ScriptObjectExists_Native = (ScriptObjectExists_NativeCall*)(0x00462340); //Thug2 offset


struct playerslot {
	SDL_GameController* controller;
	uint8_t lockedOut;
	//uint64_t signin;	// time until controller can be used after sign-in
	//SDL_GameControllerButton lockedButton;	// button that player used to sign-in, to be ignored until release
};

#define MAX_PLAYERS 2
uint8_t numplayers = 0;
struct playerslot players[MAX_PLAYERS] = { { NULL, 0 }, { NULL, 0 } };

void setUsingKeyboard(uint8_t usingKeyboard) {
	isUsingKeyboard = usingKeyboard;
}

void patchPs2Buttons();

void addController(int idx) {

	SDL_GameController* controller = SDL_GameControllerOpen(idx);

	SDL_GameControllerSetPlayerIndex(controller, -1);

	if (controller) {
		if (controllerCount == controllerListSize) {
			int tmpSize = controllerListSize + 1;
			SDL_GameController** tmp = (SDL_GameController**)realloc(controllerList, sizeof(SDL_GameController*) * tmpSize);
			if (!tmp) {
				return; // TODO: log something here or otherwise do something
			}

			controllerListSize = tmpSize;
			controllerList = tmp;
		}

		controllerList[controllerCount] = controller;
		controllerCount++;
	}
}

void addplayer(SDL_GameController* controller) {
	if (numplayers < 2) {
		// find open slot
		uint8_t found = 0;
		int i = 0;
		for (; i < MAX_PLAYERS; i++) {
			if (!players[i].controller) {
				found = 1;
				break;
			}
		}
		if (found) {
			SDL_GameControllerSetPlayerIndex(controller, i);
			players[i].controller = controller;
			numplayers++;

			players[i].lockedOut = 1;
			Log::TypedLog(CHN_SDL, "Added player %d: %s\n", i + 1, SDL_GameControllerName(controller));

			SDL_JoystickRumble(SDL_GameControllerGetJoystick(controller), 0xffff, 0xffff, 250);
		}
	}
	else {
		Log::TypedLog(CHN_SDL, "Already two players, not adding\n");
	}
}

void pruneplayers() {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (players[i].controller && !SDL_GameControllerGetAttached(players[i].controller)) {
			Log::TypedLog(CHN_SDL, "Pruned player %d\n", i + 1);

			players[i].controller = NULL;
			numplayers--;
			Log::TypedLog(CHN_SDL, "Remaining players: %d\n", numplayers);
		}
	}
}

void removeController(SDL_GameController* controller) {
	Log::TypedLog(CHN_SDL, "Controller \"%s\" disconnected\n", SDL_GameControllerName(controller));

	int i = 0;

	while (i < controllerCount && controllerList[i] != controller) {
		i++;
	}

	if (controllerList[i] == controller) {
		SDL_GameControllerClose(controller);

		int playerIdx = SDL_GameControllerGetPlayerIndex(controller);
		if (playerIdx != -1) {
			Log::TypedLog(CHN_SDL, "Removed player %d\n", playerIdx + 1);
			players[playerIdx].controller = NULL;
			numplayers--;
		}

		pruneplayers();

		for (; i < controllerCount - 1; i++) {
			controllerList[i] = controllerList[i + 1];
		}
		controllerCount--;
	}
	else {
		//setActiveController(NULL);
		Log::TypedLog(CHN_SDL, "Did not find disconnected controller in list\n");
	}
}

void initSDLControllers() {
	Log::TypedLog(CHN_SDL, "Initializing Controller Input\n");

	controllerCount = 0;
	controllerListSize = 1;
	controllerList = (SDL_GameController**)malloc(sizeof(SDL_GameController*) * controllerListSize);
	int detected = 0;

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_IsGameController(i)) {
			addController(i);
			if (!(detected)) Log::TypedLog(CHN_SDL, "Detected controller \"%s\"\n", SDL_GameControllerNameForIndex(i));
			detected = 1;
		}
	}

	// add event filter for newly connected controllers
	//SDL_SetEventFilter(controllerEventFilter, NULL);
}

uint8_t axisAbs(uint8_t val) {
	if (val > 127) {
		// positive, just remove top bit
		return val & 0x7F;
	}
	else {
		// negative
		return ~val & 0x7F;
	}
}

uint8_t getButton(SDL_GameController* controller, controllerButton button) {
	if (button == CONTROLLER_BUTTON_LEFTTRIGGER) {
		uint8_t pressure = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >> 7;
		return pressure > 0x80;
	}
	else if (button == CONTROLLER_BUTTON_RIGHTTRIGGER) {
		uint8_t pressure = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >> 7;
		return pressure > 0x80;
	}
	else {
		return SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)button);
	}
}

void getStick(SDL_GameController* controller, controllerStick stick, uint8_t* xOut, uint8_t* yOut) {
	uint8_t result_x, result_y;

	if (stick == CONTROLLER_STICK_LEFT) {
		result_x = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) >> 8) + 128);
		result_y = (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) >> 8) + 128);
	}
	else if (stick == CONTROLLER_STICK_RIGHT) {
		result_x = inputsettings.invertRXplayer1 ? 255 - (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) >> 8) + 128) : (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) >> 8) + 128);
		result_y = inputsettings.invertRYplayer1 ? 255 - (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) >> 8) + 128) : (uint8_t)((SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) >> 8) + 128);
	}
	else {
		result_x = 0x80;
		result_y = 0x80;
	}

	if (axisAbs(result_x) > axisAbs(*xOut)) {
		*xOut = result_x;
	}

	if (axisAbs(result_y) > axisAbs(*yOut)) {
		*yOut = result_y;
	}
}

void pollController(device* dev, SDL_GameController* controller) {

	//TODO: Add "if in menu: default menu binds". The button hints at the bottom of the menus don't change and are set to a default.
	//If a player is in a menu, make it default to these binds. It's basically PC controls + Spinkeys on L2 and R2, Caveman1/2 on L1 and R2
	//Same for keyboard with e+r in CAS, Enter and Backspace in Menus, correct ESC behavior, Keyboard Typing

	if (SDL_GameControllerGetAttached(controller)) {
		dev->isValid = 1;
		dev->isPluggedIn = 1;

		// buttons
		if (getButton(controller, padbinds.menu)) {
			dev->controlData[2] |= 0x01 << 3;
		}
		if (getButton(controller, padbinds.cameraToggle)) {
			dev->controlData[2] |= 0x01 << 0;
		}
		if (getButton(controller, padbinds.cameraSwivelLock)) {
			dev->controlData[2] |= 0x01 << 2;
		}
		if (getButton(controller, padbinds.focus)) {
			dev->controlData[2] |= 0x01 << 1;
		}

		if (getButton(controller, padbinds.grind)) {
			dev->controlData[3] |= 0x01 << 4;
			dev->controlData[12] = 0xff;
		}
		if (getButton(controller, padbinds.grab)) {
			dev->controlData[3] |= 0x01 << 5;
			dev->controlData[13] = 0xff;
		}
		if (getButton(controller, padbinds.ollie)) {
			dev->controlData[3] |= 0x01 << 6;
			dev->controlData[14] = 0xff;
		}
		if (getButton(controller, padbinds.kick)) {
			dev->controlData[3] |= 0x01 << 7;
			dev->controlData[15] = 0xff;
		}

		// shoulders
		if (inputsettings.isPs2Controls) //PS2 CONTROLS
		{

			if (getButton(controller, padbinds.leftSpin)) {
				dev->controlData[3] |= 0x01 << 2;
				dev->controlData[16] = 0xff;
			}

			if (getButton(controller, padbinds.rightSpin)) {
				dev->controlData[3] |= 0x01 << 3;
				dev->controlData[17] = 0xff;
			}

			if (getButton(controller, padbinds.nollie)) {
				dev->controlData[3] |= 0x01 << 0;
				dev->controlData[18] = 0xff;
			}

			if (getButton(controller, padbinds.switchRevert)) {
				dev->controlData[3] |= 0x01 << 1;
				dev->controlData[19] = 0xff;
			}

			//Two button caveman on spinkeys
			if ((getButton(controller, padbinds.leftSpin)) && getButton(controller, padbinds.rightSpin)) {
				dev->controlData[20] |= 0x01 << 0;
			}

		}
		else //NO PS2 CONTROLS
		{
			if (getButton(controller, padbinds.leftSpin)) {
				dev->controlData[3] |= 0x01 << 2;
				dev->controlData[16] = 0xff;
				dev->controlData[3] |= 0x01 << 0;
				dev->controlData[18] = 0xff;
			}

			if (getButton(controller, padbinds.rightSpin)) {
				dev->controlData[3] |= 0x01 << 3;
				dev->controlData[17] = 0xff;
				dev->controlData[3] |= 0x01 << 1;
				dev->controlData[19] = 0xff;
			}

			if (getButton(controller, padbinds.caveman)) {
				dev->controlData[20] |= 0x01 << 0;
			}
			if (getButton(controller, padbinds.caveman2)) {
				//printf("Caveman2\n");
				dev->controlData[20] |= 0x01 << 1; //Just Caveman but also "Zoom Out" in Create-A-Goal
			}
		}


		// d-pad
		if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)padbinds.up)) {
			dev->controlData[2] |= 0x01 << 4;
			dev->controlData[10] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)padbinds.right)) {
			dev->controlData[2] |= 0x01 << 5;
			dev->controlData[8] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)padbinds.down)) {
			dev->controlData[2] |= 0x01 << 6;
			dev->controlData[11] = 0xFF;
		}
		if (SDL_GameControllerGetButton(controller, (SDL_GameControllerButton)padbinds.left)) {
			dev->controlData[2] |= 0x01 << 7;
			dev->controlData[9] = 0xFF;
		}

		// sticks
		getStick(controller, padbinds.camera, &(dev->controlData[4]), &(dev->controlData[5]));
		getStick(controller, padbinds.movement, &(dev->controlData[6]), &(dev->controlData[7]));

	}
}

uint8_t getKey(SDL_Scancode key) {

	uint8_t* keyboardState = (uint8_t*)SDL_GetKeyboardState(NULL);

	if (menu_on_screen()) {
		// if a menu is on screen, ignore menu binds
		if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_UP || key == SDL_SCANCODE_DOWN || key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_RIGHT) {
			return 0;
		}
	}

	return keyboardState[key];
}

int buffer = 0;
int tauntbuffer = 0;

bool TextInputInNetGame() {

	bool has_keyboard = false;
	bool has_menu = false;
	bool has_dialog = false;
	bool has_quit_dialog = false;

	Script::LazyStruct* checkParams = Script::LazyStruct::s_create();

	// id, keyboard_anchor
	checkParams->AddChecksum(0x40C698AF, 0x31631B98);
	has_keyboard = ScriptObjectExists_Native(checkParams);
	checkParams->Clear();

	// id, current_menu_anchor
	checkParams->AddChecksum(0x40C698AF, 0xF53D1D83);
	has_menu = ScriptObjectExists_Native(checkParams);
	checkParams->Clear();

	// id, dialog_box_Anchor
	checkParams->AddChecksum(0x40C698AF, 0x3B56E746);
	has_dialog = ScriptObjectExists_Native(checkParams);
	checkParams->Clear();

	// id, quit_dialog_anchor
	checkParams->AddChecksum(0x40C698AF, 0x4C8BF619);
	has_quit_dialog = ScriptObjectExists_Native(checkParams);

	Script::LazyStruct::s_free(checkParams);

	return (!has_keyboard && !has_menu && !has_dialog && !has_quit_dialog);
	
}

void taunt(uint8_t tauntkey) {

	uint8_t modstate = SDL_GetModState();

	if (TextInputInNetGame()) {
		Script::LazyStruct* checkParams = Script::LazyStruct::s_create();

		switch (tauntkey) {
		case 1:
			checkParams->AddChecksum(0xB53D0E0F, 0xE5FD359);	// string_id, props_string
			break;
		case 2:
			checkParams->AddChecksum(0xB53D0E0F, 0xBEEA3518);	// string_id, your_daddy_string
			break;
		case 3:
			checkParams->AddChecksum(0xB53D0E0F, 0x4525ADBD);	// string_id, get_some_string
			break;
		case 4:
			checkParams->AddChecksum(0xB53D0E0F, 0xA36DBEE1);	// string_id, no_way_string
			break;
		default:
			break;
		}
		RunScript(0x2C43B5BA, checkParams, nullptr, nullptr); // Script: SendTauntMessage
		Script::LazyStruct::s_free(checkParams);
	}
}

void pollKeyboard(device* dev) {

	dev->isValid = 1;
	dev->isPluggedIn = 1;

	uint8_t* keyboardState = (uint8_t*)SDL_GetKeyboardState(NULL);

	if (buffer > 0)	
		buffer--;

	if (tauntbuffer > 0)
		tauntbuffer--;

	// ----------------------------------------------
	// STATIC KEYS
	// ----------------------------------------------

	// F1 Taunt
	if (keyboardState[0x3A] && tauntbuffer == 0 && !isKeyboardTyping()) {
		taunt(1);
		tauntbuffer = 120;
	}

	// F2 taunt
	if (keyboardState[0x3B] && tauntbuffer == 0 && !isKeyboardTyping()) {
		taunt(2);
		tauntbuffer = 120;
	}

	// F3 taunt
	if (keyboardState[0x3C] && tauntbuffer == 0 && !isKeyboardTyping()) {
		taunt(3);
		tauntbuffer = 120;
	}
	// F4 taunt (don't send taunt when quitting game with ALT+F4)
	if (keyboardState[0x3D] && !keyboardState[0xE2] && tauntbuffer == 0 && !isKeyboardTyping()) {
		taunt(4);
		tauntbuffer = 120;
	}

	// Quick chat = RETURN
	if (keyboardState[0x28] && buffer == 0 && !isKeyboardTyping()) {
		if (menu_on_screen()) {
			dev->controlData[3] |= 0x01 << 6;
			buffer = 15;
		} else if (*(uint8_t*)(0x7CCDF8)) {
			CheckChatHotkey();
			buffer = 20;
		}
		
	}

	// Menu = ESC
	if (keyboardState[0x29] && buffer == 0) {
		//if (menu_on_screen()) {
			dev->controlData[3] |= 0x01 << 5;
		//} else {
			dev->controlData[2] |= 0x01 << 3;
		//}
		buffer = 20;
	}

	// ----------------------------------------------
	// USER KEYS
	// ----------------------------------------------

	if (keyboardState[keybinds.cameraToggle]) {
		dev->controlData[2] |= 0x01 << 0;
	}
	if (keyboardState[keybinds.focus]) { // no control for left stick on keyboard
		dev->controlData[2] |= 0x01 << 1;
	}
	if (keyboardState[keybinds.cameraSwivelLock]) {
		dev->controlData[2] |= 0x01 << 2;
	}

	if (keyboardState[keybinds.grind]) {
		dev->controlData[3] |= 0x01 << 4;
		dev->controlData[12] = 0xff;
	}
	if (keyboardState[keybinds.grab]) {
		dev->controlData[3] |= 0x01 << 5;
		dev->controlData[13] = 0xff;
	}
	if (keyboardState[keybinds.ollie]) {
		dev->controlData[3] |= 0x01 << 6;
		dev->controlData[14] = 0xff;
	}
	if (keyboardState[keybinds.kick]) {
		dev->controlData[3] |= 0x01 << 7;
		dev->controlData[15] = 0xff;
	}


	//Switch/Revert +  Right Spin
	if (keyboardState[keybinds.rightSpin]) {
		/* revert */
		dev->controlData[3] |= 0x01 << 1;
		dev->controlData[19] = 0xff;
		/* right spin */
		dev->controlData[3] |= 0x01 << 3;
		dev->controlData[17] = 0xff;
	}
	// Nollie + Left Spin
	if (keyboardState[keybinds.leftSpin]) {
		dev->controlData[3] |= 0x01 << 2;
		dev->controlData[16] = 0xff;
		dev->controlData[3] |= 0x01 << 0;
		dev->controlData[18] = 0xff;
	}

	// Caveman
	if (keyboardState[keybinds.caveman]) {
		dev->controlData[20] |= 0x01 << 0;
	}
			
	if (keyboardState[keybinds.caveman2]) {
		dev->controlData[20] |= 0x01 << 1;
	}

	// create-a-park item control
	if (keyboardState[keybinds.item_up]) {
		dev->controlData[2] |= 0x01 << 4;
		dev->controlData[10] = 0xFF;
	}
	if (keyboardState[keybinds.item_right]) {
		dev->controlData[2] |= 0x01 << 5;
		dev->controlData[8] = 0xFF;
	}
	if (keyboardState[keybinds.item_down]) {
		dev->controlData[2] |= 0x01 << 6;
		dev->controlData[11] = 0xFF;
	}
	if (keyboardState[keybinds.item_left]) {
		dev->controlData[2] |= 0x01 << 7;
		dev->controlData[9] = 0xFF;
	}

	// sticks - NOTE: because these keys are very rarely used/important, SOCD handling is just to cancel
	// right
	// x
	if (keyboardState[keybinds.cameraLeft] && !keyboardState[keybinds.cameraRight]) {
		dev->controlData[4] = 0;
	}
	if (keyboardState[keybinds.cameraRight] && !keyboardState[keybinds.cameraLeft]) {
		dev->controlData[4] = 255;
	}

	// y
	if (keyboardState[keybinds.cameraUp] && !keyboardState[keybinds.cameraDown]) {
		dev->controlData[5] = 0;
	}
	if (keyboardState[keybinds.cameraDown] && !keyboardState[keybinds.cameraUp]) {
		dev->controlData[5] = 255;
	}

	// left
	// x
	if (keyboardState[keybinds.left] && !keyboardState[keybinds.right]) {
		dev->controlData[6] = 0;
	}
	if (keyboardState[keybinds.right] && !keyboardState[keybinds.left]) {
		dev->controlData[6] = 255;
	}

	// y
	if (keyboardState[keybinds.up] && !keyboardState[keybinds.down]) {
		dev->controlData[7] = 0;
	}
	if (keyboardState[keybinds.down] && !keyboardState[keybinds.up]) {
		dev->controlData[7] = 255;
	}
}

// returns 1 if a text entry prompt is on-screen so that keybinds don't interfere with text entry confirmation/cancellation
uint8_t isKeyboardTyping()
{
	uint8_t* keyboard_on_screen = (uint8_t*)0x007CE46E;
	//if (*keyboard_on_screen)
		//printf("Keyboard on screen!!!!!\n");
	return *keyboard_on_screen;
}

uint8_t menu_on_screen()
{
	uint8_t* menu_on_screen = (uint8_t*)0x007CE46F; //Every menu
	uint8_t* other_menu_on_screen = (uint8_t*)0x0069BAA8; //Only in level ESC menu (69BAA) (6EE004)

	return (*menu_on_screen | *other_menu_on_screen);

}

void do_key_input(SDL_KeyCode key) {

	//void (*key_input)(int32_t key, uint32_t param) = (void*)0x0062b1f0;
	typedef void (key_input)(int32_t key, uint32_t param);
	key_input* m_keyinput = (key_input*)0x005BDE70;

	uint8_t* keyboard_on_screen = (uint8_t*)0x007CE46E;
	//printf("Menu: %d\n", menu_on_screen());
	if (!isKeyboardTyping()) {
		return;
	}

	/* Language: 7 = GER, 9 = US */
	/* Update keyboard layout setting */
	HKL lang = ::GetKeyboardLayout(0);
	language = PRIMARYLANGID(lang);

	//printf("KEY: %d", key);

	int32_t key_out = 0;
	uint8_t modstate = SDL_GetModState();
	uint8_t shift = SDL_GetModState() & KMOD_SHIFT;
	uint8_t caps = SDL_GetModState() & KMOD_CAPS;
	uint8_t altgr = SDL_GetModState() & KMOD_RALT;

	if (key == SDLK_RETURN) {
		key_out = 0x0d;	// CR
	}
	else if (key == SDLK_BACKSPACE) {
		key_out = 0x08;	// BS
	}
	else if (key == SDLK_ESCAPE) {
		key_out = 0x1b;	// ESC
	}
	else if (key == SDLK_SPACE) {
		key_out = ' ';
	}
	else if (key >= SDLK_0 && key <= SDLK_9 && !(modstate & KMOD_SHIFT)) {
		key_out = key;
	}
	else if (key != SDLK_q && key >= SDLK_a && key <= SDLK_z) {
		key_out = key;
		if (modstate & (KMOD_SHIFT | KMOD_CAPS)) {
			key_out -= 0x20;
		}
	}
	else if (key == SDLK_MINUS) {
		if (modstate & KMOD_SHIFT) {
			key_out = '_';
		}
		else {
			key_out = '-';
		}
	}
	else if (key == SDLK_KP_0) {
		key_out = '0';
	}
	else if (key == SDLK_KP_1) {
		key_out = '1';
	}
	else if (key == SDLK_KP_2) {
		key_out = '2';
	}
	else if (key == SDLK_KP_3) {
		key_out = '3';
	}
	else if (key == SDLK_KP_4) {
		key_out = '4';
	}
	else if (key == SDLK_KP_5) {
		key_out = '5';
	}
	else if (key == SDLK_KP_6) {
		key_out = '6';
	}
	else if (key == SDLK_KP_7) {
		key_out = '7';
	}
	else if (key == SDLK_KP_8) {
		key_out = '8';
	}
	else if (key == SDLK_KP_9) {
		key_out = '9';
	}
	else if (key == SDLK_KP_MINUS) {
		key_out = '-';
	}
	else if (key == SDLK_KP_EQUALS) {
		key_out = '=';
	}
	else if (key == SDLK_KP_PLUS) {
		key_out = '+';
	}
	else if (key == SDLK_KP_DIVIDE) {
		key_out = '/';
	}
	else if (key == SDLK_KP_MULTIPLY) {
		key_out = '*';
	}
	else if (key == SDLK_KP_DECIMAL) {
		key_out = '.';
	}
	else if (key == SDLK_KP_ENTER) {
		key_out = 0x0d;
	}
	else if (language == 9) { /* US */
		if (key == SDLK_q) {
			key_out = key;
			if (modstate & (KMOD_SHIFT | KMOD_CAPS)) {
				key_out -= 0x20;
			}
		}
		else if (key == SDLK_PERIOD) {
			if (modstate & KMOD_SHIFT) {
				key_out = '>';
			}
			else {
				key_out = '.';
			}
		}
		else if (key == SDLK_COMMA) {
			if (modstate & KMOD_SHIFT) {
				key_out = '<';
			}
			else {
				key_out = ',';
			}
		}
		else if (key == SDLK_SLASH) {
			if (modstate & KMOD_SHIFT) {
				key_out = '?';
			}
			else {
				key_out = '/';
			}
		}
		else if (key == SDLK_SEMICOLON) {
			if (modstate & KMOD_SHIFT) {
				key_out = ':';
			}
			else {
				key_out = ';';
			}
		}
		else if (key == SDLK_QUOTE) {
			if (modstate & KMOD_SHIFT) {
				key_out = '\"';
			}
			else {
				key_out = '\'';
			}
		}
		else if (key == SDLK_LEFTBRACKET) {
			if (modstate & KMOD_SHIFT) {
				key_out = '{';
			}
			else {
				key_out = '[';
			}
		}
		else if (key == SDLK_RIGHTBRACKET) {
			if (modstate & KMOD_SHIFT) {
				key_out = '}';
			}
			else {
				key_out = ']';
			}
		}
		else if (key == SDLK_BACKSLASH) {
			if (modstate & KMOD_SHIFT) {
				key_out = '|';
			}
			else {
				key_out = '\\';
			}
		}
		else if (key == SDLK_EQUALS) {
			if (modstate & KMOD_SHIFT) {
				key_out = '+';
			}
			else {
				key_out = '=';
			}
		}
		else if (key == SDLK_BACKQUOTE) {
			if (modstate & KMOD_SHIFT) {
				key_out = '~';
			}
			else {
				key_out = '`';
			}
		}
		else if (key == SDLK_1 && modstate & KMOD_SHIFT) {
			key_out = '!';
		}
		else if (key == SDLK_2 && modstate & KMOD_SHIFT) {
			key_out = '@';
		}
		else if (key == SDLK_3 && modstate & KMOD_SHIFT) {
			key_out = '#';
		}
		else if (key == SDLK_4 && modstate & KMOD_SHIFT) {
			key_out = '$';
		}
		else if (key == SDLK_5 && modstate & KMOD_SHIFT) {
			key_out = '%';
		}
		else if (key == SDLK_6 && modstate & KMOD_SHIFT) {
			key_out = '^';
		}
		else if (key == SDLK_7 && modstate & KMOD_SHIFT) {
			key_out = '&';
		}
		else if (key == SDLK_8 && modstate & KMOD_SHIFT) {
			key_out = '*';
		}
		else if (key == SDLK_9 && modstate & KMOD_SHIFT) {
			key_out = '(';
		}
		else if (key == SDLK_0 && modstate & KMOD_SHIFT) {
			key_out = ')';
		}
		else {
			key_out = -1;
		}
	}
	else if (language == 7) { /* GER */
		if (key == SDLK_q && modstate & KMOD_CTRL) {
			key_out = '@';
		}
		else if (key == SDLK_q) {
			key_out = key;
			if (modstate & (KMOD_SHIFT | KMOD_CAPS)) {
				key_out -= 0x20;
			}		
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_LEFTBRACKET)) {
			if (modstate & KMOD_SHIFT) {
				key_out = 'Ü';
			}
			else {
				key_out = 'ü';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_SEMICOLON)) {
			if (modstate & KMOD_SHIFT) {
				key_out = 'Ö';
			}
			else {
				key_out = 'ö';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_APOSTROPHE)) {
			if (modstate & KMOD_SHIFT) {
				key_out = 'Ä';
			}
			else {
				key_out = 'ä';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_NONUSHASH)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '\'';
			}
			else {
				key_out = '#';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_RIGHTBRACKET)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '*';
			}
			else {
				key_out = '+';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_RIGHTBRACKET)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '*';
			}
			else {
				key_out = '+';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_PERIOD)) {
			if (modstate & KMOD_SHIFT) {
				key_out = ':';
			}
			else {
				key_out = '.';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_COMMA)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '\;'; /* not supported, will print ':' instead */
			}
			else {
				key_out = ',';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_NONUSBACKSLASH)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '>';
			}
			else {
				key_out = '<';
			}
		}
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_MINUS)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '?';
			}
			else if (modstate & KMOD_CTRL) {
				key_out = '\\';
			}
			else {
				key_out = 'ß';
			}
		}
		else if (key == SDLK_1 && modstate & KMOD_SHIFT) {
			key_out = '!';
		}
		/*
		else if (key == SDLK_2 && modstate & KMOD_SHIFT) { // not supported
			key_out = '\"';
		}
		else if (key == SDLK_3 && modstate & KMOD_SHIFT) {
			key_out = '§';
		}
		*/
		else if (key == SDLK_4 && modstate & KMOD_SHIFT) {
			key_out = '$';
		}
		/*
		else if (key == SDLK_5 && modstate & KMOD_SHIFT) { // not supported
			key_out = '%';
		}
		*/
		else if (key == SDLK_6 && modstate & KMOD_SHIFT) {
			key_out = '&';
		}
		else if (key == SDLK_7 && modstate & KMOD_SHIFT) {
			key_out = '/';
		}
		else if (key == SDLK_8 && modstate & KMOD_SHIFT) {
			key_out = '(';
		}
		else if (key == SDLK_9 && modstate & KMOD_SHIFT) {
			key_out = ')';
		}
		else if (key == SDLK_0 && modstate & KMOD_SHIFT) {
			key_out = '=';
		}
		/*
		else if (key == SDL_GetKeyFromScancode(SDL_SCANCODE_GRAVE)) {
			if (modstate & KMOD_SHIFT) {
				key_out = '`';
			}
			else {
				key_out = '´';
			}
		}
		*/ // accents need more work
	}
	m_keyinput(key_out, 0);
}

void processEvent(SDL_Event* e) {
	switch (e->type) {
	case SDL_CONTROLLERDEVICEADDED:
		if (SDL_IsGameController(e->cdevice.which)) {
			Log::TypedLog(CHN_SDL, "Adding controller: %d\n", e->cdevice.which);
			addController(e->cdevice.which);
		}
		else {
			Log::TypedLog(CHN_SDL, "Not a game controller: %s\n", SDL_JoystickNameForIndex(e->cdevice.which));
		}
		return;
	case SDL_CONTROLLERDEVICEREMOVED: {
		SDL_GameController* controller = SDL_GameControllerFromInstanceID(e->cdevice.which);
		if (controller) {
			Log::TypedLog(CHN_SDL, "Removed controller\n");
			removeController(controller);
		}
		return;
	}
	case SDL_JOYDEVICEADDED:
		Log::TypedLog(CHN_SDL, "Joystick added: %s\n", SDL_JoystickNameForIndex(e->jdevice.which));
		return;
	case SDL_KEYDOWN:
		//printf("KEY: %s\n", SDL_GetKeyName(e->key.keysym.sym));
		setUsingKeyboard(1);
		do_key_input((SDL_KeyCode)e->key.keysym.sym);
		return;
	case SDL_CONTROLLERBUTTONDOWN: {
		//printf("BUTTON: %s\n", SDL_GetKeyName(e->key.keysym.sym));
		SDL_GameController* controller = SDL_GameControllerFromInstanceID(e->cdevice.which);

		int idx = SDL_GameControllerGetPlayerIndex(controller);
		if (idx == -1) {
			addplayer(controller);
		}
		else if (players[idx].lockedOut) {
			players[idx].lockedOut++;
		}

		setUsingKeyboard(0);
		return;
	}
	case SDL_CONTROLLERBUTTONUP: {
		SDL_GameController* controller = SDL_GameControllerFromInstanceID(e->cdevice.which);

		int idx = SDL_GameControllerGetPlayerIndex(controller);
		if (idx != -1 && players[idx].lockedOut) {
			players[idx].lockedOut--;
		}

		return;
	}
	case SDL_CONTROLLERAXISMOTION:
		setUsingKeyboard(0);
		return;
	case SDL_QUIT: {
		int* shouldQuit = (int*)0x007D6A2C;
		*shouldQuit = 1;
		return;
	}
	case SDL_WINDOWEVENT: {
		//Checking windowed flag set by dx9 d3d present params
		//Without this, game freezes while alt-tabbing
		if (!(*(int*)0x00786A9C)) {
			if (e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
				*(int*)ADDR_IsFocused = 1;
				*(int*)ADDR_OtherIsFocused = 1;
				return;
			}
			else if (e->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
				*(int*)ADDR_IsFocused = 0;
				*(int*)ADDR_OtherIsFocused = 0;
				return;
			}
		}
	}
	default:
		return;
	}
}

void __cdecl processController(device* dev) {
	// cheating:
	// replace type with index
	//dev->type = 70;

	//printf("Processing Controller %d %d %d!\n", dev->index, dev->slot, dev->port);
	//printf("TYPE: %d\n", dev->type);
	//printf("ISPLUGGEDIN: %d\n", dev->isPluggedIn);
	dev->capabilities = 0x0003;
	dev->num_actuators = 2;
	dev->vibrationData_max[0] = 255;
	dev->vibrationData_max[1] = 255;
	dev->state = 2;
	dev->actuatorsDisabled = 0;

	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		processEvent(&e);
		//printf("EVENT!!!\n");
	}

	dev->isValid = 0;
	dev->isPluggedIn = 0;

	dev->controlData[0] = 0;
	dev->controlData[1] = 0x70;

	// buttons bitmap
	// this bitmap may not work how you would expect. each bit is 1 if the button is *up*, not down
	// the original code sets this initial value at 0xff and XOR's each button bit with the map
	// this scheme cannot be continuously composited, so instead we OR each button with the bitmap and bitwise negate it after all controllers are processed
	dev->controlData[2] = 0x00;
	dev->controlData[3] = 0x00;

	// buttons
	dev->controlData[12] = 0;
	dev->controlData[13] = 0;
	dev->controlData[14] = 0;
	dev->controlData[15] = 0;

	// shoulders
	dev->controlData[16] = 0;
	dev->controlData[17] = 0;
	dev->controlData[18] = 0;
	dev->controlData[19] = 0;

	// d-pad
	dev->controlData[8] = 0;
	dev->controlData[9] = 0;
	dev->controlData[10] = 0;
	dev->controlData[11] = 0;

	// sticks
	dev->controlData[4] = 127;
	dev->controlData[5] = 127;
	dev->controlData[6] = 127;
	dev->controlData[7] = 127;

	dev->controlData[20] = 0;

	if (dev->port == 0) {
		dev->isValid = 1;
		dev->isPluggedIn = 1;

		if (!isKeyboardTyping()) {
			pollKeyboard(dev);
		}
	}

	if (dev->port < MAX_PLAYERS) {
		if (players[dev->port].controller && !players[dev->port].lockedOut) {
			pollController(dev, players[dev->port].controller);
		}
	}

	dev->controlData[2] = ~dev->controlData[2];
	dev->controlData[3] = ~dev->controlData[3];

	if (0xFFFF ^ ((dev->controlData[2] << 8) | dev->controlData[3])) {
		dev->pressed = 1;
	}
	else {
		dev->pressed = 0;
	}

	if (~dev->controlData[2] & 0x01 << 3 || ~dev->controlData[3] & 0x01 << 6) {
		dev->start_or_a_pressed = 1;
	}
	else {
		dev->start_or_a_pressed = 0;
	}

	// keyboard text entry doesn't work unless these values are set
	//uint8_t* unk1 = (uint8_t*)0x0074fb42;
	//uint8_t* unk2 = (uint8_t*)0x00751dc0;
	//uint8_t* unk3 = (uint8_t*)0x0074fb43;

	//*unk2 = 1;
	//*unk3 = 0;

	//printf("UNKNOWN VALUES: 0x0074fb42: %d, 0x00751dc0: %d, 0x0074fb43: %d\n", *unk1, *unk2, *unk3);
}

void __cdecl set_actuators(int port, uint16_t high, uint16_t low) {
	//printf("SETTING ACTUATORS: %d %d %d\n", port, left, right);
	for (int i = 0; i < controllerCount; i++) {
		if (SDL_GameControllerGetAttached(controllerList[i]) && SDL_GameControllerGetPlayerIndex(controllerList[i]) == port) {
			SDL_JoystickRumble(SDL_GameControllerGetJoystick(controllerList[i]), low, high, 0);
		}
	}
}

uint32_t convert_SDL_to_OIS_keycode(uint8_t sdlKeyCode) {
	// Lookup the SDL keycode in the map
	auto it = SDL_to_OIS_map.find(sdlKeyCode);
	if (it != SDL_to_OIS_map.end()) {
		// Return the corresponding OIS keycode
		return it->second;
	}
	else {
		// If no mapping found, return an unspecified keycode
		return (uint32_t)OIS_KC_UNASSIGNED;
	}
}

void __stdcall initManager() {
	//printf("Initializing Manager!\n");
	//printf("Locale: %s\n", locale->language);
	//printf("Language: %d\n", language);


	GetModuleFileName(NULL, (LPSTR)&executableDirectory3, MAX_PATH);

	// find last slash
	char* exe = strrchr((LPSTR)executableDirectory3, '\\');
	if (exe) {
		*(exe + 1) = '\0';
	}

	// init sdl here
	SDL_Init(SDL_INIT_GAMECONTROLLER);

	//SDL_SetHint(SDL_HINT_HIDAPI_IGNORE_DEVICES, "0x1ccf/0x0000");
	SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "false");

	char controllerDbPath[MAX_PATH];
	int result = sprintf_s(controllerDbPath, MAX_PATH, "%s%s", executableDirectory3, "gamecontrollerdb.txt");

	if (result) {
		result = SDL_GameControllerAddMappingsFromFile(controllerDbPath);
		if (result) {
			Log::TypedLog(CHN_SDL, "Loaded mappings\n");
		}
		else {
			Log::TypedLog(CHN_SDL, "Failed to load %s\n", controllerDbPath);
		}

	}
	loadInputSettings(&inputsettings);
	loadKeyBinds(&keybinds);
	loadControllerBinds(&padbinds);

	initSDLControllers();

	if (inputsettings.isPs2Controls) {
		//printf("PS2 Controls enabled\n");
		patchPs2Buttons();
	}
	//else
		//printf("PS2 Controls disabled\n");
	
	// Add missing key info: Since we don't use the launcher, no registry values for our keybinds are set.
	// The game normally loads keybinds found in the registry and stores them at these addresses (starting at 0x007D6794).
	// This simulates the launcher storing its defined keybinds in memory so that they can be displayed in game (e.g., Edit Tricks menu "<- + KP4" or Tantrum Meter "Press KP8 to freak out").
	patchDWord((void*)0x007D6790, convert_SDL_to_OIS_keycode(keybinds.ollie));
	patchDWord((void*)0x007D6794, convert_SDL_to_OIS_keycode(keybinds.grab));
	patchDWord((void*)0x007D6798, convert_SDL_to_OIS_keycode(keybinds.kick));
	patchDWord((void*)0x007D679C, convert_SDL_to_OIS_keycode(keybinds.grind));
	patchDWord((void*)0x007D67B0, convert_SDL_to_OIS_keycode(keybinds.leftSpin));
	patchDWord((void*)0x007D67B4, convert_SDL_to_OIS_keycode(keybinds.rightSpin));
	patchDWord((void*)0x007D67A8, convert_SDL_to_OIS_keycode(keybinds.caveman));
	patchDWord((void*)0x007D67AC, convert_SDL_to_OIS_keycode(keybinds.caveman2));
	patchDWord((void*)0x007D67B8, convert_SDL_to_OIS_keycode(keybinds.focus));

	mWindow = getWindowHandle();
}

void patchPs2Buttons() {
	Log::TypedLog(CHN_SDL, "Patching PS2 Buttons\n");
	patchByte((void*)(0x0051F4C6 + 2), 0x05);	// change PC platform to gamecube.  this just makes it default to ps2 controls. needed for rail DD on R2

	// walk acid drop.
	// Originally, only on R2 on PS2. It will be patched to L2 | R2 here
	patchBytesM((void*)0x00527546, (BYTE*)"\x0F\x85\x0E\x00\x00\x00", 6);

	//in_air_acid_drop
	patchBytesM((void*)0x0050DA64, (BYTE*)"\x75\x0A", 2);

	//in_air_to_break
	patchBytesM((void*)0x0050CF89, (BYTE*)"\x75\x14", 2);

	//break_vert
	patchBytesM((void*)0x00507184, (BYTE*)"\x75\x20", 2);
	patchBytesM((void*)0x005071AE, (BYTE*)"\x0F\x85\xDC\x01\x00\x00", 6);
	patchBytesM((void*)0x005071B4, (BYTE*)"\x8A\x87\x00\x01\x00\x00", 6);
	patchBytesM((void*)0x005071BA, (BYTE*)"\x84\xC0", 2);
	patchBytesM((void*)0x005071BC, (BYTE*)"\x0F\x85\xCE\x01\x00\x00", 6);
	patchNop((void*)0x005071C2, 10);

	//air_recovery
	patchBytesM((void*)0x0050CAC4, (BYTE*)"\x75\x06", 2);

	//??? something else tpro and clownjob patch
	patchBytesM((void*)0x00527636, (BYTE*)"\x0F\x85\x0E\x00\x00\x00", 6);

	//No Spinlag
	patchNop((void*)ADDR_SpinLagL, 2);
	patchNop((void*)ADDR_SpinLagR, 2);
}

void patchInput() {
	// patch SIO::Device
	// process
	patchThisToCdecl((void*)0x005BDCC0, &processController); //005BDB60 //005BDBD0 //0x005BDCC0 <- looks like thaw
	patchBytesM((void*)(0x005BDCC0 + 7), (BYTE*)"\xC2\x04\x00", 3); //ret 4
	//patchByte((void*)(0x005BDBD0 + 7), 0xC3);

	// set_actuator
	// don't call read_data in activate_actuators
	patchNop((void*)0x005BDA31, 7);
	patchCall((void*)0x005BDAB6, set_actuators);
	patchCall((void*)0x005BDB17, set_actuators);
	patchCall((void*)0x005BDBBF, set_actuators);
	patchCall((void*)0x005BDC51, set_actuators);
	patchCall((void*)0x005BDCAF, set_actuators);

	// init input patch - nop direct input setup
	patchNop((void*)0x005F459D, 45);
	patchCall((void*)(0x005F459D + 5), &initManager);

	// some config call relating to the dinput devices
	patchNop((void*)0x004E2C16, 5);

}


void CheckChatHotkey() {

	// We know the key is pressed. We need to make sure that
	// we're not in a menu, dialog box, etc.

	if (TextInputInNetGame) {
		// enter_kb_chat
		RunScript(0x3B4548B8, nullptr, nullptr, nullptr);
	}
}
