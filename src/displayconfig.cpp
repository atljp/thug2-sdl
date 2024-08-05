#include <displayconfig.h>
#include <global.h>

// buffer needed for high resolutions
uint8_t resbuffer[100000];

void patchresbuffer() {
	patchDWord((void*)ADDR_Resbuffer, (uint32_t)&resbuffer);
}

