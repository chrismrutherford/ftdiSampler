
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

#define SYNCSZ 200*SCALE

using namespace std;

class Bits
{
public:
    Bits(bool val, int count)
    {
        v=val;
        c=count;
    }
    bool v;
    int  c;
};


class Sampler
{
public:
    Sampler();
    uint64_t getTimeStamp();
    uint64_t elapsed();
    int init();
    bool sample();
    int read();
    bool delay();
    bool loops(int delay);
    bool syncDetect(int reading);
    bool  procBuffer();
    bool procLine(string line);
    bool procBits();

private:
    int mK;
    uint64_t mRefTime;
    uint64_t mElapsedTime;
    FT_STATUS mFtStatus;
    FT_HANDLE mFtHandle;
    int mPortNumber;
    DWORD  mBaudRate;
    UCHAR  mPinStatus;
    int mHold;
    uint64_t mSampleNo;
    list<int> mSyncBuffer;
    list<string> mSampleBuffer;
    list<Bits> mBitBuffer;
    uint8_t mBuffer[BUFSZ];
    FILE *pFile;
};
