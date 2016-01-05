#include <stdio.h>
#include <ftd2xx.h>

#include <iostream>
#include <sstream>

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include <list>

#include "CircBuff.h"

#define BUFSZ 4096
#define SCALE 4

using namespace std;

class Replay
{
public:
    Replay();
    int init();
    int replay();
private:
    FT_STATUS mFtStatus;
    FT_HANDLE mFtHandle;
    uint8_t mBuffer[BUFSZ];
    int mPortNumber;
    DWORD  mBaudRate;
    FILE *pFile;
};
