#include "sigAnc.h"

Sampler::Sampler():mFtStatus(FT_OK),mPortNumber(0),mBaudRate(9600*SCALE),mHold(1),mSampleNo(0)
{
    long lSize;
    mElapsedTime = getTimeStamp();
    mRefTime = mElapsedTime / 100000 * 100000;

    pFile = fopen("/tmp/sample.bin","wb");
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);


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

    fwrite(mBuffer,1,readBytes,pFile);

    return readBytes;
}


bool Sampler::sample()
{
    bool retVal = false;
    while(1)
    {
        retVal = procBuffer();
        if(retVal == true)
        {
            procBits();
        }
    }
    return 0;
}

bool Sampler::procBuffer()
{
    bool retVal = false;
    static int state = 0;
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
            retVal = procLine(str);
            line.str( std::string() );
            line.clear();
            bits=0;

            if(state == 0)
            {
                state = 1;
                mBitBuffer.clear();
            }
            else if(state == 1) 
            {
                retVal = true;
                state = 0;
            }
        }

        if(bits > 15000)
        {
            line.str( std::string() );
            line.clear();
            bits=0;

        }

        line << (int)data;
    }
    return retVal;
}

bool Sampler::procLine(string line)
{
    bool retVal = false;
    string subSample;
    bool lastVal = 0;
    bool val     = false;
    int charCount = 0;
    bool first = false;

    //cout << "line len " << line.length() << endl;

    for(uint32_t i=0; i<line.length(); i++)
    {
        charCount++;
        char subChar = line.at(i);

        if(subChar == '0')
        {
            val=false;
        }
        else
        {
            val=true;
        }

        if(first == true)
        {
            first = false;
            lastVal = !val;
        }

        if(val != lastVal )
        {
            mBitBuffer.push_back(Bits(lastVal, charCount));
            charCount=0;
            lastVal = val;
        }
    }
    return retVal;
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

bool Sampler::procBits()
{
    bool retVal = false;
    list<Bits>::iterator i;
    list<Bits> result;

    //mBitBuffer.pop_front();

    for(i=mBitBuffer.begin(); i != mBitBuffer.end(); ++i)
    {
        Bits data = (*i);
        int count = data.c;
        bool val = data.v;
        //cout << val << ":";
        //cout.fill('0');
        //cout.width(3);
        //cout << count << ", ";

        if(val==0 && count > 75 && count < 150)
        {
            cout << "0";
        }
        else if(val==1 && count > 75 && count < 150)
        {
            cout << "1";
        }
        else if(val==0 && count > 275 && count < 350)
        {
            cout << "00";

        }
        else if(val==1 && count > 275 && count < 350)
        {
            cout << "11";

        }
    }
    cout << endl;
    return retVal;
}


int main ()
{


    Sampler sampler;
    sampler.init();
    sampler.sample();
    return 0;
}
