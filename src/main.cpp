#include <3ds.h>
#include <iostream>

#include "link.h"

int main(int argc, char** argv) {
	gfxInitDefault();
	hidInit();
	consoleInit(GFX_TOP, nullptr);
	std::cout << "Press (A) to start dumping" << std::endl;
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		if (hidKeysDown() & KEY_A)
			FMS::startTransfer();
		
		// Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        //Wait for VBlank
        gspWaitForVBlank();
	}
	gfxExit();
	hidExit();
}
