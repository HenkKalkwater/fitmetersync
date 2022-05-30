#include <3ds.h>
#include <iostream>

#include "link.h"
#include "reserver.h"

using namespace fms;

int main(int argc, char** argv) {
	gfxInitDefault();
	hidInit();
	consoleInit(GFX_TOP, nullptr);
    
    bool reServerStarted = false;
    bool socketsOpen = false;
    REServer s;
    const u32 BUF_ALIGN = 0x1000;
    const u32 BUF_SIZE = 0x100000;
    u32* sock_buf = static_cast<u32*>(memalign(BUF_ALIGN, BUF_SIZE));
    
    
	Link::printIfError(Link::initialise());
	std::string text = "(A) to start as main, (B) as slave, (X) to reflect, (Y) to record, (R) for server, (START) to quit.";
	std::cout << text << std::endl;
    
	
	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) {
            if (reServerStarted) {
                std::cout << "REServer quiting" << std::endl;
                s.close();
                std::cout << "REServer cleaning up" << std::endl;
                socExit();
                free(sock_buf);
                reServerStarted = false;
            }
			break;
        }
		if (hidKeysDown() & KEY_A) {
			Link::startTransfer2();
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
		if (hidKeysDown() & KEY_R) {
            if (!reServerStarted) {
                std::cout << "Starting REServer" << std::endl;
                if (sock_buf != nullptr) {
                    if (!socketsOpen) {
                        Result socInitRes = socInit(sock_buf, BUF_SIZE);
                        if (R_FAILED(socInitRes)) {
                            std::cout << "Error trying to init sockets: " << std::hex << socInitRes << std::dec << std::endl;
                            continue;
                        }
                        socketsOpen = true;
                    }
                    
                    reServerStarted = s.open();
                    std::cout << "REServer running" << std::endl;
                }
            } else {
                std::cout << "REServer quiting" << std::endl;
                s.close();
                std::cout << "REServer cleaning up" << std::endl;
                socExit();
                free(sock_buf);
                reServerStarted = false;
            }
            std::cout << text << std::endl;
        }
		//copied from keyboard example
		if (hidKeysDown() & KEY_SELECT) {
			Link::startTransfer(0, true);
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
