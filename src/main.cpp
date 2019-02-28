#include <3ds.h>
#include <iostream>
#include "link.h"

int main(int argc, char** argv) {
	gfxInitDefault();
	hidInit();
	consoleInit(GFX_TOP, nullptr);
	std::string text = "(A) to start as main, (B) as slave, (X) to reflect, (Y) to record, (START) to quit.";
	std::cout << text << std::endl;

	
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		if (hidKeysDown() & KEY_A) {
			FMS::startTransfer(0, false);
            std::cout << text << std::endl;
        }
		if (hidKeysDown() & KEY_B) {
			FMS::startTransfer(1, false);
			std::cout << text << std::endl;
		}
		if (hidKeysDown() & KEY_X) {
			FMS::startTransfer(2, false);
			std::cout << text << std::endl;
		}
		if (hidKeysDown() & KEY_Y) {
			FMS::startTransfer(3, false);
			std::cout << text << std::endl;
		}
		//copied from keyboard example
		if (hidKeysDown() & KEY_SELECT)
		{
			FMS::startTransfer(0, true);
			std::cout << text << std::endl;
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
