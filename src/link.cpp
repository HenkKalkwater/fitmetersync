#include "link.h"

void FMS::startTransfer() {
    const size_t SIZE = 0x1000;
    u32* data;
    data = (u32*) memalign(SIZE, SIZE);
    
	Result ret = iruInit(data, SIZE);
    printIfError(ret);
    std::cout << "IR:U Inited" << std::endl;
    // The frequency of the Pokewalker is about 116 279,07 and it looks a lot like a Wii Fit U meter.
    // Let's try that frequency, because you have to do something.
    // The closest 
    IRU_SetBitRate(3);
    printIfError(IRU_StartRecvTransfer(SIZE, 0));
    
    u32 received;
    // Where's my sleep?
    for (int i = 0; i < 9999; i++)
        std::cout << i << "\r" << std::flush;
    printIfError(IRU_WaitRecvTransfer(&received));
    std::cout << "Received " << received * 4 << " bytes!" << std::endl;
    
    for (u32 i = 0; i < received; i++){
        // An u32 consists of 
        for (int j = 24; j >= 0; j -= 8)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << ((data[i] >> j) & 0xFF) << " ";
        std::cout << std::endl;
        // << " " << ((data[i] >> 16) & 0xFF) << " " << ((data[i] >> 18) & 0xFF) << " " << (data[i] & 0xFF) << " " << std::endl
    }
    std::cout << std::dec << std::endl;
    
    iruExit();
}

void FMS::printIfError(Result ret) {
    if (ret != 0) {
        std::cout << "Error " << ret << ": " << osStrError(ret) << std::endl;
        u32 level = R_LEVEL(ret);
        std::cout << "Lvl: " << level << "; Smry: " << R_SUMMARY(ret) << "; Mdl: " << R_MODULE(ret) << "; Desc: " << R_DESCRIPTION(ret) << std::endl;
    }
}
