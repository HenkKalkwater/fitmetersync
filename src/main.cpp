#include <3ds.h>
#include <iostream>
#include "link.h"

using namespace FMS;

int main(int argc, char** argv) {
	gfxInitDefault();
	hidInit();
	consoleInit(GFX_TOP, nullptr);
    Link::printIfError(Link::initialise());
	std::string text = "(A) to start as main, (B) as slave, (X) to reflect, (Y) to record, (START) to quit.";
	std::cout << text << std::endl;
    
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START)
			break;
		if (hidKeysDown() & KEY_A) {
			Link::startTransfer(0, false);
            std::cout << text << std::endl;
        }
		if (hidKeysDown() & KEY_B) {
			Link::startTransfer(1, false);
			std::cout << text << std::endl;
		}
		if (hidKeysDown() & KEY_X) {
			Link::startTransfer(2, false);
			std::cout << text << std::endl;
		}
		if (hidKeysDown() & KEY_Y) {
			Link::startTransfer(3, false);
			std::cout << text << std::endl;
		}
		//copied from keyboard example
		if (hidKeysDown() & KEY_SELECT)
		{
			Link::MITMattack();
			std::cout << text << std::endl;
		}


		// Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        //Wait for VBlank
        gspWaitForVBlank();
	}
	Link::exit();
	gfxExit();
	hidExit();
}
