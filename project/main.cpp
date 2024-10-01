#include "hantek6022.h"
#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>

using namespace oscilloscopes::hantek;
using HT6022 = oscilloscopes::hantek::Hantek6022;
using oscilloscopes::OscSigframe;
using oscilloscopes::OscSignal;

Hantek6022 oscilloscope( 8'000, 1, 1 );

void readSignalToFile( const std::string& fn = "signal.txt" )
{
    std::vector<int> signal = oscilloscope.getSignalFrame( HT6022::_16KB )[0]._signal;
    std::fstream fout( fn.c_str(), ( std::ios::out | std::ios::trunc | std::ios::binary ) );
    assert( fout.is_open() );
    for( const int& it : signal )
        fout << it << std::endl;
    signal.clear();
    fout.close();
}

int main()
{
    //readSignalToFile();
    oscilloscope.onTrigger();
    oscilloscope.getSignalFromTrigger(0, 150, 2, HT6022::_1KB);
    return 0;
}


