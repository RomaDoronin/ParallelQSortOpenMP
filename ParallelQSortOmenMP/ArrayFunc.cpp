#include "ArrayFunc.h"
#include <iostream>


bool ArrayFunc::PRKK(int *arr, int arrSize)
{
    int repeatCount = 0;
    int MAX_REPEAT = 1000;

    for (int i = 0; i < arrSize - 1; i++)
    {
        if (arr[i] > arr[i + 1])
        {
            return false;
        }
        else if (arr[i] == arr[i + 1])
        {
            repeatCount++;
        }
        else
        {
            repeatCount = 0;
        }

        if (repeatCount == MAX_REPEAT)
        {
            return false;
        }
    }

    return true;
}
