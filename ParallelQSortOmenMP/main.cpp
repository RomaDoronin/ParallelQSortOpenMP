// Roman Doronin � 2017-2019
//

#include <iostream>
#include <omp.h>
#include <string>

#include "ArrayFunc.h"
#include "MathFunc.h"


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
    if (iterCount)
    {
        return (threadCount - threadCount % (threadNum / MathFunc::Step2(iterCount - 1)));
    }

    return 0;
}

// ������� ��� ������ ��� ������� ���������
int SelectJ(int _ProcNum, int _i, int _j)
{
    if (_i)
    {
        return _j + (_j / (_ProcNum / MathFunc::Step2(_i))) * (_ProcNum / MathFunc::Step2(_i));
    }

    return _j;
}

void ParallelQuickSotr(int* pData, int dataSize, int threadNum, int blockNum)
{
    //------------------------------------
    // ������ �����
    int blockSize = dataSize / blockNum;
    int iterNum = MathFunc::logCalc(blockNum);

    //------------------------------------
    // ����������� ������� ������
    int **blocksArr = new int*[blockNum];
#pragma omp parallel for
    for (int count = 0; count < blockNum; count++)
    {
        blocksArr[count] = new int[dataSize + 1];
    }

    //------------------------------------
    // ������ �� 0-�� ����� �������� ������ �����
    // �������������� ����� �������
#pragma omp parallel for
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        blocksArr[blockCount][0] = blockSize;
        memcpy(&blocksArr[blockCount][1], &pData[blockCount * blockSize], sizeof(int) * blockSize);
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
            temp[0] = blocksArr[SelectMedium(threadNum, iterCount, threadCount)][1];

            //------------------------------------
            // ���������� ����� ������ ���� j(new) ������������ �� iterCount � threadCount
            int j = SelectJ(threadNum, iterCount, threadCount);
            memcpy(&temp[1], &blocksArr[j][1], sizeof(int) * blocksArr[j][0]);
            int tempSize = 1 + blocksArr[j][0];

            int index = j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount);
            memcpy(&temp[tempSize], &blocksArr[j][1], sizeof(int) * blocksArr[index][0]);
            tempSize += blocksArr[index][0];

            //------------------------------------
            // ������������� ��������� � ����������� �� ��������
            int PivotPos = 0;
            int Pivot = temp[0];
            for (int tempCount = 1; tempCount < tempSize; tempCount++)
            {
                if (temp[tempCount] <= Pivot)
                {
                    if (tempCount != (PivotPos + 1))
                    {
                        std::swap(temp[tempCount], temp[PivotPos + 1]);
                    }
                    PivotPos++;
                }
            }

            //------------------------------------
            // ������� ������ ������
            int tempCount = 1 + PivotPos;
            blocksArr[j][0] = PivotPos;
            memcpy(&blocksArr[j][1], &temp[1], sizeof(int) * blocksArr[j][0]);

            //------------------------------------
            // ������� ������ ������
            blocksArr[index][0] = tempSize - (PivotPos + 1);
            memcpy(&blocksArr[index][1], &temp[tempCount], sizeof(int) * blocksArr[index][0]);

            delete[] temp;
        }
    }

    omp_set_num_threads(threadNum);
#pragma omp parallel for
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        //------------------------------------
        // ���������� ��������� ������
        const int arrStartIndex = 1;
        std::cout << "Thread number: " << omp_get_thread_num() << "\n";
        std::qsort(&blocksArr[blockCount][arrStartIndex], blocksArr[blockCount][0], sizeof(int),
            [](const void* a, const void* b)
        {
            int arg1 = *static_cast<const int*>(a);
            int arg2 = *static_cast<const int*>(b);

            return arg1 - arg2;
        });

        int pDataStartIndex = 0;

        for (int count = 0; count < blockCount; count++)
        {
            pDataStartIndex += blocksArr[count][0];
        }

        //------------------------------------
        // ��������� ������ � �������� ������
        memcpy(&pData[pDataStartIndex], &blocksArr[blockCount][1], sizeof(int) * blocksArr[blockCount][0]);
    }

#pragma omp parallel for
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

    std::cout << std::endl;
    int a;
    std::cin >> a;

    return 0;
}
