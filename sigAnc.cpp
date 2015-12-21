

#include <stdio.h>
#include <ftd2xx.h>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include "ringbuffer.h"

using namespace std;

class Sampler
{
public:
    Sampler();
    uint64_t getTimeStamp();
    uint64_t elapsed();
    int init();
    bool sample();
    bool read();
    bool delay();
private:
    uint64_t mLastTime;
    uint64_t mRefTime;
    FT_STATUS mFtStatus;
    FT_HANDLE mFtHandle;
    int mPortNumber;
    DWORD  mBaudRate;
    UCHAR  mPinStatus;
    int mHold;
    uint64_t mSampleNo;
    ringbuffer<string> mBuf;
};


Sampler::Sampler():mFtStatus(FT_OK),mPortNumber(0),mBaudRate(921600),mHold(500),mSampleNo(0),mBuf(1024)
{
    mRefTime = getTimeStamp()/1000000*1000000;
}

int Sampler::init()
{
    int retVal = 0;
    mFtStatus = FT_Open(mPortNumber, &mFtHandle);
    if (mFtStatus != FT_OK)
    {
        /* FT_Open can fail if the ftdi_sio module is already loaded. */
        printf("FT_Open(%d) failed (error %d).\n", mPortNumber, (int)mFtStatus);
        retVal = 1;
    }

    printf("Setting clock rate to %d\n", mBaudRate * 16);
    mFtStatus = FT_SetBaudRate(mFtHandle, mBaudRate);
    if (mFtStatus != FT_OK)
    {
        printf("FT_SetBaudRate failed (error %d).\n", (int)mFtStatus);
        retVal = 2;
    }

    return retVal;

}

bool Sampler::delay()
{
    uint64_t expectedTime = mSampleNo*mHold + mRefTime;
    uint64_t actualTime = getTimeStamp();
    int64_t laggTime = actualTime - expectedTime;
    int sleepTime = mHold - laggTime;
    if(sleepTime < 0) {
        sleepTime = 0;
    }

    /*
    */
    if(mSampleNo%10000==0)
    {
        cout <<"sleep " << sleepTime << endl;
        cout <<"lagg " << laggTime << endl;
        cout <<"exp "  << expectedTime << endl;
        cout <<"act "  << actualTime << endl;
        cout <<"reading "  << (int)mPinStatus << endl;
    }

    usleep(sleepTime);
    return 0;
}

bool Sampler::read()
{
    mFtStatus = FT_GetBitMode(mFtHandle, &mPinStatus);
    if (mFtStatus != FT_OK)
    {
        printf("FT_GetBitMode failed (error %d).\n", (int)mFtStatus);
    }
    mSampleNo++;

    return 0;
}

bool Sampler::sample()
{
    while(1)
    {
        read();
        cout <<"read " << (int)mPinStatus << endl;
        delay();
    }
    return 0;
}
uint64_t Sampler::getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

uint64_t Sampler::elapsed()
{
    uint64_t elapsed = getTimeStamp() - mLastTime;
    return elapsed;


}

int main () {


    Sampler sampler;
    sampler.init();
    sampler.sample();
    return 0;
}
