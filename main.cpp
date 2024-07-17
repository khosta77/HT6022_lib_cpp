

#include "ht6022lib.h"
#include "gnuplot.h"

void plotgnuplot()
{
    GnuplotPipe gp;
	gp.sendLine("set title 'Line'");
	gp.sendLine("set xlabel 'x'");
	gp.sendLine("set ylabel 'y'");

	gp.sendLine("set grid");
	gp.sendLine("set key below center horizontal noreverse enhanced autotitle box dashtype solid");
	gp.sendLine("set tics out nomirror");
	gp.sendLine("set border 3 front linetype black linewidth 1.0 dashtype solid");

	gp.sendLine("set yrange [-127:127]");
	//gp.sendLine("set xrange [7000:9000]");
	//gp.sendLine("set xtics 1, .5, 5");
	//gp.sendLine("set mxtics 1");
	gp.sendLine("set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 1");

	gp.sendLine("set terminal pngcairo enhanced font 'Arial,12' size 1600,1200");
	gp.sendLine("set output 'output.png'");
	gp.sendLine("plot './signal_CH1.txt' using 1:2 with lines linestyle 1 title 'data',\\");
	gp.sendLine("	'' using 1:3 with lines linestyle 2 title 'line2'");
}

int main()
{
    oscilloscopes::hantek::ht6022be osc;
    std::ofstream out_ch1("signal_CH1.txt"), out_ch2("signal_CH2.txt"); 
    for( size_t i = 0, j = 0; i < 3; ++i )
    {
        auto signal = osc.readFrame(oscilloscopes::hantek::HT6022_1MB, 0.914306640625, 9.122314453125);


        for( size_t k = 0; k < signal.first.size(); ++k, ++j )
		{
            out_ch1 << ( j ) << ' ' << ((int)signal.first[k]);
			if( k > ( signal.first.size() - 3 ) )
			{
				out_ch1 << ' ' << -127;
				std::cout << j << std::endl;
			}
			else
				out_ch1 << ' ' << 1000;
			out_ch1 << std::endl;
		}
		//for( auto it : signal.second )
        //    out_ch2 << ((int)it) << ' ';
    }
    out_ch1.close();
    out_ch2.close();
	plotgnuplot();
    return 0;
}

