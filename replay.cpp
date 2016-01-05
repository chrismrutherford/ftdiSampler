#include "replay.h"


Replay::Replay():mFtStatus(FT_OK),mPortNumber(0),mBaudRate(9600*SCALE)
{
    pFile = fopen("/tmp/sample.bin","rb");
}

int Replay::init()
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
                              0xFF, /* sets all 8 pins as outputs */
                              FT_BITMODE_ASYNC_BITBANG);
    if (mFtStatus != FT_OK)
    {
        printf("FT_SetBitMode failed (error %d).\n", (int)mFtStatus);
    }

    printf("Setting clock rate to %d\n", mBaudRate * 16);
    mFtStatus = FT_SetBaudRate(mFtHandle, mBaudRate *2 );
    if (mFtStatus != FT_OK)
    {
        printf("FT_SetBaudRate failed (error %d).\n", (int)mFtStatus);
        retVal = 2;
    }

    usleep(1000000);

    return retVal;

}

int Replay::replay()
{
    DWORD bytesWritten;
    size_t result;

    while(1)
    {
        result = fread(mBuffer,1,BUFSZ,pFile);
        if(result < BUFSZ)
        {
            rewind(pFile);
            cout << "wrap" << endl;
            usleep(1000000);
        }
        else
        {

            for(int i= 0; i < BUFSZ; i++)
            {
                int val = (mBuffer[i] & 16)/16;
                //cout << i << " " << val <<endl;
                mBuffer[i] = val*4;
                //mBuffer[i] = 1;
                //cout << i << " " << val*4 <<endl;

            }

            mFtStatus = FT_Write(mFtHandle, mBuffer, BUFSZ, &bytesWritten);
            cout << "write bytes " << result << " " << bytesWritten << endl;
        }
    }

    return bytesWritten;

}

int main ()
{


    Replay replay;
    replay.init();
    replay.replay();
    return 0;
}
