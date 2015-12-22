

#include <stdio.h>
#include <ftd2xx.h>

#include <iostream>
#include <sstream>

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include <list>

#include "CircBuff.h"

#define BUFSZ 16000

#define SYNCSZ 500

using namespace std;

class Bits
{
public:
    Bits(bool val, int count) {v=val;c=count; }
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
    void  procBuffer();
    string procLine(string line);

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
};


Sampler::Sampler():mFtStatus(FT_OK),mPortNumber(0),mBaudRate(9600),mHold(1),mSampleNo(0)
{
    mElapsedTime = getTimeStamp();
    mRefTime = mElapsedTime / 100000 * 100000;

}

bool Sampler::syncDetect(int reading)
{
    bool retVal = false;
    bool zeros = true;

    list<int>::iterator i;

    for(i=mSyncBuffer.begin(); i != mSyncBuffer.end(); ++i)
    {
        int data = (*i);
        //cout << data << endl;
        if(data != 0)
        {
            zeros = false;
        }
        
    }

    if(reading == 1 && zeros == true)
    {
        retVal = true;
        
    }

    if(mSyncBuffer.size() > SYNCSZ)
    {
        mSyncBuffer.pop_front();
    }

    mSyncBuffer.push_back(reading);
    //cout << "size " << mSyncBuffer.size() << endl;


    return retVal;
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

	printf("Selecting asynchronous bit-bang mode.\n");	
	mFtStatus = FT_SetBitMode(mFtHandle, 
	                         0x00, /* sets all 8 pins as inputs */
	                         FT_BITMODE_ASYNC_BITBANG);
	if (mFtStatus != FT_OK) 
	{
		printf("FT_SetBitMode failed (error %d).\n", (int)mFtStatus);
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

int Sampler::read()
{

    DWORD readQueue;
    mFtStatus = FT_GetQueueStatus(mFtHandle, &readQueue);
    DWORD readBytes;

    mFtStatus =  FT_Read (mFtHandle, mBuffer, BUFSZ, &readBytes);
    mSampleNo+=readBytes;

    return readBytes;
}


bool Sampler::sample()
{
    while(1)
    {
        procBuffer();
    }
    return 0;
}

void Sampler::procBuffer()
{
    static stringstream line;
    static int bits = 0;
    int readBytes = read();
    for(int i=0; i<readBytes; i++)
    {
        uint8_t data = (mBuffer[i] & 16) / 16;
        //cout << "" << (int)data ;
        bits++;
        if(syncDetect(data) == true)
        {
            string str = line.str();
            //if(str.size() > SYNCSZ)str.resize (str.size () - SYNCSZ);
            //cout << "last " <<str.find_last_of("1");
            int pos = str.find_last_of("1");
            if(pos > 0)
            {
                str.resize (pos+1);
            }
            procLine(str);
            line.str( std::string() );
            line.clear();
            bits=0;
        }

        line << (int)data;
    }
}

string Sampler::procLine(string line)
{
    string subSample;
    bool lastVal = 0;
    bool val     = false;
    int charCount = 0;
    bool first = false;
    //cout << line << " " << line.length() << endl;
    char lastChar = 'U';
    for(uint32_t i=0; i<line.length(); i++)
    {
        charCount++;
        char subChar = line.at(i);

        if(subChar == '0') { val=false; }
        else { val=true; }

        if(first == true) { first = false; lastVal = val; }

        if(val != lastVal )
        {
            cout << lastVal << ":" << charCount << ", ";
            mBitBuffer.push_back(Bits(val, charCount));
            charCount=0;
            lastChar = subChar;
            lastVal = val;
        }
    }
    cout << endl;
    return subSample;
}

uint64_t Sampler::getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

uint64_t Sampler::elapsed()
{
    uint64_t now = getTimeStamp();
    uint64_t elapsed = now - mElapsedTime;
    mElapsedTime = now;
    return elapsed;
}


int main ()
{


    Sampler sampler;
    sampler.init();
    sampler.sample();
    return 0;
}
