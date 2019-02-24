#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <iostream>
#include <3ds.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <iomanip>

namespace FMS {
    void startTransfer();
    void printIfError(Result result);
}

#endif //LINK_H
