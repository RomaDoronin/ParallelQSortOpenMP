// Roman Doronin © 2017-2019
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

// Возвращает номер блока из которого возьмется ведущий элемент
// threadNum, iterCount, threadCount
int SelectMedium(int threadNum, int iterCount, int threadCount)
{
    if (iterCount)
    {
        return (threadCount - threadCount % (threadNum / MathFunc::Step2(iterCount - 1)));
    }

    return 0;
}

// Функция для выбора пар смежных элементов
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
    // Размер блока
    int blockSize = dataSize / blockNum;
    int iterNum = MathFunc::logCalc(blockNum);

    //------------------------------------
    // Определение массива блоков
    int **blocksArr = new int*[blockNum];
#pragma omp parallel for
    for (int count = 0; count < blockNum; count++)
    {
        blocksArr[count] = new int[dataSize + 1];
    }

    //------------------------------------
    // Пускай на 0-ом месте хранится размер блока
    // Инициализируем блоки данными
#pragma omp parallel for
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        blocksArr[blockCount][0] = blockSize;
        memcpy(&blocksArr[blockCount][1], &pData[blockCount * blockSize], sizeof(int) * blockSize);
    }

    //------------------------------------
    // Параллельная сортировка данных на блоки
    for (int iterCount = 0; iterCount < iterNum; iterCount++)
    {
        omp_set_num_threads(threadNum);

#pragma omp parallel for //shared(size, ProcNum, Bloks, i)
        for (int threadCount = 0; threadCount < threadNum; threadCount++)
        {
            //------------------------------------
            // Создание временного массива
            int* temp = new int[dataSize + 1];

            //------------------------------------
            // Выбор ведущего элемента
            temp[0] = blocksArr[SelectMedium(threadNum, iterCount, threadCount)][1];

            //------------------------------------
            // Определяет какое должно быть j(new) взависимости от iterCount и threadCount
            int j = SelectJ(threadNum, iterCount, threadCount);
            memcpy(&temp[1], &blocksArr[j][1], sizeof(int) * blocksArr[j][0]);
            int tempSize = 1 + blocksArr[j][0];

            int index = j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount);
            memcpy(&temp[tempSize], &blocksArr[j][1], sizeof(int) * blocksArr[index][0]);
            tempSize += blocksArr[index][0];

            //------------------------------------
            // Перекидывание элементов в зависимости от ведущего
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
            // Заносим первый массив
            int tempCount = 1;
            for (int k = 0; k < PivotPos; k++)
            {
                blocksArr[j][k + 1] = temp[tempCount];
                tempCount++;
            }

            blocksArr[j][0] = PivotPos;

            //------------------------------------
            // Заносим второй массив
            for (int k = 0; k < tempSize - (PivotPos + 1); k++)
            {
                blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][k + 1] = temp[tempCount];
                tempCount++;
            }
            blocksArr[j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount)][0] = tempSize - (PivotPos + 1);

            delete[] temp;
        }
    }

#pragma omp parallel for
    for (int blockCount = 0; blockCount < blockNum; blockCount++)
    {
        //------------------------------------
        // Сортировка элементов блоков
        const int arrStartIndex = 1;
        std::qsort(&blocksArr[blockCount][arrStartIndex], blocksArr[blockCount][0], sizeof(int),
            [](const void* a, const void* b)
        {
            int arg1 = *static_cast<const int*>(a);
            int arg2 = *static_cast<const int*>(b);

            return (arg1 > arg2) - (arg1 < arg2);
        });

        /*printLog("Block[" + std::to_string(blockCount) + "]: [ " + std::to_string((int)blocksArr[blockCount][0]) + " ");
        for (int j = 1; j < blocksArr[blockCount][0]; j++)
        {
            printLog(std::to_string((int)blocksArr[blockCount][j]) + " ");
        }
        printLog("]\n");*/

        int pDataStartIndex = 0;

        for (int count = 0; count < blockCount; count++)
        {
            pDataStartIndex += blocksArr[count][0];
        }

        //printLog("H = " + std::to_string(pDataStartIndex) + "\n");

        //------------------------------------
        // Занесение блоков в исходный массив
        for (int elemCount = 0; elemCount < blocksArr[blockCount][0]; elemCount++)
        {
            pData[pDataStartIndex] = blocksArr[blockCount][elemCount + 1];
            //printLog(">> pData[" + std::to_string(pDataStartIndex) + "]: " + std::to_string(pData[pDataStartIndex]) + "\n");
            pDataStartIndex++;
        }
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
    // Начальные условия
    int arrSize = 10000000;
    int threadNum = 4;
    int blockNum = threadNum * 2;

    std::cout << "Array size    : " << arrSize << std::endl;
    std::cout << "Thread number : " << threadNum << std::endl;

    //------------------------------------
    // Задание массива
    int *arr = new int[arrSize];
    //printLog("Arr: [ ");
    for (int i = 0; i < arrSize; i++)
    {
        arr[i] = rand() % arrSize - arrSize / 2;
        //printLog(std::to_string((int)arr[i]) + " ");
        
    }
    //printLog("]\n");

    //------------------------------------
    // Вызов сортировки
    double start_time = omp_get_wtime();
    ParallelQuickSotr(arr, arrSize, threadNum, blockNum);
    double end_time = omp_get_wtime();

    std::cout << std::endl << "Time: " << end_time - start_time << " sec" << std::endl << std::endl;

    //------------------------------------
    // Проверка на корректрость
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
