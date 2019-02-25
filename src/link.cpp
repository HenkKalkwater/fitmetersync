#include "link.h"

void FMS::startTransfer() {
    std::cout << ":: Start listening" << std::endl;
    
    const size_t SIZE = 0x1000;
    u8* data;
    data = (u8*) memalign(SIZE, SIZE);
    
	Result ret = iruInit((u32*) data, SIZE);
    printIfError(ret);

    std::cout << "Press (B) to stop receiving data." << std::endl;
    // The frequency of the Pokewalker is about 116 279,07 and it looks a lot like a Wii Fit U meter.
    // Let's try that frequency, because you have to do something.
    // The closest 
    IRU_SetBitRate(3);
    
    u32 receivedSize;
    while (1) {
        printIfError(IRU_StartRecvTransfer(SIZE, 0));
		Handle waitEvent;
		
		printIfError(getRecvFinishedEvent(&waitEvent));
		printIfError(svcWaitSynchronization(waitEvent, U64_MAX));
        printIfError(svcClearEvent(waitEvent));
		
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
        std::cout << "Received " << receivedSize << " bytes!" << std::endl;
        
        //u32 contains 4 bytes, so we have to divide by 4
        for (u32 i = 0; i < receivedSize; i++){
            // An u32 consists of 4 bytes.
            //for (int j = 24; j >= 0; j -= 8) {
                // Split up the 4-byte number to 1 byte hexadecimal numbers.
                //std::cout << std::hex << std::setw(2) << std::setfill('0') << ((data[i] >> j) & 0xFF) << " ";
            //}
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF) << " ";
            if ((i + 1) % 8 == 0)
                std::cout << std::endl;
            // << " " << ((data[i] >> 16) & 0xFF) << " " << ((data[i] >> 18) & 0xFF) << " " << (data[i] & 0xFF) << " " << std::endl
        }
        std::cout << "Sending data back" << std::endl;
        printIfError(IRU_StartSendTransfer(data, receivedSize));
        printIfError(IRU_WaitSendTransfer());
        std::cout << "Done sending data back" << std::endl;
        std::cout << std::dec << std::endl;
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
	
	*handle = (Handle) cmdbuf[3];
	
	return ret;
}
