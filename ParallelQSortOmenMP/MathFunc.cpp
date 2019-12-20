#include "MathFunc.h"
#include "cmath"


int MathFunc::Step2(int step)
{
    return 1 << step;
}

int MathFunc::logCalc(int val)
{
    int count = -1;

    while (val)
    {
        val >>= 1;
        count++;
    }

    return count;
}
