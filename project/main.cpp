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

std::vector<std::vector<float>> downSignal( OscSigframe osf, const size_t& channelSize,
                                            const float& down = 127.0, const std::vector<float>&
                                            CHx_CALIBRATION = { CH1_CALIBRATION, CH2_CALIBRATION } )
{
    std::vector<std::vector<float>> channels;
    for( size_t i = 0; i < channelSize; ++i )
    {
        std::vector<float> channel;
        for( const auto& it:  osf[i]._signal )
            channel.push_back( ( ((float)it) - down - CHx_CALIBRATION[i] ) );
        channels.push_back(channel);
        channel.clear();
    }
    return channels;
}

float mean( const std::vector<float>& signal )
{
    float avg = 0.0;
    for( const auto& it : signal )
        avg += it;
    return ( avg / signal.size() );
}

int main()
{
    Hantek6022 oscilloscope( HT6022::_8MSa, 1, 1 );
    auto buffer = oscilloscope.getSignalFromTrigger( 0, 1.7, 2 )._signal;
    //for( const auto& it : buffer ) std::cout << it << ' ';
    return 0;
}


