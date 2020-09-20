
#include "math.h"

double pow(double x, int power) {
        if (power == 0)
                return 1;
        double ans = x;
        int curPower = 1;
        while (curPower < power) {

                if ( curPower + curPower <= power ) {
                        ans = ans*ans;
                        curPower = curPower + curPower;
                }
                else {
                        ans = ans*x;
                        curPower++;
                }
        }
        return ans;
}

double log(double x) {

        int i = 1;
        double num = 0;
        double temp = 0;
        while (i < 21) {
                if (i%2 == 0) {
                        temp = -pow(x-1, i)/i;
                } else {
                        temp = pow(x-1, i)/i;
                }
                num += temp;
                ++i;
        }
        return num;
}

double expdev(double lambda) {
        double dummy;

        do {
                
                dummy = (double)rand() / 077777;

        } while (dummy == 0.0);
	return -log(dummy) / lambda;
}

