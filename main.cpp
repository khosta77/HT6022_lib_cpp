#include "ht6022lib.h"

int main()
{
    oscilloscopes::hantek::ht6022be osc;
    auto signal = osc.readFrame(oscilloscopes::hantek::HT6022_4KB, 0.914306640625, 9.122314453125);
    std::ofstream out_ch1("signal_CH1.txt"), out_ch2("signal_CH2.txt"); 

    for( auto it : signal.first )
        out_ch1 << ((int)it) << ' ';
    for( auto it : signal.second )
        out_ch2 << ((int)it) << ' ';

    out_ch1.close();
    out_ch2.close();
    return 0;
}


