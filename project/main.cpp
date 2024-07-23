#include "hantek6022.h"
//#include "gnuplot.h"
#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>
#ifdef RASPBERRY_PI
#include <wiringPi.h>
#endif

#define CH1_CALIBRATION 0.914306640
#define CH2_CALIBRATION 9.245046616

using namespace oscilloscopes::hantek;
using HT6022 = oscilloscopes::hantek::Hantek6022;
using oscilloscopes::OscSigframe;
using oscilloscopes::OscSignal;

std::atomic<bool> done(false);

class Moderator
{
private:
    oscilloscopes::Oscilloscope *_oscilloscope;
    const size_t _memory;

    OscSigframe read( const size_t& Memory )
    {
        return _oscilloscope->getSignalFrame(Memory);
    }

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

    /** @brief Среднее значение сигнала
     * */
    double mean( const OscSignal& signal )
    {
        double avg = 0.0;
        for( auto& it : signal._signal )
            avg += it;
        return ( avg / signal._signalSize );
    }

    static void delay( const long long int& time )
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(time));
        done = true;
    }
public:
    Moderator( oscilloscopes::Oscilloscope *osc, const size_t& mem ) : _oscilloscope(osc), _memory(mem)
    {
#ifdef RASPBERRY_PI
        wiringPiSetup();
        pinMode(26, OUTPUT);
        digitalWrite(26, LOW);
        pinMode(27, OUTPUT);
        digitalWrite(27, LOW);
#endif
    }
    ~Moderator() {}

    void run( [[ maybe_unused ]] const long long int& wait )
    {
        
        std::thread t( delay, 5'000'000'000 );

#ifdef RASPBERRY_PI
        digitalWrite(26, HIGH);
#endif
        t.join();
        while(!done){}

#ifdef RASPBERRY_PI
        digitalWrite(27, HIGH);
#endif

        auto df = read(_memory);
        df = downSignal( df, 2 ); 
        std::cout << df[0]._signalSize << std::endl;
    }
};

int main()
{
    Hantek6022 A;
    Moderator moderator( &A, HT6022::_1MB );
    moderator.run(5'000'000'000);
    return 0;
}


