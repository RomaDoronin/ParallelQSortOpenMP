// Roman Doronin � 2017-2019
//

// ���� ��������, �� ������� ���������
// SelectJ - ����� �� ��� ��� ���� �������� � ����� ��������� �������� ������� �������
// ������� ��� ��� ���������� � ���� �� ���� ����� tbb, ������� � ����� "2-� ������������ ������". � ��� ��� ���� ���������.
// ��� ��� ��� ���������� �� �����, ����� ������:)

#include <iostream>
#include <omp.h>
#include <string>

#include "ArrayFunc.h"
#include "MathFunc.h"
#include "QuickSort.h"


void printLog(std::string log)
{
#ifdef _DEBUG
    std::cout << log;
#endif // DEBUG
}

// ���������� ����� ����� �� �������� ��������� ������� �������
// threadNum, iterCount, threadCount
int SelectMedium(int threadNum, int iterCount, int threadCount)
{
    if (iterCount == 0)
    {
        return 0;
    }
    else
    {
        return (threadCount - threadCount % (threadNum / MathFunc::Step2(iterCount - 1)));
    }
}

// ������� ��� ������ ��� ������� ���������
int SelectJ(int _ProcNum, int _i, int _j)
{
    if (_i == 0)
        return _j;
    else
    {
        return _j + (_j / (_ProcNum / MathFunc::Step2(_i))) * (_ProcNum / MathFunc::Step2(_i));
    }
}

void ParallelQuickSotr(int* pData, int dataSize, int threadNum, int blockNum)
{
    //------------------------------------
    // ������ �����
    int blockSize = dataSize / blockNum;
    int iterNum = MathFunc::logCalc(blockNum);

    //printLog("Data size  : " + std::to_string(dataSize)  + "\n");
    //printLog("Thread num : " + std::to_string(threadNum) + "\n");
    //printLog("Block num  : " + std::to_string(blockNum)  + "\n");
    //printLog("Block size : " + std::to_string(blockSize) + "\n");
    //printLog("Iter num   : " + std::to_string(iterNum)   + "\n");

    //------------------------------------
    // ����������� ������� ������
    int **blocksArr = new int*[blockNum];
    for (int count = 0; count < blockNum; count++)
    {
        blocksArr[count] = new int[dataSize + 1];
    }

    //------------------------------------
    // ������ �� 0-�� ����� �������� ������ �����
    // �������������� ����� �������
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        blocksArr[blockCount][0] = blockSize;
        for (int elemCount = 1; elemCount < blockSize + 1; elemCount++)
        {
            blocksArr[blockCount][elemCount] = pData[blockCount * blockSize + elemCount - 1];
        }
    }

    for (int i = 0; i < blockNum; i++)
    {
        //printLog("Block[" + std::to_string(i) + "]: [ " + std::to_string((int)blocksArr[i][0]) + " ");
        for (int j = 1; j < blockSize; j++)
        {
            //printLog(std::to_string((int)blocksArr[i][j]) + " ");
        }
        //printLog("]\n");
    }

    //------------------------------------
    // ������������ ���������� ������ �� �����
    for (int iterCount = 0; iterCount < iterNum; iterCount++)
    {
        omp_set_num_threads(threadNum);

#pragma omp parallel for //shared(size, ProcNum, Bloks, i)
        for (int threadCount = 0; threadCount < threadNum; threadCount++)
        {
            //------------------------------------
            // �������� ���������� �������
            int* temp = new int[dataSize + 1];

            //------------------------------------
            // ����� �������� ��������
            //printLog("SelectMedium(" + std::to_string(threadNum) + ", " + std::to_string(iterCount) + ", " + std::to_string(threadCount) + ")\n");
            temp[0] = blocksArr[SelectMedium(threadNum, iterCount, threadCount)][1];

            //------------------------------------
            // ���������� ����� ������ ���� j(new) ������������ �� iterCount � threadCount
            int j = SelectJ(threadNum, iterCount, threadCount);

            int tempSize = 1;
            for (int k = 1; k < blocksArr[j][0] + 1; k++)
            {
                temp[tempSize] = blocksArr[j][k];
                tempSize++;
            }
            for (int k = 1; k < blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][0] + 1; k++)
            {
                temp[tempSize] = blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][k];
                tempSize++;
            }

            //------------------------------------
            // ������������� ��������� � ����������� �� ��������
            int PivotPos = 0;
            int Pivot = temp[0];
            for (int t = 1; t < tempSize; t++)
            {
                if (temp[t] <= Pivot)
                {
                    if (t != (PivotPos + 1))
                    {
                        std::swap(temp[t], temp[PivotPos + 1]);
                    }
                    PivotPos++;
                }
            }

            //------------------------------------
            // ������� ������ ������
            int tempCount = 1;
            for (int k = 0; k < PivotPos; k++)
            {
                blocksArr[j][k + 1] = temp[tempCount];
                tempCount++;
            }

            blocksArr[j][0] = PivotPos;

            //------------------------------------
            // ������� ������ ������
            for (int k = 0; k < tempSize - (PivotPos + 1); k++)
            {
                blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][k + 1] = temp[tempCount];
                tempCount++;
            }
            blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][0] = tempSize - (PivotPos + 1);

            delete[] temp;
        }
    }

    for (int i = 0; i < blockNum; i++)
    {
        //printLog("Block[" + std::to_string(i) + "]: [ " + std::to_string((int)blocksArr[i][0]) + " ");
        for (int j = 1; j < blocksArr[i][0]; j++)
        {
            //printLog(std::to_string((int)blocksArr[i][j]) + " ");
        }
        //printLog("]\n");
    }

