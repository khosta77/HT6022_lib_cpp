#include "ht6022lib.h"

int main () {
    oscilloscopes::hantek::ht6022be osc;
    auto signal = osc.readFrame().first;
    std::ofstream out("test.txt"); 
    for( auto it : signal )
        out << ((int)it) << ' ';
    out.close();
	return 0;
}


