#include "hantek6022.h"

#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>

using namespace oscilloscopes::hantek;
using HT6022 = oscilloscopes::hantek::Hantek6022;
using oscilloscopes::OscSigframe;
using oscilloscopes::OscSignal;

Hantek6022 oscilloscope( 100, 1, 1 );

void saveSignalToFile( const std::string& fn = "signal.txt" )
{
    std::vector<int> signal = oscilloscope.getSignalFrame( HT6022::_2KB )[0]._signal;
    std::fstream fout( fn.c_str(), ( std::ios::out | std::ios::trunc | std::ios::binary ) );
    assert( fout.is_open() );
    for( const int& it : signal )
        fout << it << std::endl;
    signal.clear();
    fout.close();
}

void saveSignal( const OscSignal& oscSignal, const size_t& i )
{
    std::vector<int> signal = oscSignal._signal;
    std::string fn = ( "./signal/signal" + std::to_string(i) + ".txt" );
    std::fstream fout( fn.c_str(), ( std::ios::out | std::ios::trunc | std::ios::binary ) );
    assert( fout.is_open() );
    for( const int& it : signal )
        fout << it << std::endl;
    signal.clear();
    fout.close();
}

void displayProgressBar(int currentIteration, int maxIterations)
{
    const int barWidth = 70;
    float progress = static_cast<float>(currentIteration) / maxIterations;

    int pos = static_cast<int>(barWidth * progress);

    std::cout << "[";
    for( int i = 0; i < barWidth; ++i )
    {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(progress * 100.0) << " %\r";
    std::cout.flush();
}

int main()
{
#if 0
    auto sig = oscilloscope.getSignalFrame(HT6022::_1MB)[0];
    saveSignal(sig, 0);
#else
    oscilloscope.onTrigger();
    std::thread t1( &oscilloscopes::hantek::Hantek6022::getSignalFromTrigger,
                    &oscilloscope, 0, 136, 1, HT6022::_2KB );
    for( size_t i = 0, I = 1; i < I; ++i )
    {
        displayProgressBar( i, I );
        auto signal = oscilloscope.getLastSignalFromTrigger();
        saveSignal( signal, i );
    }
    oscilloscope.offTrigger();
    t1.join();
#endif
    return 0;
}


