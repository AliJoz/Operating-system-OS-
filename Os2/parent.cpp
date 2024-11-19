#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <random>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std;

struct ThreadData {
    int threadIndex;
    int* globalArray;
};

DWORD WINAPI ProcessThread(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    int threadIndex = data->threadIndex;

    srand(static_cast<unsigned int>(time(nullptr)) + threadIndex);

    int NumTask = data->globalArray[0];
    int NumWork = data->globalArray[1];

    int* taskTimes = new int[NumTask];
    for (int i = 0; i < NumTask; ++i) {
        taskTimes[i] = data->globalArray[2 + i];
    }
    int** TeskAllocatin = new int* [NumTask];
    for (int i = 0; i < NumTask; ++i) {
        TeskAllocatin[i] = new int[NumWork];
        for (int j = 0; j < NumWork; ++j) {
            TeskAllocatin[i][j] = data->globalArray[2 + NumTask + i * NumWork + j];
        }
    }
    auto startTime = chrono::steady_clock::now();
    auto endTime = startTime + chrono::seconds(15);

    int bestDifference = INT_MAX;
    int* taskAssignments = new int[NumTask];
    int* workerLoad = new int[NumWork];
    int* bestTaskAssignments = new int[NumTask];

    while (chrono::steady_clock::now() < endTime) {
        for (int i = 0; i < NumTask; ) {
            int randomWorker = rand() % NumWork;
            if (TeskAllocatin[i][randomWorker] == 1) {
                taskAssignments[i] = randomWorker;
                ++i;
            }
        }

       
        memset(workerLoad, 0, NumWork * sizeof(int));
        for (int i = 0; i < NumTask; ++i) {
            workerLoad[taskAssignments[i]] += taskTimes[i];
        }

      
        int maxLoad = INT_MIN;
        int minLoad = INT_MAX;

        for (int i = 0; i < NumWork; ++i) {
            if (workerLoad[i] > maxLoad) maxLoad = workerLoad[i];
            if (workerLoad[i] < minLoad) minLoad = workerLoad[i];
        }

        int currentDifference = maxLoad - minLoad;

      
        if (currentDifference < bestDifference) {
            bestDifference = currentDifference;
            memcpy(bestTaskAssignments, taskAssignments, NumTask * sizeof(int));
        }
    }

    int resultIndex = 2 + NumTask + NumTask * NumWork + threadIndex * (NumTask + 1);
    data->globalArray[resultIndex] = bestDifference;
    memcpy(&data->globalArray[resultIndex + 1], bestTaskAssignments, NumTask * sizeof(int));

    
    delete[] taskTimes;
    delete[] workerLoad;
    delete[] bestTaskAssignments;
    for (int i = 0; i < NumTask; ++i) {
        delete[] TeskAllocatin[i];
    }
    delete[] TeskAllocatin;

    return 0;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    ifstream inputFile("input.txt");
    if (!inputFile) {
        cerr << "Error opening input file!" << endl;
        return -1;
    }
    int NumTask, NumWork;
    inputFile >> NumTask >> NumWork;
    cout << "Tasks: " << NumTask << "\tWorkers: " << NumWork << endl;

    int* taskTimes = new int[NumTask];
    for (int i = 0; i < NumTask; ++i) {
        inputFile >> taskTimes[i];
    }

    int** TeskAllocatin = new int* [NumTask];
    for (int i = 0; i < NumTask; ++i) {
        TeskAllocatin[i] = new int[NumWork];
        for (int j = 0; j < NumWork; ++j) {
            inputFile >> TeskAllocatin[i][j];
        }
    }

    int numProcesses = std::thread::hardware_concurrency();
    cout << "Processors: " << numProcesses << endl;
    int res[4];
   
    int globalSize = 2 + NumTask + NumTask * NumWork + numProcesses * (1 + NumTask);
    int* globalArray = new int[globalSize];
    memset(globalArray, 0, globalSize * sizeof(int));

    globalArray[0] = NumTask;
    globalArray[1] = NumWork;
    memcpy(globalArray + 2, taskTimes, NumTask * sizeof(int));

    int index = 2 + NumTask;
    for (int i = 0; i < NumTask; ++i) {
        memcpy(globalArray + index, TeskAllocatin[i], NumWork * sizeof(int));
        index += NumWork;
    }

    ThreadData* threadData = new ThreadData[numProcesses];
    HANDLE* threads = new HANDLE[numProcesses];

    for (int i = 0; i < numProcesses; ++i) {
        threadData[i] = { i, globalArray };
        threads[i] = CreateThread(nullptr, 0, ProcessThread, &threadData[i], 0, nullptr);

        if (!threads[i]) {
            cerr << "Error creating thread " << i << endl;
            return 1;
        }
    }
    WaitForMultipleObjects(numProcesses, threads, TRUE, INFINITE);
   
    cout << "\nResults:\n";

    for (int i = 0; i < numProcesses; ++i) {
        int resultIndex = 2 + NumTask + NumTask * NumWork + i * (NumTask + 1);
        int bestDifference = globalArray[resultIndex];
        cout << "Thread " << i + 1 << " Best Difference: " << bestDifference << endl;
        res[i] = bestDifference; 
        for (int j = 0; j < NumTask; ++j) {
            cout << "  Task " << j + 1 << " -> Worker " << globalArray[resultIndex + 1 + j] << endl;
        }
        cout << "-----------------------------------------\n";
    }

    int minResult = *min_element(res, res + numProcesses);
    int bestThreadIndex = distance(res, find(res, res + numProcesses, minResult));

    cout << "\nBest Result: " << minResult << endl;
    cout << "This result is from Thread " << bestThreadIndex + 1 << endl;

  
    delete[] taskTimes;
    for (int i = 0; i < NumTask; ++i) {
        delete[] TeskAllocatin[i];
    }
    delete[] TeskAllocatin;
    delete[] globalArray;
    delete[] threadData;
    delete[] threads;

    return 0;
}
