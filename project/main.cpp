#include "ht6022lib.h"
#include "gnuplot.h"
#include <algorithm>
#include <iomanip>

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

#define CH1 0.914306640625

using namespace oscilloscopes::hantek;

using Signal = std::vector<int>;
using Signals = std::vector<Signal>;

Signals readManySignals( const size_t& size )
{
    Signals signals;
    ht6022be osc;
    for( size_t i = 0; i < size; ++i )
        signals.push_back( osc.readFrame( HT6022_1KB, CH1 ).first );
    return signals;
}

void progressbar( int i, int start, int end, const int& barSize = 70 )
{
    float progress = ( (float)i / (float)end );
    int pos = barSize * progress;
    std::cout << "[";
    for( int j = 0; j < barSize; j++ )
    {
        if( j < pos )
            std::cout << "=";
        else if( j == pos )
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << ((int)( progress * 100 )) << " %\r";
    std::cout.flush();
}


Signals findZero( Signals signals )
{
    const size_t size = signals[0].size();
    Signals total;
    for( size_t i = 0; i < size; ++i )
    {
        Signal line;
        progressbar( i, 0, size );
        size_t count = 0;
        for( size_t j = i; j < size; ++j )
        {
            for( size_t k = 0; k < ( size - i ); ++k )
            {
                if( ( signals[0][j] - signals[1][k] ) == 0 )
                {
                    ++count;
                }
                else
                {
                    if( count == 0 )
                        continue;
                    line.push_back(count);
                    count = 0;
                }
            }
        }
        total.push_back(line);
    }
    progressbar( size, 0, size );
    std::cout << std::endl;
    return total;
}

size_t sizeZero( const Signal& line)
{
    size_t size = 0;
    for( auto& it : line )
    {
        if( it == 0 )
            ++size;
    }
    return size;
}

int fmax( const Signal& line )
{
    if( line.size() == 0 )
        return -1;
    int max = line[0];
    
    for( size_t i = 1; i < line.size(); ++i )
    {
        if( line[i] > max )
            max = line[i];
    }

    return max;
}

void print( Signals total )
{
    for( size_t i = 0; i < total.size(); ++i )
    {
        std::cout << std::setw(6) << i << "|" << std::setw(6) << fmax(total[i]) << std::endl;
    }
}

int main()
{
    auto signals = readManySignals(2);
    //auto zeros = findZero(signals); 
    //print(zeros);
#if 0
    //ht6022be osc;
    std::ofstream out_ch1("signal_CH1.txt"), out_ch2("signal_CH2.txt"); 
    size_t j = 0;
    for( auto signal : signals )
    {
        //auto signal = osc.readFrame( HT6022_1KB, CH1 );


        for( size_t i = 0; i < signal.size(); ++i, ++j )
		{
            out_ch1 << ( j ) << ' ' << ((int)signal[i]) << std::endl;
		    if( i > ( signal.size() - 3 ) )
			//{
			//	out_ch1 << ' ' << -127;
				std::cout << j << std::endl;
			//}
			//else
			//	out_ch1 << ' ' << 1000;
			//out_ch1 << std::endl;
		}
		//for( auto it : signal.second )
        //    out_ch2 << ((int)it) << ' ';
    }
    out_ch1.close();
    out_ch2.close();
	//plotgnuplot();
#endif
    return 0;
}

