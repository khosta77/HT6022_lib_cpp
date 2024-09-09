#include "hantek6022.h"
#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>

#define CH1_CALIBRATION 0.0395708  // Это калибровочные значения
#define CH2_CALIBRATION 0.3779261  


using namespace oscilloscopes::hantek;
using HT6022 = oscilloscopes::hantek::Hantek6022;
using oscilloscopes::OscSigframe;
using oscilloscopes::OscSignal;
#if 0
std::atomic<bool> is;

OscSigframe downSignal( OscSigframe osf, const size_t& channelSize, const std::vector<float>&
                                            CHx_CALIBRATION = { CH1_CALIBRATION, CH2_CALIBRATION } )
{
    for( size_t i = 0; i < channelSize; ++i )
    {
        for( size_t j = 0; j < osf[i]._signalSize; ++j )
            osf[i]._signal[j] -= CHx_CALIBRATION[i];
    }
    return osf;
}

float mean( const std::vector<float>& signal )
{
    float avg = 0.0;
    for( const auto& it : signal )
        avg += it;
    return ( avg / signal.size() );
}

Hantek6022 oscilloscope( 8'000, 1, 1 );

void testSetGetRangeInputLevel()
{
    while( is )
    {
        const size_t channelSize = oscilloscope.getChannelsSize();
        auto range = oscilloscope.getRangeInputLevel();
        for( size_t channelI = 0; channelI < channelSize; ++channelI )
        {
            for( const auto& it : range )
            {
                size_t setValue = oscilloscope.setInputLevel( channelI, it );
                size_t getValue = oscilloscope.getInputLevel( channelI );
                if( ( ( it != setValue ) || ( it != getValue ) ) )
                {
                    std::cout << "InputLevel: " << it << ' ' << setValue << ' ' << getValue << std::endl;
                    return;
                }
                else
                {
                    std::this_thread::sleep_for( std::chrono::nanoseconds(50) );
                }
            }
        }
    }
}

void testSetGetRangeSampleRate()
{
    while( is )
    {
        const size_t channelSize = oscilloscope.getChannelsSize();
        auto range = oscilloscope.getRangeSampleRate();
        for( size_t channelI = 0; channelI < channelSize; ++channelI )
        {
            for( const auto& it : range )
            {
                size_t setValue = oscilloscope.setSampleRate( it );
                size_t getValue = oscilloscope.getSampleRate();
                if( ( ( it != setValue ) || ( it != getValue ) ) )
                {
                    std::cout << "SampleRate: " << it << ' ' << setValue << ' ' << getValue << std::endl;
                    return;
                }
                else
                {
                    std::this_thread::sleep_for( std::chrono::nanoseconds(50) );
                }
            }
        }
    }
}

void testTriggerLevel()
{
    std::fstream fout("signal2.txt");
    while( is )
    {
        oscilloscope.onTrigger();
        auto buffer = oscilloscope.getSignalFromTrigger( 0, 1.7, 1 )._signal;
        if( fout.is_open() )
        {
            for( const auto& it : buffer )
                fout << it << ' ';
            fout.close();
        }
        std::cout << "getSignalFromTrigger: " << buffer.size() << std::endl;
        std::this_thread::sleep_for( std::chrono::nanoseconds(50) );
    }
}

void testReadFrame()
{
    while( is )
    {
        auto range = oscilloscope.getRangeSignalFrame();
        for( const auto& it : range )
        {
            auto df = oscilloscope.getSignalFrame(it);
            df = downSignal( df, 2 );
            float meanCh0 = ( std::round( mean( df[0]._signal ) * 100 ) / 100 );
            float meanCh1 = ( std::round( mean( df[1]._signal ) * 100 ) / 100 );
            std::cout << "getRangeSignalFrame: " << ( meanCh0 == 0.0 ) << ' ' << ( meanCh1 == 0.0 ) << ' ' << it << std::endl; 
        }
        std::this_thread::sleep_for( std::chrono::nanoseconds(50) );
    }
}
#endif
Hantek6022 oscilloscope( 8'000, 1, 1 );

int main()
{

#if 0
    is = true;
    std::thread t0( testTriggerLevel );
    //std::thread t1( testSetGetRangeInputLevel );
    //std::thread t2( testSetGetRangeSampleRate );
    std::thread t3( testReadFrame );
    t0.join();
    //t1.join();
    //t2.join();
    t3.join();
#endif
    return 0;
}


