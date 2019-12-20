#include "ArrayFunc.h"


bool ArrayFunc::PRKK(double *arr, int arrSize)
{
	for (int i = 0; i < arrSize - 1; i++)
	{
		if (arr[i] > arr[i + 1])
		{
			return false;
		}
	}

	return true;
}

void ArrayFunc::ShiftMas(double *pData, int position, int shift, int c_size)
{
	if (shift >= 0)
	{
		for (int i = c_size + position - 1; i >= position; i--)
		{
			pData[i + shift] = pData[i];
		}
	}
	else
	{
		for (int i = position; i <= c_size + position - 1; i++)
		{
			pData[i + shift] = pData[i];
		}
	}
}