#pragma omp parallel for
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        //------------------------------------
        // ���������� ��������� ������
        const int arrStartIndex = 1;
        QuickSort::SerialQuickSort(blocksArr[blockCount], arrStartIndex, blocksArr[blockCount][0]);

        //printLog("Block[" + std::to_string(blockCount) + "]: [ " + std::to_string((int)blocksArr[blockCount][0]) + " ");
        for (int j = 1; j < blocksArr[blockCount][0]; j++)
        {
            //printLog(std::to_string((int)blocksArr[blockCount][j]) + " ");
        }
        //printLog("]\n");

        int pDataStartIndex = 0;

        for (int count = 0; count < blockCount; count++)
        {
            pDataStartIndex += blocksArr[count][0];
        }

        //printLog("H = " + std::to_string(pDataStartIndex) + "\n");

        //------------------------------------
        // ��������� ������ � �������� ������
        for (int elemCount = 0; elemCount < blocksArr[blockCount][0]; elemCount++)
        {
            pData[pDataStartIndex] = blocksArr[blockCount][elemCount + 1];
            //printLog(">> pData[" + std::to_string(pDataStartIndex) + "]: " + std::to_string(pData[pDataStartIndex]) + "\n");
            pDataStartIndex++;
        }
    }

    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        delete[] blocksArr[blockCount];
    }

    delete[] blocksArr;
}

int main()
{
    std::cout << "-- My QuickSort --" << std::endl;

    //------------------------------------
    // ��������� �������
    int arrSize = 10000000;
    int threadNum = 4;
    int blockNum = threadNum * 2;

    std::cout << "Array size    : " << arrSize << std::endl;
    std::cout << "Thread number : " << threadNum << std::endl;

    //------------------------------------
    // ������� �������
    int *arr = new int[arrSize];
    //printLog("Arr: [ ");
    for (int i = 0; i < arrSize; i++)
    {
        arr[i] = rand() % arrSize - arrSize / 2;
        //printLog(std::to_string((int)arr[i]) + " ");
        
    }
    //printLog("]\n");

    //------------------------------------
    // ����� ����������
    double start_time = omp_get_wtime();
    ParallelQuickSotr(arr, arrSize, threadNum, blockNum);
    double end_time = omp_get_wtime();

    std::cout << std::endl << "Time: " << end_time - start_time << " sec" << std::endl << std::endl;

    //------------------------------------
    // �������� �� ������������
    if (ArrayFunc::PRKK(arr, arrSize))
    {
        std::cout << "Correctly" << std::endl << std::endl;
    }
    else
    {
        std::cout << "Not correctly" << std::endl << std::endl;
    }

    /*std::cout << std::endl;
    int a;
    std::cin >> a;*/

    return 0;
}
