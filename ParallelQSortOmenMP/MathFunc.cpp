#include "MathFunc.h"
#include "cmath"


int MathFunc::Step2(int step)
{
    return 1 << step;
}

int MathFunc::logCalc(int twoT)
{
    return (int)log2(twoT);
}
