#include <3ds.h>
#include <iostream>

#include "link.h"

int main(int argc, char** argv) {
	gfxInitDefault();
	hidInit();
	consoleInit(GFX_TOP, nullptr);
	std::cout << "(A) to start as main, (B) as slave, (START) to quit." << std::endl;
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		if (hidKeysDown() & KEY_A) {
			FMS::startTransfer(true);
            std::cout << "(A) to start as main, (B) as slave, (START) to quit." << std::endl;
        }
		if (hidKeysDown() & KEY_B) {
			FMS::startTransfer(false);
			std::cout << "(A) to start as main, (B) as slave, (START) to quit." << std::endl;
		}
		// Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        //Wait for VBlank
        gspWaitForVBlank();
	}
	gfxExit();
	hidExit();
}
