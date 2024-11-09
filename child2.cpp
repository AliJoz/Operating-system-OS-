#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <climits>
using namespace std;
int result;
DWORD currentProcessId = GetCurrentProcessId();
void assignTasks(int numTasks, int numWorkers, int* tasks, int** canWork, int* bestWorkLoad, int** bestTaskAllocation) {
    
    srand(time(0)+ currentProcessId);
    auto start = chrono::high_resolution_clock::now();
    int minDifference = INT_MAX; 

    while (true) {
      
        auto now = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(now - start);
        if (duration.count() >= 10) break;

        
        int* workLoad = new int[numWorkers]();
        int** taskAllocation = new int* [numWorkers];
        for (int i = 0; i < numWorkers; ++i) {
            taskAllocation[i] = new int[numTasks];
            for (int j = 0; j < numTasks; ++j) {
                taskAllocation[i][j] = -1;
            }
        }

        for (int i = 0; i < numTasks; ++i) {
            int worker;
            do {
                worker = rand() % numWorkers;
            } while (canWork[i][worker] != 1); 
            for (int j = 0; j < numTasks; ++j) {
                if (taskAllocation[worker][j] == -1) {
                    taskAllocation[worker][j] = tasks[i];
                    workLoad[worker] += tasks[i];
                    break;
                }
            }
        }

       
        int maxLoad = INT_MIN, minLoad = INT_MAX;
        for (int i = 0; i < numWorkers; ++i) {
            if (workLoad[i] > maxLoad) maxLoad = workLoad[i];
            if (workLoad[i] < minLoad) minLoad = workLoad[i];
        }
        int currentDifference = maxLoad - minLoad;

       
        if (currentDifference < minDifference) {
            minDifference = currentDifference;
            result=minDifference;
            for (int i = 0; i < numWorkers; ++i) {
                bestWorkLoad[i] = workLoad[i];
                for (int j = 0; j < numTasks; ++j) {
                    bestTaskAllocation[i][j] = taskAllocation[i][j];
                }
            }
        }

        
        delete[] workLoad;
        for (int i = 0; i < numWorkers; ++i) {
            delete[] taskAllocation[i];
        }
        delete[] taskAllocation;
    }

   // cout << "Best task allocation found:\n";
  //  for (int i = 0; i < numWorkers; ++i) {
    //    cout << "Worker " << i + 1 << ": ";
    //    for (int j = 0; j < numTasks; ++j) {
    //        if (bestTaskAllocation[i][j] != -1) {
    //            cout << bestTaskAllocation[i][j] << " ";
    //        }
   //     }
    //    cout << "(Workload: " << bestWorkLoad[i] << ")\n";
   // }
   // cout << "Minimum difference: " << minDifference << endl;
}

int main() {
 
    HANDLE size = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, TEXT("OS4020"));
    int* siz = (int*)MapViewOfFile(size, FILE_MAP_ALL_ACCESS, 0, 0, 2 * sizeof(int));
    HANDLE h = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, TEXT("OS4021"));
    int* Buf = (int*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, ((siz[0] * siz[1]) + siz[0] + 2) * sizeof(int));
    int numTasks = siz[0];
    int numWorkers = siz[1];
    int* tasks = new int[numTasks];
    int** canWork = new int* [numTasks];
    for (int i = 0; i < numTasks; ++i) {
        canWork[i] = new int[numWorkers];
    }
    int index = 2;
    for (int i = 0; i < numTasks; ++i) tasks[i] = Buf[index++];
    for (int i = 0; i < numTasks; ++i)
        for (int j = 0; j < numWorkers; ++j)
            canWork[i][j] = Buf[index++];

    int* workLoad = new int[numWorkers];
    int** taskAllocation = new int* [numWorkers];
    for (int i = 0; i < numWorkers; ++i) {
        taskAllocation[i] = new int[numTasks];
    }

    assignTasks(numTasks, numWorkers, tasks, canWork, workLoad, taskAllocation);


    
   // cout << endl;
   // cout << currentProcessId << endl;
    // Use the PID to create unique names for h1 and h2
    wstring h1NameWide = L"OS4060_" + to_wstring(currentProcessId);
    wstring h2NameWide = L"OS4070_" + to_wstring(currentProcessId);
    // Convert wstring to LPCWSTR
    LPCWSTR h1Name = h1NameWide.c_str();
    LPCWSTR h2Name = h2NameWide.c_str();
    HANDLE h1 = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, h1Name);
    HANDLE h2 = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, h2Name);
    // Use the PID to create a unique name for h1
    int* hafazebache = (int*)MapViewOfFile(h1, FILE_MAP_ALL_ACCESS, 0, 0, ((siz[0] * siz[1]) + siz[0] + 2) * sizeof(int));
    int* natige = (int*)MapViewOfFile(h2, FILE_MAP_ALL_ACCESS, 0, 0, 1 * sizeof(int));
    natige[0] = result;
    index = 0;
    for (int i = 0; i < numWorkers; ++i)
    {
        for (int j = 0; j < numTasks; ++j) {
            hafazebache[index] = taskAllocation[i][j];
            index++;
        }
    }
  
    UnmapViewOfFile(siz);
    UnmapViewOfFile(Buf);
    UnmapViewOfFile(hafazebache);
    CloseHandle(h);
    CloseHandle(size);
    CloseHandle(h1);
    system("pause");
}