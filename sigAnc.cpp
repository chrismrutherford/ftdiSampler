

#include <stdio.h>
#include <ftd2xx.h>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

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
    bool loops(int delay);
private:
    int mK;
    uint64_t mLastTime;
    uint64_t mRefTime;
    FT_STATUS mFtStatus;
    FT_HANDLE mFtHandle;
    int mPortNumber;
	DWORD  mBaudRate;
	UCHAR  mPinStatus;
    int mHold;
    uint64_t mSampleNo;
};


Sampler::Sampler():mFtStatus(FT_OK),mPortNumber(0),mBaudRate(921600),mHold(1),mSampleNo(0)
{
    mLastTime = getTimeStamp()/100000*100000;
    mRefTime = mLastTime;
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
    if(sleepTime < 0){sleepTime = 0;}

    /*
    */
    if(mSampleNo%100000==0)
    {
        cout <<"sleep " << sleepTime << endl;
        cout <<"lagg " << laggTime << endl;
        cout <<"exp "  << expectedTime << endl;
        cout <<"act "  << actualTime << endl;
        cout <<"reading "  << (int)mPinStatus << endl;
    }
    
    //usleep(1);
    loops(sleepTime);
    return 0;
}

bool Sampler::loops(int delay)
{

    for(int i=0;i<delay;i++)
    {
        for(int j=0;j<delay;j++)
        {
            for(int k=0;k<delay;k++)
            {
                for(int l=0;l<delay;l++)
                {
                    mK=i+j+k+l;
                }
            }
        }
    }
    return 0;
}

bool Sampler::read() 
{
	mFtStatus = FT_GetBitMode(mFtHandle, &mPinStatus);
    mPinStatus = (mPinStatus & 16)/16;
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
        //cout <<"read " << (int)mPinStatus << endl;
        cout << (int)mPinStatus;
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
    struct timeval tv;
    gettimeofday(&tv,NULL);
    uint64_t now =  tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
    uint64_t elapsed = now-mLastTime;
    mLastTime = now;
    return elapsed;
    

}

int main () {
    Sampler sampler;
    sampler.init();
    sampler.sample();
return 0;
}
