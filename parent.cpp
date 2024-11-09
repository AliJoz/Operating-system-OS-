#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <Windows.h>
#include <string>
#include <fstream>
using namespace std;

int main() {
    int numTasks, numWorkers;
    int* tasks;
    int** canWork;
    ifstream inputFile("input.txt");
    if (inputFile.is_open()) {
        inputFile >> numTasks >> numWorkers;

     
        tasks = new int[numTasks];
        for (int i = 0; i < numTasks; ++i) {
            inputFile >> tasks[i];
        }

        
        canWork = new int* [numTasks];
        for (int i = 0; i < numTasks; ++i) {
            canWork[i] = new int[numWorkers];
        }

      
        for (int i = 0; i < numTasks; ++i) {
            for (int j = 0; j < numWorkers; ++j) {
                inputFile >> canWork[i][j];
            }
        }
        inputFile.close();
    }
    else {
        cerr << "Unable to open file" << endl;
        exit(1);
    }
    
    HANDLE size = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 2 * sizeof(int), TEXT("OS4020"));
    int* siz = (int*)MapViewOfFile(size, FILE_MAP_ALL_ACCESS, 0, 0, 2 * sizeof(int));
    siz[0] = numTasks;
    siz[1] = numWorkers;
    HANDLE h = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, ((numTasks*numWorkers)+numTasks+2) * sizeof(int), TEXT("OS4021"));

    int* Buf = (int*)MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int));
    Buf[0] = numTasks;
    Buf[1] = numWorkers;
    for (int i = 0; i < numTasks; ++i) {
        Buf[2 + i] = tasks[i];
    }
    for (int i = 0; i < numTasks; ++i) {
        for (int j = 0; j < numWorkers; ++j) {
            Buf[2 + numTasks + i * numWorkers + j] = canWork[i][j];
           
        }
    }
    HANDLE h1[4];
    HANDLE h2[4];
    STARTUPINFO s[4];
    PROCESS_INFORMATION p[4];

    for (int childCount = 0; childCount < 4; ++childCount) {

        ZeroMemory(&s[childCount], sizeof(s[childCount]));
        s[childCount].cb = sizeof(s[childCount]);
        ZeroMemory(&p[childCount], sizeof(p[childCount]));

        // sakht child
       bool b = CreateProcess(TEXT("D:\\jsu\\term9\\System\\pro\\try\\New folder\\child2\\child2\\x64\\Release\\child2.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &s[childCount], &p[childCount]);
        if (!b) {
            cout << "Create Process failed" << endl;
            return 1;
        }

        int childProcessId = GetProcessId(p[childCount].hProcess);
        cout << "process id:" << childProcessId << endl;
        // ijad text haie unique
        string h1Name = "OS4060_" + to_string(childProcessId);
        string h2Name = "OS4070_" + to_string(childProcessId);
        wstring h1NameWide = wstring(h1Name.begin(), h1Name.end());
        LPCWSTR h1NameLPCWSTR = h1NameWide.c_str();

        h1[childCount] = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int) , h1NameLPCWSTR);
        wstring h2NameWide = wstring(h2Name.begin(), h2Name.end());
        LPCWSTR h2NameLPCWSTR = h2NameWide.c_str();

        h2[childCount] = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1 * sizeof(int), h2NameLPCWSTR);
    }
    for (int i = 0; i < 4; ++i) {
        WaitForSingleObject(p[i].hProcess, INFINITE); 
    }
    //system("pause");
    int* rs1 = (int*)MapViewOfFile(h1[0], FILE_MAP_ALL_ACCESS, 0, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int));
    int* rs2 = (int*)MapViewOfFile(h1[1], FILE_MAP_ALL_ACCESS, 0, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int));
    int* rs3 = (int*)MapViewOfFile(h1[2], FILE_MAP_ALL_ACCESS, 0, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int));
    int* rs4 = (int*)MapViewOfFile(h1[3], FILE_MAP_ALL_ACCESS, 0, 0, ((numTasks * numWorkers) + numTasks + 2) * sizeof(int));
    int* avali = (int*)MapViewOfFile(h2[0], FILE_MAP_ALL_ACCESS, 0, 0, 1 * sizeof(int));
    int* dovomi = (int*)MapViewOfFile(h2[1], FILE_MAP_ALL_ACCESS, 0, 0, 1 * sizeof(int));
    int* sevomi = (int*)MapViewOfFile(h2[2], FILE_MAP_ALL_ACCESS, 0, 0, 1 * sizeof(int));
    int* chaharomi = (int*)MapViewOfFile(h2[3], FILE_MAP_ALL_ACCESS, 0, 0, 1 * sizeof(int));
    int minValue = *avali;
    if (*dovomi < minValue) minValue = *dovomi;
    if (*sevomi < minValue) minValue = *sevomi;
    if (*chaharomi < minValue) minValue = *chaharomi;

    cout << "Minimum value from shared memory: " << minValue << endl;

    
    if (*avali == minValue) {
        cout << "Task allocation per worker (from shared memory): " << avali[0] << endl;
        int index = 0;
        for (int i = 0; i < numWorkers; ++i) {
            cout << "Worker " << i + 1 << ": ";
            for (int j = 0; j < numTasks; ++j) {
                int task = rs1[index++];
                if (task != -1) cout << task << " ";
            }
            cout << endl;
        }
    }

    if (*dovomi == minValue) {
        cout << "Task allocation per worker 2 (from shared memory): " << dovomi[0] << endl;
        int index = 0;
        for (int i = 0; i < numWorkers; ++i) {
            cout << "Worker " << i + 1 << ": ";
            for (int j = 0; j < numTasks; ++j) {
                int task = rs2[index++];
                if (task != -1) cout << task << " ";
            }
            cout << endl;
        }
    }

    if (*sevomi == minValue) {
        cout << "Task allocation per worker 3 (from shared memory): " << sevomi[0] << endl;
        int index = 0;
        for (int i = 0; i < numWorkers; ++i) {
            cout << "Worker " << i + 1 << ": ";
            for (int j = 0; j < numTasks; ++j) {
                int task = rs3[index++];
                if (task != -1) cout << task << " ";
            }
            cout << endl;
        }
    }

    if (*chaharomi == minValue) {
        cout << "Task allocation per worker 4 (from shared memory): " << chaharomi[0] << endl;
        int index = 0;
        for (int i = 0; i < numWorkers; ++i) {
            cout << "Worker " << i + 1 << ": ";
            for (int j = 0; j < numTasks; ++j) {
                int task = rs4[index++];
                if (task != -1) cout << task << " ";
            }
            cout << endl;
        }
    }

    return 0;
}
