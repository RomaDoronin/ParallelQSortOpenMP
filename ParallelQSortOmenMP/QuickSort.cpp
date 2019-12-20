#include "QuickSort.h"
#include <iostream>


void QuickSort::SerialQuickSort(double* pData, int first, int last)
{
	if (first >= last)
	{
		return;
	}

	int PivotPos = first;
	double Pivot = pData[first];

	for (int i = first + 1; i <= last; i++) {
		if (pData[i] >= Pivot)
		{
			continue;
		}

		if (i != PivotPos + 1)
		{
			std::swap(pData[i], pData[PivotPos + 1]);
		}

		PivotPos++;
	}

	std::swap(pData[first], pData[PivotPos]);

	SerialQuickSort(pData, first, PivotPos - 1);
	SerialQuickSort(pData, PivotPos + 1, last);
}
