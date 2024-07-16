#include "gnuplot.h"

int main(){
    GnuplotPipe gp;
	gp.sendLine("set terminal png");
	gp.sendLine("set output 'output.png'");
    gp.sendLine("plot [-pi/2:pi] cos(x),-(sin(x) > sin(x+1) ? sin(x) : sin(x+1))");
    return 0;
}
#if 0
#include "ht6022lib.h"
//#include <set>
//#include <matplot/matplot.h>

int main()
{
#if 0
    oscilloscopes::hantek::ht6022be osc;
    std::ofstream out_ch1("signal_CH1.txt"), out_ch2("signal_CH2.txt"); 
    for( size_t i = 0; i < 10; ++i )
    {
        auto signal = osc.readFrame(oscilloscopes::hantek::HT6022_1KB, 0.914306640625, 9.122314453125);

        for( auto it : signal.first )
            out_ch1 << ((int)it) << ' ';
        for( auto it : signal.second )
            out_ch2 << ((int)it) << ' ';
    }
    out_ch1.close();
    out_ch2.close();
#endif
    return 0;
}
#endif

