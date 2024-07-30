#include "hantek6022.h"
#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>

#define CH1_CALIBRATION 0.914306640
#define CH2_CALIBRATION 9.245046616

using namespace oscilloscopes::hantek;
using HT6022 = oscilloscopes::hantek::Hantek6022;
using oscilloscopes::OscSigframe;
using oscilloscopes::OscSignal;



OscSigframe downSignal( OscSigframe osf, const size_t& channelSize, const int& down = 127,
                        const std::vector<float>& CHx_CALIBRATION = { CH1_CALIBRATION,
                        CH2_CALIBRATION } )
{
    for( size_t i = 0; i < channelSize; ++i )
    {
        for( size_t j = 0; j < osf[i]._signalSize; ++j )
            osf[i]._signal[j] -= ( down + CHx_CALIBRATION[i] );
    }
    return osf;
}

double mean( const OscSignal& signal )
{
    double avg = 0.0;
    for( auto& it : signal._signal )
        avg += it;
    return ( avg / signal._signalSize );
}

int main()
{
    Hantek6022 oscilloscope;


    return 0;
}


