#include "link.h"

void FMS::startTransfer() {
    std::cout << ":: Start listening" << std::endl;
    
    const size_t SIZE = 0x1000;
    u32* data;
    data = (u32*) memalign(SIZE, SIZE);
    
	Result ret = iruInit(data, SIZE);
    printIfError(ret);

    std::cout << "Press (B) to stop receiving data." << std::endl;
    std::cout << "IR:U Inited" << std::endl;
    // The frequency of the Pokewalker is about 116 279,07 and it looks a lot like a Wii Fit U meter.
    // Let's try that frequency, because you have to do something.
    // The closest 
    IRU_SetBitRate(3);
    
    u32 receivedSize;

    u64 slept = 0;
    u64 realSlept = 0;
    while (1) {
        printIfError(IRU_StartRecvTransfer(SIZE, 0));
		Handle* waitEvent;
		
		printIfError(getRecvFinishedEvent(waitEvent));
		std::cout << "Sleeping" << std::endl;
		printIfError(svcWaitSynchronization(*waitEvent, U64_MAX));
		std::cout << "Done sleeping" << std::endl;
		
        // Where's my sleep?
        //for (int i = 0; i < 9999; i++)
            //std::cout << i << "\r" << std::flush;
        // Sleep for a little.
        
        hidScanInput();
        if (hidKeysDown() & KEY_B)
            break;
        printIfError(IRU_WaitRecvTransfer(&receivedSize));
        
        if (receivedSize == 0)  {
            continue;
        }
        //u32 contains 4 bytes, so we have to divide by 4
        std::cout << "Received " << receivedSize / 4 << " bytes!" << std::endl;
        
        for (u32 i = 0; i < receivedSize / 4; i++){
            // An u32 consists of 4 bytes.
            for (int j = 24; j >= 0; j -= 8) {
                // Split up the 4-byte number to 1 byte hexadecimal numbers.
                std::cout << std::hex << std::setw(2) << std::setfill('0') << ((data[i] >> j) & 0xFF) << " ";
            }
            std::cout << std::endl;
            // << " " << ((data[i] >> 16) & 0xFF) << " " << ((data[i] >> 18) & 0xFF) << " " << (data[i] & 0xFF) << " " << std::endl
        }
        std::cout << std::dec << std::endl;
        break;
    }
    std::cout << ":: Stopped listening" << std::endl;
    iruExit();
}

void FMS::printIfError(Result ret) {
    if (ret != 0) {
        std::cout << "Error " << ret << ": " << osStrError(ret) << std::endl;
        u32 level = R_LEVEL(ret);
        std::cout << "Lvl: " << level << "; Smry: " << R_SUMMARY(ret) << "; Mdl: " << R_MODULE(ret) << "; Desc: " << R_DESCRIPTION(ret) << std::endl;
    }
}

Result FMS::getRecvFinishedEvent(Handle* handle) {
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	
	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000
	
	if(R_FAILED(ret = svcSendSyncRequest(iruGetServHandle())))return ret;
	
	ret = (Result)cmdbuf[1];
	
	handle = (Handle*) cmdbuf[3];
	
	return ret;
}
