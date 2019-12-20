// Roman Doronin © 2017-2019
//

#include <iostream>
#include <omp.h>
#include <string>
#include <random>

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

int getNormPivot(int j, int dataSize, int iterCount, int blockNum)
{    
    if (iterCount == 0)
    {
        return 0;
    }

    if (iterCount == 1)
    {
        int val = (dataSize / 4);
        if (j < blockNum / 2)
        {
            return (-1) * val;
        }
        else
        {
            return val;
        }
    }

    if (iterCount == 2)
    {
        int val = (dataSize / 8);
        switch (j)
        {
        case 0:
            return val - dataSize / 2;
        case 2:
            return val * 3 - dataSize / 2;
        case 4:
            return val;
        case 6:
            return val * 3;
        default:
            break;
        }
    }

    return -1;
}

void ParallelQuickSotr(int* pData, int dataSize, int threadNum, int blockNum)
{
    omp_set_num_threads(threadNum);

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
#pragma omp parallel for //shared(size, ProcNum, Bloks, i)
        for (int threadCount = 0; threadCount < threadNum; threadCount++)
        {
            //------------------------------------
            // Создание временного массива
            int* temp = new int[dataSize + 1];

            //------------------------------------
            // Определяет какое должно быть j(new) взависимости от iterCount и threadCount
            int j = SelectJ(threadNum, iterCount, threadCount);
            memcpy(&temp[1], &blocksArr[j][1], sizeof(int) * blocksArr[j][0]);
            int tempSize = 1 + blocksArr[j][0];

            //------------------------------------
            // Выбор ведущего элемента
            temp[0] = getNormPivot(j, dataSize, iterCount, blockNum);
            printLog("Pivot: " + std::to_string(temp[0]) + "\n");

            int index = j + MathFunc::Step2(MathFunc::logCalc(threadNum) - iterCount);
            memcpy(&temp[tempSize], &blocksArr[index][1], sizeof(int) * blocksArr[index][0]);
            tempSize += blocksArr[index][0];

            //------------------------------------
            // Перекидывание элементов в зависимости от ведущего
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
            // Заносим первый массив
            int tempCount = 1 + PivotPos;
            blocksArr[j][0] = PivotPos;
            memcpy(&blocksArr[j][1], &temp[1], sizeof(int) * blocksArr[j][0]);

            //------------------------------------
            // Заносим второй массив
            blocksArr[index][0] = tempSize - (PivotPos + 1);
            memcpy(&blocksArr[index][1], &temp[tempCount], sizeof(int) * blocksArr[index][0]);

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

            return arg1 - arg2;
        });

        int pDataStartIndex = 0;

        for (int count = 0; count < blockCount; count++)
        {
            pDataStartIndex += blocksArr[count][0];
        }

        //------------------------------------
        // Занесение блоков в исходный массив
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
    // Начальные условия
    int arrSize = 10000000;
    int threadNum = 4;
    int blockNum = threadNum * 2;

    std::cout << "Array size    : " << arrSize << std::endl;
    std::cout << "Thread number : " << threadNum << std::endl;

    //------------------------------------
    // Задание массива
    int *arr = new int[arrSize];
    std::random_device rd;
    std::mt19937 mersenne(rd());
    for (int i = 0; i < arrSize; i++)
    {
        arr[i] = mersenne() % arrSize - arrSize / 2;
    }

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
